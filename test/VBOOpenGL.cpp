//
//  VBOOpenGL.cpp
//  Nanocat
//
//  Created by Neko Code on 11/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "VBOOpenGL.h"


void ncGLVertexBufferObject::Create() {
    glGenBuffers( 1, &Id );
}

void ncGLVertexBufferObject::Delete() {
    glDeleteBuffers( 1, &Id );
}

void ncGLVertexBufferObject::Bind( void ) {
    glBindBuffer( Target, Id );
}

void ncGLVertexBufferObject::AddData( GLsizeiptr size, const GLvoid *data, GLenum usage ) {
    glBufferData( Target, size, data, usage );
}

void ncGLVertexBufferObject::Unbind() {
    glBindBuffer( Target, 0 );
}

void ncGLVertexBufferObject::SetTarget( GLenum target ) {
    Target = target;
}
