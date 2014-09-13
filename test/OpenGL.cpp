//
//  Nanocat engine.
//
//  OpenGL manager..
//
//  Created by Neko Vision on 05/09/2013.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "Camera.h"
#include "Renderer.h"
#include "NCString.h"
#include "OpenGL.h"
#include "LocalGame.h"

ncConsoleVariable  GLSL_Version("glsl", "version", "OpenGL Shading language version.", "400", CVFLAG_NEEDSREFRESH );
ncConsoleVariable  render_openglversion("opengl", "version", "OpenGL version to be used.", "4.1c", CVFLAG_NEEDSREFRESH );

ncOpenGL _opengl;

// yes
// well...

/*
    OpenGL stuff initialization.
*/

void ncOpenGL::Initialize( void ) {
    if( _core.UseGraphics ) {
        glGetIntegerv( GL_MAJOR_VERSION, &_majorVersion );
        glGetIntegerv( GL_MINOR_VERSION, &_minorVersion );
        
        _core.Print( LOG_INFO, "Initializing OpenGL parameters..\n" );
        _core.Print( LOG_INFO, "OpenGL version: %d.%d\n", _majorVersion, _minorVersion );
        
        _renderer.renderHeight = render_modeHeight.GetInteger();
        _renderer.renderWidth  = render_modeWidth.GetInteger();

        // Initial values.
        _renderer.windowHeight      = _renderer.renderHeight;
        _renderer.windowWidth       = _renderer.renderWidth;

        glViewport( 0, 0, _renderer.renderWidth, _renderer.renderHeight );

        // 2.0f for OVR.
        float aspectRatio = _renderer.windowWidth / ( render_ovr.GetInteger() ? 2.0f : 1.0f ) / _renderer.windowHeight;
        
        _camera.ProjectionMatrix.CreatePerspective( clientgame_fov.GetFloat(), aspectRatio, NEAR, FAR );
        
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClearDepth( 1.0f );
        glClearStencil( 1.0f );

        glEnable( GL_DEPTH_TEST );
        glDepthFunc( GL_LEQUAL );

        glEnable( GL_CULL_FACE );
    }

    // Renderer initialization.
    _renderer.Initialize();
}

/*
    Called on window resize.
*/
void ncOpenGL::OnResize( int w, int h ) {
    if( !Initialized )
        return;

    if( !_core.UseGraphics )
        return;

    if ( ( w <= 300 || h <= 300 ) ){
        _core.Print( LOG_WARN, "Too small window size for resizing..\n" );
        w = 800;
        h = 600;

       // win_resize( w, h );
    }

    glViewport( 0, 0, w, h );

    _renderer.windowWidth   = w;
    _renderer.windowHeight  = h;
    
    // 2.0f for OVR.
    float aspectRatio = w / ( render_ovr.GetInteger() ? 2.0f : 1.0f ) / h;
    
    _camera.ProjectionMatrix.CreatePerspective( clientgame_fov.GetFloat(), aspectRatio, NEAR, FAR );
 
    /*
        TODO: Fix or remove me.
        This crashes on window resize.
    */
    _renderer.UpdateFramebufferObject( w, h );
}

/*
    Print graphics information.
*/
void ncOpenGL::ShowInfo( void ) {

    if( !_core.UseGraphics )
        return;

    _core.Print( LOG_INFO, "Graphic device information: \n" );
    _core.Print( LOG_INFO, "OpenGL version: %s\n", glGetString(GL_VERSION) );
    _core.Print( LOG_INFO, "GPU: %s\n", glGetString(GL_RENDERER) );
    _core.Print( LOG_INFO, "Vendor: %s\n", glGetString(GL_VENDOR) );
    _core.Print( LOG_INFO, "Shader model version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION) );
}

/*
    Get OpenGL version.
*/
uint ncOpenGL::GetMajorVersion( void ) {
    return _majorVersion;
}

uint ncOpenGL::GetMinorVersion( void ) {
    return _minorVersion;
}
