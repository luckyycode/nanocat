//
//  SharedMac.h
//
//  Common includes and definitions for Mac/iOS systems.
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef SharedMac_h
#define SharedMac_h

#include <mach/mach.h>
#include <pwd.h>

#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/param.h>

// Network includes.
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netdb.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <CoreFoundation/CoreFoundation.h>

/*
    List of supported Apple devices:  
 
        iPhone 5s and above.
        iPad Air and above.
 
        All Mac computers with OpenGL 3.0 and above.
        All iOS devices with OpenGL ES 3.0 support.
 
    Use iOS_BUILD define when building an iOS build.
*/
 
#ifdef iOS_BUILD
    // OpenGL ES 3.0.
    #include <OpenGLES/ES3/gl.h>
    #include <OpenGLES/ES3/glext.h>
#else
    // OpenGL 3.0 and above.
    #include <OpenGL/gl3.h>
#endif

// Some common functions.
const char  *GetSystemVersion();
const char  *GetBundlePath();

void    MassageBox( const char *msg );
void    SetCursorPos( int x, int y );

#endif
