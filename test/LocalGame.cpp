//
//  Nanocat engine.
//
//  Client game..
//
//  Created by Neko Vision on 1/13/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "ncBSP.h"
#include "Console.h"
#include "ConsoleCommand.h"
#include "console.h"
#include "NCString.h"
#include "GameWorld.h"
#include "LocalGame.h"
#include "LevelEnvironment.h"
#include "Core.h"
#include "Terrain.h"

ncClientGame local_clientgame;
ncClientGame *cg_LocalGame = &local_clientgame;

/*
    Local client game.
*/

ncConsoleVariable     World_Name( "level", "name", "Current level name", "world", CVFLAG_READONLY );

/*
    Initialize client game.
*/

void ncClientGame::Initialize( void ) {
    g_Core->Print( LOG_INFO, "Client game initializing..\n" );
}

/*
    Just for fun and testing purposes.
*/
void ncClientGame::Error( void ) {
    if ( c_CommandManager->ArgCount() < 1 ) {
        g_Core->Print( LOG_INFO, "USAGE: error <reason>\n" );
        return;
    }
    
    g_Core->Error( ERR_FATAL, c_CommandManager->Arguments(0) );
}

/*

    Load level map

*/

bool ncClientGame::Loadmap( const NString map_name ) {
    g_Core->Print( LOG_DEVELOPER, "Loading %s..\n", map_name );

    // Remove current world ( if exists ).
    g_Console->Execute( "removeworld" );

    // Get map file and load it.
    if( g_staticWorld->Load( NC_TEXT( "%s.bsp", map_name ) ) ) {
        World_Name.Set( map_name );

        g_LevelEnvironment->Prepare();
        
        g_Core->Print( LOG_INFO, "Map load success..\n" );
        g_Core->Print( LOG_INFO, "Creating map entities..\n" );

        // Refresh map stuff.
        // console_exec( str("readconfig config/%s.nconf", map_name) );

        // Well, instant apply.
        g_Console->Execute( "glrefresh" );

        g_gameWorld->Active = true;
        g_gameWater->InUse = true;
    
        g_staticWorld->InUse = false;
        
        return true;
    }

    return false;
}
