//
//  BeautifulEnvironment.cpp
//  Nanocat
//
//  Created by Neko Code on 11/30/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "BeautifulEnvironment.h"
#include "AssetManager.h"
#include "MaterialLoader.h"
#include "Camera.h"
#include "ShaderLoader.h"
#include "Terrain.h"

ncConsoleVariable CGrass_Lod1( "grass", "lod1", "Grass near level of detail.", "25.0", CVFLAG_NEEDSREFRESH );
ncConsoleVariable CGrass_Lod2( "grass", "lod2", "Grass far level of detail.", "100.0", CVFLAG_NEEDSREFRESH );

ncBeautifulEnvironment local_beautifulEnvironment;
ncBeautifulEnvironment *bEnv = &local_beautifulEnvironment;

ncGLShader *mGrassShader;

GLuint m_VBO[3];
GLuint m_VAO;

ncCGrass::ncCGrass() {
    
}

/*
 *      Get random position in world.
 */
void ncCGrass::GetPositions() {
    mPos[0] = bEnv->mTerrain->GetRandomPos().x;
    mPos[1] = bEnv->mTerrain->GetRandomPos().z;
}

/*
 *      Get random rotations in world.
 */
void ncCGrass::GetRotations() {
    mRotation[0] = rand() % 180;
    mRotation[1] = rand() % 90;
}

/*
 *      Set grass type.
 */
void ncCGrass::SetType( ncCGrassTypes type ) {
    m_type = type;
}

/*
 *      Render grass piece.
 */
void ncCGrass::Render()
{
    // Compute grass cell distance to camera.
    float mDist[2];
    
    mDist[0] = g_playerCamera->g_vEye.x - mPos[0];
    if(mDist[0] < 0.0f) mDist[0] = -mDist[0];
    
    mDist[1] = g_playerCamera->g_vEye.z - mPos[1];
    if(mDist[1] < 0.0f) mDist[1] = -mDist[1];

    // Pass positions to shader.
    mGrassShader->SetUniform("mPosition", mPos[0], mPos[1]);
    
    // Now based upon distance in X and Z axes decide which LOD to use.
    if(mDist[0] < CGrass_Lod1.GetFloat() && mDist[1] < CGrass_Lod1.GetFloat() ) {
        /*
         *  Render high-detail mesh here.
         */
        RenderCell();
    }
    else if(mDist[0] < CGrass_Lod2.GetFloat() && mDist[1] < CGrass_Lod2.GetFloat() ) {
        /*
         *  Here we render a mid-detail mesh.
         */
        RenderCell();
    }
}

/*
 *      Render grass cell.
 */
void ncCGrass::RenderCell() {
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
}

/*
 *      Render our Beautiful Environment!
 */
void ncBeautifulEnvironment::Render( ncSceneEye eye ) {
    if( !Created )
        return;
    
    // In case if positions are not set or whatever.

    RenderTerrain(eye);
    
    
    // Enable Sample alpha to coverage (antialiased transparency! :P).
    glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    mGrassShader->Use();
    
    glActiveTexture(GL_TEXTURE0); glBindTexture( GL_TEXTURE_2D, g_materialManager->Find("tgrass")->Image.TextureID );
    glActiveTexture(GL_TEXTURE1); glBindTexture( GL_TEXTURE_2D, g_materialManager->Find("angry")->Image.TextureID );
    
    ncMatrix4 rotation;
    
    mGrassShader->SetUniform( "mProjection", 1, false, g_playerCamera->ProjectionMatrix.m );
    mGrassShader->SetUniform( "mView", 1, false, g_playerCamera->ViewMatrix.m );
    mGrassShader->SetUniform( "mGrassLod", CGrass_Lod1.GetFloat(), CGrass_Lod2.GetFloat() );
    mGrassShader->SetUniform( "mCameraPos", g_playerCamera->g_vEye.x, g_playerCamera->g_vEye.y, g_playerCamera->g_vEye.z, 1.0 );
    mGrassShader->SetUniform( "mTime", g_Core->Time / 5000.0f );
    
    glBindVertexArray( m_VAO );
    
    // Render all the cells.
    for(int i = 0; i < GRASS_CELLS_COUNT; i++){
        rotation.Identity();
        rotation.RotateY( mCell[i].mRotation[0] );
        // rotation.RotateX( mCell[i].mRotation[1] );
        
        mGrassShader->SetUniform( "mRotation", 1, false, rotation.m );
        mCell[i].Render();
    }
    
    glBindVertexArray( 0 );
    
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    mGrassShader->Next();
    
    // And disable Sample alpha to coverage.
    glDisable(GL_BLEND);
    glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
}

/*
 *      "Setup" our Beautiful Environment!
 */
void ncBeautifulEnvironment::Makeup() {

    // Setup terrain.
    if( !mTerrain ) {
        mTerrain = new ncLODTerrain();
        mTerrain->Load( TERRAIN_EDIT_LOAD, "angry", ncVec3( 0.0, 0.0, 0.0 ) );
        
        CanRenderTerrain = true;
    }
    
    // Setup shader.
    mGrassShader = f_AssetManager->FindShaderByName( "beautifulgrass" );
    
    mGrassShader->Use();
    mGrassShader->SetUniform( "mHeightMap", 1 );
    mGrassShader->SetUniform( "mDiffuseMap", 0 );
    mGrassShader->Next();
    
    // Vertex objects.
    glGenVertexArrays( 1, &m_VAO );
    glBindVertexArray( m_VAO );
    
    // Create simple quad with texture coordinates.
    float uvData[] = {  0.0f, 0.0f,
        1.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f };
    
    float vertexData[] = { -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f };
    
    Byte indicesData[] = {
        0, 1, 2,
        2, 3, 0
    };
    
    glGenBuffers( 3, m_VBO );
    
    // Vertex Data.
    glBindBuffer( GL_ARRAY_BUFFER, m_VBO[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW );
    glEnableVertexAttribArray(0);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    // Texture Coordinates.
    glBindBuffer( GL_ARRAY_BUFFER, m_VBO[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(uvData), uvData, GL_STATIC_DRAW );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    // Indices data.
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_VBO[2] );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesData) * sizeof(Byte), indicesData, GL_STATIC_DRAW );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    
    glBindVertexArray( 0 );

    // Add more beauty to our world!
    // Grass, bushes, trees, whatever!
    MakeFoliage();
    
    Created = true;
}

/*
 *      Setup grass cells position and rotation.
 */
void ncBeautifulEnvironment::MakeFoliage() {
    
    // Place grass in our Galaxy.
    for( int i = 0; i < GRASS_CELLS_COUNT; i++ ) {
        mCell[i] = ncCGrass();
        // Make random positions.
        mCell[i].GetPositions();
        mCell[i].GetRotations();
    }
 
    // Can render grass now.
    CanRenderGrass = true;
}

/*
 *      Render terrain.
 */
void ncBeautifulEnvironment::RenderTerrain( ncSceneEye eye ) {
    if( !CanRenderTerrain )
        return;
    
    if( !mTerrain )
        return;
    
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, mTerrain->m_texturemap_id[3] );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, mTerrain->m_texturemap_id[1] );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, mTerrain->m_texturemap_id[2] );
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, mTerrain->m_texturemap_id[0] );
    glActiveTexture( GL_TEXTURE4 ); glBindTexture( GL_TEXTURE_2D, mTerrain->m_heightmap_id );
    glActiveTexture( GL_TEXTURE5 ); glBindTexture( GL_TEXTURE_2D, mTerrain->m_detail_id );
    
    // Draw now.
    mTerrain->Render( eye );
    
    glActiveTexture( GL_TEXTURE5 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE4 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );

}