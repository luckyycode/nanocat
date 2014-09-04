//
//  Nanocat engine.
//
//  Game shader loader.
//
//  Created by Neko Vision on 06/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "shader.h"
#include "core.h"
#include "files.h"
#include "ncstring.h"
#include "system.h"
#include "gl.h"

// Private stuff.
ncShaderManager _shaderManager;

/*
    Precache shader stuff.
*/
void ncShaderManager::Initialize( void ) {
    shaderCount = 0;
    shaders = (ncGLShader*)malloc( sizeof(ncGLShader) * MAX_SHADERS );
}

/* 
    Compile shader.
*/
GLuint ncShaderManager::Compile( const char *shadername, char *data, GLenum type ) {
    int          status;
    long         length;
    
    GLuint       shader;
    
    shader = glCreateShader(type);
    length = strlen(data);
    
    const char *versionId = _stringhelper.STR( "#version %i\n", glsl_version.GetInteger() );
    const char *sources[2] = { versionId, data };
    
    glShaderSource( shader, 2, (const char**)&sources, NULL );
    glCompileShader( shader );
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

    if( !status ) {
        char error[1024];
        
        glGetProgramInfoLog(shader, 1024, NULL, error);
        
        _core.Print( LOG_ERROR, "Could not compile %s shader. Here's log:\n", shadername );
        _core.Print( LOG_NONE, error );
        
#ifdef WARN_LEVEL_ERROR
        _core.Error( ERC_ASSET, "Could not compile shader %s.\n See console log for more information.\n", shadername );
#endif
        
        glDeleteShader(shader);
        
        return 0;
    }
    
    return shader;
}

/*
    Load shader from file.
*/
void ncShaderManager::CompileFromFile( const char *file, GLuint *vs, GLuint *fs, GLuint *gs ) {

    long    result;
    int     i;
    
    char    *p;
    char    *data;
    char    *shaders[3];
    
    result = _filesystem.Load( _stringhelper.STR("%s/%s/%s.nshdr", filesystem_path.GetString(), SHADER_FOLDER, file), (void**)&data );
    
    zeromem( shaders, sizeof(shaders) );
    
    i = 0;
    p = strtok (data, "\"");
    
    while ( p ) {
        shaders[i++] = p;
        p = strtok (NULL, "\"");
    }
    
    if( (!shaders[0] || !shaders[1] ) ) {
        _core.Error( ERC_ASSET, "Could not load %s shader. Syntax error.\n", file );
        return;
    }

    *vs = Compile( file, shaders[0], GL_VERTEX_SHADER );
    *fs = Compile( file, shaders[1], GL_FRAGMENT_SHADER );
    
    
    if( shaders[2] )
        *gs = Compile( file, shaders[2], GL_GEOMETRY_SHADER );
    else *gs = 0;
}

/*
    Simply load shader.
*/
void ncShaderManager::Load( const char *file ) {

    uint    program, vs, fs, gs;
    float   t1, t2;
    int     err, infolen;

    ncGLShader *s_temp;

    if( shaderCount > MAX_SHADERS ) {
        _core.Error( ERC_ASSET, "Could not load %s shader. Limit %i exceeded.\n", file, MAX_SHADERS );
        return;
    }

    t1 = _system.Milliseconds();

    CompileFromFile( file, &vs, &fs, &gs );
    
    program = glCreateProgram();

    if( gs )
        glAttachShader(program, gs);
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &err);

    if(!err) {
        _core.Print( LOG_ERROR, "Could not link %s shader.\n", file );

        infolen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);

        if(infolen > 0) {

            char error[1024];
            glGetProgramInfoLog(program, 1024, NULL, error);

            _core.Print( LOG_ERROR, "Could not link %s shader. Here's log for you.\n", file );
            _core.Print( LOG_NONE, "%s\n", error );

#ifdef WARN_LEVEL_ERROR
            _core.Error( ERC_ASSET, "Could not link %s shader.\nLook console for shader log.\n", file );
#endif
            //free(error);
            return;
        }
    }
    else {
        if( strlen(file) > MAX_SHADERNAME_LENGTH ) {
            _core.Error( ERC_ASSET, "LoadShader: Too long shader name.\n" );
            return;
        }
        
        s_temp = &shaders[shaderCount];

        //memset( s_temp, 0, sizeof(ncGLShader) );

        s_temp->fragment = fs;
        s_temp->vertex = vs;
        s_temp->geometry = gs;
        s_temp->shader_id = program;

        _stringhelper.Copy( s_temp->s_name, file );

        t2 = _system.Milliseconds();
        _core.Print( LOG_DEVELOPER, "LoadShader: %s loaded.. ( %4.2f ms )\n", file, t2-t1 );

        shaderCount++;
    }
}

/*
    Remove the shader.
*/
void ncShaderManager::Delete( ncGLShader *shader ) {
    glDetachShader(shader->shader_id, shader->vertex);
    glDetachShader(shader->shader_id, shader->fragment);

    glDeleteShader(shader->vertex);
    glDeleteShader(shader->fragment);

    glDeleteProgram(shader->shader_id);

    bzero( &shader, sizeof( shader ) );
}

