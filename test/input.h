//
//  Nanocat engine.
//
//  Game input..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef input_h
#define input_h

enum ncInputKeyType {
    NCKEY_FORWARD = 0,
    NCKEY_BACKWARD,
    NCKEY_STRAFELEFT,
    NCKEY_STRAFERIGHT,
    NCKEY_ESCAPE
};

class ncInput {
public:
    void Initialize( void );
    void OnKeyPress( char key );
    void OnKeyUp( char key );
    void OnMouseMove( int x, int y );
    void OnMouseDown( int x, int y );
    void OnMouseUp( int x, int y );
    
    char GetKeyFromNum( uint key );
    
    void MakeKeyEvent( char key );
    void MakeKeyEvent( ncInputKeyType type );
    
    uint GetMouseX( void );
    uint GetMouseY( void );
    
    bool IsMouseHold( void );
    void SetMouseHold( bool value );
    void SetMouseY( uint value );
    void SetMouseX( uint value );
    
private:
    // Mouse stuff.
    bool Holding;
    uint x;
    uint y;
};

extern ncInput *g_Input;

// DEVELOPER
extern ncConsoleVariable       dev_showcursorpos;                         // Show cursor position on screen.


#endif
