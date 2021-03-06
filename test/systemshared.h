//
//  Nanocat engine.
//
//  Common headers and defines..
//
//  Created by Neko Vision on 24/07/2014.
//  Copyright (c) 2013 Neko Vision. All rights reserved.
//

/*
    This header contains all system headers.
    and some necessary stuff.

 
 *                ,MMM8&&&.            *
        *        MMMM88&&&&&    .
                MMMM88&&&&&&&
 *              MMM88&&&&&&&&
                MMM88&&&&&&&&
                'MMM88&&&&&&'
                  'MMM8&&&'      *
    *    |\___/|
         )     (             .              '
        >\     /<
          )===(       *
         /     \
         |     |
        /       \           *
        \       /
 _/\_/\_/\__  _/_/\_/\_/\_/\_/\_/\_/\_/\_/\_
 |  |  |  |( (  |  |  |  |  |  |  |  |  |  |
 |  |  |  | ) ) |  |  |  |  |  |  |  |  |  |
 |  |  |  |(_(  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |                    |                    |
 */


#ifndef SHARED_H_INCLUDED
#define SHARED_H_INCLUDED

#include <stdio.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

/*                ,MMM8&&&.            *
        *        MMMM88&&&&&    .
                MMMM88&&&&&&&
 *              MMM88&&&&&&&&
                MMM88&&&&&&&&
                'MMM88&&&&&&'
                  'MMM8&&&'      *
    *    |\___/|
         )     (             .              '
        >\     /<
          )===(       *
         /     \
         |     |
        /       \           *
        \       /
 _/\_/\_/\__  _/_/\_/\_/\_/\_/\_/\_/\_/\_/\_
 |  |  |  |( (  |  |  |  |  |  |  |  |  |  |
 |  |  |  | ) ) |  |  |  |  |  |  |  |  |  |
 |  |  |  |(_(  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |                    |                    |
 */

// We don't need this on mobile devices.
// #define OCULUSVR_SUPPORTED

/*
     Mac OS and iOS.
*/

#ifdef __APPLE__

// Use this while building iOS build.
//#define iOS_BUILD

#include "SharedApple.h"

    #define MAX_PATH            256

    /*      Default Mac key codes.      */

    #define KEY_ENTER           13
    #define KEY_SPACE           49
    #define KEY_BACKSPACE       127
    #define KEY_TILDE           50
    #define KEY_ESCAPE          53

    #define KEY_ARROW_UP        126
    #define KEY_ARROW_DOWN      125
    #define KEY_ARROW_LEFT      123
    #define KEY_ARROW_RIGHT     124

#ifndef iOS_BUILD
    #define _osname             "Mac OS X"
#else 
    #define _osname             "iOS"
#endif

    #define DZeroMemory( type, size ) memset( type, 0, size )

/*
     Windows
     Using native window creation.
*/

#elif _WIN32

    #pragma once

    #include "SharedWin.h"

    #define _osname "Win"

    #define DZeroMemory( type, size ) memset( type, 0, size )

/*
 
     Linux

*/
#elif __linux__ || linux

    #include "SharedLinux.h"

    #define _osname     "Linux"

    // For some time, got it working on Linux,
    // but lazy to do something on it.
    #error Implement me!

#endif


/*
                *     ,MMM8&&&.            *
        *           MMMM88&&&&&    .                       *       *
           *       MMMM88&&&&&&&      *
    *              MMM88&&&&&&&&                   *
             *     MMM88&&&&&&&&           *           *
                  'MMM88&&&&&&'               *               *
        *            'MMM8&&&'      *    _
         |\___/|                      \\                *
   *    =) ^Y^ (=   |\_/|              ||    '      *
         \  ^  /    )a a '._.-""""-.  //                *       *
    *     )=*=(    =\T_= /    ~  ~  \//             *
         /     \     `"`\   ~   / ~  /    *                 *
  *      |     |         |~   \ |  ~/                               *
        /| | | |\         \  ~/- \ ~\           *
        \| | |_|/|        || |  // /`
 /_\_/\_//_// __//\_/\_/\_((_|\((_//\_/\_/\_
 |  |  |  | \_) |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |  |  |  |  |  |  |  |
 |                    |                    |
 
 I love you all, lazy ones.  */
#define ulong  unsigned long
#define uint   unsigned int
#define byte   unsigned char
#define Byte   unsigned char

#define NString char *
#define NChar char

#define ifarrdelete( w ) if( w ) \
                      { \
                        delete [] w; \
                      } \

// Uh...
// We throw error if something goes wrong, or just warn the user.
#define WARN_LEVEL_ERROR

// Error help text.
#define ERROR_REPORT                            ""

// System stuff.
#define _version                                "7k"

#define CORE_VERSION_MAJOR                      7
#define CORE_VERSION_MINOR                      1
#define CORE_VERSION_MM                         7.1

// Default directory & file names.
#define DEFAULT_EXEC_PATH                       "Debug"
#define MAPS_FOLDER                             "maps"
#define SHADER_FOLDER                           "shaders"
#define TEXTURE_FOLDER                          "textures"
#define MATERIAL_FOLDER                         "material"
#define MODEL_FOLDER                            "meshes"

#define MATERIAL_DEFAULT_NAME                   "main"
#define MAIN_CONFIG_FILE                        "config.nconf"

#define MEGABYTE                                1048576 // 1024*1024

// Server, client, network
#define MAX_SERVER_COMMAND                      1024
#define MAX_CLIENT_COMMAND                      1024

// Maximum server world entities ( clients, etc )
#define MAX_SERVER_ENTITIES                     4095

// Maximum reliable messages to be sent from server/client.
#define MAX_RELIABLESERVERMESSAGE               16384

// Command headers.
#define COMMANDHEADER_ACK                       22
#define COMMANDHEADER_MOVE                      23
#define COMMANDHEADER_SERVERACK                 21
#define COMMANDHEADER_SERVERENTITY              24

// System
void win_resize( int w, int h );

#endif

/*          *           *         *             *   *
        *        *    ,MMM8&&&&.            *      *
                    MMMM88&&&&&&    .                         *
                   MMMM88&&&&&&&&
        *           MMM88&&&&&&&&&         *
             *      MMM88&&&&&&&&&  *
                    'MMM88&&&&&&'             *
          *           'MMM8&&&'      *    *       *
         |\___/|     /\___/\
    *    )     (     )    ~( .      *       '
        =\     /=   =\~    /=
    *     )===(       ) ~ (     *               *
         /     \     /     \        *
         |     |     ) ~   (            *
        /       \   /     ~ \   *               *
        \       /   \~     ~/
 _/\_/\_/\__  _/_/\_/\__~__/_/\_/\_/\_/\_/\_
 |  |  |  |( (  |  |  | ))  |  |  |  |  |  |
 |  |  |  | ) ) |  |  |//|  |  |  |  |  |  |
 |  |  |  |(_(  |  |  (( |  |  |  |  |  |  |
 |  |  |  |  |  |  |  |\)|  |  |  |  |  |  |
 |                    |                    |
*/


