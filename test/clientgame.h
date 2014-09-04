//
//  Nanocat engine.
//
//  Client game.
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
    bool Loadmap( const char *map_name );
};

extern ncClientGame _clientgame;

// CLIENT SETTINGS
extern ConsoleVariable       clientgame_mousesensivity;                 // Mouse sensitivity.
extern ConsoleVariable       clientgame_fov;                            // Field of view.
extern ConsoleVariable       cam_speed;                                 // Default movement speed.

// GAME SYSTEM
extern ConsoleVariable       log_verbose;                       // More detailed logging.
extern ConsoleVariable       namevar;                              // Player name.
extern ConsoleVariable       version;                           // Game version.
extern ConsoleVariable       mapname;                           // Current map name, probably always going to be "world".


#endif
