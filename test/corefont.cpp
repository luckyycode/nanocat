
//
//  Nanocat engine.
//
//  Font loader and renderer..
//
//  Created by Neko Vision on 02/01/2014.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "SystemShared.h"
#include "AssetManager.h"
#include "MaterialLoader.h"
#include "CoreFont.h"
#include "Renderer.h"
#include "OpenGL.h"
#include "GameMath.h"

// Font manager.
ncCoreFontRenderer local_font;
ncCoreFontRenderer *g_coreFont = &local_font;

// Settings.
ncConsoleVariable Font_Size( "font", "size", "Font size.", "0.0625", CVFLAG_NONE );
ncConsoleVariable Font_Width( "font", "width", "Font character width.", "1.0", CVFLAG_NONE );
ncConsoleVariable Font_Height( "font", "height", "Font character height.", "1.0", CVFLAG_NONE );
ncConsoleVariable Font_Skip( "font", "skip", "Font character skip size.", "32", CVFLAG_NONE );


/*
    Print text somewhere on screen.
*/
void ncCoreFontRenderer::Print2D( ncVec4 color, int x, int y, int size, const NString msg, ... ) {

    if( !gl_Core->Initialized && !g_mainRenderer->Initialized )
        return;
    
    va_list     argptr;
    char        text[256];

    va_start( argptr, msg );
    vsnprintf( text, sizeof(text), msg, argptr );
    va_end( argptr );

    ulong length = strlen(text);

    // Awful.... yea.... I know.
    ncVec2 m_vertices[FONT_VERTICES];
    ncVec2 UVs[FONT_VERTICES];

    zeromem( m_vertices, sizeof(ncVec2) * FONT_VERTICES );
    zeromem( UVs, sizeof(ncVec2) * FONT_VERTICES );

    int i, c = 0;
    
    float fheight = Font_Height.GetFloat();
    float fwidth = Font_Width.GetFloat();
    
    for ( i = 0; i < length; i++ ) {

        ncVec2 vertex_up_left    = ncVec2( x + i * size - 5.0, y - size - fheight );
		ncVec2 vertex_up_right   = ncVec2( x + i * size + size + fwidth - 5.0, y - size - fheight );
		ncVec2 vertex_down_right = ncVec2( x + i * size + size + fwidth - 5.0, y );
		ncVec2 vertex_down_left  = ncVec2( x + i * size - 5.0, y );

        char character = text[i] - FONT_CHARACTERSKIP;

		float uv_x = ( character % 16 ) / 16.0f;
		float uv_y = ( character / 16 ) / 16.0f;

        float size = Font_Size.GetFloat();
        
		ncVec2 uv_up_left    = ncVec2( uv_x, 1 - uv_y - size );
		ncVec2 uv_up_right   = ncVec2( uv_x + size, 1 - uv_y - size );
		ncVec2 uv_down_right = ncVec2( uv_x + size, 1 - uv_y );
		ncVec2 uv_down_left  = ncVec2( uv_x, 1 - uv_y );

        // Build character map.
        m_vertices[c].x = vertex_up_left.x;
        m_vertices[c].y = vertex_up_left.y;

        UVs[c].x = uv_up_left.x;
        UVs[c].y = uv_up_left.y;

        c++;
        m_vertices[c].x = vertex_down_left.x;
        m_vertices[c].y = vertex_down_left.y;

        UVs[c].x = uv_down_left.x;
        UVs[c].y = uv_down_left.y;

        c++;
        m_vertices[c].x = vertex_up_right.x;
        m_vertices[c].y = vertex_up_right.y;

        UVs[c].x = uv_up_right.x;
        UVs[c].y = uv_up_right.y;

        c++;
        m_vertices[c].x = vertex_down_right.x;
        m_vertices[c].y = vertex_down_right.y;

        UVs[c].x = uv_down_right.x;
        UVs[c].y = uv_down_right.y;

        c++;
        m_vertices[c].x = vertex_up_right.x;
        m_vertices[c].y = vertex_up_right.y;

        UVs[c].x = uv_up_right.x;
        UVs[c].y = uv_up_right.y;

        c++;
        m_vertices[c].x = vertex_down_left.x;
        m_vertices[c].y = vertex_down_left.y;

        UVs[c].x = uv_down_left.x;
        UVs[c].y = uv_down_left.y;

		c++;
	}

	/*
        Real time generation.
    */

    glEnable( GL_BLEND );
    glBlendFunc( GL_ONE, GL_SRC_ALPHA );
    
    g_coreFont->shader->Use();
    
    glBindVertexArray( font_vao );
    
	glEnableVertexAttribArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, font_vbo[FONT_VBO_ID] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(ncVec2) * c, &m_vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof(ncVec2), 0 );
    
	glEnableVertexAttribArray( 1 );
	glBindBuffer( GL_ARRAY_BUFFER, font_vbo[FONT_UV_ID] );
	glBufferData( GL_ARRAY_BUFFER, sizeof(ncVec2) * c, &UVs[0], GL_STATIC_DRAW );
	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(ncVec2), 0 );
    
	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, g_coreFont->texture );

    // Apply color.
    glUniform3f( g_coreFont->colorID, color.x, color.y, color.z );

    // Bind vertex object and render font.
	glDrawArrays( GL_TRIANGLES, 0, c );
    
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, 0 );
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 1 );
    
    glBindVertexArray( 0 );

    glBindVertexArray( 0 );
    g_coreFont->shader->Next();

	glDisable( GL_BLEND );
}

/*
    Load font system.
*/
void ncCoreFontRenderer::Initialize( void ) {

    glGenBuffers( 2, font_vbo );
	glGenVertexArrays( 1, &font_vao );

    //f_AssetManager->FindShaderByName( "font", &g_coreFont->shader );

    shader = f_AssetManager->FindShaderByName( "font" );
    
    g_coreFont->texture = g_materialManager->Find( "default_font" )->Image.TextureID;

    glUseProgram( g_coreFont->shader->GetId() );

    // Font color.
    g_coreFont->colorID = shader->UniformLocation( "colorModifier" );
    
    // Font map.
    g_coreFont->texID = shader->UniformLocation( "font_texture" );

    g_coreFont->shader->SetUniform( g_coreFont->texID, 0 );
    g_coreFont->shader->SetUniform( g_coreFont->colorID, 1.0f, 1.0f, 1.0f );

    glUseProgram(0);
}

