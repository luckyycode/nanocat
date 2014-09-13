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

ncClientGame _clientgame;

/*
    Local client game.
*/

ncConsoleVariable     World_Name( "level", "name", "Current level name", "world", CVFLAG_READONLY );

/*
    Initialize client game.
*/

void ncClientGame::Initialize( void ) {
    _core.Print( LOG_INFO, "Client game initializing..\n" );
}

/*
    Just for fun and testing purposes.
*/
void ncClientGame::Error( void ) {
    if ( _commandManager.ArgCount() < 1 ) {
        _core.Print( LOG_INFO, "USAGE: error <reason>\n" );
        return;
    }
    
    _core.Error( ERC_CRASH, _commandManager.Arguments(0) );
}

/*

    Load level map

*/

bool ncClientGame::Loadmap( const char *map_name ) {
    _core.Print( LOG_DEVELOPER, "Loading %s..\n", map_name );

    // Remove current world ( if exists ).
    if( _gameworld.Active )
        _gconsole.Execute( "removeworld" );

    // Get map file and load it.
    if( _bspmngr.Load( _stringhelper.STR( "%s.bsp", map_name ) ) ) {
        World_Name.Set( map_name );

        _levelenvironment.Prepare();
        
        _core.Print( LOG_INFO, "Map load success..\n" );
        _core.Print( LOG_INFO, "Creating map entities..\n" );

        // Refresh map stuff.
        // console_exec( str("readconfig config/%s.nconf", map_name) );

        // Well, instant apply.
        _gconsole.Execute( "glrefresh" );

        _gameworld.Active   = true;
        _gamewater.InUse    = true;
        _bspmngr.InUse = true;
        
        return true;
    }

    return false;
}
