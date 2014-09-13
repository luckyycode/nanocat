//
//  Nanocat engine.
//
//  OpenGL manager..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef header_h
#define header_h

#define NEAR    0.1
#define FAR     100000.0

class ncOpenGL {
public:
    void Initialize( void );
    void OnResize( int w, int h );
    void ShowInfo( void );
    
    unsigned int GetMajorVersion( void );
    unsigned int GetMinorVersion( void );
    
    bool Initialized;
    
private:
    int _majorVersion;
    int _minorVersion;
};

extern ncOpenGL _opengl;

extern ncConsoleVariable     GLSL_Version;                              // Game GLSL version to use.

#endif
