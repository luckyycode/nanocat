//
//  Nanocat engine.
//
//  Game world stuff.
//
//  Created by Neko Code on 8/30/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef world_h
#define world_h

#include "gmath.h"
#include "consolevar.h"
#include "renderer.h"

/*
    Game World.
*/
class ncGameWorld {
public:
    unsigned int cmodel_count;
    unsigned int spawned_models;
    
    bool Active;
};

/*
    Game beautiful manager.
*/
class ncGameWater {
    friend class ncGameWorld;
    
public:
    // Water.
    void    Initialize( void );
    void    Render( eye_t eye );
    void    Remove( void );
    void    Refresh( void );
    void    Spawn( ncVec3 position, float size );

    unsigned int    water_sectors;
    
    bool Initialized;
    bool InUse;
};

extern ncGameWorld _gameworld;
extern ncGameWater _gamewater;

extern ConsoleVariable water_distance;                         // Water rendering distance.

#endif
