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

class ncMouse {
public:
    bool Holding;
    uint x;
    uint y;
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
};

extern ncInput _input;
extern ncMouse _imouse;

// DEVELOPER
extern ncConsoleVariable       dev_showcursorpos;                         // Show cursor position on screen.


#endif
