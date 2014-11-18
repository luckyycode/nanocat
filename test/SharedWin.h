//
//  SharedWin.h
//  SkyCatCPP
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef SharedWin_h
#define SharedWin_h

// I was lazy to rewrite some, so
#define uint    UINT
#define bool    boolean
#define Byte    byte
#define false   FALSE
#define true    TRUE

// For MEMORYSTATUS.
#define _WIN32_WINNT 0x0500

#include <ws2tcpip.h>
#include <winsock.h>
#include <winsock2.h>
#include <fcntl.h>
#include <sys/fcntl.h>

#include <windows.h>
#include <malloc.h>
#include <Windowsx.h>
#include <psapi.h>

#define GLEW_STATIC

// Used for OpenGL extensions, not window creation
// TODO - custom extensions without 3rd party libs
#include "glew.h"
#include "wglew.h"

#include <GL/gl.h>

#define GLEW_STATIC

#pragma comment ( lib, "Kernel32.lib" )
#pragma comment ( lib, "Ws2_32.lib" )

#pragma comment( lib, "glew32.lib" )
#pragma comment( lib, "glew32s.lib" )

// No GLU on newer OpenGL versions.
#define GLEW_NO_GLU

#endif
