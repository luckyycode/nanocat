//
//  Nanocat engine.
//
//  Utilities..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef system_h
#define system_h

#include "Core.h"

/*
    Device types.
*/
enum ncDeviceType {
    UNKNOWN = -1, // Unknown.
    
    OSX = 0, // Macintosh
    
    // iOS based devices.
    IOS_iPad,
    IOS_iPod,
    IOS_iPhone,
    
    // Android.
    ANDROID,
    
    // Windows based devices,
    WINDOWS_DESKTOP,
    WINDOWS_MOBILE
};

class ncSystem {
public:
    void Initialize( void );
    void Frame( void );
    void Quit( const NString rmsg );
    
    const NString GetCurrentUsername( void );
    int Milliseconds( void );
    void ShowInfo( void );
    
    void PrintSystemTime( void );

private:
    int GetSysCTLValue( const char key[], void *dest );
    int GetSysFreeMemory( void );
    
    int         mem_available;
    int         mem_totalphysical;
    int         mem_totalvirtual;
    int         mem_used;
    
    ncDeviceType    m_CurrentDevice = UNKNOWN;
};

extern ncSystem *c_coreSystem;


// SYSTEM
extern ncConsoleVariable       System_GPUname;                                // System GPU name.
extern ncConsoleVariable       system_glversion;                          // OpenGL version.
extern ncConsoleVariable       system_glslversion;                        // GLSL version.

#endif
