//
//  Nanocat engine.
//
//  Main game renderer..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef renderer_h
#define renderer_h

#include "Core.h"
#include "ShaderLoader.h"
#include "VBOOpenGL.h"

enum ncSceneEye {
    EYE_FULL = 0,
    EYE_LEFT = 1,
    EYE_RIGHT = 2
};

enum ncRenderState {
    // Rendering right now.
    RENDER_ACTIVE,
    // Not initialized/stopped.
    RENDER_IDLE,
    RENDER_ERROR,
    RENDER_LOADING
};

class ncFramebuffer {
public:
    // Depth buffer.
    uint depth;
    // Render buffer.
    uint render;
    // Frame buffer.
    uint frame;
    // Depth texture.
    uint depthtex;
    // Scene texture.
    uint scene;
};

class ncRenderer {
public:
    // Renderer common stuff.
    // Just leave it here.
    // Scene 2d filter.
    ncGLShader *sceneShader;
    
    // Shader uniforms.
    // Used for main 2d screen filter.
    // Note/Todo: I should make something like dictionary.
    int     scene_texture;
    int     depth_texture;
    int     lensdirt_texture;
    int     usefx_id;
    int     usela_id;
    int     usedof_id;
    int     time_id;
    int     width_id;
    int     height_id;
    int     ovr_id;
    // ------------
    
    uint    eye_vao[3];
    ncGLVertexBufferObject    g_eyeVBO[3];
    ncGLVertexBufferObject    g_eyeUV[3];
  
    // Precached materials.
    uint    lens_texture;
    
    // Some settings.
    uint    renderWidth;
    uint    renderHeight;
    uint    windowWidth;
    uint    windowHeight;
    
    ncRenderState State;
    bool Initialized;

    void Initialize( void );
    void CreateFramebufferObject( uint *tex, uint *fbo, uint *rbo, uint *dbo, uint *depth, int winx, int winy );
    void UpdateFramebufferObject( int w, int h );
    void Refresh( void );
    void RemoveWorld( const NString msg );
    void MakeScreenshot( void );
    void Render( int msec );
    void RenderWorld( int msec, ncSceneEye eye ); // Not private.
    void PreRender( void );
    void Preload( void );
    void Shutdown( void );
    
    void RemoveNCFramebuffer( ncFramebuffer *buffer );
    void DeleteMainBuffers( void );
private:
    void RenderGrass( void );
    void UpdateMatrixRotation( void );
    void RenderBeautifulWater( void );
    void RenderToShader( ncSceneEye eye );
    void RenderBSP( int reflected, ncSceneEye oc );
    void PrecacheWorld( void );
    void UpdateViewMatrix( void );
    void Precache( void );
};

extern ncRenderer *g_mainRenderer;

// Framebuffers.
extern ncFramebuffer g_waterRefractionBuffer;
extern ncFramebuffer g_waterReflectionBuffer;
extern ncFramebuffer g_sceneBuffer[3];

// Renderer console variables.
extern ncConsoleVariable       OpenGL_Version;                   // OpenGL version.
extern ncConsoleVariable       Render_Wireframe;                       // Wireframe mode.
extern ncConsoleVariable       Render_Fullscreen;                      // Is game running full screen?
extern ncConsoleVariable       Render_Fog;                             // Is fog enabled?
extern ncConsoleVariable       Render_Fog_maxdist;                     // Fog maximum distance.
extern ncConsoleVariable       Render_Fog_mindist;                     // Fog minimum distance. ( Fix me: rewritten in shaders )
extern ncConsoleVariable       Render_Fog_colorR;                      // Fog red color.
extern ncConsoleVariable       Render_Fog_colorG;                      // Fog green color.
extern ncConsoleVariable       Render_Fog_colorB;                      // Fog blue color.
extern ncConsoleVariable       render_glow;                            // Scene gamma.
extern ncConsoleVariable       Render_Normalmaps;                       // Is normal mapping enabled?
extern ncConsoleVariable       Render_Reflections;                     // Are environment (water) reflections are enabled?
extern ncConsoleVariable       Render_Water;                           // Render water?
extern ncConsoleVariable       Render_Sky;                             // Render sky?

extern ncConsoleVariable       render_usefbo;                          // Use frame buffer object? ( developer )
extern ncConsoleVariable       Render_VSync;                           // Is vertical syncing enabled?
extern ncConsoleVariable       Render_Width;                       // Renderer width.
extern ncConsoleVariable       Render_Height;                      // Renderer height.
extern ncConsoleVariable       Render_CurveTesselation;                // Level curve tesselation.
extern ncConsoleVariable       Render_LightmapGamma;                   // Lightmap gamma.
extern ncConsoleVariable       Render_UseScreenFX;                     // 2D screen filter.
extern ncConsoleVariable       Render_DepthOfField;                             // Depth of field.
extern ncConsoleVariable       Render_ScreenLens;                    // Lens anamorph.

extern ncConsoleVariable        Render_OVR;                             // Virtual realilty.

// FONTS
extern ncConsoleVariable       render_fontspacing;                     // Font character line spacing.
extern ncConsoleVariable       render_fonttype;                        // Font type.

extern ncConsoleVariable        Render_CalculateVisibleData;

#endif
