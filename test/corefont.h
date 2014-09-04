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

#define FONT_SIZE 0.0625f
#define FONT_WIDTH 1.0
#define FONT_HEIGHT 1.0
#define FONT_VERTICES 1024
#define FONT_CHARACTER_SET_SKIP 32

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

#endif
