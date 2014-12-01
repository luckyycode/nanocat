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
ncConsoleVariable  OpenGL_Version("opengl", "version", "OpenGL version to be used.", "4.1c", CVFLAG_NEEDSREFRESH );

ncOpenGL local_opengl;
ncOpenGL *gl_Core = &local_opengl;

// yes
// well...

/*
    OpenGL stuff initialization.
*/

void ncOpenGL::Initialize( void ) {

    if( !g_Core->UseGraphics )
        return;
    
    glGetIntegerv( GL_MAJOR_VERSION, &_majorVersion );
    glGetIntegerv( GL_MINOR_VERSION, &_minorVersion );
    
    g_Core->Print( LOG_INFO, "Initializing OpenGL parameters..\n" );
    g_Core->Print( LOG_INFO, "OpenGL version: %d.%d\n", _majorVersion, _minorVersion );
    
    g_mainRenderer->renderHeight = Render_Height.GetInteger();
    g_mainRenderer->renderWidth  = Render_Width.GetInteger();
    
    g_mainRenderer->renderHalfWidth = g_mainRenderer->renderWidth / 2.0f;
    g_mainRenderer->renderHalfHeight = g_mainRenderer->renderHeight / 2.0f;
    
    // Initial values.
    g_mainRenderer->windowHeight = g_mainRenderer->renderHeight;
    g_mainRenderer->windowWidth = g_mainRenderer->renderWidth;
    
    glViewport( 0, 0, g_mainRenderer->renderWidth, g_mainRenderer->renderHeight );
    
    // 2.0f for OVR.
    float aspectRatio = g_mainRenderer->windowWidth / ( Render_OVR.GetInteger() ? 2.0f : 1.0f ) / g_mainRenderer->windowHeight;
    
    g_playerCamera->ProjectionMatrix.CreatePerspective( GameView_FieldOfView.GetFloat(), aspectRatio, NEAR, FAR );
    
    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
    glClearDepthf( 1.0f );
    glClearStencil( 1.0f );

    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    
    //
    // Renderer initialization.
    //
    g_mainRenderer->Initialize();
}

/*
    Called on window resize.
*/
void ncOpenGL::OnResize( int w, int h ) {
    if( !Initialized )
        return;

    if( !g_Core->UseGraphics )
        return;

    if ( ( w <= 300 || h <= 300 ) ){
        g_Core->Print( LOG_WARN, "Too small window size for resizing..\n" );
        w = 800;
        h = 600;

       // win_resize( w, h );
    }

    glViewport( 0, 0, w, h );

    g_mainRenderer->windowWidth   = w;
    g_mainRenderer->windowHeight  = h;
    
    // 2.0f for OVR.
    float aspectRatio = w / ( Render_OVR.GetInteger() ? 2.0f : 1.0f ) / h;
    float fieldOfView = GameView_FieldOfView.GetFloat();
    
    g_playerCamera->ProjectionMatrix.CreatePerspective( fieldOfView, aspectRatio, NEAR, FAR );
 
    /*
        TODO: Fix or remove me.
        This crashes on multiple window resize.
    */
    g_mainRenderer->UpdateFramebufferObject( w, h );
}

/*
    Print graphics information.
*/
void ncOpenGL::ShowInfo( void ) {

    if( !g_Core->UseGraphics )
        return;

    g_Core->Print( LOG_INFO, "OpenGL information - \n" );
    g_Core->Print( LOG_INFO, "OpenGL version: %s\n", glGetString( GL_VERSION ) );
    g_Core->Print( LOG_INFO, "GPU: %s\n", glGetString( GL_RENDERER ) );
    g_Core->Print( LOG_INFO, "Vendor: %s\n", glGetString( GL_VENDOR ) );
    g_Core->Print( LOG_INFO, "Shader model version: %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );
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
