//
//  Nanocat engine.
//
//  Game world environment.
//
//  Created by Neko Vision on 3/5/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "bsp.h"
#include "ncstring.h"
#include "levelenvironment.h"

ncLevelEnvironment _levelenvironment;

typedef struct _light {
    ncVec3 position;
    ncVec3 color;
    
    float dotSize;
} light_t;

light_t _lights[4];

void env_setlights( void ) {
    int i;
    
    // Temp.
    _lights[0].color.x = 0.8;
    _lights[0].color.y = 0.4;
    _lights[0].color.z = 0.3;
    _lights[0].position.x = -11.0;
    _lights[0].position.y = 50.0;
    _lights[0].position.z = -11.0;
    _lights[0].dotSize = 1.0;
    
    _lights[1].color.x = 0.8;
    _lights[1].color.y = 0.4;
    _lights[1].color.z = 0.3;
    _lights[1].position.x = -367.0;
    _lights[1].position.y = 27.0;
    _lights[1].position.z = -5.0;
    _lights[1].dotSize = 1.0;
    
    _lights[2].color.x = 0.8;
    _lights[2].color.y = 0.4;
    _lights[2].color.z = 0.3;
    _lights[2].position.x = -102.0;
    _lights[2].position.y = 1.33;
    _lights[2].position.z = -97.0;
    _lights[2].dotSize = 1.0;
    
    _lights[3].color.x = 0.8;
    _lights[3].color.y = 0.4;
    _lights[3].color.z = 0.3;
    _lights[3].position.x = -24.0;
    _lights[3].position.y = 37.0;
    _lights[3].position.z = -215.0;
    _lights[3].dotSize = 1.0;

    glUseProgram( _bspmngr.bspShader.shader_id );
    
    for( i = 0; i < 3; i++ ) {
        glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("lights[%i].position", i) ), _lights[i].position.x, _lights[i].position.y, _lights[i].position.z );
        glUniform3f( glGetUniformLocation( _bspmngr.bspShader.shader_id, _stringhelper.STR("lights[%i].diffuse", i) ), _lights[i].color.x, _lights[i].color.y, _lights[i].color.z );
    }
    
    glUseProgram( 0 );
    
}

void ncLevelEnvironment::Prepare( void ) {
    env_setlights();
}
