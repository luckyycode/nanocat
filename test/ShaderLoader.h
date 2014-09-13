//
//  Nanocat engine.
//
//  Shader manager..
//
//  Created by Neko Code on 8/27/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef shaderloader_h
#define shaderloader_h

#include "SystemShared.h"

#define MAX_SHADERNAMELEN 18

// Shaders.
#define MAX_SHADERS 64
#define MAX_SHADERNAME_LENGTH 12

class ncGLShader {
public:
    GLuint fragment;
    GLuint vertex;
    GLuint geometry;
    
    char s_name[MAX_SHADERNAMELEN];
    
    GLuint shader_id;
};

class ncShaderManager {
public:
    void Initialize( void );
    GLuint Compile( const char *shadername, char *data, GLenum type );
    void CompileFromFile( const char *file, GLuint *vs, GLuint *fs, GLuint *gs );
    void Load( const char *file );
    void Delete( ncGLShader *shader );
    
    int shaderCount;
    ncGLShader    *shaders;
};

extern ncShaderManager _shaderManager;

#endif
