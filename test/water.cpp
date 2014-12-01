//
//  Nanocat engine.
//
//  Beautiful water creator & renderer..
//
//  Created by Neko Vision on 2/8/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "AssetManager.h"
#include "ShaderLoader.h"
#include "Camera.h"
#include "MaterialLoader.h"
#include "Renderer.h"
#include "GameWorld.h"

ncGameWater local_gameWater;
ncGameWorld local_gameWorld;

ncGameWater *g_gameWater = &local_gameWater;
ncGameWorld *g_gameWorld = &local_gameWorld;

// Water rendering.

#define GFX_WATER_SCALE     512.0
#define MAX_WATER_SECTORS   64

ncGLShader    *water_shader;

// AAAAAAAAAAW!!
// Global variables.
GLuint      normal_vbo;
GLuint      quad_vbo;
GLuint      texture_coord;

GLuint      waterVAO[1];

GLuint      waterVBO;
GLuint      waterNVBO;
GLuint      waterUVVBO;

ncConsoleVariable    water_distance("water", "distance", "Water distance to render.", "1000", CVFLAG_NEEDSREFRESH);

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
        g_Core->Print( LOG_WARN, "GFXWater already initialized, ignoring.\n" );
        return;
    }

    // Find the shader.
    //f_AssetManager->FindShaderByName( "water", water_shader );
    water_shader = f_AssetManager->FindShaderByName( "water" );
    
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
void ncGameWater::Render( ncSceneEye eye ) {
    
    ncMatrix4 model, pos;
    ncVec3 position = ncVec3( 0.0f, (-GFX_WATER_SCALE * 1.0f) + 24.0f, 0.0f );
    ncVec3 scale = ncVec3( 1.0 );
    
    model.Identity();
    pos.Identity();
    
    pos.Translate( position );
    pos.Scale( scale );

    model = model * g_playerCamera->ViewMatrix;
    pos = model * pos;
  
#ifdef OCULUSVR_SUPPORTED
    float ipd = 10.64;
    ncVec3 offset = ncVec3( ipd / 2.0f, 0.0f, 0.0f );
    ncVec3 minus_offset = ncVec3( -(ipd / 2.0f), 0.0f, 0.0f );
    
    ncMatrix4 ls = ncMatrix4();
    ncMatrix4 rs = ncMatrix4();
    ls.Translate( offset );
    rs.Translate( minus_offset );
    
    if( eye == EYE_LEFT ) {
        pos = ls * pos;
    } else {
        pos = rs * pos;
    }
#endif
    
    ncMatrix4 projectionModelView = g_playerCamera->ProjectionMatrix * pos;
    
    water_shader->Use();
    
    glBindVertexArray( waterVAO[0] );
    
    water_shader->SetUniform( g_waterUniforms[WTIME_UNIFORM], (g_Core->Time / 100.0f) );
    water_shader->SetUniform( g_waterUniforms[WMODELMATRIX_UNIFORM], 1, false, pos.m );
    water_shader->SetUniform( g_waterUniforms[WPROJECTIONMATRIX_UNIFORM], 1, false, g_playerCamera->ProjectionMatrix.m );
    water_shader->SetUniform( g_waterUniforms[WMVP_UNIFORM], 1, false, projectionModelView.m );
    
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_waterReflectionBuffer->SceneTexture );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_waterRefractionBuffer->SceneTexture );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, g_materialManager->Find("env_water2_n")->Image.TextureID );
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[eye]->DepthTexture );

    // Draw now.
    glDrawArrays( GL_TRIANGLE_FAN, 0, sizeof(vertex_water_data) / sizeof(ncVec3) );

    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    glBindVertexArray( 0 );
    water_shader->Next();
}

/*
    Update water graphic settings.
*/
void ncGameWater::Refresh( void ) {
    
    // Set up parameters
    water_shader->Use();

    g_waterUniforms[WTIME_UNIFORM] = water_shader->UniformLocation( "time" );
    g_waterUniforms[WMODELMATRIX_UNIFORM] = water_shader->UniformLocation( "ModelMatrix" );
    g_waterUniforms[WPROJECTIONMATRIX_UNIFORM] = water_shader->UniformLocation( "ProjMatrix" );
    g_waterUniforms[WMVP_UNIFORM] = water_shader->UniformLocation( "MVP" );
    
    // Water samplers.
    water_shader->SetUniform( "reflection_texture", 0 );
    water_shader->SetUniform( "refraction_texture", 1 );
    water_shader->SetUniform( "normal_texture", 2 );
    water_shader->SetUniform( "depth_texture", 3 );
    
    water_shader->Next();
}
