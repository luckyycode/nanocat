//
//  Nanocat engine.
//
//  Game world environment..
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef levelenvironment_h
#define levelenvironment_h

class ncLevelEnvironment {
public:
    void Initialize( void );
    void Prepare( void );
    
    // Temp ( maybe )
    void PassShader( uint *id );
};

extern ncLevelEnvironment _levelenvironment;

#endif
