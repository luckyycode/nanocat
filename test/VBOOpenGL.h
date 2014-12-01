//
//  VBOOpenGL.h
//  Nanocat
//
//  Created by Neko Code on 11/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef _VBOOpenGL__
#define _VBOOpenGL__

#include "SystemShared.h"

class ncGLVertexBufferObject {
public:
    GLuint Id;

    GLenum Target = GL_ARRAY_BUFFER;
    
    void Create( void );
    void Delete( void );
    
    void SetTarget( GLenum target );
    void Bind( void );
    void Unbind( void );
    
    void AddData( GLsizeiptr size, const GLvoid *data, GLenum usage = GL_STATIC_DRAW );
    
};

#endif