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
#include "Terrain.h"

// Main beautiful game renderer.

ncRenderer local_appRenderer;
ncRenderer *g_mainRenderer = &local_appRenderer;

ncConsoleVariable  Render_Fullscreen("render", "fullscreen", "Fullscreen mode.", "0", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_Fog("fog", "enabled", "Is fog enabled?", "0", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_Fog_maxdist("fog", "maxdist", "Fog maximum distance.", "150.0", CVFLAG_NONE);
ncConsoleVariable  Render_Fog_mindist("fog", "mindist", "Fog minimum distance.", "50.0", CVFLAG_NONE);

ncConsoleVariable  Render_Fog_colorR("fog", "red", "Fog red color.", "1.0", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_Fog_colorG("fog", "green", "Fog green color.", "1.0", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_Fog_colorB("fog", "blue", "Fog blue color.", "1.0", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_Wireframe("render", "wireframe", "Wireframe mode.", "0", CVFLAG_NONE);

ncConsoleVariable  Render_Normalmaps("env", "normalmap", "Is normal mapping enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_Multisamples("render", "msaa", "Is multisampling enabled?", "1", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_Refractions("env", "refractions", "Is refraction mapping enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_Reflections("env", "reflections", "Is reflection mapping enabled?", "1", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_Sky("render", "sky", "Render sky?", "1", CVFLAG_NONE);
ncConsoleVariable  Render_Water("render", "water", "Render water?", "1", CVFLAG_NONE);

ncConsoleVariable  Render_VSync("render", "vsync", "Vertical sync enabled?", "1", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_Width("render", "width", "Rendering width.", "1366", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_Height("render", "height", "Rendering height.", "768", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_CurveTesselation("bsp", "curvetesselation", "Curve tesselation.", "7", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_CalculateVisibleData("render", "pvs", "Calculate visibility data?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_LightmapGamma("bsp", "lightmapgamma", "Lightmap gamma.", "2.5", CVFLAG_NEEDSREFRESH);

ncConsoleVariable  Render_UseScreenFX("render", "sfx", "Use screen effects?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_DepthOfField("render", "dof", "Is Depth of Field enabled?", "1", CVFLAG_NEEDSREFRESH);
ncConsoleVariable  Render_ScreenLens("render", "lens", "Lens effects enabled?", "1", CVFLAG_NEEDSREFRESH);

// Uh oh, a bit of magic we got.
ncConsoleVariable   Render_OVR("render", "ovr", "Is Virtual reality mode turned on?", "0", CVFLAG_NEEDSREFRESH );

ncFramebuffer g_waterReflectionBuffer;
ncFramebuffer g_waterRefractionBuffer;

// Left, Right, Full eyes.
ncFramebuffer g_sceneBuffer[3];

void lazyScreenshot( void ) {
    g_mainRenderer->MakeScreenshot();
}

void lazyRefreshGraphics( void ) {
    g_mainRenderer->Refresh();

    
    
}

/*
    Precache world.
*/
void ncRenderer::PrecacheWorld( void ) {

}

/*
    Init console variables.
*/
void ncRenderer::Preload( void ) {
    
    // Global ones.

    // Initial values.
    Initialized = false;
    g_gameWater->Initialized = false;
    g_gameTerrain->Initialized = false;
}

/*
    Calculate view matrix.
*/
void ncRenderer::UpdateViewMatrix( void ) {
    
    g_playerCamera->ViewMatrix.Identity();
    
    
    g_playerCamera->g_vLook.Normalize();
    
    g_playerCamera->g_vRight.Cross( g_playerCamera->g_vLook, g_playerCamera->g_vUp );
    g_playerCamera->g_vRight.Normalize();
    
    g_playerCamera->g_vUp.Cross( g_playerCamera->g_vRight, g_playerCamera->g_vLook );
    g_playerCamera->g_vUp.Normalize();
    
    // Setup view matrix.
    g_playerCamera->ViewMatrix.m[0] =  g_playerCamera->g_vRight.x;
    g_playerCamera->ViewMatrix.m[1] =  g_playerCamera->g_vUp.x;
    g_playerCamera->ViewMatrix.m[2] = -g_playerCamera->g_vLook.x;
    g_playerCamera->ViewMatrix.m[3] =  0.0f;
    
    g_playerCamera->ViewMatrix.m[4] =  g_playerCamera->g_vRight.y;
    g_playerCamera->ViewMatrix.m[5] =  g_playerCamera->g_vUp.y;
    g_playerCamera->ViewMatrix.m[6] = -g_playerCamera->g_vLook.y;
    g_playerCamera->ViewMatrix.m[7] =  0.0f;
    
    g_playerCamera->ViewMatrix.m[8]  =  g_playerCamera->g_vRight.z;
    g_playerCamera->ViewMatrix.m[9]  =  g_playerCamera->g_vUp.z;
    g_playerCamera->ViewMatrix.m[10] = -g_playerCamera->g_vLook.z;
    g_playerCamera->ViewMatrix.m[11] =  0.0f;
    
    g_playerCamera->ViewMatrix.m[12] = -ncVec3_Dot(g_playerCamera->g_vRight, g_playerCamera->g_vEye);
    g_playerCamera->ViewMatrix.m[13] = -ncVec3_Dot(g_playerCamera->g_vUp, g_playerCamera->g_vEye);
    g_playerCamera->ViewMatrix.m[14] =  ncVec3_Dot(g_playerCamera->g_vLook, g_playerCamera->g_vEye);
    g_playerCamera->ViewMatrix.m[15] =  1.0f;
}

/*
    Precache ( setup ) some stuff after pre-load.
*/
void ncRenderer::Precache( void ) {
    
    // Initial stuff.
    g_playerCamera->g_vEye = ncVec3( 5.0f, 15.0f, 5.0f );
    g_playerCamera->g_vLook = ncVec3( -10.5f, -0.5f, -0.5f );
    g_playerCamera->g_vUp = ncVec3( 1.0f, 1.0f, 0.0f );
    g_playerCamera->g_vRight = ncVec3( 1.0f, 0.0f, 0.0f );
    
    g_playerCamera->ViewMatrix.Identity();
    g_playerCamera->RotationMatrix.Identity();
    g_playerCamera->ProjectionMatrix.Identity();

    // Scene shader.
    sceneShader = f_AssetManager->FindShaderByName( "passthru" );
    
    static float g_sceneEyeVertexData[3][6][3] = {
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
    
    static float g_sceneEyeUVData[3][6][2] = {
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

    sceneShader->Use();
    
    // Generate vertex data for eyes.
    
    //
    // Scene view "eyes".
    // Full, Left, Right.
    for(int i = 0; i < 3; i++) {
        glGenVertexArrays( 1, &eye_vao[i] );
        glBindVertexArray( eye_vao[i] );
        
        g_eyeVBO[i].Create();
        g_eyeVBO[i].Bind();
        g_eyeVBO[i].AddData( sizeof(g_sceneEyeVertexData[i]), &g_sceneEyeVertexData[i][0][0] );
        
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, 0 );
        glEnableVertexAttribArray( 0 );
        g_eyeVBO[i].Unbind();
        
        g_eyeUV[i].Create();
        g_eyeUV[i].Bind();
        g_eyeUV[i].AddData( sizeof(g_sceneEyeUVData[i]), &g_sceneEyeUVData[i][0][0] );
        
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, 0 );
        glEnableVertexAttribArray( 1 );
        g_eyeUV[i].Unbind();
        
        glBindVertexArray( 0 );
    }
    
    // Onscreen texture.
    lens_texture = g_materialManager->Find("lensdirt")->Image.TextureID;
    
    // Shader uniforms.
    scene_texture = sceneShader->UniformLocation( "renderedTexture" );
    depth_texture = sceneShader->UniformLocation( "depthTexture" );

    lensdirt_texture = sceneShader->UniformLocation( "uLensColor" );

    // Ids.
    usela_id = sceneShader->UniformLocation( "use_lensanam" );
    usefx_id = sceneShader->UniformLocation( "aFX" );
    time_id = sceneShader->UniformLocation( "time" );
    width_id = sceneShader->UniformLocation( "width" );
    height_id = sceneShader->UniformLocation( "height" );
    usedof_id = sceneShader->UniformLocation( "use_dof" );
    ovr_id = sceneShader->UniformLocation( "ovr" );
    
    sceneShader->SetUniform( scene_texture, 0 );
    sceneShader->SetUniform( depth_texture, 1 );
    sceneShader->SetUniform( lensdirt_texture, 2 );
    // -------
    sceneShader->SetUniform( width_id, (float)g_mainRenderer->windowWidth );
    sceneShader->SetUniform( height_id, (float)g_mainRenderer->windowHeight );
    sceneShader->SetUniform( usela_id, Render_ScreenLens.GetInteger() );
    sceneShader->SetUniform( usefx_id, Render_UseScreenFX.GetInteger() );
    sceneShader->SetUniform( usedof_id, Render_DepthOfField.GetInteger() );
    
    sceneShader->Next();
    
    PrecacheWorld();
}

/*
 
    Initialize renderer stuff here.
    Called from ncOpenGL class.
 
*/
void ncRenderer::Initialize( void ) {
    
    if( !g_Core->UseGraphics ) {
        g_Core->Print( LOG_DEVELOPER, "Ignoring renderer load, I am dedicated server!\n" );
        return;
    }
    
    g_Core->Print( LOG_INFO, "Renderer initializing..\n" );
    
    if( Initialized ) {
        g_Core->Print( LOG_WARN, "Renderer already initialized, ignoring..\n" );
        return;
    }
    
    int g_err;
    float t1, t2; t1 = c_coreSystem->Milliseconds();
    
    State = RENDER_IDLE;
    
    // Initialize asset system and load assets.
    f_AssetManager->Initialize();
    
    // World nyashes.
    g_modelManager->Initialize();   // Load all models & precache 'em.
    g_coreFont->Initialize();   // Load core font.
    g_gameWater->Initialize();  // Initialize game water.
    //g_gameTerrain->Initialize();    // Initialize game terrain.
    g_staticWorld->Initialize();    // Initialize static world.
    
    // Initialize framebuffer stuff.
    UpdateFramebufferObject( renderWidth, renderHeight );
    
    // Some commands.
    c_CommandManager->Add( "glrefresh", lazyRefreshGraphics );
    c_CommandManager->Add( "ss", lazyScreenshot );
    
    // Precache some stuff. :>
    Precache();
    
    // Check for some errors or warnings.
    g_err = glGetError();
    if ( g_err != GL_NO_ERROR ) {
        g_Core->Print( LOG_WARN, "Last OpenGL warning after renderer initialization: 0x%x\n", g_err );
    }
    
    t2 = c_coreSystem->Milliseconds();
    g_Core->Print( LOG_INFO, "Renderer took %.1f ms to load.\n", t2 - t1 );
    
    State = RENDER_ACTIVE;
    
    Initialized = true;
}

/*
 
    Generate frame buffer.
 
*/
void ncRenderer::CreateFramebufferObject( uint *tex, uint *fbo, uint *rbo, uint *dbo, uint *depth, int winx, int winy ) {
    
    if( !g_Core->UseGraphics )
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
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, winx, winy);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, *dbo);
    
    // Depth texture.
    glGenTextures(1, depth);
    glBindTexture(GL_TEXTURE_2D, *depth);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, winx, winy, 0, GL_DEPTH_ATTACHMENT, GL_FLOAT, 0);
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
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_COMPONENT24, GL_TEXTURE_2D, *depth, 0);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    
    // Check for existing errors.
    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if( status != GL_FRAMEBUFFER_COMPLETE ) {
        g_Core->Error( ERR_OPENGL, "Could not initialize frame buffer object. Code %i\n", status );
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
        
    // Advanced scenes.
    // Used for OVR. Three scene eyes, - Left, Right, Center ( full ).
    int i;
    for( i = 0; i < 3; i++ ) {
        CreateFramebufferObject( &g_sceneBuffer[i].scene,
                        &g_sceneBuffer[i].frame,
                        &g_sceneBuffer[i].depth,
                        &g_sceneBuffer[i].render,
                        &g_sceneBuffer[i].depthtex, w, h);
    }
    
    // Water refraction.
    CreateFramebufferObject( &g_waterRefractionBuffer.scene,
               &g_waterRefractionBuffer.frame,
               &g_waterRefractionBuffer.depth,
               &g_waterRefractionBuffer.render,
               &g_waterRefractionBuffer.depthtex, w, h );
    
    // Water reflections.
    CreateFramebufferObject( &g_waterReflectionBuffer.scene,
               &g_waterReflectionBuffer.frame,
               &g_waterReflectionBuffer.depth,
               &g_waterReflectionBuffer.render,
               &g_waterReflectionBuffer.depthtex, w, h);
}

void ncRenderer::RenderGrass( void ) {
    // Grass.
}

/*
    Just render the world. :)
*/
void ncRenderer::RenderWorld( int msec, ncSceneEye eye ) {
    
    // Render static world.
    RenderBSP( false, eye );
    
    //g_gameTerrain->Render( eye );
    
    // Render beautiful sky.
    //gmodel_sky_render();
    
    // Render beautiful water.
    g_gameWater->Render( eye );
    
    // Render models.
    //_modelLoader.Render( false, eye );

    // Render grass.
    //RenderGrass();
}

/*
    Update view matrix rotation.
*/
void ncRenderer::UpdateMatrixRotation( void ) {
    
    g_playerCamera->g_curMousePosition.x = c_Mouse->x;
    g_playerCamera->g_curMousePosition.y = c_Mouse->y;
    
    if( c_Mouse->Holding ) {
        g_playerCamera->RotationMatrix.Identity();
        
        int nXDiff = (g_playerCamera->g_curMousePosition.x - g_playerCamera->g_lastMousePosition.x);
        int nYDiff = (g_playerCamera->g_curMousePosition.y - g_playerCamera->g_lastMousePosition.y);
    
        if( nYDiff != 0 ) {

            g_playerCamera->RotationMatrix.Rotate( -(float)nYDiff / 3.0f, g_playerCamera->g_vRight );

            Matrix4_TransformVector( &g_playerCamera->RotationMatrix, &g_playerCamera->g_vLook );
            Matrix4_TransformVector( &g_playerCamera->RotationMatrix, &g_playerCamera->g_vUp );
        }
        
        if( nXDiff != 0 ){
            g_playerCamera->RotationMatrix.Rotate( -(float)nXDiff / 3.0f, VECTOR_UP );
            
            Matrix4_TransformVector( &g_playerCamera->RotationMatrix, &g_playerCamera->g_vLook );
            Matrix4_TransformVector( &g_playerCamera->RotationMatrix, &g_playerCamera->g_vUp );
        }
    }
    
    g_playerCamera->g_lastMousePosition.x = g_playerCamera->g_curMousePosition.x;
    g_playerCamera->g_lastMousePosition.y = g_playerCamera->g_curMousePosition.y;
}

/*
    Beautiful water environment.
*/
void ncRenderer::RenderBeautifulWater( void ) {
    
    if ( !Render_Water.GetInteger() )
        return;
    
    glBindFramebuffer( GL_FRAMEBUFFER, g_waterReflectionBuffer.frame );
    glBindRenderbuffer( GL_RENDERBUFFER, g_waterReflectionBuffer.render );

    //glViewport( 0, 0, 640, 480 );
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Water reflections.
    if( Render_Reflections.GetInteger() ) {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        
        RenderBSP( true, EYE_FULL );
        //_modelLoader.Render( true, EYE_FULL );
        
        glDisable(GL_CULL_FACE);
    }
    
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // Water refraction.
    glBindFramebuffer( GL_FRAMEBUFFER, g_waterRefractionBuffer.frame );
    glBindRenderbuffer( GL_RENDERBUFFER, g_waterRefractionBuffer.render );
    
    // Having an issues on different screen sizes on ES, added this.
    //glViewport( 0, 0, 640, 480 );
    // ------
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Water refractions.
    if ( Render_Refractions.GetInteger() ) {
        glEnable(GL_DEPTH_TEST);
        glCullFace(GL_BACK);
        
        RenderBSP( false, EYE_FULL );
        //_modelLoader.Render( false, EYE_FULL );

        glDisable(GL_CULL_FACE);
    }
    
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


/*
    Render to texture
*/
void ncRenderer::PreRender( void ) {
    
#ifndef iOS_BUILD
    // Update mouse matrix rotation. ( Not required for mobile devices ).
    UpdateMatrixRotation();
#endif
    
    // Update matrix view.
    UpdateViewMatrix();
    
    // Update bounding box.
    Frustum_Update();
    
    // Beautiful water.
    RenderBeautifulWater();
}

void ncRenderer::RenderToShader( ncSceneEye eye ) {

#ifndef iOS_BUILD
    
    // Scene buffer.
    glBindFramebuffer( GL_FRAMEBUFFER, g_sceneBuffer[eye].frame );
    glBindRenderbuffer( GL_RENDERBUFFER, g_sceneBuffer[eye].render );
    
    if( eye == EYE_LEFT ) {
        glViewport( 0, 0, windowWidth / 2.0, windowHeight );
    }
    else if( eye == EYE_RIGHT ) {
        glViewport( (windowWidth + 1) / 2.0, 0.0, windowWidth / 2.0, windowHeight );
    }
    else {
        glViewport( 0.0, 0.0, windowWidth, windowHeight );
    }
    
    glClearColor( 0.0, 0.0, 0.0, 1.0 );
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    RenderWorld( g_Core->Time, eye );
    
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    
    // Here comes shader.
    sceneShader->Use();
    
#ifdef OCULUSVR_BUILD
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
    
    if( eye == EYE_LEFT ) {
        
        sceneShader->SetUniform( "LensCenter", x + (w + DistortionXCenterOffset * 0.5f) * 0.5f, y + h * 0.5f );
        sceneShader->SetUniform( "ScreenCenter", x + w * 0.5f, y + h * 0.5f );
        sceneShader->SetUniform( "Scale", (w / 2.0f) * scaleFactor, (h / 2.0f) * scaleFactor * as );
        sceneShader->SetUniform( "ScaleIn",  (2.0f / w), (2.0f / h) / as );
        sceneShader->SetUniform( "HmdWarpParam", K0, K1, K2, K3 );

    } else if( eye == EYE_RIGHT ) {

        sceneShader->SetUniform( "LensCenter", rx + (rw + rDistortionXCenterOffset * 0.5f) * 0.5f, ry + rh * 0.5f );
        sceneShader->SetUniform( "ScreenCenter", rx + rw * 0.5f, ry + rh * 0.5f );
        sceneShader->SetUniform( "Scale", (rw / 2.0f) * scaleFactor, (rh / 2.0f) * scaleFactor * as );
        sceneShader->SetUniform( "ScaleIn",  (2.0f / rw), (2.0f / rh) / ras );
        sceneShader->SetUniform( "HmdWarpParam", K0, K1, K2, K3 );
        
    }
#endif
    
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, g_sceneBuffer[eye].scene );
    
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, g_sceneBuffer[eye].depthtex );
    
    glActiveTexture( GL_TEXTURE2 );
    glBindTexture( GL_TEXTURE_2D, lens_texture );
    
    glUniform1f( time_id, (float)( g_Core->Time * 10.0f ) );
    
    glViewport( 0.0, 0.0, windowWidth, windowHeight );
    
    glBindVertexArray( eye_vao[eye] );
    glDrawArrays( GL_TRIANGLES, 0, 6 );
    glBindVertexArray( 0 );
    
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    sceneShader->Next();
    
#endif
}

/*
    BSP level rendering.
*/
void ncRenderer::RenderBSP( int reflected, ncSceneEye eye ) {
    g_staticWorld->CalculateVisibleData( g_playerCamera->g_vEye );
    g_staticWorld->Render( reflected, eye );
}

/*
    Lets see the world!
 
    Mobile devices using another Rendering method defined 
    in its files!
*/
void ncRenderer::Render( int msec ) {
    if( ( !gl_Core->Initialized || !Initialized ) )
        return;
    
    glClear ( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    // Here we render the world.
    
    /*
        Render some stuff first.
    */
    PreRender();

    // Here you can tell what you want to render.
    
    if( Render_OVR.GetInteger() ) {
        RenderToShader( EYE_RIGHT );
        RenderToShader( EYE_LEFT );
    } else {
        RenderToShader( EYE_FULL );
    }

    // Console is moved out from *render to texture* since we don't care about its rendering,
    // it must be always shown correctly.
    //g_Console->Render();
    
#ifdef _WIN32
    SwapBuffers(_win.hDC);
#endif
}

/*
    Refresh graphical values.
*/
void ncRenderer::Refresh( void ) {
    if( !g_gameWorld->Active )
        return;
    
    g_gameWater->Refresh();
    
    // Update 2d screen filter shader.
    sceneShader->Use();
    
    sceneShader->SetUniform( width_id, (float)windowWidth );
    sceneShader->SetUniform( height_id, (float)windowHeight );
    sceneShader->SetUniform( usela_id, Render_ScreenLens.GetInteger() );
    sceneShader->SetUniform( usefx_id, Render_UseScreenFX.GetInteger() );
    sceneShader->SetUniform( usedof_id, Render_DepthOfField.GetInteger() );
    sceneShader->SetUniform( ovr_id, Render_OVR.GetInteger() );
    
    sceneShader->Next();
}

/*
    Remove every active object.
    Don't touch sky shader.
*/
void ncRenderer::RemoveWorld( const NString msg ) {
    if( !g_gameWorld->Active ) {
        return;
    }
    
    int i;
    
    g_Core->Print( LOG_INFO, "Removing current world. Reason: \"%s\".\n", msg );
    g_Core->Print( LOG_NONE, "\n");
    
    // Set render states to false.
    g_gameWorld->Active = false;
    g_gameWater->InUse = false;
    

    // Destroy terrain.
    //terrain_destroy();
    
    // Remove all models ( not the loaded ones! )
    g_gameWorld->spawned_models           = 0;
  
    g_modelManager->RemoveSpawnedModels();
    
    // Remove BSP level if exists.
    g_staticWorld->visibleFaces.ClearAll();
    g_staticWorld->Unload();
    
    g_playerCamera->Reset();
}

void ncRenderer::Shutdown( void ) {
    if( !Initialized )
        return;
    
    RemoveWorld( "Renderer shutdown" );
    
    // Object cleanup.
    DeleteMainBuffers();
    
    // Remove all shaders.
    g_shaderManager->Shutdown();
}

/*
    Delete all available buffers.
*/
void ncRenderer::DeleteMainBuffers( void ) {
    
    glDeleteBuffers( 3, eye_vao );

    
    // Delete scene buffers.
    for( int i = 0; i < 3; i++ ) {
        RemoveNCFramebuffer( &g_sceneBuffer[i] );
        g_eyeVBO[i].Delete();
        g_eyeUV[i].Delete();
    }
}

/*
    Remove generated framebuffer.
*/
void ncRenderer::RemoveNCFramebuffer( ncFramebuffer *buffer ) {
    glDeleteFramebuffers( 1, &buffer->frame );
    glDeleteTextures( 1, &buffer->scene );
    glDeleteRenderbuffers( 1, &buffer->render );
    glDeleteTextures( 1, &buffer->depthtex );
}

/*
    Take the game screenshot.
    FIX ME: Flipped image.
*/
void ncRenderer::MakeScreenshot( void ) {


}




