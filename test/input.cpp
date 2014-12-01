//
//  Nanocat engine.
//
//  Game input..
//
//  Created by Neko Vision on 1/1/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "System.h"
#include "Console.h"
#include "Input.h"
#include "Camera.h"
#include "SystemShared.h"

// Temp.
#include "ncBSP.h"
#include "BeautifulEnvironment.h"

ncConsoleVariable  clientgame_mousesensivity("input", "mousesens", "Mouse sensivity", "0.001", CVFLAG_NONE);
ncConsoleVariable  clientgame_invertmouse("input", "invertmouse", "Mouse invertion", "0", CVFLAG_NONE);

ncConsoleVariable  dev_showcursorpos("dev", "showcursorpos", "Show cursor position?", "0", CVFLAG_NONE);

ncInput local_coreInput;
ncInput *g_Input = &local_coreInput;

/*
    Initialize user input stuff.
*/
void ncInput::Initialize( void ) {
    
    g_Core->LoadState = NCCLOAD_INPUT;
    g_Core->Print( LOG_INFO, "Initializing input..\n" );

    SetMouseX( 0.0f );
    SetMouseY( 0.0f );
    SetMouseHold( false );
    
    g_playerCamera->Initialize();
}

void ncInput::MakeKeyEvent( char key ) {
    OnKeyPress( key );
}

void ncInput::MakeKeyEvent( ncInputKeyType type ) {
    switch ( type ) {
        case NCKEY_FORWARD:
            OnKeyPress( 'w' );
            break;
            
        case NCKEY_BACKWARD:
            OnKeyPress( 's' );
            break;
            
        case NCKEY_ESCAPE:
            OnKeyPress( '3' );
            break;
            
        case NCKEY_STRAFELEFT:
            OnKeyPress( 'a' );
            break;
            
        case NCKEY_STRAFERIGHT:
            OnKeyPress( 'd' );
            break;
            
        default:
            g_Core->Print( LOG_WARN, "ncInput::MakeKeyEvent - wrong key event given.\n" );
            break;
    }
}

/*
        Key press handle.
*/
#include "Models.h"
ncVec3 scatpos = ncVec3( 0.0, 100.0, 0.0 );
void ncInput::OnKeyPress( char key )
{
    // Game console.
    g_Console->KeyInput( key );
   
    // Camera movement.
    g_playerCamera->Movement( key );

    switch( key ) {
            
        case 'r':
            g_Console->Execute( "glrefresh" );
            break;
            
        case '3':
            c_coreSystem->Quit("User quit.");
        break;
            
        case 'f':
            g_staticWorld->InUse = true;
        break;
            
            
        case 'a':
            
            
            g_modelManager->Spawn(MODEL_DEFAULT, "cat.sm1", "model", scatpos, scatpos, false);
            g_modelManager->Spawn(MODEL_DEFAULT, "cat.sm1", "model", scatpos, scatpos, false);
            break;
            
        case 'h': {
            ncVec3 w = bEnv->mTerrain->GetRandomPos();
            g_playerCamera->lastPosition = w;
            }
            break;
        case 'l':
            g_Input->SetMouseHold( !g_Input->IsMouseHold() );
            break;

	}
}

/*
    Handles key release.
*/
void ncInput::OnKeyUp( char key ) {
    g_playerCamera->deltaMove = 0;
}

/*
    Mouse move.
*/
void ncInput::OnMouseMove( int x, int y ) {
    SetMouseX( x );
    SetMouseY( y );
}

/*
    Mouse key press.
*/

void ncInput::OnMouseDown( int x, int y ) {
   
    // Right handed.
    /*
    float mx = (float)((x - 1366.0 * 0.5) * (1.0 / 1366.0) * 85.0 * 0.5);
    float my = (float)((y - 768.0 * 0.5) * (1.0 / 1366.0) * 85.0 * 0.5);
    ncVec3 dx = g_playerCamera->g_vRight * mx;
    ncVec3 dy = g_playerCamera->g_vUp * my;
    
    ncVec3 dxdy = dx + dy;
    
    ncVec3 dir = g_playerCamera->g_vEye + dxdy * 2.0;
    dir = dir - ncVec3(100.0);
    ncVec4 ray_clip = ncVec4( dir.x, dir.y, -1.0, 1.0 );
    g_modelManager->Spawn( MODEL_DEFAULT, "cat.sm1", "model", dir, dir, false );
    
    ncMatrix4 g_InvertedProjection = g_playerCamera->ProjectionMatrix;
    g_InvertedProjection.Inverse();
    g_InvertedProjection.Scale( ncVec3(ray_clip.x, ray_clip.y, ray_clip.z) );
    ncVec3 ray_eye = ncVec3(g_InvertedProjection.m[0], g_InvertedProjection.m[5], g_InvertedProjection.m[10]);
    
    ncMatrix4 g_InvertedViewMatrix = g_playerCamera->ViewMatrix;
    g_InvertedViewMatrix.Inverse();
    g_InvertedViewMatrix.Scale( ray_eye );
    
    ncVec3 ray_wor = ncVec3(g_InvertedViewMatrix.m[0], g_InvertedViewMatrix.m[5], g_InvertedViewMatrix.m[10]);
    // don't forget to normalise the vector at some point
    ray_wor.Normalize();
    
    
    // Method 2

    int x1 = x;
    int y1 = y;
    ncVec3 view = g_playerCamera->g_vLook - g_playerCamera->g_vEye; // 3D float vector
   // view.Normalize();
    
    ncVec3 h;
    h.Cross( view, g_playerCamera->g_vUp ); // 3D float vector
   // h.Normalize();
    
    ncVec3 v;
    v.Cross( h, view ); // 3D float vector
    //v.Normalize();
    
    float rad = 85.0f * M_PI / 180.0f;
    float vLength = tan( rad / 2 ) * 0.01;
    float hLength = vLength * (1360.0 / 768.0);
    
    v = v * vLength;
    h = h * hLength;

    x1 -= 1360.0 / 2;
    y1 -= 768.0 / 2;
    
    // scale mouse coordinates so that half the view port width and height
    // becomes 1
    y1 /= (768.0 / 2);
    x1 /= (1366.0 / 2);
    
    // linear combination to compute intersection of picking ray with
    // view port plane
    ncVec3 pos = g_playerCamera->g_vEye + view*0.01f + h*x + v*y;
    
    // compute direction of picking ray by subtracting intersection point
    // with camera position
    ncVec3 r_dir = pos - g_playerCamera->g_vEye;
    
    //dir.Normalize();
    g_modelManager->Spawn(MODEL_DEFAULT, "cat.sm1", "model", r_dir, r_dir, false);
    
    //float g_Gotit;
    //RaySphereIntersection( g_playerCamera->g_vEye, dir, scatpos, 6.0f, g_Gotit );
    */
  
}


/*
    Mouse key release.
*/
void ncInput::OnMouseUp( int x, int y ) {

}

/*
    Get key character.
*/

char ncInput::GetKeyFromNum( uint key ) {
    // mac keycodes
    //12 |13 |14 |15 |17 |16 |32 |34 |31 |35 |33 |30 |0 |1 |2 |3 |5 |4 |38 |40 |37 |41 |39 |42 |6 |7 |8 |9 |11 |45 |46 |43 |47 |44 |53 |
    //18 |19 |20 |21 |23 |22 |26 |28 |25 |29 |53 |
    
#ifdef __APPLE__
    switch (key) {
            
        case 18: return '1';
        case 19: return '2';
        case 20: return '3';
        case 21: return '4';
        case 23: return '5';
        case 22: return '6';
        case 26: return '7';
        case 28: return '8';
        case 25: return '9';
        case 29: return '0';
            
        case 12: return 'q';
        case 13: return 'w';
        case 14: return 'e';
        case 15: return 'r';
        case 17: return 't';
        case 16: return 'y';
        case 32: return 'u';
        case 34: return 'i';
        case 31: return 'o';
        case 35: return 'p';
        case 33: return '[';
        case 30: return ']';
        case 0: return 'a';
        case 1: return 's';
        case 2: return 'd';
        case 3: return 'f';
        case 5: return 'g';
        case 4: return 'h';
        case 38: return 'j';
        case 40: return 'k';
        case 37: return 'l';
        case 41: return ':';
        case 39: return 'g';
        case 42: return '\\';
        case 6: return 'z';
        case 7: return 'x';
        case 8: return 'c';
        case 9: return 'v';
        case 11: return 'b';
        case 45: return 'n';
        case 46: return 'm';
        case 43: return ',';
        case 47: return '.';
        case 44: return '/';
        case 49: return ' ';
        case 51: return ' ';
        case 27: return '_';
            
        default:
            return ' ';
    }
#elif __linux__
    switch(key)
    {
        case 94: return '<';
        case 10: return '1';
        case 11: return '2';
        case 12: return '3';
        case 13: return '4';
        case 14: return '5';
        case 15: return '6';
        case 16: return '7';
        case 17: return '8';
        case 18: return '9';
        case 19: return '0';
        case 20: return '_'; // Fix me
        case 21: return '=';
        case 24: return 'q';
        case 25: return 'w';
        case 26: return 'e';
        case 27: return 'r';
        case 28: return 't';
        case 29: return 'y';
        case 30: return 'u';
        case 31: return 'i';
        case 32: return 'o';
        case 33: return 'p';
        case 34: return '[';
        case 35: return ']';
        case 38: return 'a';
        case 39: return 's';
        case 40: return 'd';
        case 41: return 'f';
        case 42: return 'g';
        case 43: return 'h';
        case 44: return 'j';
        case 45: return 'k';
        case 46: return 'l';
        case 47: return ';';
        case 48: return '"';
        case 51: return '\\';
        case 49: return '`';
        case 52: return 'z';
        case 53: return 'x';
        case 54: return 'c';
        case 55: return 'v';
        case 56: return 'b';
        case 57: return 'n';
        case 58: return 'm';
        case 59: return ',';
        case 60: return '.';
        case 61: return '/';
            
        default:
            return ' ';
    }
#elif _WIN32
    switch(key)
    {
        case 129: return '~';
        case 49: return '1';
        case 50: return '2';
        case 51: return '3';
        case 52: return '4';
        case 53: return '5';
        case 54: return '6';
        case 55: return '7';
        case 56: return '8';
        case 57: return '9';
        case 48: return '0';
        case 189: return '_';
            // 8 - backspace
        case 81: return 'q';
        case 87: return 'w';
        case 69: return 'e';
        case 82: return 'r';
        case 84: return 't';
        case 89: return 'y';
        case 85: return 'u';
        case 73: return 'i';
        case 79: return 'o';
        case 80: return 'p';
        case 219: return '[';
        case 221: return ']';
        case 65: return 'a';
        case 83: return 's';
        case 68: return 'd';
        case 70: return 'f';
        case 71: return 'g';
        case 72: return 'h';
        case 74: return 'j';
        case 75: return 'k';
        case 76: return 'l';
        case 186: return ';';
        case 222: return '\'';
        case 220: return '\\';
        case 226: return '|';
        case 90: return 'z';
        case 88: return 'x';
        case 67: return 'c';
        case 86: return 'v';
        case 66: return 'b';
        case 78: return 'n';
        case 77: return 'm';
        case 188: return ',';
        case 190: return '.';
        case 191: return '/';
            // 32 - space
            
        default: return ' ';
    }
#endif // _WIN32
    
    return ' ';
}

// Mouse stuff.
bool ncInput::IsMouseHold() {
    return Holding;
}

uint ncInput::GetMouseX() {
    return x;
}

uint ncInput::GetMouseY() {
    return y;
}

void ncInput::SetMouseX( uint value ) {
    x = value;
}

void ncInput::SetMouseY( uint value ) {
    y = value;
}

void ncInput::SetMouseHold( bool value ) {
    Holding = value;
}