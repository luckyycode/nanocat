//
//  Nanocat engine.
//
//  System manager.
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "system.h"
#include "network.h"
#include "console.h"
#include "files.h"
#include "ncstring.h"

ConsoleVariable    system_gpu("sys", "gpu", "Graphics device name.", "Unknown", CVAR_SYS);
ConsoleVariable    system_glversion("sys", "glversion", "OpenGL version.", "0", CVAR_SYS);
ConsoleVariable    system_glslversion("sys", "glslversion", "OpenGL shading language version", "0", CVAR_SYS);
ConsoleVariable    system_minmemory("sys", "minmemory", "Minimum system memory required.", "256", CVAR_SYS);
ConsoleVariable    system_cpucores("sys", "cpucores", "Processor cores count.", "1", CVAR_SYS);
ConsoleVariable    system_cpuspeed("sys", "cpuspeed", "Processor clock speed.", "0", CVAR_SYS);
ConsoleVariable    system_physmem("sys", "physmem", "Physical memory available.", "0", CVAR_SYS);
ConsoleVariable    system_virtmem("sys", "virtmem", "Virtual memory available.", "0", CVAR_SYS);

ncSystem _system;

/*
    System frame function.
*/
void ncSystem::Frame( void ) {
    //system_updatemem( &_system );
}

/*
    System initialization.
*/
void ncSystem::Initialize( void ) {

    _core.Print( LOG_INFO, "System initializing..\n" );
    
    _system.mem_used = 0;
    
    // Non-blocking stdin.
    //fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
    fcntl(0, F_SETFL, fcntl (0, F_GETFL, 0) | FNDELAY);
    
#ifdef __APPLE__

    ulong long ull;
    ulong ul;
    
    bool error;

    _core.Print( LOG_INFO, "OSX Version: %s\n", GetSystemVersion() );

    error = GetSysCTLValue("hw.memsize", &ull);
    
    if( error ) {
        _core.Print( LOG_WARN, "Couldn't get physical memory size.\n" );
    } else {
        _core.Print( LOG_INFO, "Physical memory installed: %d mb\n", (ull >> 20) );
        system_physmem.Set( _stringhelper.STR("%u", ( ull >> 20 ) ) );
    }
    
    error = GetSysCTLValue("hw.usermem", &ul);
    
    if( error ) {
        _core.Print( LOG_WARN, "Couldn't get user memory size.\n" );
    } else {
        _core.Print( LOG_INFO, "User memory: %d mb\n", (ul >> 20) / MEGABYTE );
    }
    
    error = GetSysCTLValue("hw.cpufrequency", &ull);
    
    if( error ) {
        ull = 0;
        _core.Print( LOG_WARN, "Couldn't determine CPU frequency.\n" );
    } else {
        ull /= 1000000;
        _core.Print( LOG_INFO, "Processor clock frequency: %d mhz\n", ull );
        system_cpuspeed.Set( _stringhelper.STR( "%u", ull ) );
    }
    
    error = GetSysCTLValue("hw.ncpu", &ul);
    
    if( error ) {
        _core.Print( LOG_WARN, "Couldn't determine CPU number of cores.\n" );
    } else {
        _core.Print( LOG_INFO, "Available processor cores: %i\n", ul );
        system_cpucores.Set( _stringhelper.STR( "%u", ul ) );
    }
    
#else
    
    MEMORYSTATUSEX statex;
    
    statex.dwLength = sizeof( statex );
    
    if( GlobalMemoryStatusEx( &statex ) ) {
        consolevar_set( "system_physmem", _stringhelper.STR( "%u", statex.ullTotalPhys / MEGABYTE ), true );
        consolevar_set( "system_virtmem", _stringhelper.STR( "%u", statex.ullTotalVirtual / MEGABYTE ), true );
        
        _core.Print( LOG_INFO, "Installed physical memory: %u\n", statex.ullTotalPhys / MEGABYTE );
        _core.Print( LOG_INFO, "Available virtual memory: %u\n", statex.ullTotalVirtual / MEGABYTE );
    } else {
        _core.Print( LOG_WARN, "Could not get memory status.\n" );
    }
    
#endif
    
    _core.Print( LOG_INFO, "Not bad at all!\n" );

    unsigned int availableMemory = system_physmem.GetInteger();
    
    if( availableMemory < system_minmemory.GetInteger() ) {
        _core.Error( ERC_FATAL, "Couldn't launch the game, your device should have more than %i MB of ram. You have %i MB.\n", system_minmemory.GetInteger(), availableMemory );
        return;
    }
}

/*
    Get current user name.
*/
const char *ncSystem::GetCurrentUsername( void ) {
    
#ifdef __WIN32
    
    char username[256 + 1];
    uint username_len = 256 + 1;
    GetUserName(username, &username_len);

    return username;
    
#else
    
    struct passwd *u;

    if ( !( u = getpwuid( getuid() ) ) ) {
        return "User";
    }

    return u->pw_name;
    
#endif

    return "Unknown";
}

/*
    Shutdown everything and quit safety.
*/
void ncSystem::Quit( const char *rmsg ) {
    
    // Notify the user.
    _core.Print( LOG_NONE, "\n" );
    _core.Print( LOG_INFO, "Game quit\n" );
    _core.Print( LOG_INFO, "%s\n", rmsg );

    // Disconnect from server. ( If connected )
    _core.Disconnect();

    // Shutdown server.
    _gconsole.Execute("lazykillserver");
    
    // Shutdown the networking.
    _netmanager.Shutdown();

    // Some information.
    _core.Print( LOG_INFO, "Game was launched for %i minute(s)\n", ( _core.Time / 60000 ) );
    _core.Print( LOG_INFO, "Seems like everything is okay.. quitting!\n" );

    // Shutdown the file system.
    _filesystem.Shutdown();

    // Clear the console buffer & write to the file.
    _gconsole.Execute( "clear" );

    // Remove console variables.
    _cvarmngr.Shutdown();
    
    // Now just quit
    exit( EXIT_SUCCESS );
}

/*
    Get current time in milliseconds.
*/

ulong systemTime = 0;
int curtime;

int ncSystem::Milliseconds( void ) {
    struct timeval tp;
    
    gettimeofday( &tp, NULL );
    
    if ( !systemTime ) {
        systemTime = tp.tv_sec;
        return tp.tv_usec / 1000;
    }
    
    curtime = (int)(tp.tv_sec - systemTime) * 1000 + tp.tv_usec / 1000;
    
    return curtime;
}

#ifdef __APPLE__
int ncSystem::GetSysCTLValue( const char key[], void *dest ) {
    size_t len = 0;
    int err;
    
    err = sysctlbyname(key, NULL, &len, NULL, 0);
    if (err == 0) {
        err = sysctlbyname(key, dest, &len, NULL, 0);
    }
    return err;
}
#endif
