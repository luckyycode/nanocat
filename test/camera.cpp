//
//  Nanocat engine.
//
//  Game camera..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "Camera.h"
#include "Console.h"

ncCamera local_camera;
ncCamera *g_playerCamera = &local_camera;

ncConsoleVariable  GCamera_Speed( "camera", "speed", "Camera movement speed.", "20", CVFLAG_KID );
ncConsoleVariable  GameView_FieldOfView( "camera", "fov", "Client field of view.", "85.0", CVFLAG_NEEDSREFRESH );

void ncCamera::Initialize() {
    NC_LOG( "Camera initializing...\n" );
    
    deltaMove = 0;
    
    lastMoveAt = 0.0;
    lastPosition = ncVec3( 10.0 );
    
   /* "_color" "0.5 0.9 0.4"
    "light" "300"
    "origin" "216 -216 128"
    "classname" "light"*/
}

void ncCamera::Reset() {
    
}

void ncCamera::Frame( float msec ) {
    
    float t = 1.0f - ( (lastMoveAt - msec) / 0.3 );
    
    g_playerCamera->g_vEye.x = Math_Lerpf2( g_playerCamera->g_vEye.x, lastPosition.x, (t / 30.0f) / 25.0f );
    g_playerCamera->g_vEye.y = Math_Lerpf2( g_playerCamera->g_vEye.y, lastPosition.y, (t / 30.0f) / 25.0f );
    g_playerCamera->g_vEye.z = Math_Lerpf2( g_playerCamera->g_vEye.z, lastPosition.z, (t / 30.0f) / 25.0f );
}

void ncCamera::Movement( char key ) {
    if( g_Console->IsShown() )   // Disable controls while using game (internal) console.
        return;
    
    
#ifdef PLAYER_SERVERLESS_CONTROL
    ncVec3 tmpLook  = g_vLook;
    ncVec3 tmpRight = g_vRight;
    
    float speed = GCamera_Speed.GetFloat(); 
#endif
    
    switch( key ) {

        // Forward.
        case 'w':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpLook * -speed;
            lastPosition = lastPosition - s;
#else
            g_playerCamera->deltaMove = 1;
#endif
        }
        break;
            
        // Backwards.
        case 's':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpLook * -speed;
            lastPosition = lastPosition + s;
#else
            g_playerCamera->deltaMove = 2;
#endif
        }
            
        break;
            
        // Left.
        case 'a':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpRight * speed;
            lastPosition = lastPosition - s;
#else
            g_playerCamera->deltaMove = 3;
#endif
        }
            
        break;
            
        // Right.
        case 'd':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpRight * speed;
            lastPosition = lastPosition + s;
#else
            g_playerCamera->deltaMove = 4;
#endif
        }
        break;
            
        default: return;
            
    }
}

/* 
    Get last move time.
*/
float ncCamera::GetLastMoveTime( void ) {
    return lastMoveAt;
}

/*
    Get last move position.
*/
ncVec3 ncCamera::GetLastMovePosition( void ) {
    return lastPosition;
}

