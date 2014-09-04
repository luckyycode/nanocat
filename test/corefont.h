//
//  Nanocat engine.
//
//  Game font renderer.
//
//  Created by Neko Code on 8/29/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef corefont_h
#define corefont_h

#include "gmath.h"
#include "shader.h"

#define FONT_VERTICES 1024
#define FONT_CHARACTERSKIP 32

class ncCoreFontRenderer {
public:
    void Initialize( void );
    void Print( ncVec4 color, int x, int y, int size, const char *msg, ... );
    
    uint VBO;
    uint UV;
    uint VAO;
    uint colorID;
    uint texID;
    uint texture;
    
    ncGLShader shader;
};

extern ncCoreFontRenderer _font;

extern ConsoleVariable font_size;
extern ConsoleVariable font_width;
extern ConsoleVariable font_height;
extern ConsoleVariable font_charskip;

#endif
