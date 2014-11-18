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

#include <vector>
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

#pragma once

/********************************
 
 Class:		CVertexBufferObject
 
 Purpose:	Wraps OpenGL vertex buffer
 object.
 
 ********************************/

class CVertexBufferObject
{
public:
    void CreateVBO(int a_iSize = 0);
    void DeleteVBO();
    
    void* MapBufferToMemory(int iUsageHint);
    void* MapSubBufferToMemory(int iUsageHint, uint uiOffset, uint uiLength);
    void UnmapBuffer();
    
    void BindVBO(int a_iBufferType = GL_ARRAY_BUFFER);
    void UploadDataToGPU(int iUsageHint);
    
    void AddData(void* ptrData, uint uiDataSize);
    
    void* GetDataPointer();
    uint GetBufferID();
    
    int GetCurrentSize();
    
    CVertexBufferObject();
    
private:
    uint uiBuffer;
    int iSize;
    int iCurrentSize;
    int iBufferType;
    std::vector<Byte> data;
    
    bool bDataUploaded;
};

#endif
