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

typedef void (*ncFunctionCall) (void);

class ncConsoleCommand {
public:
    char            name[32];
    ncFunctionCall  function;
};

class ncCommandManager {
public:
    void Add( const NString cmd, ncFunctionCall command );
    NString Arguments( int c );
    int ArgCount( void );
    void Initialize( void );
    void Execute( const NString cmd[] );
    
    unsigned int CommandCount;
};

extern ncCommandManager *c_CommandManager;

#endif
