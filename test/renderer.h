//
//  Nanocat engine.
//
//  Main game renderer.
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef renderer_h
#define renderer_h

#include "Core.h"
#include "ShaderLoader.h"

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
    ncGLShader sceneShader;
    
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
    uint    eye_vbo[3];
    uint    eye_uv[3];
    
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
    void RemoveWorld( const char *msg );
    void MakeScreenshot( void );
    void Render( int msec );
    void Preload( void );
    
private:
    void RenderGrass( void );
    void RenderWorld( int msec, ncSceneEye eye );
    void UpdateMatrixRotation( void );
    void RenderBeautifulWater( void );
    void PreRender( void );
    void RenderToShader( ncSceneEye eye );
    void RenderBSP( int reflected, ncSceneEye oc );
    void PrecacheWorld( void );
    void UpdateViewMatrix( void );
    void Precache( void );
};

extern ncRenderer _renderer;

// Framebuffers.
extern ncFramebuffer _waterrefraction;
extern ncFramebuffer _waterreflection;
extern ncFramebuffer _scene;

// Renderer console variables.
extern ncConsoleVariable       render_openglversion;                   // OpenGL version.
extern ncConsoleVariable       render_wireframe;                       // Wireframe mode.
extern ncConsoleVariable       render_fullscreen;                      // Is game running full screen?
extern ncConsoleVariable       render_fog;                             // Is fog enabled?
extern ncConsoleVariable       render_fog_maxdist;                     // Fog maximum distance.
extern ncConsoleVariable       render_fog_mindist;                     // Fog minimum distance. ( Fix me: rewritten in shaders )
extern ncConsoleVariable       render_fog_colorR;                      // Fog red color.
extern ncConsoleVariable       render_fog_colorG;                      // Fog green color.
extern ncConsoleVariable       render_fog_colorB;                      // Fog blue color.
extern ncConsoleVariable       render_glow;                            // Scene gamma.
extern ncConsoleVariable       render_normalmap;                       // Is normal mapping enabled?
extern ncConsoleVariable       render_reflections;                     // Are environment (water) reflections are enabled?
extern ncConsoleVariable       render_water;                           // Render water?
extern ncConsoleVariable       render_sky;                             // Render sky?

extern ncConsoleVariable       render_usefbo;                          // Use frame buffer object? ( developer )
extern ncConsoleVariable       render_vsync;                           // Is vertical syncing enabled?
extern ncConsoleVariable       render_modeWidth;                       // Renderer width.
extern ncConsoleVariable       render_modeHeight;                      // Renderer height.
extern ncConsoleVariable       render_curvetesselation;                // Level curve tesselation.
extern ncConsoleVariable       render_lightmapgamma;                   // Lightmap gamma.
extern ncConsoleVariable       render_usescreenfx;                     // 2D screen filter.
extern ncConsoleVariable       render_dof;                             // Depth of field.
extern ncConsoleVariable       render_lensanamorph;                    // Lens anamorph.

extern ncConsoleVariable        render_ovr;                             // Virtual realilty.

// FONTS
extern ncConsoleVariable       render_fontspacing;                     // Font character line spacing.
extern ncConsoleVariable       render_fonttype;                        // Font type.

extern ncConsoleVariable          render_updatepvs;
#endif
