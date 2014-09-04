//
//  Nanocat engine.
//
//  Font loader and renderer. 
//
//  Created by Neko Vision on 02/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "shader.h"
#include "assetmanager.h"
#include "material.h"
#include "corefont.h"
#include "renderer.h"
#include "gl.h"
#include "gmath.h"

// Font manager.
ncCoreFontRenderer _font;

// Settings.
ConsoleVariable font_size( "font", "size", "Font size.", "0.0625", CVFLAG_NONE );
ConsoleVariable font_width( "font", "width", "Font character width.", "1.0", CVFLAG_NONE );
ConsoleVariable font_height( "font", "height", "Font character height.", "1.0", CVFLAG_NONE );
ConsoleVariable font_charskip( "font", "skip", "Font character skip size.", "32", CVFLAG_NONE );

/*
    Print text somewhere on screen.
*/
void ncCoreFontRenderer::Print( ncVec4 color, int x, int y, int size, const char *msg, ... ) {

    if( !_opengl.Initialized && !_renderer.Initialized )
        return;
    
    va_list     argptr;
    char        text[256];

    va_start( argptr, msg );
    vsnprintf( text, sizeof(text), msg, argptr );
    va_end( argptr );

    ulong length = strlen(text);

    // Awful.... yea.... I know.
    ncVec2 vertices[FONT_VERTICES];
    ncVec2 UVs[FONT_VERTICES];

    zeromem( vertices, sizeof(ncVec2) * FONT_VERTICES );
    zeromem( UVs, sizeof(ncVec2) * FONT_VERTICES );

    int i, c = 0;
	for ( i = 0; i < length; i++ ) {

        // Too much calculations in a loop!?!?!
        
        
        float fheight = font_height.GetFloat();
        float fwidth = font_width.GetFloat();

        ncVec2 vertex_up_left    = ncVec2( x + i * size - 5.0, y - size - fheight );
		ncVec2 vertex_up_right   = ncVec2( x + i * size + size + fwidth - 5.0, y - size - fheight );
		ncVec2 vertex_down_right = ncVec2( x + i * size + size + fwidth - 5.0, y );
		ncVec2 vertex_down_left  = ncVec2( x + i * size - 5.0, y );

        char character = text [i] - FONT_CHARACTERSKIP;

		float uv_x = ( character % 16 ) / 16.0f;
		float uv_y = ( character / 16 ) / 16.0f;

        float size = font_size.GetFloat();
        
		ncVec2 uv_up_left    = ncVec2( uv_x, 1 - uv_y - size );
		ncVec2 uv_up_right   = ncVec2( uv_x + size, 1 - uv_y - size );
		ncVec2 uv_down_right = ncVec2( uv_x + size, 1 - uv_y );
		ncVec2 uv_down_left  = ncVec2( uv_x, 1 - uv_y );

        // Build character map.
        vertices[c].x = vertex_up_left.x;
        vertices[c].y = vertex_up_left.y;

        UVs[c].x = uv_up_left.x;
        UVs[c].y = uv_up_left.y;

        c++;
        vertices[c].x = vertex_down_left.x;
        vertices[c].y = vertex_down_left.y;

        UVs[c].x = uv_down_left.x;
        UVs[c].y = uv_down_left.y;

        c++;
        vertices[c].x = vertex_up_right.x;
        vertices[c].y = vertex_up_right.y;

        UVs[c].x = uv_up_right.x;
        UVs[c].y = uv_up_right.y;

        c++;
        vertices[c].x = vertex_down_right.x;
        vertices[c].y = vertex_down_right.y;

        UVs[c].x = uv_down_right.x;
        UVs[c].y = uv_down_right.y;

        c++;
        vertices[c].x = vertex_up_right.x;
        vertices[c].y = vertex_up_right.y;

        UVs[c].x = uv_up_right.x;
        UVs[c].y = uv_up_right.y;

        c++;
        vertices[c].x = vertex_down_left.x;
        vertices[c].y = vertex_down_left.y;

        UVs[c].x = uv_down_left.x;
        UVs[c].y = uv_down_left.y;

		c++;
	}

	/*
        Real time generation.
    */
    glBindVertexArray( _font.VAO );

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _font.VBO );
	glBufferData(GL_ARRAY_BUFFER, sizeof(ncVec2) * FONT_VERTICES, &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, _font.UV );
	glBufferData(GL_ARRAY_BUFFER, sizeof(ncVec2) * FONT_VERTICES, &UVs[0], GL_STATIC_DRAW);
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray( 0 );

    /*
        Font rendering.
    */
    glBindVertexArray( _font.VAO );
    glEnable(GL_BLEND);
    glBlendFunc( GL_ONE, GL_SRC_ALPHA );

	glUseProgram( _font.shader.shader_id );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, _font.texture );

    // Apply color.
    glUniform3f( _font.colorID, color.x, color.y, color.z );

    // Bind vertex object and render font.    
	glDrawArrays( GL_TRIANGLES, 0, sizeof(ncVec2) * FONT_VERTICES );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );

	glUseProgram( 0 );

	glDisable( GL_BLEND );
	glBindVertexArray( 0 );
}

/*
    Load font system.
*/
void ncCoreFontRenderer::Initialize( void ) {

    glGenBuffers( 1, &_font.VBO );
	glGenBuffers( 1, &_font.UV );
	glGenVertexArrays( 1, &_font.VAO );

    _assetmanager.FindShader( "font", &_font.shader );

    _font.texture = _materials.Find( "default_font" ).texture.tex_id;

    glUseProgram( _font.shader.shader_id );

    // Font color.
    _font.colorID = glGetUniformLocation( _font.shader.shader_id, "colorModifier" );

    // Font map.
    _font.texID = glGetUniformLocation( _font.shader.shader_id, "font_texture" );

    glUniform1i( _font.texID, 0 );
    glUniform3f( _font.colorID, 1.0, 1.0, 1.0 );

    glUseProgram(0);
}

