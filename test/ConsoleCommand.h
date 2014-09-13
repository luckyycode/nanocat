//
//  Nanocat engine.
//
//  Command manager..
//
//  Created by Neko Code on 8/28/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef command_h
#define command_h

typedef void (*functioncall_t) (void);

class ncConsoleCommand {
public:
    char            name[32];
    functioncall_t  function;
};

class ncCommandManager {
public:
    void Add( const char *cmd, functioncall_t command );
    char *Arguments( int c );
    int ArgCount( void );
    void Initialize( void );
    void Execute( const char *cmd[] );
    
    unsigned int CommandCount;
};

extern ncCommandManager _commandManager;

#endif
