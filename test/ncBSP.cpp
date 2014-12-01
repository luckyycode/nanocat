
//
//  Nanocat engine.
//
//  Static world loader & renderer..
//
//  Created by Neko Code on 6/15/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

/*
 
    Static world manager..

*/

#include "ncBSP.h"
#include "Core.h"
#include "AssetManager.h"
#include "Camera.h"
#include "NCString.h"
#include "MaterialLoader.h"
#include "Renderer.h"
#include "System.h"
#include "LevelEnvironment.h"

ncBSP local_bspMap;
ncBSP *g_staticWorld = &local_bspMap;

/*
    Initialize.
*/
void ncBSP::Initialize( void ) {
    if( Initialized ) {
        g_Core->Print( LOG_WARN, "Called ncBSP::Initialize, but it was already initialized.\n" );
        return;
    }
    
    // Set shader values.
    bspShader = f_AssetManager->FindShaderByName( "level" );
    
    // Setup shader values.
    bspShader->Use();
    
    g_bspUniforms[SHMVP_UNIFORM] = bspShader->UniformLocation( "MVP" );
    g_bspUniforms[SHMV_UNIFORM] = bspShader->UniformLocation( "ModelMatrix" );
    g_bspUniforms[SHCAMPOS_UNIFORM] = bspShader->UniformLocation( "cameraPos" );
    g_bspUniforms[SHDECALMAP_UNIFORM] = bspShader->UniformLocation( "decalMap" );
    g_bspUniforms[SHLIGHTMAP_UNIFORM] = bspShader->UniformLocation( "lightMap" );
    g_bspUniforms[SHDEPTHMAP_UNIFORM] = bspShader->UniformLocation( "depthMap");
    g_bspUniforms[SHNORMALMAP_UNIFORM] = bspShader->UniformLocation( "normalMap");
    
    // Level decal textures.
    bspShader->SetUniform( g_bspUniforms[SHDECALMAP_UNIFORM], 0 );
    // Level light map texture.
    bspShader->SetUniform( g_bspUniforms[SHLIGHTMAP_UNIFORM], 1 );
    // Depth screen texture.
    bspShader->SetUniform( g_bspUniforms[SHDEPTHMAP_UNIFORM], 2 );
    // Normal decal map texture.
    bspShader->SetUniform( g_bspUniforms[SHNORMALMAP_UNIFORM], 3 );
    
    bspShader->Next();
    
    Initialized = true;
}

/*
    Clean up.
*/
void ncBSP::Unload( void ) {

    if( meshIndices )
        delete [] meshIndices;
    if( leafFaces )
        delete [] leafFaces;
    if( f_bspType )
        delete[] f_bspType;
    if( m_patches )
        delete [] m_patches;
    if( m_leaves )
        delete [] m_leaves;
    if( m_planes )
        delete [] m_planes;
    
    if( m_EntityData )
        delete [] m_EntityData;
    
    if( m_vertices )
        delete [] m_vertices;
    if( m_polygonFaces )
        delete [] m_polygonFaces;
    if( m_meshFaces )
        delete [] m_meshFaces;
    if( m_nodes )
        delete [] m_nodes;
    if( m_pFaces )
        delete [] m_pFaces;
    
    // Remove textures.
    if( m_decalTextures )
        delete [] m_decalTextures;
    if( m_lightmapTextures )
        delete [] m_lightmapTextures;
    if( m_normalTextures )
        delete [] m_normalTextures;

    DZeroMemory( &m_visibilityData, sizeof(m_visibilityData) );
    visibleFaces.Delete();
    
    InUse = false;
}

/*
    Check file header and version.
*/
bool ncBSP::IsValid( ncBSPHeader *header ) {
    NC_ASSERTWARN( header );
    
    if( header->string[0] != 'I' && header->string[1] != 'B' && header->string[2] != 'S' && header->string[3] != 'P' ) {
        return false;
    }
    
    if( header->version != 0x2e ) {
        return false;
    }
    
    return true;
}


/*
    Load binary spaced file.
*/
bool ncBSP::Load( const NString filename ) {
    
    if( !Initialized ) {
        g_Core->Print( LOG_WARN, "ncBSP::Load - Loading map while static world is not initialized yet.\n" );
        return false;
    }
    
    InUse = false;
    
    FILE *g_mapFile;
    float t1, t2;
    
    t1 = c_coreSystem->Milliseconds();
    
    g_mapFile = fopen( NC_TEXT("%s/%s", Filesystem_Path.GetString(), filename ), "rb" );
    
    if( !g_mapFile ) {
        g_Core->Print( LOG_ERROR, "Couldn't load %s map file.\n", filename );
        return false;
    }
    
    // Read and check its header!
    DZeroMemory( &header, sizeof( header ) );
    
    // Clear memory, don't make it static.
    fread( &header, sizeof( ncBSPHeader ), 1, g_mapFile );
    
    // Check header and version.
    if(	!IsValid( &header ) ) {
        g_Core->Print( LOG_ERROR, "%s is corrupted or has wrong version.\n", filename );
        
        fclose( g_mapFile );
        return false;
    }
    
    mainFile = g_mapFile;

    SetupObjects();
    
    LoadEntityString();
    LoadBrushes();
    LoadVertices();
    LoadMeshes();
    LoadFaces();
    LoadTextures();
    LoadLightmaps();
    LoadLightVols();
    LoadData();
    
    fclose( mainFile );
    
    t2 = c_coreSystem->Milliseconds();
    
    g_Core->Print( LOG_DEVELOPER, "%s loaded. ( %4.2f msec )\n", filename, t2 - t1 );
    
    return true;
}

/*
    Vertex buffer and array data.
*/
bool ncBSP::SetupObjects( void ) {
    
    // Setup objects.
    glGenVertexArrays( BSP_VERTEXARRAY_COUNT, m_bspVertexArray );
    
    glGenBuffers( BSP_FACEVBO_COUNT, m_bspFaceVBOs );
    glGenBuffers( BSP_MESHVBO_COUNT, m_bspMeshVBOs );
    glGenBuffers( BSP_INDEXELEMENTS_COUNT, m_bspIndexElementsData );
    
    return true;
}

bool ncBSP::CleanupGraphics( void ) {
    glDeleteVertexArrays( BSP_VERTEXARRAY_COUNT, m_bspVertexArray );
    
    glDeleteBuffers( BSP_FACEVBO_COUNT, m_bspFaceVBOs );
    glDeleteBuffers( BSP_MESHVBO_COUNT, m_bspMeshVBOs );
    glDeleteBuffers( BSP_INDEXELEMENTS_COUNT, m_bspIndexElementsData );
    
    return true;
}

/*
    Load entity data now.
*/
void ncBSP::LoadEntityString( void ) {
    int size = header.dirEntry[SW_ENTITIES].length / sizeof(char);
    m_EntityData = new char[size];
    
    fseek( mainFile, header.dirEntry[SW_ENTITIES].offset, SEEK_SET );
    fread( m_EntityData, header.dirEntry[SW_ENTITIES].length, 1, mainFile );
    
    // TODO: Make a parser.
}

/*
    Load brush data.
*/
void ncBSP::LoadBrushes( void ) {
    // Read brushes.
    int m_iNumBrushes = header.dirEntry[SW_BRUSHES].length / sizeof(ncBSPBrush);
    m_pBrushes = new ncBSPBrush[m_iNumBrushes];
    
    fseek( mainFile, header.dirEntry[SW_BRUSHES].offset, SEEK_SET );
    fread( m_pBrushes, header.dirEntry[SW_BRUSHES].length, 1, mainFile );
    
    // Read brush sides.
    int m_iNumBrushSides =  header.dirEntry[SW_BRUSHSIDES].length / sizeof(ncBSPBrushSide);
    m_pBrushSides = new ncBSPBrushSide[m_iNumBrushSides];
    
    fseek( mainFile, header.dirEntry[SW_BRUSHSIDES].offset, SEEK_SET );
    fread( m_pBrushSides, header.dirEntry[SW_BRUSHSIDES].length, 1, mainFile );
    
    // Read leaf brushes.
    int m_iNumLeafBrushes = header.dirEntry[SW_LEAFBRUSHES].length / sizeof(int);
    m_pLeafBrushes = new int[m_iNumLeafBrushes];
    
    fseek( mainFile, header.dirEntry[SW_LEAFBRUSHES].offset, SEEK_SET );
    fread( m_pLeafBrushes, header.dirEntry[SW_LEAFBRUSHES].length, 1, mainFile );
}

/*
    Load mesh indices.
*/
void ncBSP::LoadMeshes( void ) {
    // Meshes.
    int numMeshIndices = header.dirEntry[SW_MESHINDICES].length / sizeof(int);
    
    meshIndices = new int[numMeshIndices];
    
    if( !meshIndices ) {
        g_Core->Error( ERR_FATAL, "Failed to allocated memory for %i mesh indices.\n", numMeshIndices );
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_MESHINDICES].offset, SEEK_SET );
    fread( meshIndices, header.dirEntry[SW_MESHINDICES].length, 1, mainFile );
}

/*
    Load vertices.
*/
void ncBSP::LoadVertices( void ) {
    
    vertexCount = header.dirEntry[SW_VERTICES].length / sizeof(ncBSPLoadVertex);
    
    g_Core->Print( LOG_DEVELOPER, "Loading vertice data.. ( %i vertices )\n", vertexCount );

    fseek( mainFile, header.dirEntry[SW_VERTICES].offset, SEEK_SET );
   
    m_vertices = new ncBSPVertex[vertexCount];
    
    if( !m_vertices ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i m_vertices.\n", vertexCount );
        Unload();
        return;
    }
    
    int i;
    
    for( i = 0 ; i < vertexCount; i++ ) {
        
        fread( &m_vertices[i], 1, sizeof(ncBSPVertex), mainFile );
        
        // Invert Z.
        float tempY = m_vertices[i].position.y;
        m_vertices[i].position.y = m_vertices[i].position.z;
        m_vertices[i].position.z = -tempY;
        
        // Scale.
        m_vertices[i].position.x /= MAX_BSP_SCALE;
        m_vertices[i].position.y /= MAX_BSP_SCALE;
        m_vertices[i].position.z /= MAX_BSP_SCALE;
        
        // Texture coordinates.
        m_vertices[i].decal.y = -m_vertices[i].decal.y;
        
        // Normals.
        float tempNormalY = m_vertices[i].normal.y;
        m_vertices[i].normal.y = m_vertices[i].normal.z;
        m_vertices[i].normal.z = -tempNormalY;
    }
}

/*
    Load faces.
*/
void ncBSP::LoadFaces( void ) {
    
    ncBSPLoadFace *faceData;
    
    facesCount = header.dirEntry[SW_FACES].length / sizeof(ncBSPLoadFace);
    
    g_Core->Print( LOG_DEVELOPER, "Loading mesh faces.. ( %i faces )\n", facesCount );
    
    faceData = new ncBSPLoadFace[facesCount];
    
    if( !faceData ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i load faces. \n", facesCount );
        Unload();
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_FACES].offset, SEEK_SET );
    fread( faceData, header.dirEntry[SW_FACES].length, 1, mainFile );
    
    f_bspType = new ncBSPDirectoryEntry[facesCount];
    
    if( !f_bspType ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i face entries!\n", facesCount );
        Unload();
        
        return;
    }
    
    // Visibility faces data.
    visibleFaces.Initialize( facesCount );
    
    int i;
    for( i = 0; i < facesCount; i++ ) {
        if( faceData[i].type == SW_POLYGON )
            polygonCount++;
        
        if( faceData[i].type == SW_PATCH )
            patchCount++;
        
        if( faceData[i].type == SW_MESH )
            meshCount++;
    }
    
    // All face data.
    m_pFaces = new ncBSPLoadFace[facesCount];
    if( !m_pFaces ) {
        g_Core->Error( ERR_FATAL, "Couldn't allocate %i world faces.\n", facesCount );
    }
    
    int currentFace = 0;
    
    for( int i = 0; i < facesCount; i++ ) {
        if( faceData[i].type != SW_POLYGON )
            continue;
        
        m_pFaces[i].texture = faceData[i].texture;
        m_pFaces[i].lightmapIndex = faceData[i].lightmapIndex;
        m_pFaces[i].firstVertexIndex = faceData[i].firstVertexIndex;
        m_pFaces[i].vertexCount = faceData[i].vertexCount;
        m_pFaces[i].numMeshIndices = faceData[i].numMeshIndices;
        m_pFaces[i].firstMeshIndex = faceData[i].firstMeshIndex;
        
        f_bspType[i].faceType = SW_FACE;
        f_bspType[i].typeFaceNumber = currentFace;
        
        currentFace++;
    }
    
    g_Core->Print( LOG_DEVELOPER, "We got %i polygons, %i patches, %i meshes\n", polygonCount, patchCount, meshCount );
    
    m_polygonFaces = new ncBSPLoadFace[polygonCount];
    
    if( !m_polygonFaces ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i polygon faces.\n", polygonCount );
        Unload();
        return;
    }
    
    currentFace = 0;
    for( i = 0; i < polygonCount; i++ ) {
        if( faceData[i].type != SW_POLYGON )
            continue;
        
        m_polygonFaces[currentFace].firstVertexIndex = faceData[i].firstVertexIndex;
        m_polygonFaces[currentFace].vertexCount = faceData[i].vertexCount;
        m_polygonFaces[currentFace].texture = faceData[i].texture;
        m_polygonFaces[currentFace].lightmapIndex = faceData[i].lightmapIndex;
        
        //f_bspType[i].faceType = SW_POLYGON;
        //f_bspType[i].typeFaceNumber = currentFace;
        
        currentFace++;
    }
    
    m_meshFaces = new ncBSPMeshFace[facesCount];
    
    if( !m_meshFaces ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i mesh faces.\n", meshCount );
        Unload();
        return;
    }
    
    int currentMeshFace = 0;
    
    for( i = 0; i < facesCount; ++i ) {
        if( faceData[i].type != SW_MESH )
            continue;
        
        m_meshFaces[currentMeshFace].firstVertexIndex = faceData[i].firstVertexIndex;
        m_meshFaces[currentMeshFace].vertexCount = faceData[i].vertexCount;
        m_meshFaces[currentMeshFace].textureIndex = faceData[i].texture;
        m_meshFaces[currentMeshFace].lightmapIndex = faceData[i].lightmapIndex;
        m_meshFaces[currentMeshFace].firstMeshIndex = faceData[i].firstMeshIndex;
        m_meshFaces[currentMeshFace].numMeshIndices = faceData[i].numMeshIndices;
        
        f_bspType[i].faceType = SW_MESH;
        f_bspType[i].typeFaceNumber = currentMeshFace;
        
        ++currentMeshFace;
    }
    
    m_patches = new ncBSPPatch[patchCount];
    
    if( !m_patches ) {
        g_Core->Print( LOG_ERROR, "Failed to allocate memory for %i patch faces.\n", patchCount );
        Unload();
        
        return;
    }

    int currentPatch = 0;
    g_Core->DPrint( "tesselation begin\n" );
    for( i = 0; i < patchCount; ++i ) {
        if(faceData[i].type != SW_PATCH)
            continue;
        
        m_patches[currentPatch].textureIdx = faceData[i].texture;
        m_patches[currentPatch].lightmapIdx = faceData[i].lightmapIndex;
        m_patches[currentPatch].width = faceData[i].patchSize[0];
        m_patches[currentPatch].height = faceData[i].patchSize[1];
        
        f_bspType[i].faceType = SW_PATCH;
        f_bspType[i].typeFaceNumber = currentPatch;
        
        ncBSPPatch *newPatch = new ncBSPPatch;
        
        newPatch->textureIdx  = faceData[i].texture;
        newPatch->lightmapIdx = faceData[i].lightmapIndex;
        newPatch->width  = faceData[i].patchSize[0];
        newPatch->height = faceData[i].patchSize[1];
        
        int numPatchesWidth  = ( newPatch->width - 1  ) >> 1;
        int numPatchesHeight = ( newPatch->height - 1 ) >> 1;
        
        newPatch->quadraticPatches = new ncBSPBiquadPatch[ numPatchesHeight * numPatchesWidth ];
  
        for(int y = 0; y < numPatchesHeight; ++y)
        {
            for(int x = 0; x < numPatchesWidth; ++x)
            {
                for(int row = 0; row < 3; ++row)
                {
                    for(int col = 0; col < 3; ++col)
                    {
                        int patchIdx = y * numPatchesWidth + x;
                        int cpIdx = row * 3 + col;
                        newPatch->quadraticPatches[ patchIdx ].controlPoints[ cpIdx ] =
                        
                        m_vertices[faceData[i].firstVertexIndex + (y * 2 * newPatch->width + x * 2) + row * newPatch->width + col];
                    }
                }
                
                newPatch->quadraticPatches[ y * numPatchesWidth + x ].Tesselate( 10 );
            }
        }
        
        
        ++currentPatch;
    }
    
    if( faceData )
        free( faceData );
    else
        g_Core->Error( ERR_FATAL, "ncBSP::LoadFaces - Unexpected error." );
    
    faceData = NULL;
}

/*
    Load textures.
*/
void ncBSP::LoadTextures( void ) {
    
    ncBSPLoadTexture *g_loadTextures;
    
    decalTextureCount = header.dirEntry[SW_TEXTURES].length / sizeof(ncBSPLoadTexture);
    
    g_Core->Print( LOG_DEVELOPER, "Loading material entries.. ( %i entries )\n", decalTextureCount );
    
    g_loadTextures = new ncBSPLoadTexture[decalTextureCount];
    
    if( !g_loadTextures ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i materials.\n", decalTextureCount );
        Unload();
        
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_TEXTURES].offset, SEEK_SET );
    fread( g_loadTextures, 1, header.dirEntry[SW_TEXTURES].length, mainFile );
    
    m_decalTextures = new GLuint[decalTextureCount];
    m_normalTextures = new GLuint[decalTextureCount];
    
    if( !m_decalTextures || !m_normalTextures ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i materials.\n", decalTextureCount );
        
        Unload();
        return;
    }
    
    // Temp!!!

    
    int i;
    for( i = 0; i < decalTextureCount; i++ ) {
        int rands = rand() % 2;
        ncMaterial *textureImage, *normalImage;
        switch (rands) {
            case 0:
                textureImage = g_materialManager->Find("floor");
                normalImage = g_materialManager->Find("floor_n");
                break;
            case 1:
                textureImage = g_materialManager->Find("wood_c");
                normalImage = g_materialManager->Find("wood_n");
                break;
            default:
                textureImage = g_materialManager->Find("brick");
                normalImage = g_materialManager->Find("brick_n");
                break;
        }
        
        m_decalTextures[i] = textureImage->Image.TextureID;
        m_normalTextures[i] = normalImage->Image.TextureID;
    }
    
    if( g_loadTextures )
        free( g_loadTextures );
    
    g_loadTextures = NULL;
}

/*
    Load light volumes.
*/
void ncBSP::LoadLightVols( void ) {
    ncBSPLightVol *g_loadLightVols;
    
    int lightVolCount = header.dirEntry[SW_LIGHTVOLS].length / sizeof( ncBSPLightVol );
    
    
    g_Core->Print( LOG_DEVELOPER, "Loading lightvols.. ( %i vols )\n", lightVolCount );
}

/*
    Load lightmaps.
*/
#define LIGHTMAP_SIZE 128
void ncBSP::LoadLightmaps( void ) {
    
    ncBSPLoadLightmap *g_loadLightmaps;
    
    lightmapCount = header.dirEntry[SW_LIGHTMAPS].length / sizeof(ncBSPLoadLightmap);
    
    g_Core->Print( LOG_DEVELOPER, "Loading lightmaps.. ( %i lightmaps ) \n", lightmapCount );
    
    g_loadLightmaps = new ncBSPLoadLightmap[lightmapCount];
    
    if( !g_loadLightmaps ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i load lightmaps.\n", lightmapCount );
        Unload();
        
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_LIGHTMAPS].offset, SEEK_SET );
    fread( g_loadLightmaps, 1, header.dirEntry[SW_LIGHTMAPS].length, mainFile );
    
    m_lightmapTextures = new uint[lightmapCount];
    
    if( !m_lightmapTextures ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i lightmaps.\n", lightmapCount );
        Unload();
        
        return;
    }
    
    float gamma = Render_LightmapGamma.GetFloat();
    int i, j;
    
    for( i = 0; i < lightmapCount; i++ ) {
        for( j = 0; j < 128 * 128; j++ ) {
            
            float r;
            float g;
            float b;
            
            r = g_loadLightmaps[i].lightmapData[j * 3 + 0];
            g = g_loadLightmaps[i].lightmapData[j * 3 + 1];
            b = g_loadLightmaps[i].lightmapData[j * 3 + 2];
            
            r *= gamma / 255.0f;
            g *= gamma / 255.0f;
            b *= gamma / 255.0f;
            
            float scale = 1.0f;
            float temp;
            
            if( r > 1.0f && (temp = (1.0f / r) ) < scale ) scale = temp;
            if( g > 1.0f && (temp = (1.0f / g) ) < scale ) scale = temp;
            if( b > 1.0f && (temp = (1.0f / b) ) < scale ) scale = temp;
            
            // Fix the colors.
            scale *= 255.0f;
            
            r *= scale;
            g *= scale;
            b *= scale;
            
            // Set the light array data now.
            g_loadLightmaps[i].lightmapData[j * 3 + 0] = (GLubyte)r;
            g_loadLightmaps[i].lightmapData[j * 3 + 1] = (GLubyte)g;
            g_loadLightmaps[i].lightmapData[j * 3 + 2] = (GLubyte)b;
        }
    }
    
    glGenTextures( lightmapCount, m_lightmapTextures );
    
    for( i = 0; i < lightmapCount; i++ ) {
        
        glBindTexture( GL_TEXTURE_2D, m_lightmapTextures[i] );
        
        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, g_loadLightmaps[i].lightmapData );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
        
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
        
        glGenerateMipmap( GL_TEXTURE_2D );
    }
    
    glGenTextures( 1, &m_defaultLightTexture );
    glBindTexture( GL_TEXTURE_2D, m_defaultLightTexture );
    
    float white_tex[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, 1.0, 1.0, 0, GL_RGB, GL_FLOAT, white_tex );
    
    glGenerateMipmap( GL_TEXTURE_2D );
    
    glBindTexture( GL_TEXTURE_2D, 0 );
    
    
    if( g_loadLightmaps )
        free( g_loadLightmaps );
    else
        g_Core->Error( ERR_FATAL, "ncBSP::LoadLightmaps - Unexpected error." );
    
    g_loadLightmaps = NULL;
}

/*
 
    Load leaf and plane data.
 
*/
void ncBSP::LoadData( void ) {
    
    leafCount = header.dirEntry[SW_LEAVES].length / sizeof(ncBSPLoadLeaf);
    
    g_Core->Print( LOG_DEVELOPER, "Loading map data..\n" );
    
    ncBSPLoadLeaf *m_leafData = new ncBSPLoadLeaf[leafCount];
    
    if( !m_leafData ) {
        
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i load leaves..\n", leafCount );
        Unload();
        
        return;
    }
    
    m_leaves = new ncBSPLeaf[leafCount];
    if( !m_leaves ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i leaves.\n", leafCount );
        Unload();
        
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_LEAVES].offset, SEEK_SET );
    fread( m_leafData, 1, header.dirEntry[SW_LEAVES].length, mainFile );
    
    int i, j;
    for( i = 0; i < leafCount; i++ ) {
        
        m_leaves[i].cluster = m_leafData[i].cluster;
        m_leaves[i].firstLeafFace = m_leafData[i].firstLeafFace;
        m_leaves[i].numFaces = m_leafData[i].numFaces;
        
        // Build a bounding box.
        m_leaves[i].boundingBoxVertices[0] = ncVec3( (float)m_leafData[i].mins[0], (float)m_leafData[i].mins[2],-(float)m_leafData[i].mins[1] );
        m_leaves[i].boundingBoxVertices[1] = ncVec3( (float)m_leafData[i].mins[0], (float)m_leafData[i].mins[2],-(float)m_leafData[i].maxs[1] );
        m_leaves[i].boundingBoxVertices[2] = ncVec3( (float)m_leafData[i].mins[0], (float)m_leafData[i].maxs[2],-(float)m_leafData[i].mins[1] );
        m_leaves[i].boundingBoxVertices[3] = ncVec3( (float)m_leafData[i].mins[0], (float)m_leafData[i].maxs[2],-(float)m_leafData[i].maxs[1] );
        m_leaves[i].boundingBoxVertices[4] = ncVec3( (float)m_leafData[i].maxs[0], (float)m_leafData[i].mins[2],-(float)m_leafData[i].mins[1] );
        m_leaves[i].boundingBoxVertices[5] = ncVec3( (float)m_leafData[i].maxs[0], (float)m_leafData[i].mins[2],-(float)m_leafData[i].maxs[1] );
        m_leaves[i].boundingBoxVertices[6] = ncVec3( (float)m_leafData[i].maxs[0], (float)m_leafData[i].maxs[2],-(float)m_leafData[i].mins[1] );
        m_leaves[i].boundingBoxVertices[7] = ncVec3( (float)m_leafData[i].maxs[0], (float)m_leafData[i].maxs[2],-(float)m_leafData[i].maxs[1] );
        
        for( j = 0; j < 8; j++ ) {
            m_leaves[i].boundingBoxVertices[j].x /= MAX_BSP_SCALE;
            m_leaves[i].boundingBoxVertices[j].y /= MAX_BSP_SCALE;
            m_leaves[i].boundingBoxVertices[j].z /= MAX_BSP_SCALE;
        }
        
    }
    
    int numLeafFaces = header.dirEntry[SW_LEAFFACES].length / sizeof(int);
    g_Core->Print( LOG_DEVELOPER, "Loading leaf faces.. ( %i entries )\n", numLeafFaces );
    
    leafFaces = new int[numLeafFaces];
    
    if( !leafFaces ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i leaf faces.\n", numLeafFaces );
        Unload();
        
        return;
    }
    
    
    fseek( mainFile, header.dirEntry[SW_LEAFFACES].offset, SEEK_SET );
    fread( leafFaces, 1, header.dirEntry[SW_LEAFFACES].length, mainFile );
    
    planeCount = header.dirEntry[SW_PLANES].length / sizeof(ncPlane);
    g_Core->Print( LOG_DEVELOPER, "Loading map planes.. ( %i entries )\n", planeCount );
    
    m_planes = new ncPlane[planeCount];
    
    if( !m_planes ){
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i planes.\n", planeCount );
        Unload();
        
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_PLANES].offset, SEEK_SET );
    fread( m_planes, 1, header.dirEntry[SW_PLANES].length, mainFile );
    
    // Reverse the intercept.
    for( i = 0; i < planeCount; i++ ) {
        // Negate Z.
        float temp = m_planes[i].normal.y;
        m_planes[i].normal.y = m_planes[i].normal.z;
        m_planes[i].normal.z = -temp;
        
        m_planes[i].intercept = -m_planes[i].intercept;
        m_planes[i].intercept /= MAX_BSP_SCALE;
    }
    
    nodeCount = header.dirEntry[SW_NODES].length / sizeof(ncBSPNode);
    
    m_nodes = new ncBSPNode[nodeCount];
    
    if( !m_nodes ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for %i m_nodes.\n", nodeCount );
        Unload();
        
        return;
    }
    
    fseek( mainFile, header.dirEntry[SW_NODES].offset, SEEK_SET );
    fread( m_nodes, 1, header.dirEntry[SW_NODES].length, mainFile );
    
    fseek( mainFile, header.dirEntry[SW_VISIBLEDATA].offset, SEEK_SET );
    fread( &m_visibilityData, 2, sizeof(int), mainFile );
    
    int bitsetSize = m_visibilityData.numClusters * m_visibilityData.bytesPerCluster;
    
    m_visibilityData.bitset = new Byte[bitsetSize];
    
    if( !m_visibilityData.bitset ) {
        g_Core->Error( ERR_FATAL, "Failed to allocate memory for visibility data.\n" );
        Unload();
        return;
    }
    
    fread( m_visibilityData.bitset, 1, bitsetSize, mainFile );
    
    delete [] m_leafData;
}


/* Visibility data calculation methods. */

/*
    Calculate camera leaf.
*/
int ncBSP::CalculateLeaf( ncVec3 cameraPosition ) {
    if( !m_planes )
        return 0;
    
    int currentNode = 0;
    
    while( currentNode >= 0 ) {
        // Find the closest plane.
        if( m_planes[m_nodes[currentNode].planeIndex].ClassifyPoint( cameraPosition ) == POINT_IN_FRONT_OF_PLANE )
            currentNode = m_nodes[currentNode].front;
        else
            currentNode = m_nodes[currentNode].back;
    }
    
    return ~currentNode;
}

/*
    Check if cluster is visible.
*/
int ncBSP::IsClusterVisible( int cameraCluster,
                            int testCluster ) {
    
    int index =	cameraCluster * m_visibilityData.bytesPerCluster + testCluster / 8;
    
    /* Note to myself. */
    /* If index will go less than zero application will explode. */
    /* Do not remove. */
    if( index < 0 )
        return 0;
    
    int returnValue = m_visibilityData.bitset[index] & (1 << (testCluster & 7));
    
    return returnValue;
}

/*
    Calculate visible faces.
*/
void ncBSP::CalculateVisibleData( ncVec3 cameraPosition ) {
    // Do not calculate visibility data while
    // there's no static world loaded.
    if( !Initialized )
        return;
    
    if ( !InUse )
        return;
    
    if( !Render_CalculateVisibleData.GetInteger() )
        return;
    
    // External.
    if( !m_leaves )
        return;
    
    // Remove previous visibility data.
    visibleFaces.ClearAll();
 
    // Calculate new visibility data.
    int cameraLeaf = CalculateLeaf( cameraPosition );
    int cameraCluster = m_leaves[cameraLeaf].cluster;
    
    int i, j;
    
    for( i = 0; i < leafCount; i++ ) {
        
        if( !IsClusterVisible( cameraCluster, m_leaves[i].cluster ) ) {
            continue;
        }
        
        if( !Frustum_IsBoxInside( m_leaves[i].boundingBoxVertices ) ) {
            continue;
        }
        
        for( j = 0; j < m_leaves[i].numFaces; j++ ) {
            visibleFaces.Set( leafFaces[m_leaves[i].firstLeafFace + j] );
        }
    }
}

/*
    Drawing functions.
*/
void ncBSP::Render( bool reflection, ncSceneEye oc ) {
    
    if( !Initialized )
        return;
    
    if( !InUse )
        return;
    
    glEnable( GL_CULL_FACE );
    glFrontFace( GL_CW );
    
    bspShader->Use();
    
    ncMatrix4 pos;

    pos.Identity();
    pos.Translate( bsp_position );
    
    ncVec3 reflectedScale = ncVec3( 1.0f, -1.0f, 1.0f );
    ncVec3 normalScale = ncVec3( 1.0, 1.0, 1.0 );
    
    if( reflection )
        pos.Scale( reflectedScale );
    else
        pos.Scale( normalScale );
    
#ifdef OCULUSVR_SUPPORTED
    
    float ipd = 0.64;
    
    ncVec3 offset = ncVec3( ipd / 2.0f, 0.0f, 0.0f );
    ncVec3 minus_offset = ncVec3( -(ipd / 2.0f), 0.0f, 0.0f );
    
    ncMatrix4 ls = ncMatrix4();
    ncMatrix4 rs = ncMatrix4();
    
    ls.Translate( offset );
    rs.Translate( minus_offset );
    
    if( oc == EYE_LEFT ) {
        pos = ls * pos;
    } else {
        pos = rs * pos;
    }
    
#endif
    
    ncMatrix4 mvp = g_playerCamera->ProjectionMatrix * g_playerCamera->ViewMatrix * pos;
    
    bspShader->SetUniform( g_bspUniforms[SHMV_UNIFORM], 1, false, pos.m );
    bspShader->SetUniform( g_bspUniforms[SHMVP_UNIFORM], 1, false, mvp.m );
    bspShader->SetUniform( g_bspUniforms[SHCAMPOS_UNIFORM], g_playerCamera->g_vEye );


    int i;
    for( i = 0; i < facesCount; ++i ) {
        //if( Render_CalculateVisibleData.GetInteger() ) {
            
            if( visibleFaces.IsSet( i ) ) {
                RenderFace( i, reflection, oc );
            }
            
        //} else {
            
        //    RenderFace( i, reflection, oc );
            
        //}
    }
    
    glUseProgram( 0 );
    
    glFrontFace( GL_CCW );
    glDisable( GL_CULL_FACE );
}

void ncBSP::RenderFace( int faceNumber, bool reflection, ncSceneEye oc ) {
    if( f_bspType[faceNumber].faceType == 0 )
        return;
    
    if( f_bspType[faceNumber].faceType == SW_MESH )
        RenderMesh( f_bspType[faceNumber].typeFaceNumber );
    
    //if( f_bspType[faceNumber].faceType == SW_POLYGON )
    //    RenderPolygon( f_bspType[faceNumber].typeFaceNumber );
    
    //if( f_bspType[faceNumber].faceType == SW_PATCH )
    //    RenderPatch( f_bspType[faceNumber].typeFaceNumber );

    if( f_bspType[faceNumber].faceType == SW_FACE )
        RenderFaces( faceNumber );
}


// Polygon
void ncBSP::RenderPolygon( int polygonFaceNumber ) {
    
    ncBSPLoadFace *pFace = &m_polygonFaces[polygonFaceNumber];
    
    // Static vertices.
    glBindVertexArray( m_bspVertexArray[0] );
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_decalTextures[pFace->texture]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_lightmapTextures[pFace->lightmapIndex]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[EYE_FULL]->DepthTexture );

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_normalTextures[pFace->texture]);
    
    // Draw now.
    glDrawArrays( GL_TRIANGLE_FAN, pFace->firstVertexIndex, pFace->vertexCount );
    
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    glBindVertexArray( 0 );
}

void ncBSP::RenderMesh( int meshFaceNumber ) {
    
    ncBSPMeshFace *pFace = &m_meshFaces[meshFaceNumber];
    
    // Bind vertex array.
    glBindVertexArray( m_bspVertexArray[1] );

    // Vertices.
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspMeshVBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].position), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    // Light map coordinates.
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspMeshVBOs[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].light), GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    // Decal map coordinates.
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspMeshVBOs[2] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].decal), GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    // Normals.
    glEnableVertexAttribArray( 3 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspMeshVBOs[3] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].normal), GL_STATIC_DRAW );
    glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bspIndexElementsData[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pFace->numMeshIndices * sizeof(uint), &meshIndices[pFace->firstMeshIndex], GL_STATIC_DRAW);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_decalTextures[pFace->textureIndex]);
    
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_lightmapTextures[pFace->lightmapIndex]);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[0]->DepthTexture );

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_normalTextures[pFace->textureIndex] );

    glDrawElements( GL_TRIANGLES, pFace->numMeshIndices, GL_UNSIGNED_INT, NULL );
    
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 3 );
    
    glBindVertexArray( 0 );
}

void ncBSP::RenderFaces( int faceNumber ) {
    if( !InUse )
        return;
    
    if( !faceNumber )
        return;
    
    ncBSPLoadFace *pFace = &m_pFaces[faceNumber];

    if( !pFace )
        return;
    
    glBindVertexArray( m_bspVertexArray[3] );
    
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].position), GL_STATIC_DRAW );
    
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].light), GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[2] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].decal), GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glEnableVertexAttribArray( 3 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[3] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * pFace->numMeshIndices, &(m_vertices[pFace->firstVertexIndex].normal), GL_STATIC_DRAW );
    glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bspIndexElementsData[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, pFace->numMeshIndices * sizeof(uint), &meshIndices[pFace->firstMeshIndex], GL_STATIC_DRAW);
    
    // Bind textures to shader.
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_decalTextures[pFace->texture] );
   
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, m_lightmapTextures[pFace->lightmapIndex] );
  
    glActiveTexture( GL_TEXTURE2);
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[EYE_FULL]->DepthTexture );
    
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, m_normalTextures[pFace->texture] );
    
    // Draw now.
    glDrawElements( GL_TRIANGLES, pFace->numMeshIndices, GL_UNSIGNED_INT, NULL );

    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 3 );
    
    glBindVertexArray( 0 );
}


/*
    
    Following functions are biquadric rendering patch related..
 
*/
 
void ncBSP::RenderPatch( int patchNumber )
{
    ncBSPPatch *pFace = &m_patches[patchNumber];
    
    if( !pFace )
        return;

    glBindVertexArray( m_bspVertexArray[2] );
    
    glEnableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[0] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &(m_vertices[0].position.x), GL_STATIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glEnableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[1] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &(m_vertices[0].light), GL_STATIC_DRAW );
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glEnableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[2] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &(m_vertices[0].decal), GL_STATIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    glEnableVertexAttribArray( 3 );
    glBindBuffer( GL_ARRAY_BUFFER, m_bspFaceVBOs[3] );
    glBufferData( GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &(m_vertices[0].normal), GL_STATIC_DRAW );
    glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), (void*)0 );
    
    // Bind textures to shader.
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, m_decalTextures[pFace->textureIdx] );
    
    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, m_lightmapTextures[pFace->lightmapIdx] );
    
    glActiveTexture( GL_TEXTURE2);
    glBindTexture( GL_TEXTURE_2D, g_mainRenderer->g_sceneBuffer[EYE_FULL]->DepthTexture );
    
    glActiveTexture( GL_TEXTURE3 );
    glBindTexture( GL_TEXTURE_2D, m_normalTextures[pFace->textureIdx] );
    
    // Draw now.
    int quadPatchesCount = sizeof(m_patches) / sizeof(m_patches[0]);

    for( int i = 0; i < quadPatchesCount; i++ ){
        for(int row=0; row < m_patches[patchNumber].quadraticPatches[i].m_tesselationLevel; ++row)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bspIndexElementsData[1]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * vertexCount, &m_patches[patchNumber].quadraticPatches[i].m_indices[ row * 2 * (m_patches[patchNumber].quadraticPatches[i].m_tesselationLevel + 1) ], GL_STATIC_DRAW);
            
            glDrawElements(GL_TRIANGLE_STRIP, 2 * (m_patches[patchNumber].quadraticPatches[i].m_tesselationLevel + 1), GL_UNSIGNED_INT,
                           NULL);
            glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
        }
    }
    
    glActiveTexture( GL_TEXTURE3 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE2 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE1 ); glBindTexture( GL_TEXTURE_2D, 0 );
    glActiveTexture( GL_TEXTURE0 ); glBindTexture( GL_TEXTURE_2D, 0 );
    
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 1 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 2 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glDisableVertexAttribArray( 3 );
    
    glBindVertexArray( 0 );
}

void ncBSP::ComputeTangentData( void ) {
 
    /*
    int verticeSize = sizeof( m_vertices ) / sizeof( m_vertices[0] );
    int polygonSize = sizeof( m_polygonFaces ) / sizeof( m_polygonFaces[0] );
    polygonSize = polygonSize / 3;
    
    ts = 0;
    bs = 0;
    //ncVec4 *tangents = (ncVec4*)malloc( sizeof(ncVec4) * verticeSize );
    
    for (long a = 0; a < polygonSize; a += 3)
    {
        long i1 = m_polygonFaces[a + 0].firstVertexIndex;
        long i2 = m_polygonFaces[a + 1].firstVertexIndex;
        long i3 = m_polygonFaces[a + 2].firstVertexIndex;
        
        ncVec3 v1 = m_vertices[i1].position;
        ncVec3 v2 = m_vertices[i2].position;
        ncVec3 v3 = m_vertices[i3].position;
        
        ncVec2 w1 = m_vertices[i1].decal;
        ncVec2 w2 = m_vertices[i2].decal;
        ncVec2 w3 = m_vertices[i3].decal;
        
        float x1 = v2.x - v1.x;
        float x2 = v3.x - v1.x;
        float y1 = v2.y - v1.y;
        float y2 = v3.y - v1.y;
        float z1 = v2.z - v1.z;
        float z2 = v3.z - v1.z;
        
        float s1 = w2.x - w1.x;
        float s2 = w3.x - w1.x;
        float t1 = w2.y - w1.y;
        float t2 = w3.y - w1.y;
        
        float r = 1.0f / (s1 * t2 - s2 * t1);
        
        ncVec3 sdir = ncVec3((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
        ncVec3 tdir = ncVec3((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
        
        tangentData[i1] = tangentData[i1] + sdir;
        tangentData[i2] = tangentData[i2] + sdir;
        tangentData[i3] = tangentData[i3] + sdir;
        
        bitangentData[i1] = bitangentData[i1] + tdir;
        bitangentData[i2] = bitangentData[i2] + tdir;
        bitangentData[i2] = bitangentData[i2] + tdir;
        
        ts++;
    }
    
    for (long a = 0; a < vertexCount; ++a)
    {
        ncVec3 n = m_vertices[a].normal;
        ncVec3 t = tangentData[a];
        
        //Vector3 tmp = (t - n * Vector3.Dot(n, t)).normalized;
        //tangents[a] = new Vector4(tmp.x, tmp.y, tmp.z);
        
        n.Normalize();
        t.Normalize();
        
        tangents[a].x = t.x;
        tangents[a].y = t.y;
        tangents[a].z = t.z;
        
        ncVec3 c = Cross( n, t );
        tangents[a].w = ncVec3_Dot( c, bitangentData[a] ) < 0.0f ? -1.0f : 1.0f;
    }
    */
}

void ncBSPBiquadPatch::Tesselate(int tessLevel)
{
    m_tesselationLevel = tessLevel;
    m_m_vertices = new ncBSPVertex[ (m_tesselationLevel+1) * (m_tesselationLevel+1) ];
    
    for(int i = 0; i <= m_tesselationLevel; ++i)
    {
        float a = (float)i / m_tesselationLevel;
        float b = 1.f - a;
        
        m_m_vertices[i]= controlPoints[0] * (b * b) +
        controlPoints[3] * ( 2 * b * a ) +
        controlPoints[6] * ( a * a );
    }
    
    for(int i = 1; i <= m_tesselationLevel; ++i)
    {
        float a = (float)i / m_tesselationLevel;
        float b = 1.f - a;
        
        ncBSPVertex temp[3];
        
        for(int j = 0, k = 0; j < 3; ++j, k = 3 * j)
        {
            temp[j] = controlPoints[k + 0] * ( b * b ) +
            controlPoints[k + 1] * ( 2 * b * a) +
            controlPoints[k + 2] * ( a * a );
        }
        
        for(int j = 0; j <= m_tesselationLevel; ++j)
        {
            float a = (float)j / m_tesselationLevel;
            float b = 1.f - a;
            
            m_m_vertices[ i * (m_tesselationLevel + 1) + j] = temp[0] * ( b * b ) +
            temp[1] * ( 2 * b * a ) +
            temp[2] * ( a * a );
        }
    }
    
    
    m_indices = new uint[m_tesselationLevel * (m_tesselationLevel+1) * 2];
    for(int row = 0; row < m_tesselationLevel; ++row)
    {
        for(int col = 0; col <= m_tesselationLevel; ++col)
        {
            m_indices[ ( row * (m_tesselationLevel + 1) + col ) * 2 + 1 ] =  row      * (m_tesselationLevel + 1) + col;
            m_indices[ ( row * (m_tesselationLevel + 1) + col ) * 2 ]     = (row + 1) * (m_tesselationLevel + 1) + col;
        }
    }
    
    m_trianglesPerRow  = new int[m_tesselationLevel];
    m_rowIndexPointers = new unsigned int *[m_tesselationLevel];
    
    for(int row = 0; row < m_tesselationLevel; ++row)
    {
        m_trianglesPerRow[row] = 2 * (m_tesselationLevel + 1);
        m_rowIndexPointers[row] = &m_indices[ row * 2 * (m_tesselationLevel + 1) ];
    }
}

void ncBSPBiquadPatch::Render( void )
{
    for(int row=0; row < m_tesselationLevel; ++row)
    {
        glDrawElements(GL_TRIANGLE_STRIP, 2 * (m_tesselationLevel + 1), GL_UNSIGNED_INT,
                       &m_indices[ row * 2 * (m_tesselationLevel + 1) ]);
    }
    
}

