//
//  Nanocat engine.
//
//  Game shader loader..
//
//  Created by Neko Vision on 06/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "ShaderLoader.h"
#include "Core.h"
#include "FileSystem.h"
#include "NCString.h"
#include "System.h"
#include "OpenGL.h"


/*
 
 
    Shader functions.
 
 
*/

// Use current shader.
void ncGLShader::Use() {
    glUseProgram( Id );
}

// Stop using current shader.
void ncGLShader::Next() {
    glUseProgram( 0 );
}

// Shader Id.
GLuint ncGLShader::GetId() {
    return Id;
}

// Shader name.
const NString ncGLShader::GetName() {
    return Name;
}

// Get uniform location.
GLint ncGLShader::UniformLocation( const NString uniname ) {
    return glGetUniformLocation( Id, uniname );
}

// Float methods.
void ncGLShader::SetUniform( const NString uniname,
                            GLfloat value ) {
    glUniform1f( glGetUniformLocation( Id, uniname ), value );
}

void ncGLShader::SetUniform( const NString uniname,
                            GLfloat v0, GLfloat v1 ) {
    glUniform2f( glGetUniformLocation( Id, uniname ), v0, v1 );
}

void ncGLShader::SetUniform( const NString uniname,
                            GLfloat v0, GLfloat v1, GLfloat v2 ) {
    glUniform3f( glGetUniformLocation( Id, uniname ), v0, v1, v2 );
}

void ncGLShader::SetUniform( const NString uniname,
                            GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) {
    glUniform4f( glGetUniformLocation( Id, uniname ), v0, v1, v2, v3 );
}

void ncGLShader::SetUniform( const NString uniname, GLsizei count, const GLfloat *values ) {
    glUniform1fv( glGetUniformLocation( Id, uniname ), count, values );
}

// Integer operations.
void ncGLShader::SetUniform( const NString uniname, GLint value ) {
    glUniform1i( glGetUniformLocation( Id, uniname), value );
}

void ncGLShader::SetUniform( const NString uniname, GLint v0, GLint v1 ) {
    glUniform2i( glGetUniformLocation( Id, uniname ), v0, v1 );
}

void ncGLShader::SetUniform( const NString uniname, GLint v0, GLint v1, GLint v2 ) {
    glUniform3i( glGetUniformLocation( Id, uniname ), v0, v1, v2 );
}

void ncGLShader::SetUniform( const NString uniname, GLint v0, GLint v1, GLint v2, GLint v3 ) {
    glUniform4i( glGetUniformLocation( Id, uniname ), v0, v1, v2, v3 );
}

void ncGLShader::SetUniform( const NString uniname, GLsizei count, const GLint *values ) {
    glUniform1iv( glGetUniformLocation( Id, uniname ), count, values );
}

// Matrix.
void ncGLShader::SetUniform( const NString uniname, GLsizei count, GLboolean transpose, const GLfloat *values ) {
    glUniformMatrix4fv( glGetUniformLocation( Id, uniname ), count, transpose, values );
}

// With glGetUniformLocation outside.

// Float methods.
void ncGLShader::SetUniform( GLint loc,
                            GLfloat value ) {
    glUniform1f( loc, value );
}

void ncGLShader::SetUniform( GLint loc,
                            GLfloat v0, GLfloat v1 ) {
    glUniform2f( loc, v0, v1 );
}

void ncGLShader::SetUniform( GLint loc,
                            GLfloat v0, GLfloat v1, GLfloat v2 ) {
    glUniform3f( loc, v0, v1, v2 );
}

void ncGLShader::SetUniform( GLint loc,
                            GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3 ) {
    glUniform4f( loc, v0, v1, v2, v3 );
}

void ncGLShader::SetUniform( GLint loc,
                            GLsizei count, const GLfloat *values ) {
    glUniform1fv( loc, count, values );
}

// Integer operations.
void ncGLShader::SetUniform( GLint loc, GLint value ) {
    glUniform1i( loc, value );
}

void ncGLShader::SetUniform( GLint loc,
                            GLint v0, GLint v1 ) {
    glUniform2i( loc, v0, v1 );
}

void ncGLShader::SetUniform( GLint loc,
                            GLint v0, GLint v1, GLint v2 ) {
    glUniform3i( loc, v0, v1, v2 );
}

void ncGLShader::SetUniform( GLint loc,
                            GLint v0, GLint v1, GLint v2, GLint v3 ) {
    glUniform4i( loc, v0, v1, v2, v3 );
}

void ncGLShader::SetUniform( GLint loc,
                            GLsizei count, const GLint *values ) {
    glUniform1iv( loc, count, values );
}

// Matrix.
void ncGLShader::SetUniform( GLint loc,
                            GLsizei count, GLboolean transpose, const GLfloat *values ) {
    glUniformMatrix4fv( loc, count, transpose, values );
}

// Advanced.
void ncGLShader::SetUniform( const NString uniname, ncVec2 to ) {
    glUniform2f( glGetUniformLocation( Id, uniname ), to.x, to.y );
}

void ncGLShader::SetUniform( const NString uniname, ncVec3 to ) {
    glUniform3f( glGetUniformLocation( Id, uniname ), to.x, to.y, to.z );
}

void ncGLShader::SetUniform( const NString uniname, ncVec4 to ) {
    glUniform4f( glGetUniformLocation( Id, uniname ), to.x, to.y, to.z, to.w );
}

void ncGLShader::SetUniform( GLint loc, ncVec2 to ) {
    glUniform2f( loc, to.x, to.y );
}

void ncGLShader::SetUniform( GLint loc, ncVec3 to ) {
    glUniform3f( loc, to.x, to.y, to.z );
}

void ncGLShader::SetUniform( GLint loc, ncVec4 to ) {
    glUniform4f( loc, to.x, to.y, to.z, to.w );
}

// Nice stuff.
ncShaderManager local_shaderManager;
ncShaderManager *g_shaderManager = &local_shaderManager;

/*
    Precache shader stuff.
*/
void ncShaderManager::Initialize( void ) {
    shaderCount = 0;
    shaders = new ncGLShader[MAX_SHADERS];
}

/* 
    Compile shader.
*/
GLuint ncShaderManager::Compile( const NString shadername, NString data, GLenum type ) {
    int          status, info_log;
    long         length;
    
    GLuint       shader;

    shader = glCreateShader( type );
    length = strlen( data );
    
    const NString versionId = _stringhelper.STR( "#version %s\n", GLSL_Version.GetString() );
    const NString sources[2] = { versionId, data };
    
    glShaderSource( shader, 2, (const char**)&sources, NULL );
    glCompileShader( shader );
    glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

    glGetProgramiv( shader, GL_INFO_LOG_LENGTH, &info_log );

    if( !status ) {

        if( info_log > 1 ) {
            NString error_log = new char[info_log + 1];
        
            glGetProgramInfoLog( shader, 1024, NULL, error_log );
        
            g_Core->Print( LOG_ERROR, "Could not compile %s shader. Here's log:\n", shadername );
            g_Core->Print( LOG_NONE, error_log );

            g_Core->Error( ERR_ASSET, "Could not compile shader %s.\n See console log for more information.\n", shadername );

            glDeleteShader( shader );
    
            delete [] error_log;
        
            return 0;
            
        } else {
            g_Core->Error( ERR_ASSET, "Could not compile shader %s.\n No log available.\n", shadername );
            
            glDeleteShader( shader );
            return 0;
            
        }
    }
    
    return shader;
}

/*
    Load shader from file.
*/
void ncShaderManager::CompileFromFile( const NString file, GLuint *vs, GLuint *fs, GLuint *gs ) {

    long    result;
    int     i;
    
    char    *p;
    char    *data;
    char    *shaders[3];
    
    result = c_FileSystem->Load( _stringhelper.STR("%s/%s/%s.nshdr", Filesystem_Path.GetString(), SHADER_FOLDER, file), (void**)&data );
    
    zeromem( shaders, sizeof(shaders) );
    
    i = 0;
    p = strtok (data, "\"");
    
    while ( p ) {
        shaders[i++] = p;
        p = strtok (NULL, "\"");
    }
    
    if( (!shaders[0] || !shaders[1] ) ) {
        g_Core->Error( ERR_ASSET, "Could not load %s/%s/%s.nshdr shader. Syntax error.", Filesystem_Path.GetString(), SHADER_FOLDER, file );
        return;
    }

    *vs = Compile( file, shaders[0], GL_VERTEX_SHADER );
    *fs = Compile( file, shaders[1], GL_FRAGMENT_SHADER );
    
    
    //if( shaders[2] )
    //    *gs = Compile( file, shaders[2], GL_GEOMETRY_SHADER );
    *gs = 0;
}

/*
    Simply load shader.
*/
void ncShaderManager::Load( const NString file ) {

    GLuint    program, vs, fs, gs;
    float   t1, t2;
    int     err, infolen;

    ncGLShader *s_temp;

    if( shaderCount > MAX_SHADERS ) {
        g_Core->Error( ERR_ASSET, "Could not load %s shader. %i shaders exceeded.\n", file, MAX_SHADERS );
        return;
    }

    t1 = c_coreSystem->Milliseconds();

    CompileFromFile( file, &vs, &fs, &gs );
    
    program = glCreateProgram();

    if( gs )
        glAttachShader(program, gs);
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &err);

    if( !err ) {
        infolen = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);

        if( infolen > 1 ) {

            NString log_string = new char[infolen + 1];
            glGetProgramInfoLog(program, infolen, NULL, log_string);

            g_Core->Print( LOG_ERROR, "Could not link %s shader. Here's log for you.\n", file );
            g_Core->Print( LOG_NONE, "%s\n", log_string );

            g_Core->Error( ERR_ASSET, "Could not link %s shader.\nLook console for shader log.\n", file );
        
            delete [] log_string;
            
            return;
        }
    }
    else {
        if( strlen(file) > MAX_SHADERNAME_LENGTH ) {
            g_Core->Error( ERR_ASSET, "LoadShader: Too long shader name.\n" );
            return;
        }
        
        s_temp = &shaders[shaderCount];

        //memset( s_temp, 0, sizeof(ncGLShader) );

        s_temp->Fragment = fs;
        s_temp->Vertex = vs;
        s_temp->Geom = gs;
        s_temp->Id = program;

        _stringhelper.Copy( s_temp->Name, file );

        t2 = c_coreSystem->Milliseconds();
        g_Core->Print( LOG_DEVELOPER, "LoadShader: %s loaded.. ( %4.2f ms )\n", file, t2-t1 );

        shaderCount++;
    }
}

/*
    Remove the shader.
*/
void ncShaderManager::Delete( ncGLShader *shader ) {
    glDetachShader( shader->Id, shader->Vertex );
    glDetachShader( shader->Id, shader->Fragment );

    glDeleteShader( shader->Vertex );
    glDeleteShader( shader->Fragment );

    glDeleteProgram( shader->Id );

    bzero( &shader, sizeof( shader ) );
}

/*
    Mostly called on application shutdown.
*/
void ncShaderManager::Shutdown( void ) {
    if( shaders ) delete [] shaders;
    
    shaderCount = 0;
}