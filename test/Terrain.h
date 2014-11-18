//
//  Terrain.h
//  Nanocat
//
//  Created by Neko Code on 11/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __Terrain__
#define __Terrain__

/*
    Beautiful game terrain.
*/
class ncHeightMapTerrain {
    
public:
    ncHeightMapTerrain( const NString image ) {
        Create( image );
    }
    
    uint i_Rows;
    uint i_Cols;
    
    uint i_ColCount;
    
    ncVec3  **v_VertexData;
    ncVec2  **v_CoordsData;
    ncVec3  **v_NormalData[2];
    
    ncGLVertexBufferObject g_NormalVBO;
    ncGLVertexBufferObject g_TexCoordVBO;
    ncGLVertexBufferObject g_VertexVBO;
    
    void Create( const NString imageName );
};

/*
    Game beautiful terrain renderer.
*/
class ncTerrainRenderer {
    friend class ncGameWorld;
    
public:
    // Terrain.
    void    Initialize( void );
    void    Render( ncSceneEye eye );
    void    Remove( void );
    void    Refresh( void );
    void    Spawn();
    
    bool Initialized;
    bool InUse;
};

extern ncTerrainRenderer *g_gameTerrain;

#endif
