//
//  Nanocat engine.
//
//  System manager.
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef system_h
#define system_h

#include "Core.h"

class ncSystem {
public:
    void Initialize( void );
    void Frame( void );
    void Quit( const char *rmsg );
    
    const char *GetCurrentUsername( void );
    int Milliseconds( void );
    void ShowInfo( void );
    
private:
    int GetSysCTLValue( const char key[], void *dest );
    
    int         mem_available;
    int         mem_totalphysical;
    int         mem_totalvirtual;
    int         mem_used;
    
};

extern ncSystem _system;


// SYSTEM
extern ncConsoleVariable       system_gpu;                                // System GPU name.
extern ncConsoleVariable       system_glversion;                          // OpenGL version.
extern ncConsoleVariable       system_glslversion;                        // GLSL version.

#endif
