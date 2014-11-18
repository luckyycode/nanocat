//
//  Nanocat engine.
//
//  Beautiful terrain creator & renderer..
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
#include "Terrain.h"

ncTerrainRenderer local_terrainRender;
ncTerrainRenderer *g_gameTerrain = &local_terrainRender;
using namespace std;
// Terrain rendering.

#define GFX_TERRAIN_SCALE     512.0

ncGLShader  *terrain_shader;

// AAAAAAAAAAW!!
// Global variables.


GLuint      g_TerrainVAO[1];

GLuint      g_TerrainVBO;
GLuint      g_TerrainNVBO;
GLuint      g_TerrainUVVBO;

GLuint      g_TerrainIndiceVBO;
#define FOR(q,n) for(int q=0;q<n;q++)
/*
    Generate terrain from heightmap.
*/
void ncHeightMapTerrain::Create( const NString imageName ) {
    ncMaterial *g_heightMap = g_materialManager->Find( imageName );
    
    if( !g_heightMap ) {
        g_Core->Print( LOG_ERROR, "Couldn't to create a terrain, missing material.\n" );
        return;
    }
    
    i_Cols = g_heightMap->Image.Width;
    i_Rows = g_heightMap->Image.Heigth;
    
    i_ColCount = i_Cols * i_Rows;
    
    v_VertexData = new ncVec3*[i_ColCount];
    v_CoordsData = new ncVec2*[i_ColCount];
    
    for(int i = 0; i < 512; i++)
        v_VertexData[i] = new ncVec3[512];
    
    for(int i = 0; i < 512; i++)
        v_CoordsData[i] = new ncVec2[512];
    
    float fTextureU = float( i_Cols ) * 0.1f;
    float fTextureV = float( i_Rows ) * 0.1f;
    
    Byte *bHeightMapData = g_heightMap->Image.ImageData;
    
    uint ptr_inc = g_heightMap->Image.BitsPerPixel == 24 ? 3 : 1;
    uint row_step = ptr_inc * i_Cols;
    

    CVertexBufferObject vboHeightmapData;
    
    vboHeightmapData.CreateVBO();
    // All vertex data are here (there are iRows*iCols vertices in this heightmap), we will get to normals later
    int iRows = i_Rows;
    int iCols = i_Cols;
    
    vector<vector<ncVec3>> vVertexData( iRows, vector<ncVec3>(iCols) );
    vector<vector<ncVec2>> vCoordsData( iRows, vector<ncVec2>(iCols) );
    
  
    FOR(i, iRows)
    {
        FOR(j, iCols)
        {
            float fScaleC = float(j)/float(iCols-1);
            float fScaleR = float(i)/float(iRows-1);
            float fVertexHeight = float(*(bHeightMapData+row_step*i+j*ptr_inc))/255.0f;
            vVertexData[i][j] = ncVec3(-0.5f+fScaleC, fVertexHeight, -0.5f+fScaleR);
            vCoordsData[i][j] = ncVec2(fTextureU*fScaleC, fTextureV*fScaleR);
        }
    }
    
    // Normals are here - the heightmap contains ( (iRows-1)*(iCols-1) quads, each one containing 2 triangles, therefore array of we have 3D array)
    vector< vector<ncVec3> > vNormals[2];
    FOR(i, 2)vNormals[i] = vector< vector<ncVec3> >(iRows-1, vector<ncVec3>(iCols-1));
    
    FOR(i, iRows-1)
    {
        FOR(j, iCols-1)
        {
            ncVec3 vTriangle0[] =
            {
                vVertexData[i][j],
                vVertexData[i+1][j],
                vVertexData[i+1][j+1]
            };
            ncVec3 vTriangle1[] =
            {
                vVertexData[i+1][j+1],
                vVertexData[i][j+1],
                vVertexData[i][j]
            };
            
            ncVec3 vTriangleNorm0;  vTriangleNorm0.Cross(vTriangle0[0]-vTriangle0[1], vTriangle0[1]-vTriangle0[2]);
            ncVec3 vTriangleNorm1;  vTriangleNorm1.Cross(vTriangle1[0]-vTriangle1[1], vTriangle1[1]-vTriangle1[2]);
            
            vTriangleNorm0.Normalize();
            vTriangleNorm1.Normalize();
            
            vNormals[0][i][j] = vTriangleNorm0;
            vNormals[1][i][j] = vTriangleNorm1;
        }
    }
    
    vector< vector<ncVec3> > vFinalNormals = vector< vector<ncVec3> >(iRows, vector<ncVec3>(iCols));
    
    FOR(i, iRows)
    FOR(j, iCols)
    {
        ncVec3 vFinalNormal = ncVec3(0.0f, 0.0f, 0.0f);
        
        // Look for upper-left triangles
        if(j != 0 && i != 0)
            FOR(k, 2)vFinalNormal = vFinalNormal + vNormals[k][i-1][j-1];
        // Look for upper-right triangles
        if(i != 0 && j != iCols-1)vFinalNormal = vFinalNormal + vNormals[0][i-1][j];
        // Look for bottom-right triangles
        if(i != iRows-1 && j != iCols-1)
            FOR(k, 2)vFinalNormal = vFinalNormal + vNormals[k][i][j];
        // Look for bottom-left triangles
        if(i != iRows-1 && j != 0)
            vFinalNormal = vFinalNormal + vNormals[1][i][j-1];
        vFinalNormal.Normalize();
        
        vFinalNormals[i][j] = vFinalNormal; // Store final normal of j-th vertex in i-th row
    }
    
    // First, create a VBO with only vertex data
    vboHeightmapData.CreateVBO(iRows*iCols*(2*sizeof(ncVec3)+sizeof(ncVec3))); // Preallocate memory
    FOR(i, iRows)
    {
        FOR(j, iCols)
        {
            vboHeightmapData.AddData(&vVertexData[i][j], sizeof(ncVec3)); // Add vertex
            vboHeightmapData.AddData(&vCoordsData[i][j], sizeof(ncVec2)); // Add tex. coord
            vboHeightmapData.AddData(&vFinalNormals[i][j], sizeof(ncVec3)); // Add normal
        }
    }
    // Now create a VBO with heightmap indices
    CVertexBufferObject vboHeightmapIndices;
    vboHeightmapIndices.CreateVBO();
    int iPrimitiveRestartIndex = iRows*iCols;
    FOR(i, iRows-1)
    {
        FOR(j, iCols)
        FOR(k, 2)
        {
            int iRow = i+(1-k);
            int iIndex = iRow*iCols+j;
            vboHeightmapIndices.AddData(&iIndex, sizeof(int));
        }
        // Restart triangle strips
        vboHeightmapIndices.AddData(&iPrimitiveRestartIndex, sizeof(int));
    }
    
    glGenVertexArrays(1, &g_TerrainVAO[0]);
    glBindVertexArray(g_TerrainVAO[0]);
    // Attach vertex data to this VAO
    vboHeightmapData.BindVBO();
    vboHeightmapData.UploadDataToGPU(GL_STATIC_DRAW);
    
    // Vertex positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(ncVec3) + sizeof(ncVec2), 0);
    // Texture coordinates
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(ncVec3) + sizeof(ncVec2), (void*)sizeof(ncVec3));
    // Normal vectors
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(ncVec3) + sizeof(ncVec2), (void*)(sizeof(ncVec3) + sizeof(ncVec2)));
    
    
    vboHeightmapIndices.BindVBO(GL_ELEMENT_ARRAY_BUFFER);
    vboHeightmapIndices.UploadDataToGPU(GL_STATIC_DRAW);
    
}



/*
    Initialize water place.
*/
void ncTerrainRenderer::Initialize( void ) {
    if( Initialized ) {
        g_Core->Print( LOG_WARN, "NCTerrain already initialized, ignoring.\n" );
        return;
    }
    
    // Find the shader.
    terrain_shader = f_AssetManager->FindShaderByName( "terrain" );
    
    // Upload the values to the shader.
    Refresh();
    
    terrain_shader->Use();

    for( int i = 0; i < 4; i++ )
        terrain_shader->SetUniform( NC_TEXT( "gSampler[%i]", i ), i );

    terrain_shader->Next();
    
    //ncHeightMapTerrain *ter = new ncHeightMapTerrain( "heightmap_dev" );
    
    Initialized = true;
}


/*
    Render our terrains.
*/
void ncTerrainRenderer::Render( ncSceneEye eye ) {
    
    ncMatrix4 model, pos;
  
    
    //model.Identity();
    pos.Identity();
    
    //pos.Translate( position );
    //pos.Scale( scale );
    
    //model = model * g_playerCamera->ViewMatrix;
  

    ncMatrix4 projectionModelView = g_playerCamera->ProjectionMatrix * g_playerCamera->ViewMatrix * pos;
    
    terrain_shader->Use();
    
    glBindVertexArray( g_TerrainVAO[0] );
    
    terrain_shader->SetUniform( "time", ( g_Core->Time / 100.0f ) );
    terrain_shader->SetUniform( "ModelMatrix", 1, false, pos.m );
    terrain_shader->SetUniform( "ProjMatrix", 1, false, g_playerCamera->ProjectionMatrix.m );
    terrain_shader->SetUniform( "ViewMatrix", 1, false, g_playerCamera->ViewMatrix.m );
    terrain_shader->SetUniform( "MVP", 1, false, projectionModelView.m );

    terrain_shader->SetUniform( "fRenderHeight", 4000.0f );
    terrain_shader->SetUniform( "fMaxTextureU", float(512) * 0.1f );
    terrain_shader->SetUniform( "fMaxTextureV", float(512) * 0.1f );
    
    terrain_shader->SetUniform( "cameraPos", g_playerCamera->g_vEye );
    
    
    ncMatrix4 heightScale;
    heightScale.Scale( ncVec3( 4000.0, 4000.0, 4000.0 ) );
    
    terrain_shader->SetUniform( "HeightmapScaleMatrix", 1, false, heightScale.m );

    
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D,  g_materialManager->Find("floor")->Image.TextureID );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D,  g_materialManager->Find("wood_c")->Image.TextureID );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, g_materialManager->Find("wallrock_c")->Image.TextureID );
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D,  g_materialManager->Find("heightmap_dev")->Image.TextureID );
    glActiveTexture( GL_TEXTURE4 ); glBindTexture( GL_TEXTURE_2D, g_materialManager->Find("heightmap_dev")->Image.TextureID );
    
    // Draw now.

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(128*128);
    
    int iNumIndices = (512-1)*512*2 + 512-1;
    glDrawElements( GL_LINES, iNumIndices, GL_UNSIGNED_INT, 0);
    

    glActiveTexture( GL_TEXTURE4 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    glBindVertexArray( 0 );
    glUseProgram( 0 );
}

/*
 Update water graphic settings.
 */
void ncTerrainRenderer::Refresh( void ) {

}
