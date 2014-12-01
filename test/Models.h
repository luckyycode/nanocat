//
//  Nanocat engine.
//
//  Model loader and manager..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef model_h
#define model_h

#include "ShaderLoader.h"
#include "FileSystem.h"
#include "MaterialLoader.h"
#include "GameMath.h"
#include "Renderer.h"

#define MODEL_NAMELEN 16

// Maximum models ( ncPrecachedModel and ncModel uses MAX_MODELS limit ).
#define MAX_MODELS                              128

// Model header data.
#define SM1HEADER                               (('1'<<24)+('T'<<16)+('M'<<8)+'S') // SMt1
#define SM1VERSION                              38

// *.sm model header.
struct ncSM1Header {
    int     id;
    int     poly_num;
    char    material[12];
    
    // Vertices, normals, UVs.
    ncFileChunk  chunk[3];
};

enum ncModelType {
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
    
    // Decal, normal, specular.
    ncMaterial  *material[3];
    
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
    
    ncGLShader  *g_shader;
    
    bool        follow_player;
    ncModelType type;
};

class ncModelLoader {
public:
    void Initialize( void );
    void Spawn( ncModelType type, const NString name, const NString shadername,
                     ncVec3 pos, ncVec3 rot, bool followsplayer );
    
    ncModel Find( NString name );
    void Load( const NString filename );
    void Render( bool reflection, ncSceneEye eye );
    
    void RemoveSpawnedModels( void );
    
    // Precached Models.
    // Models which are used in the game.
    // It contains data like position, rotation, etc..
    ncPrecachedModel    *_gmodel;
    
    // Models.
    // Used by loader. Contains data like m_vertices,
    // normals, uvs, etc..
    ncModel             *_model;
};

extern ncModelLoader *g_modelManager;

#endif
