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
#include "GameMath.h"

#define MAX_SHADERNAMELEN 18

// Shaders.
#define MAX_SHADERS 64
#define MAX_SHADERNAME_LENGTH 12

class ncGLShader {
    friend class ncShaderManager;
    
public:
    void Use( void );
    
    // glGetUniformLocation included.
    // Floats.
    void SetUniform( const NString uniname, GLfloat value );
    void SetUniform( const NString uniname, GLfloat v0, GLfloat v1 );
    void SetUniform( const NString uniname, GLfloat v0, GLfloat v1, GLfloat v2 );
    void SetUniform( const NString uniname, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 );
    void SetUniform( const NString uniname, GLsizei count, const GLfloat *values );
    
    // Integers.
    void SetUniform( const NString uniname, GLint value );
    void SetUniform( const NString uniname, GLint v0, GLint v1 );
    void SetUniform( const NString uniname, GLint v0, GLint v1, GLint v2 );
    void SetUniform( const NString uniname, GLint v0, GLint v1, GLint v2, GLint v3 );
    void SetUniform( const NString uniname, GLsizei count, const GLint *values );
    
    // Matrices.
    void SetUniform( const NString uniname, GLsizei count, GLboolean transpose, const GLfloat *values );
    
    // glGetUniformLocation not included.
    // Floats.
    void SetUniform( GLint loc, GLfloat value );
    void SetUniform( GLint loc, GLfloat v0, GLfloat v1 );
    void SetUniform( GLint loc, GLfloat v0, GLfloat v1, GLfloat v2 );
    void SetUniform( GLint loc, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 );
    void SetUniform( GLint loc, GLsizei count, const GLfloat *values );
    
    // Integers.
    void SetUniform( GLint loc, GLint value );
    void SetUniform( GLint loc, GLint v0, GLint v1 );
    void SetUniform( GLint loc, GLint v0, GLint v1, GLint v2 );
    void SetUniform( GLint loc, GLint v0, GLint v1, GLint v2, GLint v3 );
    void SetUniform( GLint loc, GLsizei count, const GLint *values );
    
    // Matrices.
    void SetUniform( GLint loc, GLsizei count, GLboolean transpose, const GLfloat *values );
    
    // Advanced.
    void SetUniform( const NString uniname, ncVec3 to );
    void SetUniform( const NString uniname, ncVec2 to );
    void SetUniform( const NString uniname, ncVec4 to );
    
    void SetUniform( GLint loc, ncVec3 to );
    void SetUniform( GLint loc, ncVec2 to );
    void SetUniform( GLint loc, ncVec4 to );
    
    
    void Next( void );

    GLint UniformLocation( const NString uniname );

    GLuint GetId( void );
    const NString GetName( void );
    
private:
    GLuint Fragment;
    GLuint Vertex;
    GLuint Geom;
    
    char Name[MAX_SHADERNAMELEN];
    
    GLuint Id;
};

class ncShaderManager {
public:
    void Initialize( void );
    GLuint Compile( const NString shadername, NString data, GLenum type );
    void CompileFromFile( const NString file, GLuint *vs, GLuint *fs, GLuint *gs );
    void Load( const NString file );
    void Delete( ncGLShader *shader );
    void Shutdown( void );
    
    int shaderCount;
    ncGLShader    *shaders;
};

extern ncShaderManager *g_shaderManager;

#endif
