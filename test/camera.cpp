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

ncCamera _camera;

ncConsoleVariable  cam_speed( "camera", "speed", "Camera movement speed.", "2", CVFLAG_KID );
ncConsoleVariable  clientgame_fov( "camera", "fov", "Client field of view.", "85.0", CVFLAG_NEEDSREFRESH );

void ncCamera::Initialize() {
    NC_LOG( "Camera initializing...\n" );
    
    deltaMove = 0;

}

void ncCamera::Reset() {
    
}

void ncCamera::Movement( char key ) {
    if( _gconsole.IsShown() )   // Disable controls while using game (internal) console.
        return;
    
#ifdef PLAYER_SERVERLESS_CONTROL
    ncVec3 tmpLook  = g_vLook;
    ncVec3 tmpRight = g_vRight;
    
    float speed = cam_speed.GetFloat(); 
#endif
    
    switch( key ) {

        // Forward.
        case 'w':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpLook * -speed;
            _camera.g_vEye = _camera.g_vEye - s;
#else
            _camera.deltaMove = 1;
#endif
        }
        break;
            
        // Backwards.
        case 's':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpLook * -speed;
            _camera.g_vEye = _camera.g_vEye + s;
#else
            _camera.deltaMove = 2;
#endif
        }
            
        break;
            
        // Left.
        case 'a':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpRight * speed;
            _camera.g_vEye = _camera.g_vEye - s;
#else
            _camera.deltaMove = 3;
#endif
        }
            
        break;
            
        // Right.
        case 'd':
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpRight * speed;
            _camera.g_vEye = _camera.g_vEye + s;
#else
            _camera.deltaMove = 4;
#endif
        }
        break;
            
        default: return;
            
    }
}