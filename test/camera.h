//
//  Nanocat engine.
//
//  Game camera..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef camera_h
#define camera_h

#include "GameMath.h"
#include "SystemShared.h"

// (Dev) Uncomment to use non-server based player control.
#define PLAYER_SERVERLESS_CONTROL

class ncCamera {
public:
    
    ncVec3 g_vEye;
    ncVec3 g_vLook;
    ncVec3 g_vUp;
    ncVec3 g_vRight;
    
    ncCamera() {

    }
    
    void Initialize( void );
    void Reset( void );
    void Movement( char key );
    void Frame( float msec );
 
    int deltaMove;
    
    ncVec2  g_lastMousePosition;
    ncVec2  g_curMousePosition;
    
    float GetLastMoveTime( void );
    ncVec3 GetLastMovePosition( void );
    
    ncMatrix4 RotationMatrix;
    ncMatrix4 ProjectionMatrix;
    ncMatrix4 ViewMatrix;
   
private:
    float lastMoveAt;
    ncVec3 lastPosition;
};

extern ncCamera *g_playerCamera;

extern ncConsoleVariable GCamera_Speed;

#endif
