//
//  SharedMac.h
//  SkyCatCPP
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef SharedMac_h
#define SharedMac_h

#include <mach/mach.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/param.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>

#include <pwd.h>

// OpenGL 3 and more.
#include <OpenGL/gl3.h>

#include <netinet/in.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <net/if_types.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>



const char  *GetSystemVersion();
const char  *GetBundlePath();

void            MassageBox( const char *msg );          // Popup window.
void            SetCursorPos( int x, int y );

#endif
