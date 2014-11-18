//
//  Nanocat engine.
//
//  Game font renderer..
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef corefont_h
#define corefont_h

#include "GameMath.h"
#include "ShaderLoader.h"

#define FONT_VERTICES 1024
#define FONT_CHARACTERSKIP 32

#define FONT_VBO_ID 0
#define FONT_UV_ID 1

#define FONT_VBO_COUNT 2

class ncCoreFontRenderer {
public:
    void Initialize( void );
    void Print2D( ncVec4 color, int x, int y, int size, const NString msg, ... );
    
    GLuint font_vbo[FONT_VBO_COUNT];
    GLuint font_vao;
    
    uint colorID;
    uint texID;
    uint texture;
    
    ncGLShader *shader;
};

extern ncCoreFontRenderer *g_coreFont;

extern ncConsoleVariable Font_Size;
extern ncConsoleVariable Font_Width;
extern ncConsoleVariable Font_Height;
extern ncConsoleVariable Font_Skip;

#endif
