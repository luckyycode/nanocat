//
//  Nanocat engine.
//
//  Model loader and manager.
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef model_h
#define model_h

#include "shader.h"
#include "files.h"
#include "material.h"
#include "gmath.h"
#include "renderer.h"

#define MODEL_NAMELEN 16

// Maximum models ( ncPrecachedModel and ncModel uses MAX_MODELS limit ).
#define MAX_MODELS                              128

// Model header data.
#define SM1HEADER                               (('1'<<24)+('T'<<16)+('M'<<8)+'S') // SMt1
#define SM1VERSION                              38

// *.sm model header.
struct sm1header_t {
    int     id;
    int     poly_num;
    char    material[12];
    
    // Vertices, normals, UVs.
    filechunk_t  chunk[3];
};

enum modeltype_t {
    MODEL_TREE,             // Tree model.
    MODEL_TERRAIN,          // Terrain model.
    MODEL_SKY,              // Sky model.
    MODEL_DEFAULT           // Just a model.
};

/*
    Contains main model data.
    Not used in game.
*/
class ncModel {
public:
    // Some fields are gonna to be used for
    // model manager ( including converter ) but not in the game.
    char    m_name[MODEL_NAMELEN];
    
    ncMaterial  material;
    
    int    _faces;
    
    uint m_vbo; // Vertex buffer object.
    uint m_vao; // Vertex array object.
    uint m_normals; // Normals.
    uint m_uv; // Texture coords.
    uint m_indexes; // Indices.
};

/*
    Precached model.
    Used in game.
*/
class ncPrecachedModel {
public:
    ncModel     *g_model;
    
    bool        in_use;
    
    ncVec3        position;
    ncVec3        rotation;
    
    bool        use_materials;
    bool        use_shader;
    
    ncGLShader    g_shader;
    
    bool        follow_player;
    modeltype_t type;
};

class ncModelLoader {
public:
    void Initialize( void );
    void Spawn( modeltype_t type, const char *name, const char *shadername,
                     ncVec3 pos, ncVec3 rot, bool followsplayer );
    
    ncModel Find( char *name );
    void Load( const char *filename );
    void Render( bool reflection, eye_t eye );
    
    // Precached Models.
    // Models which are used in the game.
    // It contains data like position, rotation, etc..
    ncPrecachedModel    *_gmodel;
    
    // Models.
    // Used by loader. Contains data like vertices,
    // normals, uvs, etc..
    ncModel             *_model;
};

extern ncModelLoader _modelLoader;

#endif
