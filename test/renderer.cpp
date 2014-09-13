//
//  Nanocat engine.
//
//  Main game renderer..
//
//  Created by Neko Vision on 31/12/2013.
//  Copyright (c) 2013 Neko Vision. All rights

#include "ncBSP.h"
#include "Core.h"
#include "AssetManager.h"
#include "Models.h"
#include "ConsoleCommand.h"
#include "Console.h"
#include "Camera.h"
#include "Renderer.h"
#include "OpenGL.h"
#include "MaterialLoader.h"
#include "CoreFont.h"
#include "GameWorld.h"
#include "Input.h"
#include "System.h"

// Main beautiful game renderer.

ncRenderer _renderer;

GLuint  vao;
GLuint  vertexBuffer, uvBuffer;
GLuint FramebufferName = 0;
GLuint depthTexture;

int eye_width = 640;
int eye_height = 800;

ncConsoleVariable  render_fullscreen("render", "fullscreen", "Fullscreen mode.", "0", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_fog("fog", "enabled", "Is fog enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_fog_maxdist("fog", "maxdist", "Fog maximum distance.", "150.0", CVFLAG_NONE);
ncConsoleVariable  render_fog_mindist("fog", "mindist", "Fog minimum distance.", "50.0", CVFLAG_NONE);

ncConsoleVariable  render_fog_colorR("fog", "red", "Fog red color.", "1.0", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_fog_colorG("fog", "green", "Fog green color.", "1.0", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_fog_colorB("fog", "blue", "Fog blue color.", "1.0", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_wireframe("render", "wireframe", "Wireframe mode.", "0", CVFLAG_NONE);

ncConsoleVariable  render_normalmap("env", "normalmap", "Is normal mapping enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_multisampling("render", "msaa", "Is multisampling enabled?", "0", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_refractions("env", "refractions", "Is refraction mapping enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_reflections("env", "reflections", "Is reflection mapping enabled?", "1", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_sky("render", "sky", "Render sky?", "1", CVFLAG_NONE);
ncConsoleVariable  render_water("render", "water", "Render water?", "1", CVFLAG_NONE);

ncConsoleVariable  render_fontspacing("font", "spacing", "Font symbol spacing.", "0.625", CVFLAG_NONE);

ncConsoleVariable  render_vsync("render", "vsync", "Vertical sync enabled?", "1", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_modeWidth("render", "width", "Rendering width.", "600", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_modeHeight("render", "height", "Rendering height.", "480", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_curvetesselation("bsp", "curvetes", "Curve tesselation.", "7", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_calculatevisdata("render", "pvs", "Calculate visibility data?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_lightmapgamma("bsp", "lightmapgamma", "Lightmap gamma.", "2.5", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_updatepvs("render", "pvs", "Update visibility data?", "1", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  render_usescreenfx("render", "sfx", "Use screen effects?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_dof("render", "dof", "Is Depth of Field enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  render_lensanamorph("render", "lens", "Lens effects enabled?", "1", CVFLAG_NEEDSREFRESH);


// Uh oh, a bit of magic we got.
ncConsoleVariable   render_ovr("render", "ovr", "Is Virtual reality mode turned on?", "0", CVFLAG_NEEDSREFRESH );

ncFramebuffer _waterreflection;
ncFramebuffer _waterrefraction;
ncFramebuffer _scene;

// Left, Right, Full.
ncFramebuffer _scene_adv[3];

//shader_t grass;
void ncRenderer::PrecacheWorld( void ) {

    
    //asset_load( ASSET_SHADER, "grass" );
    //asset_findshader( "grass", &grass );
    
    
    /*
     
     GLuint  length_texture;
     GLuint  orientation_texture;
     GLuint  grasspalette_texture;
     GLuint  grasscolor_texture;
     GLuint  bend_texture;

     
     glUseProgram(grass.shader_id);
    glUniform1i(glGetUniformLocation(grass.shader_id, "length_texture"), 0);
    glUniform1i(glGetUniformLocation(grass.shader_id, "orientation_texture"), 1);
    glUniform1i(glGetUniformLocation(grass.shader_id, "grasspalette_texture"), 2);
    glUniform1i(glGetUniformLocation(grass.shader_id, "grasscolor_texture"), 3);
    glUniform1i(glGetUniformLocation(grass.shader_id, "bend_texture"), 4);
    
    static const GLfloat grass_blade[] =
    {
        -10.3f, 10.0f,
        0.3f, 10.0f,
        -0.20f, 11.0f,
        0.1f, 11.3f,
        -0.05f, 12.3f,
        0.0f, 13.3f
    };
    
    static float texture_water_data[] = {
        1.0, 0.0,
        1.0, 1.0,
        0.0, 1.0,
        0.0, 0.0
    };
    
    // Greate a vertex array object and a vertex buffer for the quad
    // including position and texture coordinates
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(grass_blade), grass_blade, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texture_water_data), texture_water_data, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
    glUseProgram(0);*/
    

}

/*
    Init console variables.
*/
void ncRenderer::Preload( void ) {
    
    // Global ones.
    /* 
        OpenGL versions:
     
        * 3.2 - OpenGL 3.0
        * 3.2c - OpenGL 3.2 core
        * 4.1 - OpenGL 4.0
        * 4.1c - OpenGL 4.1 core
    */
    
    // Initial values.
    Initialized = false;
    _gamewater.Initialized = false;
    
}

void lazyScreenshot( void ) {
    _renderer.MakeScreenshot();
}

void lazyRefreshGraphics( void ) {
    _renderer.Refresh();
}

/*
 Calculate view matrix.
*/
void ncRenderer::UpdateViewMatrix( void ) {
    
    _camera.ViewMatrix.Identity();
    
    _camera.g_vLook.Normalize();
    
    _camera.g_vRight.Cross( _camera.g_vLook, _camera.g_vUp );
    _camera.g_vRight.Normalize();
    
    _camera.g_vUp.Cross( _camera.g_vRight, _camera.g_vLook );
    _camera.g_vUp.Normalize();
    
    _camera.ViewMatrix.m[0] =  _camera.g_vRight.x;
    _camera.ViewMatrix.m[1] =  _camera.g_vUp.x;
    _camera.ViewMatrix.m[2] = -_camera.g_vLook.x;
    _camera.ViewMatrix.m[3] =  0.0f;
    
    _camera.ViewMatrix.m[4] =  _camera.g_vRight.y;
    _camera.ViewMatrix.m[5] =  _camera.g_vUp.y;
    _camera.ViewMatrix.m[6] = -_camera.g_vLook.y;
    _camera.ViewMatrix.m[7] =  0.0f;
    
    _camera.ViewMatrix.m[8]  =  _camera.g_vRight.z;
    _camera.ViewMatrix.m[9]  =  _camera.g_vUp.z;
    _camera.ViewMatrix.m[10] = -_camera.g_vLook.z;
    _camera.ViewMatrix.m[11] =  0.0f;
    
    _camera.ViewMatrix.m[12] = -ncVec3_Dot(_camera.g_vRight, _camera.g_vEye);
    _camera.ViewMatrix.m[13] = -ncVec3_Dot(_camera.g_vUp, _camera.g_vEye);
    _camera.ViewMatrix.m[14] =  ncVec3_Dot(_camera.g_vLook, _camera.g_vEye);
    _camera.ViewMatrix.m[15] =  1.0f;
}

/*
    Precache ( setup ) some stuff after pre-load.
*/
void ncRenderer::Precache( void ) {
    
    // Initial stuff.
    _camera.g_vEye = ncVec3( 5.0f, 5.0f, 5.0f );
    _camera.g_vLook = ncVec3( -10.5f, -0.5f, -0.5f );
    _camera.g_vUp = ncVec3( 1.0f, 1.0f, 0.0f );
    _camera.g_vRight = ncVec3( 1.0f, 0.0f, 0.0f );
    
    _camera.ViewMatrix.Identity();
    _camera.RotationMatrix.Identity();
    _camera.ProjectionMatrix.Identity();

    _assetmanager.FindShader( "passthru", &sceneShader );
    
    static float g_vertex_buffer_data[3][6][3]=
    {
        // Full view.
        {
            { -1.0f, -1.0f, 0.0f, },
            {  1.0f, -1.0f, 0.0f, },
            { -1.0f,  1.0f, 0.0f, },
            { -1.0f,  1.0f, 0.0f, },
            { 1.0f,  -1.0f, 0.0f, },
            { 1.0f,   1.0f, 0.0f, }
        },
        
        // Left eye.
        {
            { -1.0f, -1.0f, 0.0f, },
            {  0.0f, -1.0f, 0.0f, },
            { -1.0f,  1.0f, 0.0f, },
            { -1.0f,  1.0f, 0.0f, },
            { 0.0f,  1.0f, 0.0f, },
            { 0.0f,  -1.0f, 0.0f, },
        },
        
        // Right eye.
        {
            {  0.0f, -1.0f, 0.0f, },
            {  1.0f, -1.0f, 0.0f, },
            {  0.0f,  1.0f, 0.0f, },
            {  0.0f,  1.0f, 0.0f, },
            {  1.0f,  -1.0f, 0.0f, },
            {  1.0f,  1.0f, 0.0f, },
        },
    };
    
    static float g_uv_buffer_data[3][6][2] =
    {
        // Full view.
        {
            { 0.0f, 0.0f, },
            { 1.0f, 0.0f, },
            { 0.0f, 1.0f, },
            { 0.0f, 1.0f, },
            { 1.0f, 0.0f, },
            { 1.0f, 1.0f, },
        },
        
        // Left eye.
        {
            { 0.0f, 0.0f, },
            { 0.5f, 0.0f, },
            { 0.0f, 1.0f, },
            { 0.0f, 1.0f, },
            { 0.5f, 1.0f, },
            { 0.5f, 0.0f, },
        },
        
        // Right eye.
        {
            { 0.5f, 0.0f, },
            { 1.0f, 0.0f, },
            { 0.5f, 1.0f, },
            { 0.5f, 1.0f, },
            { 1.0f, 0.0f, },
            { 1.0f, 1.0f, }
        }
    };

    glUseProgram( sceneShader.shader_id );
    
    // Generate vertex data for eyes.
    
    //
    // Center eye. ( full scene )
    glGenVertexArrays( 1, &eye_vao[EYE_FULL] );
    glBindVertexArray( eye_vao[EYE_FULL] );
    
    glGenBuffers( 1, &eye_vbo[EYE_FULL] );
    glBindBuffer( GL_ARRAY_BUFFER, eye_vbo[EYE_FULL] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data[EYE_FULL]), &g_vertex_buffer_data[EYE_FULL][0][0], GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glGenBuffers( 1, &eye_uv[EYE_FULL] );
    glBindBuffer( GL_ARRAY_BUFFER, eye_uv[EYE_FULL] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data[EYE_FULL]), &g_uv_buffer_data[EYE_FULL][0][0], GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glBindVertexArray( 0 );
    
    //
    // Left eye. ( left part of the scene )
    glGenVertexArrays( 1, &eye_vao[EYE_LEFT] );
    glBindVertexArray( eye_vao[EYE_LEFT] );
    
    glGenBuffers( 1, &eye_vbo[EYE_LEFT] );
    glBindBuffer( GL_ARRAY_BUFFER, eye_vbo[EYE_LEFT] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data[EYE_LEFT]), &g_vertex_buffer_data[EYE_LEFT][0][0], GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glGenBuffers( 1, &eye_uv[EYE_LEFT] );
    glBindBuffer( GL_ARRAY_BUFFER, eye_uv[EYE_LEFT] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data[EYE_LEFT]), &g_uv_buffer_data[EYE_LEFT][0][0], GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glBindVertexArray( 0 );
    
    //
    // Right eye. ( right part of the scene )
    glGenVertexArrays( 1, &eye_vao[EYE_RIGHT] );
    glBindVertexArray( eye_vao[EYE_RIGHT] );
    
    glGenBuffers( 1, &eye_vbo[EYE_RIGHT] );
    glBindBuffer( GL_ARRAY_BUFFER, eye_vbo[EYE_RIGHT] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data[EYE_RIGHT]), &g_vertex_buffer_data[EYE_RIGHT][0][0], GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glGenBuffers( 1, &eye_uv[EYE_RIGHT] );
    glBindBuffer( GL_ARRAY_BUFFER, eye_uv[EYE_RIGHT] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data[EYE_RIGHT]), &g_uv_buffer_data[EYE_RIGHT][0][0], GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glBindVertexArray( 0 );
    
    lens_texture = _materials.Find("lensdirt").texture.tex_id;
    
    scene_texture = glGetUniformLocation(sceneShader.shader_id, "renderedTexture");
    depth_texture = glGetUniformLocation(sceneShader.shader_id, "depthTexture");
    lensdirt_texture = glGetUniformLocation(sceneShader.shader_id, "uLensColor" );
    
    usela_id = glGetUniformLocation(sceneShader.shader_id, "use_lensanam" );
    usefx_id = glGetUniformLocation(sceneShader.shader_id, "aFX" );
    time_id = glGetUniformLocation(sceneShader.shader_id, "time");
    width_id = glGetUniformLocation(sceneShader.shader_id, "width");
    height_id = glGetUniformLocation(sceneShader.shader_id, "height");
    usedof_id = glGetUniformLocation(sceneShader.shader_id, "use_dof");
    ovr_id = glGetUniformLocation(sceneShader.shader_id, "ovr");
    
    glUniform1i( scene_texture, 0 );
    glUniform1i( depth_texture, 1 );
    glUniform1i( lensdirt_texture, 2 );
    
    glUniform1f( width_id, (float)_renderer.windowWidth );
    glUniform1f( height_id, (float)_renderer.windowHeight );
    glUniform1i( usela_id, render_lensanamorph.GetInteger() );
    glUniform1i( usefx_id, render_usescreenfx.GetInteger() );
    glUniform1i( usedof_id, render_dof.GetInteger() );
    glUniform1i( ovr_id, render_ovr.GetInteger() );
    
    glUseProgram(0);
    
    PrecacheWorld();
    

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
 
    glGenFramebuffers(1, &FramebufferName);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferName);
    
    // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
    
    glGenTextures(1, &depthTexture);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT16, 1024, 1024, 0,GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthTexture, 0);
    
    glDrawBuffer(GL_NONE); // No color buffer is drawn to.
    
    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return;
}

/*
 
    Initialize renderer stuff here.
    Called from ncOpenGL.
 
*/

void ncRenderer::Initialize( void ) {
    
    if( !_core.UseGraphics ) {
        _core.Print( LOG_DEVELOPER, "Ignoring renderer load, I am dedicated server!\n" );
        return;
    }
    
    _core.Print( LOG_INFO, "Renderer initializing..\n" );
    
    if( Initialized ) {
        _core.Print( LOG_WARN, "Renderer already initialized, ignoring..\n" );
        return;
    }
    
    int g_err;
    float t1, t2; t1 = _system.Milliseconds();
    
    State  = RENDER_IDLE;
    
    _shaderManager.Initialize(); // Initialize shader system.
    
    // Load shaders first because some another assets may require them.
    _assetmanager.Load( ASSET_SHADER, "oculusvr" );
    _assetmanager.Load( ASSET_SHADER, "passthru" );
    _assetmanager.Load( ASSET_SHADER, "model" );
    _assetmanager.Load( ASSET_SHADER, "font" );
    _assetmanager.Load( ASSET_SHADER, "water" );
    _assetmanager.Load( ASSET_SHADER, "level" );
    
    _materials.Initialize();  // Initialize materials and load textures.
    
    _assetmanager.Load( ASSET_MATERIAL, "main" );
    
    _modelLoader.Initialize();      // Load all models & precache 'em.
    _font.Initialize();             // Load fonts.
    _gamewater.Initialize();        // Initialize water.
    _bspmngr.Initialize();          // Initialize static world.
    
    UpdateFramebufferObject( renderWidth, renderHeight );   // Initialize FBO stuff.
    
    _commandManager.Add( "glrefresh", lazyRefreshGraphics );
    _commandManager.Add( "ss", lazyScreenshot );
    
    g_err = glGetError();   // Check for some errors.
    if ( g_err != GL_NO_ERROR ) {
        _core.Print( LOG_WARN, "Last OpenGL warning after renderer initialization: 0x%x\n", g_err );
    }
    
    Precache();             // Precache some stuff. :>
    
    t2 = _system.Milliseconds();
    _core.Print( LOG_INFO, "Renderer took %.1f ms to load.\n", t2 - t1 );
    
    State = RENDER_ACTIVE;
    
    Initialized = true;
}

/*
 
 Generate frame buffer.
 
*/
void ncRenderer::CreateFramebufferObject( uint *tex, uint *fbo, uint *rbo, uint *dbo, uint *depth, int winx, int winy ) {
    if( !_core.UseGraphics )
        return;
    
    GLenum status;
    
    // Clear previous data.
    glDeleteFramebuffers( 1, fbo );
    glDeleteTextures( 1, tex );
    glDeleteRenderbuffers( 1, rbo );
    
    glGenFramebuffers(1, fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, *fbo);
    
    glGenRenderbuffers(1, rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, *rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, winx, winy);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, *rbo);
    
    // Depth & stencil buffer.
    glGenRenderbuffers(1, dbo);
    glBindRenderbuffer(GL_RENDERBUFFER, *dbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, winx, winy);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT24, GL_RENDERBUFFER, *dbo);
    
    // Depth texture.
    glGenTextures(1, depth);
    glBindTexture(GL_TEXTURE_2D, *depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, winx, winy, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    // Generate texture.
    glGenTextures(1, tex);
    glBindTexture(GL_TEXTURE_2D, *tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8,  winx, winy, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    
    /*
     Note to myself.
     
     * Do not edit, it wasn't working on Mac before
     now it's perfect.
     */
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *tex, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, *depth, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Check for existing errors.
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( status != GL_FRAMEBUFFER_COMPLETE ) {
        _core.Error( ERC_GL, "Could not initialize frame buffer object. (Code %i)\n", status );
        return;
    }
    
    GLenum drawBuffers[] = {
        GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT
    };
    
    glDrawBuffers( 1, drawBuffers );
    glReadBuffer( GL_NONE );
    
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

/*
 
 We need to call this to create FBOs.
 Also called on window resize.
 
*/
void ncRenderer::UpdateFramebufferObject( int w, int h ) {
    
    CreateFramebufferObject( &_scene.scene,
               &_scene.frame,
               &_scene.depth,
               &_scene.render,
               &_scene.depthtex, w, h);
    
    // Advanced scenes.
    // Used for VR.
    int i;
    for( i = 0; i < 3; i++ ) {
        CreateFramebufferObject( &_scene_adv[i].scene,
                        &_scene_adv[i].frame,
                        &_scene_adv[i].depth,
                        &_scene_adv[i].render,
                        &_scene_adv[i].depthtex, w, h);
    }
    
    // Water refraction.
    CreateFramebufferObject( &_waterrefraction.scene,
               &_waterrefraction.frame,
               &_waterrefraction.depth,
               &_waterrefraction.render,
               &_waterrefraction.depthtex, w, h );
    
    // Water reflections.
    CreateFramebufferObject( &_waterreflection.scene,
               &_waterreflection.frame,
               &_waterreflection.depth,
               &_waterreflection.render,
               &_waterreflection.depthtex, w, h);
}

void ncRenderer::RenderGrass( void ) {
    // Grass.
    /*ncMatrix4 model, pos;
    
    mat4_identity( &model );
    mat4_identity( &pos );
    
    mat4_translate( &pos, vec3f(0.0, -8.0, 0.0) );
    //mat4_rotate(&pos, -12.0, vec3f(1.0, 0.0, 0.0));
    //mat4_rotate(&pos, _cmain.time * 0.001, vec3f(0.0, 1.0, 0.0));
    
    
    glUseProgram(grass.shader_id);
    glUniformMatrix4fv( glGetUniformLocation( grass.shader_id, "ProjMatrix" ), 1, false, _view.projectionMatrix.m );
    glUniformMatrix4fv( glGetUniformLocation( grass.shader_id, "ModelMatrix" ), 1, false, pos.m );
    glUniformMatrix4fv( glGetUniformLocation( grass.shader_id, "ViewMatrix" ), 1, false, _view.viewMatrix.m );
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, material_find("length").texture.tex_id );
    glActiveTexture(GL_TEXTURE1);
    glBindTexture( GL_TEXTURE_2D, material_find("orientation").texture.tex_id );
    glActiveTexture(GL_TEXTURE2);
    glBindTexture( GL_TEXTURE_2D, material_find("grass").texture.tex_id );
    glActiveTexture(GL_TEXTURE3);
    glBindTexture( GL_TEXTURE_2D, material_find("color").texture.tex_id );
    glActiveTexture(GL_TEXTURE4);
    glBindTexture( GL_TEXTURE_2D, material_find("bend").texture.tex_id );
    
    glBindVertexArray(vao);
    
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 3, 1024 * 1024);
    
    glBindVertexArray(0);
    
    glActiveTexture(4); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture(3); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture(2); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture(1); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture(0); glBindTexture( GL_TEXTURE_2D, 0 );
    
    glUseProgram(0);*/
}

/*
    Just render the world. :)
*/
void ncRenderer::RenderWorld( int msec, ncSceneEye eye ) {
    
    // Render static world.
    RenderBSP( false, eye );
    
    // Render beautiful sky.
    //gmodel_sky_render();
    
    // Render beautiful water.
    _gamewater.Render( eye );
    
    // Render models.
    _modelLoader.Render( false, eye );

    // Render grass.
    RenderGrass();
}

/*
    Update view matrix rotation.
*/
void ncRenderer::UpdateMatrixRotation( void ) {
    
    _camera.g_curMousePosition.x = _imouse.x;
    _camera.g_curMousePosition.y = _imouse.y;
    
    if( _imouse.Holding ) {
        _camera.RotationMatrix.Identity();
        
        int nXDiff = (_camera.g_curMousePosition.x - _camera.g_lastMousePosition.x);
        int nYDiff = (_camera.g_curMousePosition.y - _camera.g_lastMousePosition.y);
    
        if( nYDiff != 0 ) {
            _camera.RotationMatrix.Rotate( -(float)nYDiff / 3.0f, _camera.g_vRight );

            Matrix4_TransformVector( &_camera.RotationMatrix, &_camera.g_vLook );
            Matrix4_TransformVector( &_camera.RotationMatrix, &_camera.g_vUp );
        }
        
        if( nXDiff != 0 ){
            _camera.RotationMatrix.Rotate( -(float)nXDiff / 3.0f, VECTOR_UP );
            
            Matrix4_TransformVector( &_camera.RotationMatrix, &_camera.g_vLook );
            Matrix4_TransformVector( &_camera.RotationMatrix, &_camera.g_vUp );
        }
    }
    
    _camera.g_lastMousePosition.x = _camera.g_curMousePosition.x;
    _camera.g_lastMousePosition.y = _camera.g_curMousePosition.y;
}

/*
    Beautiful water environment.
*/
void ncRenderer::RenderBeautifulWater( void ) {
    
    if ( !render_water.GetInteger() )
        return;
    
    
    glBindFramebuffer( GL_FRAMEBUFFER, _waterreflection.frame );
    glBindRenderbuffer( GL_RENDERBUFFER, _waterreflection.render );

    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Water reflections.
    if ( render_reflections.GetInteger() ) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        
        RenderBSP( 1, EYE_FULL );
        _modelLoader.Render( true, EYE_FULL );
        
        glDisable(GL_CULL_FACE);
    }
    
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // Water refraction.
    glBindFramebuffer( GL_FRAMEBUFFER, _waterrefraction.frame );
    glBindRenderbuffer( GL_RENDERBUFFER, _waterrefraction.render );
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Water refractions.
    if ( render_refractions.GetInteger() ) {
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        
        RenderBSP( false, EYE_FULL );
        _modelLoader.Render( false, EYE_FULL );
        RenderGrass();
        
        glDisable(GL_CULL_FACE);
    }
    
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


/*
    Render to texture
*/
void ncRenderer::PreRender( void ) {
    if( !Initialized )
        return;
    
    // Update matrix view.
    UpdateMatrixRotation();
    UpdateViewMatrix();
    
    // Update bounding box.
    Frustum_Update();
    
    // Beautiful water.
    RenderBeautifulWater();
}

void ncRenderer::RenderToShader( ncSceneEye eye ) {

    // Scene buffer.
    glBindFramebuffer( GL_FRAMEBUFFER, _scene_adv[eye].frame );
    glBindRenderbuffer( GL_RENDERBUFFER, _scene_adv[eye].render );
    
    if( eye == EYE_LEFT )
        glViewport( 0, 0, windowWidth / 2.0, windowHeight );
    else if( eye == EYE_RIGHT )
        glViewport( (windowWidth + 1) / 2.0, 0.0, windowWidth / 2.0, windowHeight );
    else
        glViewport( 0.0, 0.0, windowWidth, windowHeight );
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    RenderWorld( _core.Time, eye );
    
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    float rx = 0.5f;
    float ry = 0.0f;
    float rw = 0.5f;
    float rh = 1.0f;
    
    float rDistortionXCenterOffset = -0.25f;
    
    float x = 0.0f;
    float y = 0.0f;
    float w = 0.5f;
    float h = 1.0f;
    
    float K0 = 1.0f;
    float K1 = 0.22f;
    float K2 = 0.24f;
    float K3 = 0.0f;
    float ras = rw / rh;
    float as = w / h;
    
    float DistortionXCenterOffset = 0.25f;
    float scaleFactor = .7f;

    // Here comes shader.
    glUseProgram( sceneShader.shader_id );
    
    if( eye == EYE_LEFT ) {
        
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "LensCenter"),  x + (w + DistortionXCenterOffset * 0.5f) * 0.5f,
                    y + h * 0.5f  );
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "ScreenCenter"), x + w * 0.5f, y + h * 0.5f );
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "Scale"), (w / 2.0f) * scaleFactor, (h / 2.0f) * scaleFactor * as);
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "ScaleIn"), (2.0f / w), (2.0f / h) / as );
        glUniform4f( glGetUniformLocation( sceneShader.shader_id, "HmdWarpParam"),  K0, K1, K2, K3 );
        
    } else if( eye == EYE_RIGHT ) {
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "LensCenter"),  rx + (rw + rDistortionXCenterOffset * 0.5f) * 0.5f,
                    ry + rh * 0.5f  );
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "ScreenCenter"), rx + rw * 0.5f, ry + rh * 0.5f );
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "Scale"), (rw / 2.0f) * scaleFactor, (rh / 2.0f) * scaleFactor * as);
        glUniform2f( glGetUniformLocation( sceneShader.shader_id, "ScaleIn"), (2.0f / rw), (2.0f / rh) / ras );
        glUniform4f( glGetUniformLocation( sceneShader.shader_id, "HmdWarpParam"),  K0, K1, K2, K3 );
    }
                
    glActiveTexture(GL_TEXTURE0);
    glBindTexture( GL_TEXTURE_2D, _scene_adv[eye].scene );
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture( GL_TEXTURE_2D, _scene_adv[eye].depthtex );
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture( GL_TEXTURE_2D, lens_texture );
    
    glUniform1f( time_id, (float)(_core.Time * 10.0f) );
    
    glViewport( 0.0, 0.0, windowWidth, windowHeight );
    
    glBindVertexArray( eye_vao[eye] );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( 0 );
    
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    
    glUseProgram(0);
}

/*
    BSP level rendering.
*/
void ncRenderer::RenderBSP( int reflected, ncSceneEye eye ) {
    _bspmngr.CalculateVisibleData( _camera.g_vEye );
    _bspmngr.Render( reflected, eye );
}

/*
    Lets see the world!
*/
void ncRenderer::Render( int msec ) {
    //if( !_core.UseGraphics )
    //    return;
    
    if( ( !_opengl.Initialized || !Initialized ) )
        return;
    
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Here we render the world.
    
    /*
        Render some stuff first.
    */
    PreRender();

    // Here you can tell what you want to render.
    
    if( render_ovr.GetInteger() ) {
        RenderToShader( EYE_RIGHT );
        RenderToShader( EYE_LEFT );
    } else {
        RenderToShader( EYE_FULL );
    }
    
    //RenderToShader( EYE_FULL );
    
    // Console is moved out from *render to texture* since we don't care about its rendering,
    // it must be always shown correctly.
    _gconsole.Render();
    
#ifdef _WIN32
    SwapBuffers(_win.hDC);
#endif
    // Swap for Mac is done in its files.
}

/*
    Refresh graphical values.
*/
void ncRenderer::Refresh( void ) {
    if( !_gameworld.Active )
        return;
    
    _gamewater.Refresh();
    
    // Update 2d screen filter shader.
    glUseProgram(sceneShader.shader_id);
    
    glUniform1f( width_id, (float)windowWidth );
    glUniform1f( height_id, (float)windowHeight );
    glUniform1i( usela_id, render_lensanamorph.GetInteger() );
    glUniform1i( usefx_id, render_usescreenfx.GetInteger() );
    glUniform1i( usedof_id, render_dof.GetInteger() );
    glUniform1i( ovr_id, render_ovr.GetInteger() );
    
    glUseProgram(0);
}

/*
    Remove every active object.
    Don't touch sky shader.
*/
void ncRenderer::RemoveWorld( const char *msg ) {
    if( !_gameworld.Active ) {
        _core.Print( LOG_WARN, "I've tried to clear the world, but there's no active world!\n" );
        return;
    }
    
    int i;
    
    _core.Print( LOG_INFO, "Removing current world. Reason: \"%s\".\n", msg );
    _core.Print( LOG_NONE, "\n");
    
    // Set render states to false.
    _gameworld.Active = false;
    _gamewater.InUse = false;
    

    // Destroy terrain.
    //terrain_destroy();
    
    // Remove all models ( not the loaded ones! )
    _gameworld.spawned_models           = 0;
  
    for( i = 0; i < MAX_MODELS; i++ ) {
        if( &_modelLoader._gmodel[i] )
            memset(&_modelLoader._gmodel[i].g_model, 0, sizeof(ncPrecachedModel));
    }
    
    // Remove BSP level if exists.
    _bspmngr.visibleFaces.ClearAll();
    _bspmngr.Unload();
    
    _camera.Reset();
}

/*
    Take the game screenshot.
    FIX ME: Flipped image.
*/
void ncRenderer::MakeScreenshot( void ) {
    byte    *data;
    
    data = (byte*)malloc( renderHeight * renderWidth * 3 );
    
    glPixelStorei( GL_PACK_SWAP_BYTES, 1 );
    glReadPixels( 0, 0, render_modeWidth.GetInteger(), render_modeHeight.GetInteger(), GL_BGR, GL_UNSIGNED_BYTE, data );
    
    _image.CreateImage( renderWidth, renderHeight, data, BMP_IMAGE, "screenshot" );
    free( data );
}




