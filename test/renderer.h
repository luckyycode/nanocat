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

#include "core.h"
#include "shader.h"

enum eye_t {
    EYE_FULL = 0,
    EYE_LEFT = 1,
    EYE_RIGHT = 2
};

enum renderstate_t {
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
    int     scene_texture;
    int     depth_texture;
    int     lensdirt_texture;
    int     usefx_id;
    int     usela_id;
    int     usedof_id;
    int     time_id;
    int     width_id;
    int     height_id;
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
    
    renderstate_t State;
    bool Initialized;
    
    void PrecacheWorld( void );
    void Preload( void );
    void UpdateViewMatrix( void );
    void Precache( void );
    void Initialize( void );
    void CreateFramebufferObject( uint *tex, uint *fbo, uint *rbo, uint *dbo, uint *depth, int winx, int winy );
    void UpdateFramebufferObject( int w, int h );
    void RenderGrass( void );
    void RenderWorld( int msec, eye_t eye );
    void UpdateMatrixRotation( void );
    void RenderBeautifulWater( void );
    void PreRender( void );
    void RenderToShader( eye_t eye );
    void RenderBSP( int reflected, eye_t oc );
    void Render( int msec );
    void Refresh( void );
    void RemoveWorld( const char *msg );
    void MakeScreenshot( void );
};

extern ncRenderer _renderer;

// Framebuffers.
extern ncFramebuffer _waterrefraction;
extern ncFramebuffer _waterreflection;
extern ncFramebuffer _scene;

// Renderer console variables.
extern ConsoleVariable       render_openglversion;                   // OpenGL version.
extern ConsoleVariable       render_wireframe;                       // Wireframe mode.
extern ConsoleVariable       render_fullscreen;                      // Is game running full screen?
extern ConsoleVariable       render_fog;                             // Is fog enabled?
extern ConsoleVariable       render_fog_maxdist;                     // Fog maximum distance.
extern ConsoleVariable       render_fog_mindist;                     // Fog minimum distance. ( Fix me: rewritten in shaders )
extern ConsoleVariable       render_fog_colorR;                      // Fog red color.
extern ConsoleVariable       render_fog_colorG;                      // Fog green color.
extern ConsoleVariable       render_fog_colorB;                      // Fog blue color.
extern ConsoleVariable       render_glow;                            // Scene gamma.
extern ConsoleVariable       render_normalmap;                       // Is normal mapping enabled?
extern ConsoleVariable       render_reflections;                     // Are environment (water) reflections are enabled?
extern ConsoleVariable       render_water;                           // Render water?
extern ConsoleVariable       render_sky;                             // Render sky?

extern ConsoleVariable       render_usefbo;                          // Use frame buffer object? ( developer )
extern ConsoleVariable       render_vsync;                           // Is vertical syncing enabled?
extern ConsoleVariable       render_modeWidth;                       // Renderer width.
extern ConsoleVariable       render_modeHeight;                      // Renderer height.
extern ConsoleVariable       render_curvetesselation;                // Level curve tesselation.
extern ConsoleVariable       render_lightmapgamma;                   // Lightmap gamma.
extern ConsoleVariable       render_usescreenfx;                     // 2D screen filter.
extern ConsoleVariable       render_dof;                             // Depth of field.
extern ConsoleVariable       render_lensanamorph;                    // Lens anamorph.

// FONTS
extern ConsoleVariable       render_fontspacing;                     // Font character line spacing.
extern ConsoleVariable       render_fonttype;                        // Font type.

extern ConsoleVariable          render_updatepvs;
#endif
