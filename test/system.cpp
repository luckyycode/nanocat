//
//  Nanocat engine.
//
//  System manager..
//
//  Created by Neko Vision on 23/08/2013.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "System.h"
#include "Network.h"
#include "Console.h"
#include "FileSystem.h"
#include "NCString.h"
#include "Core.h"
#include "Renderer.h"

ncConsoleVariable    System_GPUname("sys", "gpu", "Graphics device name.", "Unknown", CVFLAG_SYS);
ncConsoleVariable    System_MinimumMemory("sys", "minmemory", "Minimum system memory required.", "256", CVFLAG_SYS);
ncConsoleVariable    System_CPUCores("sys", "cpucores", "Processor cores count.", "1", CVFLAG_SYS);
ncConsoleVariable    System_CPUSpeed("sys", "cpuspeed", "Processor clock speed.", "0", CVFLAG_SYS);
ncConsoleVariable    System_PhysMemory("sys", "physmem", "Physical memory available.", "0", CVFLAG_SYS);
ncConsoleVariable    System_VirtMemory("sys", "virtmem", "Virtual memory available.", "0", CVFLAG_SYS);

ncSystem local_coreSystem;
ncSystem *c_coreSystem = &local_coreSystem;

/*
    System initialization.
*/
void ncSystem::Initialize( void ) {

    g_Core->LoadState = NCCLOAD_SYSTEM;
    g_Core->Print( LOG_INFO, "System initializing..\n" );
    
    PrintSystemTime();
    
    c_coreSystem->mem_used = 0;
    
    // Non-blocking stdin.
    //fcntl (0, F_SETFL, fcntl (0, F_GETFL, 0) & ~FNDELAY);
    fcntl( 0, F_SETFL, fcntl( 0, F_GETFL, 0 ) | FNDELAY );
    
#ifdef __APPLE__

    ulong long ull;
    ulong ul;
    
    bool error;

    g_Core->Print( LOG_INFO, "OSX Version: %s\n", GetSystemVersion() );

    // Memory size.
    error = GetSysCTLValue("hw.memsize", &ull);
    if( error ) {
        g_Core->Print( LOG_WARN, "Couldn't get physical memory size.\n" );
    } else {
        g_Core->Print( LOG_INFO, "Physical memory installed: %d mb\n", (ull >> 20) );
        System_PhysMemory.Set( NC_TEXT( "%u", ( ull >> 20 ) ) );
    }
    
    g_Core->Print( LOG_INFO, "%i mb free memory available.\n", GetSysFreeMemory() );
    
    // User memory.
    error = GetSysCTLValue("hw.usermem", &ul);
    if( error ) {
        g_Core->Print( LOG_WARN, "Couldn't get user memory size.\n" );
    } else {
        g_Core->Print( LOG_INFO, "User memory: %d mb\n", (ul >> 20) / MEGABYTE );
    }
    
    // CPU clock frequency.
    error = GetSysCTLValue("hw.cpufrequency", &ull);
    if( error ) {
        ull = 0;
        g_Core->Print( LOG_WARN, "Couldn't determine CPU frequency.\n" );
    } else {
        ull /= 1000000;
        g_Core->Print( LOG_INFO, "Processor clock frequency: %d mhz\n", ull );
        System_CPUSpeed.Set( NC_TEXT( "%u", ull ) );
    }
    
#ifndef iOS_BUILD
    
    // Number of cores.
    error = GetSysCTLValue("hw.ncpu", &ul);
    if( error ) {
        g_Core->Print( LOG_WARN, "Couldn't determine CPU number of cores.\n" );
    } else {
        g_Core->Print( LOG_INFO, "Available processor cores: %i\n", ul );
        System_CPUCores.Set( NC_TEXT( "%u", ul ) );
    }
    
#endif
    
#else   // WINDOWS
    
    MEMORYSTATUSEX statex;
    
    statex.dwLength = sizeof( statex );
    
    if( GlobalMemoryStatusEx( &statex ) ) {
        consolevar_set( "System_PhysMemory", NC_TEXT( "%u", statex.ullTotalPhys / MEGABYTE ), true );
        consolevar_set( "System_VirtMemory", NC_TEXT( "%u", statex.ullTotalVirtual / MEGABYTE ), true );
        
        g_Core->Print( LOG_INFO, "Installed physical memory: %u\n", statex.ullTotalPhys / MEGABYTE );
        g_Core->Print( LOG_INFO, "Available virtual memory: %u\n", statex.ullTotalVirtual / MEGABYTE );
    } else {
        g_Core->Print( LOG_WARN, "Could not get memory status.\n" );
    }
    
#endif
    
    g_Core->Print( LOG_INFO, "Not bad at all!\n" );

    unsigned int availableMemory = System_PhysMemory.GetInteger();
    
    if( availableMemory < System_MinimumMemory.GetInteger() ) {
        g_Core->Error( ERR_FATAL, "Couldn't launch the game, your device should have more than %i MB of ram. You have %i MB.\n", System_MinimumMemory.GetInteger(), availableMemory );
        return;
    }
}

/*
    Get current user name.
*/
const NString ncSystem::GetCurrentUsername( void ) {
    
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
void ncSystem::Quit( const NString rmsg ) {
    
    // Notify the user.
    g_Core->Print( LOG_NONE, "\n" );
    g_Core->Print( LOG_INFO, "Application quit. %s\n", rmsg );

    // Disconnect from server. ( If connected )
    g_Core->Disconnect();

    // Shutdown server.
    g_Console->Execute("lazykillserver");
    
    // Shutdown the networking.
    g_networkManager->Shutdown();

    // Some information.
    g_Core->Print( LOG_INFO, "Game was launched for %i minute(s)\n", ( g_Core->Time / 60000 ) );
    g_Core->Print( LOG_INFO, "Seems like everything is okay.. quitting!\n" );

    // Shutdown the file system.
    c_FileSystem->Shutdown();

    // Renderer shutdown.
    g_mainRenderer->Shutdown();
    
    // Clear the console buffer & write to the file.
    g_Console->Execute( "clear" );

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
    struct timeval Current;
    
    gettimeofday( &Current, NULL );
    
    if ( !systemTime ) {
        systemTime = Current.tv_sec;
        return Current.tv_usec / 1000;
    }
    
    curtime = (int)(Current.tv_sec - systemTime) * 1000 + Current.tv_usec / 1000;
    
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

int ncSystem::GetSysFreeMemory( void ) {
    // Get free memory.
    vm_size_t g_pageSize;
    host_page_size( mach_host_self(), &g_pageSize );
    
    // Get memory stats.
    vm_statistics g_stats;
    mach_msg_type_number_t g_statsSize = sizeof( g_stats );
    host_statistics( mach_host_self(), HOST_VM_INFO, (host_info_t)&g_stats, &g_statsSize );
    
    return ( g_stats.free_count * g_pageSize ) / MEGABYTE;
}

#endif

void ncSystem::PrintSystemTime( void ) {
    struct tm *tmp;
    time_t s;
    

    s = time( NULL );
    tmp = localtime( &s );
    
    if( !tmp ) {
        g_Core->Print( LOG_ERROR, "Unable to get system time.\n" );
        return;
    }
    
    g_Core->Print( LOG_INFO, "System local time: %i:%i:%i\n", tmp->tm_hour, tmp->tm_min, tmp->tm_sec );
    
}

/*
    System frame function.
*/
void ncSystem::Frame( void ) {
    
}