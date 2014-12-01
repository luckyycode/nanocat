//
//  BeautifulEnvironment.h
//  Nanocat
//
//  Created by Neko Code on 11/30/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __Nanocat__BeautifulEnvironment__
#define __Nanocat__BeautifulEnvironment__

#include "SystemShared.h"
#include "Terrain.h"

// Grass cells to be rendered near.
#define GRASS_CELLS_COUNT 32000

enum ncCGrassTypes {
    GRASS_BRUSH = 0,
    GRASS_LEAF,
};

/*
 *
 *      Beautiful environment renderer.
 *
 */

class ncCGrass {
public:
    friend class ncBeautifulEnvironment;
    
    float mPos[2];
    float mRotation[2];
    
    ncCGrass();
    
    // Setup positions and rotations.
    void GetPositions();
    void GetRotations();
    
    void SetType( ncCGrassTypes type );
    
    ncCGrass( float mPositionX, float mPositionY ) {
        // Just store the passed position
        mPos[0] = mPositionX;
        mPos[1] = mPositionY;
    }
    
    // Render method 
    void Render();
    void RenderCell();
    
private:
    ncCGrassTypes m_type = GRASS_BRUSH;
};


/*
 *  Main.
 */
class ncBeautifulEnvironment {
public:
    
    void Render( ncSceneEye eye );
    void Makeup();
    
    // Rendering.
    void RenderTerrain( ncSceneEye eye );
    
    void MakeFoliage();
    
    // Grass cells in the Galaxy..
    ncCGrass mCell[GRASS_CELLS_COUNT];
    
    bool Created = false;
    bool CanRenderGrass = false;
    bool CanRenderTerrain = false;
    
    ncLODTerrain *mTerrain = NULL;
};

extern ncBeautifulEnvironment   *bEnv;

#endif /* defined(__Nanocat__BeautifulEnvironment__) */
