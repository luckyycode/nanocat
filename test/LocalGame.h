//
//  Nanocat engine.
//
//  Client game..
//
//  Created by Neko Code on 8/31/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef clientgame_h
#define clientgame_h

// Local client game.

class ncClientGame {
public:
    void Initialize( void );
    void Error( void );
    bool Loadmap( const NString map_name );
};

extern ncClientGame *cg_LocalGame;

// CLIENT SETTINGS
extern ncConsoleVariable       clientgame_mousesensivity;                 // Mouse sensitivity.
extern ncConsoleVariable       GameView_FieldOfView;                            // Field of view.
extern ncConsoleVariable       GCamera_Speed;                                 // Default movement speed.

// GAME SYSTEM
extern ncConsoleVariable       log_verbose;                       // More detailed logging.
extern ncConsoleVariable       NameVar;                              // Player name.
extern ncConsoleVariable       core_version;                           // Game version.
extern ncConsoleVariable       World_Name;                           // Current map name, probably always going to be "world".


#endif
