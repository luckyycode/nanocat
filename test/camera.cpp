//
//  camera.cpp
//  SkyCatCPP
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "camera.h"
#include "console.h"

ncCamera _camera;

ConsoleVariable  cam_speed("camera", "speed", "Camera movement speed.", "2", CVFLAG_KID);
ConsoleVariable  clientgame_fov("camera", "fov", "Client field of view.", "120.0", CVFLAG_NEEDSREFRESH);

void ncCamera::Initialize() {
    _core.Print( LOG_INFO, "Camera initializing...\n" );
}

void ncCamera::Reset() {
    
}

void ncCamera::Movement( uint key ) {
    if( _gconsole.IsShown() )   // Disable controls while using internal console.
        return;
    
    
#ifdef PLAYER_SERVERLESS_CONTROL
    ncVec3 tmpLook  = g_vLook;
    ncVec3 tmpRight = g_vRight;
    
    float speed = cam_speed->value;
#endif
    
    // Too much ifdefs, make me better.
    switch(key)
    {
            // 87 83 65 68
            
            // forward
#ifdef _WIN32
        case 87:
#else
        case 13:
        case 25:
#endif
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpLook * -speed;
            _camera.g_vEye = _camera.g_vEye - s;
            
            
#else
            _camera.deltaMove = 1;
#endif
        }
            break;
            
            // back
#ifdef _WIN32
        case 83:
#else
        case 1:
        case 39:
#endif
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpLook * -speed;
            _camera.g_vEye = _camera.g_vEye + s;
#else
            _camera.deltaMove = 2;
#endif
        }
            
            break;
            
            // left
#ifdef _WIN32
        case 65:
#else
        case 0:
#endif
        {
#ifdef PLAYER_SERVERLESS_CONTROL
            ncVec3 s = tmpRight * speed;
            _camera.g_vEye = _camera.g_vEye - s;
#else
            _camera.deltaMove = 3;
#endif
        }
            
            break;
            // right
#ifdef _WIN32
        case 68:
#else
        case 2:
#endif
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