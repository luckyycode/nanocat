//
//  Nanocat engine.
//
//  Beautiful water creator & renderer.
//
//  Created by Neko Vision on 2/8/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "assetmanager.h"
#include "shader.h"
#include "camera.h"
#include "material.h"
#include "renderer.h"
#include "world.h"

ncGameWater _gamewater;
ncGameWorld _gameworld;

// Water rendering.

#define GFX_WATER_SCALE     1024.0
#define MAX_WATER_SECTORS   64

ncGLShader    water_shader;

// AAAAAAAAAAW!!
// Global variables.
GLuint      normal_vbo;
GLuint      quad_vbo;
GLuint      texture_coord;

GLuint      waterVAO[1];

GLuint      waterVBO;
GLuint      waterNVBO;
GLuint      waterUVVBO;

ConsoleVariable    water_distance("water", "distance", "Distance to be rendered.", "1000", CVAR_NEEDSREFRESH);

const float normal_water_data[] = {
    0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, -1.0f,
    -1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 0.0f,
    0.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,
};

const float vertex_water_data[] = {
        -1.0 * GFX_WATER_SCALE, 1.0 * GFX_WATER_SCALE, 1.0 * GFX_WATER_SCALE,
        -1.0 * GFX_WATER_SCALE, 1.0 * GFX_WATER_SCALE, -1.0 * GFX_WATER_SCALE,
        1.0 * GFX_WATER_SCALE, 1.0 * GFX_WATER_SCALE, -1.0 * GFX_WATER_SCALE,
        1.0 * GFX_WATER_SCALE, 1.0 * GFX_WATER_SCALE, 1.0 * GFX_WATER_SCALE,
};

const float texture_water_data[] = {
        1.0, 0.0, 1.0,
        1.0, 0.0, 1.0,
        0.0, 0.0
};

/*
    Spawn water piece.
*/
void ncGameWater::Spawn( ncVec3 position, float size ) {
   
}

/*
    Initialize water place.
*/
void ncGameWater::Initialize( void ) {
    if( Initialized ) {
        _core.Print( LOG_WARN, "GFXWater already created, ignoring.\n" );
        return;
    }

    // Find the shader.
    _assetmanager.FindShader( "water", &water_shader );
    
    // Upload the values to the shader.
    Refresh();

    glGenVertexArrays( 1, &waterVAO[0] );
    glBindVertexArray( waterVAO[0] );

    // Initialize water vertice and texture coordinates.
    glGenBuffers( 1, &quad_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, quad_vbo );
    glEnableVertexAttribArray(0);
    glBufferData( GL_ARRAY_BUFFER, 10 * 3 * sizeof(float), vertex_water_data, GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );

    glGenBuffers( 1, &texture_coord );
    glBindBuffer( GL_ARRAY_BUFFER, texture_coord );
    glEnableVertexAttribArray(1);
    glBufferData( GL_ARRAY_BUFFER, 10 * 2 * sizeof(float), texture_water_data, GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glGenBuffers( 1, &normal_vbo );
    glBindBuffer( GL_ARRAY_BUFFER, normal_vbo );
    glEnableVertexAttribArray(2);
    glBufferData( GL_ARRAY_BUFFER, 10 * 2 * sizeof(float), normal_water_data, GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glBindVertexArray( 0 );

    Initialized = true;
}


/*
    Remove water ( called on application quit )
*/
void ncGameWater::Remove( void ) {
    glDeleteBuffers( 1, &quad_vbo );
    glDeleteBuffers( 1, &texture_coord );
}

/*
    Render our water.
*/
void ncGameWater::Render( eye_t eye ) {
    
    ncMatrix4 model, pos;
    ncVec3 position = ncVec3( 0.0f, (-GFX_WATER_SCALE * 1.0f) + 4.0f, 0.0f );
    ncVec3 scale = ncVec3( 1.0 );
    
    model.Identity();
    pos.Identity();
    
    pos.Translate( position );
    pos.Scale( scale );

    model = model * _camera.ViewMatrix;
    pos = model * pos;
    
    float ipd = 10.64;
    ncVec3 offset = ncVec3( ipd / 2.0f, 0.0f, 0.0f );
    ncVec3 minus_offset = ncVec3( -(ipd / 2.0f), 0.0f, 0.0f );
    
    ncMatrix4 ls = ncMatrix4();
    ncMatrix4 rs = ncMatrix4();
    ls.Translate( offset );
    rs.Translate( minus_offset );
    
    pos = EYE_LEFT ? ls * pos : rs * pos;
    
    glUseProgram(water_shader.shader_id);
    
    glBindVertexArray( waterVAO[0] );
    
    glUniform1f( glGetUniformLocation(water_shader.shader_id, "time"), ( _core.Time / 100.0 ) );
    
    glUniformMatrix4fv( glGetUniformLocation(water_shader.shader_id, "ModelMatrix"), 1, false, pos.m );
    glUniformMatrix4fv( glGetUniformLocation(water_shader.shader_id, "ProjMatrix"), 1, false, _camera.ProjectionMatrix.m );
    
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, _waterreflection.scene );
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, _waterrefraction.scene );
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, _materials.Find("env_water2_n").texture.tex_id);
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, _scene.depthtex );
    
    switch ( render_wireframe.GetInteger() ) {
        case 0:
            glDrawArrays( GL_TRIANGLE_FAN, 0, sizeof(vertex_water_data) / sizeof(float) / 3 );   break;
        case 1:
            glDrawArrays( GL_LINES, 0, sizeof(vertex_water_data) / sizeof(float) / 3 );   break;
        case 2:
            glDrawArrays( GL_POINTS, 0, sizeof(vertex_water_data) / sizeof(float) / 3 );  break;
        default:
            glDrawArrays( GL_QUADS, 0, sizeof(vertex_water_data) / sizeof(float) / 3 );   break;
    }
    
    glActiveTexture(GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindVertexArray( 0 );
    glUseProgram( 0 );
}

/*
    Update water graphic settings.
*/
void ncGameWater::Refresh( void ) {
    // Set up parameters
    glUseProgram( water_shader.shader_id );

    // Water samplers.
    glUniform1i(glGetUniformLocation(water_shader.shader_id, "reflection_texture"), 0);
    glUniform1i(glGetUniformLocation(water_shader.shader_id, "refraction_texture"), 1);
    glUniform1i(glGetUniformLocation(water_shader.shader_id, "normal_texture"),     2);
    glUniform1i(glGetUniformLocation(water_shader.shader_id, "depth_texture"),     3);

    glUseProgram(0);
}
