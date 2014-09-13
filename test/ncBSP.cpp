//
//  Nanocat engine.
//
//  Static world loader & renderer..
//
//  Created by Neko Code on 6/15/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "ncBSP.h"
#include "Core.h"
#include "AssetManager.h"
#include "Camera.h"
#include "NCString.h"
#include "MaterialLoader.h"
#include "Renderer.h"
#include "System.h"
#include "LevelEnvironment.h"

/*
 * We use binary space partition level type to make
 * static world. Also I am lazy to write custom map editor...
*/

const ncVec3 bsp_position = ncVec3( 0.0, -10.0, 0.0 );

ncBSP _bspmngr;

/*
    Initialize.
*/
void ncBSP::Initialize( void ) {
    if( Initialized ) {
        _core.Print( LOG_WARN, "Called ncBSP::Initialize, but it was already initialized.\n" );
        return;
    }

    // Set shader values.
    _assetmanager.FindShader( "level", &bspShader );

    glUseProgram( bspShader.shader_id );

    glUniform1i( glGetUniformLocation(bspShader.shader_id, "decalMap" ), 0 );        // Level decal textures.
    glUniform1i( glGetUniformLocation(bspShader.shader_id, "lightMap" ), 1 );        // Level light map.

    glUseProgram( 0 );
    
    Initialized = true;
}

/*
    Just remove the allocated stuff.
    TODO: Improve me.
*/
void ncBSP::Unload( void ) {
    _core.Print( LOG_DEVELOPER, "Unloading static world...\n" );
    
    InUse = false;

    leafCount       = 0;
    lightmapCount   = 0;
    meshCount       = 0;
    nodeCount       = 0;
    patchCount      = 0;
    planeCount      = 0;
    polygonCount    = 0;
    decalTextureCount = 0;
    facesCount      = 0;
    vertexCount     = 0;

    if( vertices )          free( vertices );
    if( patches )           free( patches );
    if( meshFaces )         free( meshFaces );
    if( polygonFaces )      free( polygonFaces );
    if( decalTextures )     free( decalTextures );
    if( lightmapTextures )  free( lightmapTextures );
    if( leaves )            free( leaves );
    if( leafFaces )         free( leafFaces );
    if( planes )            free( planes );
    if( nodes )             free( nodes );
}

/*
    Load binary spaced file.
*/
bool ncBSP::Load( const char *filename ) {

    if( !Initialized ) {
        _core.Print( LOG_WARN, "ncBSP::Load - Loading map while static world is not initialized yet.\n" );
        return false;
    }

    InUse = false;

	FILE    *g_file;
    
	float   t1, t2;
    bool    error = false;

    t1 = _system.Milliseconds();

	g_file = fopen( _stringhelper.STR("%s/%s", Filesystem_Path.GetString(), filename ), "rb" );

	if( !g_file ) {
        _core.Print( LOG_INFO, "Could not find %s/%s map file.\n", Filesystem_Path.GetString(), filename );
		return false;
	}

    zeromem( &header, sizeof( header ) );
	fread( &header, sizeof( bspheader_t ), 1, g_file );

    // iBSP
	if(	header.string[0] != 'I' && header.string[1] != 'B' && header.string[2] != 'S' && header.string[3] != 'P' ) {
        _core.Print( LOG_ERROR, "%s is corrupted.\n", filename );
        
        fclose( g_file );
		return false;
	}

    // 38
	if( header.version != 0x2e ) {
	    _core.Print( LOG_ERROR, "%s has wrong version.\n", filename );
        
        fclose( g_file );
        return false;
    }

    /*
        Load specific bsp data now.
    */

    // Load vertices.
    if( !LoadVertices(g_file) )
        error = true;
    
    // Load mesh data.
    if( !LoadMeshes(g_file) )
        error = true;

	// Load faces.
	if( !LoadFaces(g_file) )
		error = true;

	// Load textures.
	if( !LoadTextures(g_file) )
		error = true;

	// Load light maps.
	if( !LoadLightmaps(g_file) )
		error = true;

    // Load bsp data.
	if( !LoadData(g_file) )
		error = true;

    // * something failed *
    if( error ) {
        fclose( g_file );
        return false;
    }
    
	fclose(g_file);

    if( !SetupObjects() ) {
        return false;
    }
    
	t2 = _system.Milliseconds();

	_core.Print( LOG_DEVELOPER, "%s loaded. ( %4.2f msec )\n", filename, t2 - t1 );
    
    InUse = true;

	return true;
}


/*
    Vertex buffer and array data.
*/
bool ncBSP::SetupObjects( void ) {
    // Setup rendering data now.
    glGenVertexArrays(1, &vaoID[0]);
    glBindVertexArray(vaoID[0]);
    
    /* Vertex. */
    glGenBuffers(1, &verticeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticeVBO);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[0].position, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    /* Lightmap textures. */
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glEnableVertexAttribArray(1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[0].light.x, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);//&vertices[0].lightmapS);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    /* Decal textures. */
    glGenBuffers(1, &decalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, decalVBO);
    glEnableVertexAttribArray(2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[0].decal.x, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);//&vertices[0].decalS);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    /* Normals. */
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glEnableVertexAttribArray(3);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[0].normal, GL_STATIC_DRAW);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);//&vertices[0].decalS);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(0);
    
    glGenVertexArrays(1, &vaoID[1]);
    glBindVertexArray(vaoID[1]);
    
    glDeleteBuffers( 1, &verticeVBO );
    glDeleteBuffers( 1, &lightVBO );
    glDeleteBuffers( 1, &decalVBO );
    
    /* Vertex. */
    glGenBuffers(1, &verticeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, verticeVBO);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[meshFaces[0].firstVertexIndex].position, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    /* Lightmap textures. */
    glGenBuffers(1, &lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glEnableVertexAttribArray(1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[meshFaces[0].firstVertexIndex].light.x, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);//&vertices[0].lightmapS);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    /* Decal textures. */
    glGenBuffers(1, &decalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, decalVBO);
    glEnableVertexAttribArray(2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ncBSPVertex) * vertexCount, &vertices[meshFaces[0].firstVertexIndex].decal.x, GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), 0);//&vertices[0].decalS);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    glBindVertexArray(0);
    
    return true;
}

/*
    Load map data now.
*/

bool ncBSP::LoadMeshes( FILE *file ) {
    // Meshes.
    int numMeshIndices = header.dirEntry[SW_MESHINDICES].length / sizeof(int);

    meshIndices = (int*)malloc( sizeof(int) * numMeshIndices );
    
    if( !meshIndices ) {
        _core.Print( LOG_ERROR, ".. failed to allocated memory for %i mesh indices.\n", numMeshIndices );
        return false;
    }
    
    fseek( file, header.dirEntry[SW_MESHINDICES].offset, SEEK_SET );
    fread( meshIndices, header.dirEntry[SW_MESHINDICES].length, 1, file );
    
    return true;
}

bool ncBSP::LoadVertices( FILE *file ) {
    
    ncBSPLoadVertex *vertexData;
    
	vertexCount = header.dirEntry[SW_VERTICES].length / sizeof(ncBSPLoadVertex);
    
    _core.Print( LOG_DEVELOPER, "Loading vertex data.. ( %i vertices )\n", vertexCount );
    
    vertexData = (ncBSPLoadVertex*)malloc( sizeof(ncBSPLoadVertex) * vertexCount );
	if( !vertexData ) {
        _core.Print( LOG_DEVELOPER, ".. Failed to allocate memory for %i load vertices.\n", vertexCount );
        _bspmngr.Unload();
        
		return false;
	}

    
	fseek( file, header.dirEntry[SW_VERTICES].offset, SEEK_SET );
	fread( vertexData, header.dirEntry[SW_VERTICES].length, 1, file );

	vertices = (ncBSPVertex*)malloc( sizeof(ncBSPVertex) * vertexCount );

	if( !vertices ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i vertices.\n", vertexCount );
        _bspmngr.Unload();
		return false;
	}

    int i;

	for( i = 0 ; i < vertexCount; i++ ) {
        
		// Invert Z.
        vertices[i].position.x = vertexData[i].position.x;
        vertices[i].position.y = vertexData[i].position.z;
        vertices[i].position.z = -vertexData[i].position.y;

		// Scale.
        vertices[i].position.x /= MAX_BSP_SCALE;
        vertices[i].position.y /= MAX_BSP_SCALE;
        vertices[i].position.z /= MAX_BSP_SCALE;
        
		// Texture coordinates.
		vertices[i].decal.x = vertexData[i].decal.x;
		vertices[i].decal.y = -vertexData[i].decal.y;

		// Light map texture coordinates.
		vertices[i].light.x = vertexData[i].light.x;
		vertices[i].light.y = vertexData[i].light.y;
        
        // Normals.
        vertices[i].normal.x = vertexData[i].normal.x;
        vertices[i].normal.y = vertexData[i].normal.y;
        vertices[i].normal.z = vertexData[i].normal.z;
	}

    if( vertexData )
		free( vertexData );
    else
        _core.Error( ERC_FATAL, "ncBSP::LoadVertices: Unexpected error." );
    
	vertexData = NULL;

	return true;
}

bool ncBSP::LoadFaces( FILE *file ) {
    
    ncBSPLoadFace *faceData;
    
	facesCount = header.dirEntry[SW_FACES].length / sizeof(ncBSPLoadFace);
    
    _core.Print( LOG_DEVELOPER, "Loading mesh faces.. ( %i faces )\n", facesCount );

	faceData = (ncBSPLoadFace*)malloc( sizeof(ncBSPLoadFace) * facesCount );
	if( !faceData ) {
        _core.Print( LOG_DEVELOPER, ".. Failed to allocate memory for %i load faces. \n", facesCount );
        _bspmngr.Unload();
		return false;
	}

	fseek( file, header.dirEntry[SW_FACES].offset, SEEK_SET );
	fread( faceData, header.dirEntry[SW_FACES].length, 1, file );

	bspDirectory = (ncBSPDirectoryEntry*)malloc( sizeof(ncBSPDirectoryEntry) * facesCount );
    
	if( !bspDirectory ) {
        _core.Print( LOG_DEVELOPER, ".. Failed to allocate memory for %i face entries!\n", facesCount );
        _bspmngr.Unload();
        
		return false;
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

    _core.Print( LOG_DEVELOPER, "We got %i polygons, %i patches, %i meshes\n", polygonCount, patchCount, meshCount );

	polygonFaces = (ncBSPPolygonFace*)malloc( sizeof(ncBSPPolygonFace) * polygonCount );
    
	if( !polygonFaces ) {
        _core.Print( LOG_ERROR, ".. failed to allocate memory for %i polygon faces.\n", polygonCount );
        _bspmngr.Unload();
		return false;
	}

	int currentFace = 0;
	for( i = 0; i < facesCount; i++ ) {
		if( faceData[i].type != SW_POLYGON )
			continue;

		polygonFaces[currentFace].firstVertexIndex = faceData[i].firstVertexIndex;
		polygonFaces[currentFace].vertexCount = faceData[i].vertexCount;
		polygonFaces[currentFace].textureIndex = faceData[i].texture;
		polygonFaces[currentFace].lightmapIndex = faceData[i].lightmapIndex;

		bspDirectory[i].faceType = SW_POLYGON;
		bspDirectory[i].typeFaceNumber = currentFace;

		currentFace++;
	}

	meshFaces = (ncBSPMeshFace*)malloc( sizeof(ncBSPMeshFace) * meshCount );
	if( !meshFaces ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i mesh faces.\n", meshCount );
        _bspmngr.Unload();
		return false;
	}

	int currentMeshFace = 0;

	for( i = 0; i < facesCount; ++i ) {
		if( faceData[i].type != SW_MESH )
			continue;

		meshFaces[currentMeshFace].firstVertexIndex = faceData[i].firstVertexIndex;
		meshFaces[currentMeshFace].vertexCount = faceData[i].vertexCount;
		meshFaces[currentMeshFace].textureIndex = faceData[i].texture;
		meshFaces[currentMeshFace].lightmapIndex = faceData[i].lightmapIndex;
		meshFaces[currentMeshFace].firstMeshIndex = faceData[i].firstMeshIndex;
		meshFaces[currentMeshFace].numMeshIndices = faceData[i].numMeshIndices;

		bspDirectory[i].faceType = SW_MESH;
		bspDirectory[i].typeFaceNumber = currentMeshFace;

		++currentMeshFace;
	}

	patches = (ncBSPPatch*)malloc( sizeof(ncBSPPatch) * patchCount );
    
	if( !patches ) {
        _core.Print( LOG_ERROR, "..Failed to allocate memory for %i patch faces.\n", patchCount );
        Unload();
        
		return false;
	}

    
// Oh goosh this is bad.
#ifdef USE_TESSELATION
	int currentPatch = 0;
    
	for( i = 0; i < facesCount; ++i ) {
		if(faceData[i].type != SW_PATCH)
			continue;

		patches[currentPatch].textureIndex = faceData[i].texture;
		patches[currentPatch].lightmapIndex = faceData[i].lightmapIndex;
		patches[currentPatch].width = faceData[i].patchSize[0];
		patches[currentPatch].height = faceData[i].patchSize[1];
        
        _core.Print( LOG_NONE, "w: %i h: %i\n", (faceData[i].patchSize[0]-1)/2, (faceData[i].patchSize[1]-1)/2 );

		bspDirectory[i].faceType = SW_PATCH;
		bspDirectory[i].typeFaceNumber = currentPatch;
        
		int patchCountWide = (patches[currentPatch].width-1) / 2;
		int patchCountHigh = (patches[currentPatch].height-1) / 2;

        _core.Print( LOG_NONE, "%i MB", (patchCountWide*patchCountHigh) / MEGABYTE );
        
        patches[currentPatch].numQuadraticPatches = 102;
		patches[currentPatch].quadraticPatches = new ncBSPBiquadricPatch[ patches[currentPatch].numQuadraticPatches ];

		if( !patches[currentPatch].quadraticPatches ) {
            _core.Error( ERC_FATAL, "Could not allocate %i kbytes for quadratic patches.", patches[currentPatch].numQuadraticPatches / 1024 );
            //level_unload();
			return false;
		}

        
		// Quadratic patches increment.
        for(int y=0; y<patchCountHigh; ++y)
        {
            for(int x=0; x<patchCountWide; ++x)
            {
                for(int row=0; row<3; ++row)
                {
                    for(int point=0; point<3; ++point)
                    {
                        patches[currentPatch].quadraticPatches[y*patchCountWide+x].
                        controlPoints[row*3+point]=vertices[faceData[i].firstVertexIndex+
                                                            (y*2*patches[currentPatch].width+x*2)+
                                                            row*patches[currentPatch].width+point];
                    }
                }
                
                //tesselate the patch
                patches[currentPatch].quadraticPatches[y*patchCountWide+x].Tesselate(render_curvetesselation.GetInteger());
            }
            
            
        }
        
		++currentPatch;
        _core.Print( LOG_INFO, "Generated %i patch\n", currentPatch );
	}
    
#endif
    
    if( faceData )
		free( faceData );
    else
        _core.Error( ERC_FATAL, "ncBSP::LoadFaces - Unexpected error." );
    
	faceData = NULL;

	return true;
}

bool ncBSP::LoadTextures( FILE *file ) {
    
    ncBSPLoadTexture *g_loadTextures;
    
	decalTextureCount = header.dirEntry[SW_TEXTURES].length / sizeof(ncBSPLoadTexture);
    
    _core.Print( LOG_DEVELOPER, "Loading material entries.. ( %i entries )\n", decalTextureCount );

	g_loadTextures = (ncBSPLoadTexture*)malloc( sizeof(ncBSPLoadTexture) * decalTextureCount );
	if( !g_loadTextures ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i materials.\n", decalTextureCount );
        Unload();
        
		return false;
	}

	fseek( file, header.dirEntry[SW_TEXTURES].offset, SEEK_SET );
	fread( g_loadTextures, 1, header.dirEntry[SW_TEXTURES].length, file );

	decalTextures = (uint*)malloc( sizeof(uint) * decalTextureCount );
	if( !decalTextures ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i materials.\n", decalTextureCount );
        Unload();
		return false;
	}

    // Temp!!!
    ncMaterial textureImage = _materials.Find( "brick" );

    int i;
	for( i = 0; i < decalTextureCount; i++ ) {
        decalTextures[i] = textureImage.texture.tex_id;
	}

	if( g_loadTextures )
		free( g_loadTextures );

	g_loadTextures = NULL;

	return true;
}

#define LIGHTMAP_SIZE 128
bool ncBSP::LoadLightmaps( FILE * file ) {
    
    ncBSPLoadLightmap *g_loadLightmaps;
    
	lightmapCount = header.dirEntry[SW_LIGHTMAPS].length / sizeof(ncBSPLoadLightmap);
    
    _core.Print( LOG_DEVELOPER, "Loading lightmaps.. ( %i lightmaps ) \n", lightmapCount );
    
	g_loadLightmaps = (ncBSPLoadLightmap*)malloc( sizeof(ncBSPLoadLightmap) * lightmapCount );
	if( !g_loadLightmaps ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i load lightmaps.\n", lightmapCount );
        _bspmngr.Unload();
        
		return false;
	}
    
	fseek(file, header.dirEntry[SW_LIGHTMAPS].offset, SEEK_SET);
	fread(g_loadLightmaps, 1, header.dirEntry[SW_LIGHTMAPS].length, file);

	lightmapTextures = (uint*)malloc( sizeof(uint) * lightmapCount );
    
	if( !lightmapTextures ) {
        _core.Print( LOG_ERROR, ".. failed to allocate memory for %i lightmaps.\n", lightmapCount );
        _bspmngr.Unload();
        
		return false;
	}

	glGenTextures( lightmapCount, lightmapTextures );

	float gamma = render_lightmapgamma.GetFloat();
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

			if(r > 1.0f && (temp = (1.0f / r)) < scale) scale = temp;
			if(g > 1.0f && (temp = (1.0f / g)) < scale) scale = temp;
			if(b > 1.0f && (temp = (1.0f / b)) < scale) scale = temp;

            // Fix the colors.
			scale *= 255.0f;

			r *= scale;
			g *= scale;
			b *= scale;

			g_loadLightmaps[i].lightmapData[j * 3 + 0] = (Byte)r;
			g_loadLightmaps[i].lightmapData[j * 3 + 1] = (Byte)g;
			g_loadLightmaps[i].lightmapData[j * 3 + 2] = (Byte)b;
		}
	}
    
	for( i = 0; i < lightmapCount; i++ ) {
		glBindTexture( GL_TEXTURE_2D, lightmapTextures[i] );

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, LIGHTMAP_SIZE, LIGHTMAP_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, g_loadLightmaps[i].lightmapData );

        glGenerateMipmap( GL_TEXTURE_2D );
        
        glBindTexture( GL_TEXTURE_2D, 0 );
	}

	glGenTextures( 1, &whiteTexture );
	glBindTexture( GL_TEXTURE_2D, whiteTexture );

    float white_tex[] = { 1.0f, 1.0f, 1.0f, 1.0f };

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );

    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, 1.0, 1.0, 0, GL_RGB, GL_FLOAT, white_tex );

    glGenerateMipmap( GL_TEXTURE_2D );
    
    glBindTexture( GL_TEXTURE_2D, 0 );
    
	if( g_loadLightmaps )
		free( g_loadLightmaps );
    else
        _core.Error( ERC_FATAL, "ncBSP::LoadLightmaps - Unexpected error." );
    
	g_loadLightmaps = NULL;

	return true;
}

bool ncBSP::LoadData( FILE *file ) {
    ncBSPLoadLeaf *leafData;
    
	leafCount = header.dirEntry[SW_LEAVES].length / sizeof(ncBSPLoadLeaf);
    
    _core.Print( LOG_DEVELOPER, "Loading map data..\n" );
    
	leafData = (ncBSPLoadLeaf*)malloc( sizeof(ncBSPLoadLeaf) * leafCount );
	if( !leafData ) {
        
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i load leaves..\n", leafCount );
        Unload();
        
		return false;
	}

	leaves = (ncBSPLeaf*)malloc( sizeof(ncBSPLeaf) * leafCount );
	if(!leaves) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i leaves.\n", leafCount );
        Unload();
        
		return false;
	}

	fseek( file, header.dirEntry[SW_LEAVES].offset, SEEK_SET );
	fread( leafData, 1, header.dirEntry[SW_LEAVES].length, file );

	int i, j;
	for( i = 0; i < leafCount; i++ ) {
        
		leaves[i].cluster = leafData[i].cluster;
		leaves[i].firstLeafFace = leafData[i].firstLeafFace;
		leaves[i].numFaces = leafData[i].numFaces;

        // Build a bounding box.
        leaves[i].boundingBoxVertices[0] = ncVec3( (float)leafData[i].mins[0], (float)leafData[i].mins[2],-(float)leafData[i].mins[1] );
        leaves[i].boundingBoxVertices[1] = ncVec3( (float)leafData[i].mins[0], (float)leafData[i].mins[2],-(float)leafData[i].maxs[1] );
        leaves[i].boundingBoxVertices[2] = ncVec3( (float)leafData[i].mins[0], (float)leafData[i].maxs[2],-(float)leafData[i].mins[1] );
        leaves[i].boundingBoxVertices[3] = ncVec3( (float)leafData[i].mins[0], (float)leafData[i].maxs[2],-(float)leafData[i].maxs[1] );
        leaves[i].boundingBoxVertices[4] = ncVec3( (float)leafData[i].maxs[0], (float)leafData[i].mins[2],-(float)leafData[i].mins[1] );
        leaves[i].boundingBoxVertices[5] = ncVec3( (float)leafData[i].maxs[0], (float)leafData[i].mins[2],-(float)leafData[i].maxs[1] );
        leaves[i].boundingBoxVertices[6] = ncVec3( (float)leafData[i].maxs[0], (float)leafData[i].maxs[2],-(float)leafData[i].mins[1] );
        leaves[i].boundingBoxVertices[7] = ncVec3( (float)leafData[i].maxs[0], (float)leafData[i].maxs[2],-(float)leafData[i].maxs[1] );
        
        for( j = 0; j < 8; j++ ) {
            leaves[i].boundingBoxVertices[j].x /= MAX_BSP_SCALE;
            leaves[i].boundingBoxVertices[j].y /= MAX_BSP_SCALE;
            leaves[i].boundingBoxVertices[j].z /= MAX_BSP_SCALE;
        }
        
	}

	int numLeafFaces = header.dirEntry[SW_LEAFFACES].length / sizeof(int);
    _core.Print( LOG_DEVELOPER, "Loading leaf faces.. ( %i entries )\n", numLeafFaces );

	leafFaces = (int*)malloc( sizeof(int) * numLeafFaces );
    
	if( !leafFaces ) {
        _core.Print( LOG_DEVELOPER, ".. failed to allocate memory for %i leaf faces.\n", numLeafFaces );
        Unload();
        
		return false;
	}

    
	fseek( file, header.dirEntry[SW_LEAFFACES].offset, SEEK_SET );
	fread( leafFaces, 1, header.dirEntry[SW_LEAFFACES].length, file );

	planeCount = header.dirEntry[SW_PLANES].length / sizeof(ncPlane);
    _core.Print( LOG_DEVELOPER, "Loading map planes.. ( %i entries )\n", planeCount );

	planes = (ncPlane*)malloc( sizeof(ncPlane) * planeCount );
	if( !planes ){
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i planes.\n", planeCount );
        Unload();
        
		return false;
	}

	fseek( file, header.dirEntry[SW_PLANES].offset, SEEK_SET );
	fread( planes, 1, header.dirEntry[SW_PLANES].length, file );

	// Reverse the intercept.
    for( i = 0; i < planeCount; i++ ) {
		// Negate Z.
		float temp = planes[i].normal.y;
		planes[i].normal.y = planes[i].normal.z;
		planes[i].normal.z = -temp;

		planes[i].intercept = -planes[i].intercept;
		planes[i].intercept /= MAX_BSP_SCALE;
	}

	nodeCount = header.dirEntry[SW_NODES].length / sizeof(ncBSPNode);

	nodes = (ncBSPNode*)malloc( sizeof(ncBSPNode) * nodeCount );
    if( !nodes ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for %i nodes.\n", nodeCount );
        Unload();
        
		return false;
	}

	fseek( file, header.dirEntry[SW_NODES].offset, SEEK_SET );
	fread( nodes, 1, header.dirEntry[SW_NODES].length, file );

	fseek( file, header.dirEntry[SW_VISIBLEDATA].offset, SEEK_SET );
	fread( &visibilityData, 2, sizeof(int), file );

	int bitsetSize = visibilityData.numClusters * visibilityData.bytesPerCluster;

	visibilityData.bitset = (Byte*)malloc( sizeof(Byte) * bitsetSize );
    
	if( !visibilityData.bitset ) {
        _core.Print( LOG_ERROR, ".. Failed to allocate memory for visibility data.\n" );
        _bspmngr.Unload();
		return false;
	}

	fread( visibilityData.bitset, 1, bitsetSize, file );

	if( leafData )
		free( leafData );
    else
        _core.Error( ERC_FATAL, "ncBSP::LoadData - Unexpected error." );

	leafData = NULL;

	return true;
}

/*
    Calculate camera leaf.
*/
int ncBSP::CalculateLeaf( ncVec3 cameraPosition ) {
	int currentNode = 0;

	while( currentNode >= 0 ) {
        // Find the closest plane.
        if( planes[nodes[currentNode].planeIndex].ClassifyPoint( cameraPosition ) == POINT_IN_FRONT_OF_PLANE )
			currentNode = nodes[currentNode].front;
		else
			currentNode = nodes[currentNode].back;
	}

	return ~currentNode;
}

/*
    Check if cluster is visible.
*/
int ncBSP::IsClusterVisible( int cameraCluster,
                            int testCluster ) {

	int index =	cameraCluster * visibilityData.bytesPerCluster + testCluster / 8;

    /* Note to myself. */
	/* If index will go less than zero application will explode. */
	/* Do not remove. */
	if( index < 0 )
        return 0;

	int returnValue = visibilityData.bitset[index] & (1 << (testCluster & 7));

	return returnValue;
}

/*
    Calculate visible faces.
*/
void ncBSP::CalculateVisibleData( ncVec3 cameraPosition ) {
    // Do not calculate visibility data while
    // there's no static world loaded/initialized.
    if( !Initialized )
        return;
    
    if ( !InUse )
        return;

    // Remove previous visibility data.
    visibleFaces.ClearAll();

    // Calculate new visibility data.
	int cameraLeaf = CalculateLeaf(cameraPosition);
	int cameraCluster = leaves[cameraLeaf].cluster;

    int i, j;

	for( i = 0; i < leafCount; i++ ) {
        
        if( render_updatepvs.GetInteger() ) {
            if( !IsClusterVisible( cameraCluster, leaves[i].cluster ) )
                continue;

            if( !Frustum_IsBoxInside( leaves[i].boundingBoxVertices ) )
                continue;
        }
        
		for( j = 0; j < leaves[i].numFaces; j++ ) {
            visibleFaces.Set( leafFaces[leaves[i].firstLeafFace + j] );
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

    int i;
	for( i = 0; i < facesCount; ++i ) {
        if( visibleFaces.IsSet( i ) ) {
            _bspmngr.RenderFace( i, reflection, oc );
		}
	}

	glFrontFace( GL_CCW );
    glDisable( GL_CULL_FACE );
}

void ncBSP::RenderFace( int faceNumber, bool reflection, ncSceneEye oc ) {

	if( bspDirectory[faceNumber].faceType == 0 )
		return;

    ncMatrix4 model;
    ncMatrix4 pos;

    glUseProgram( bspShader.shader_id );

    model.Identity();
    pos.Identity();
    
    pos.Translate( bsp_position );
    
    ncVec3 reflectedScale = ncVec3( 1.0f, -1.0f, 1.0f );
    ncVec3 normalScale = ncVec3( 1.0, 1.0, 1.0 );
    
    if( reflection )
        pos.Scale( reflectedScale );
    else
        pos.Scale( normalScale );
    
    float ipd = 0.64;
    ncVec3 offset = ncVec3( ipd / 2.0f, 0.0f, 0.0f );
    ncVec3 minus_offset = ncVec3( -(ipd / 2.0f), 0.0f, 0.0f );
    
    ncMatrix4 ls = ncMatrix4();
    ncMatrix4 rs = ncMatrix4();
    ls.Translate( offset );
    rs.Translate( minus_offset );
    
    pos = oc == EYE_LEFT ? ls * pos : rs * pos;

    glUniformMatrix4fv( glGetUniformLocation( bspShader.shader_id, "ProjectionMatrix" ), 1, false, _camera.ProjectionMatrix.m );
    glUniformMatrix4fv( glGetUniformLocation( bspShader.shader_id, "ModelMatrix" ), 1, false, pos.m );
    glUniformMatrix4fv( glGetUniformLocation( bspShader.shader_id, "ViewMatrix" ), 1, false, _camera.ViewMatrix.m );
    
    glUniform3f( glGetUniformLocation( bspShader.shader_id, "cameraPos" ), _camera.g_vEye.x, _camera.g_vEye.y, _camera.g_vEye.z );
	if( bspDirectory[faceNumber].faceType == SW_POLYGON )
		_bspmngr.RenderPolygon( bspDirectory[faceNumber].typeFaceNumber );

	if( bspDirectory[faceNumber].faceType == SW_MESH )
		_bspmngr.RenderMesh( bspDirectory[faceNumber].typeFaceNumber );

#ifdef USE_TESSELATION
	if( bspDirectory[faceNumber].faceType == SW_PATCH )
        _bspmngr.RenderPatch( bspDirectory[faceNumber].typeFaceNumber );
#endif 
    
    glUseProgram( 0 );
}


// Polygon
void ncBSP::RenderPolygon( int polygonFaceNumber ) {
    glBindVertexArray( vaoID[0] );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, decalTextures[polygonFaces[polygonFaceNumber].lightmapIndex]);
    glUniform1i( glGetUniformLocation( bspShader.shader_id, "decalMap"), 0 );

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, lightmapTextures[polygonFaces[polygonFaceNumber].lightmapIndex]);
    glUniform1i( glGetUniformLocation( bspShader.shader_id, "lightMap"), 1 );

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, _scene.depthtex
                  );
    glUniform1i( glGetUniformLocation( bspShader.shader_id, "depthMap"), 2 );
    
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, _materials.Find("brick_n").texture.tex_id);
    glUniform1i( glGetUniformLocation( bspShader.shader_id, "normalMap"), 3 );
    
    /* Draw now. */
    switch( render_wireframe.GetInteger() ) {
        case 0:
            glDrawArrays(GL_TRIANGLE_FAN, polygonFaces[polygonFaceNumber].firstVertexIndex,
                          polygonFaces[polygonFaceNumber].vertexCount);
        break;
        case 1:
            glDrawArrays(GL_LINES, polygonFaces[polygonFaceNumber].firstVertexIndex,
                          polygonFaces[polygonFaceNumber].vertexCount);
        break;
        case 2:
            glDrawArrays(GL_POINTS, polygonFaces[polygonFaceNumber].firstVertexIndex,
                          polygonFaces[polygonFaceNumber].vertexCount);
        break;
        default:
            glDrawArrays(GL_TRIANGLE_FAN, polygonFaces[polygonFaceNumber].firstVertexIndex,
                          polygonFaces[polygonFaceNumber].vertexCount);
        break;
    }

    glBindVertexArray( 0 );
}

void ncBSP::RenderMesh( int meshFaceNumber ) {
    glBindVertexArray( vaoID[1] );
    
    glEnableVertexAttribArray(2);
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), &vertices[meshFaces[meshFaceNumber].firstVertexIndex].decal.x );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex),&vertices[meshFaces[meshFaceNumber].firstVertexIndex].light.x );

    glEnableVertexAttribArray(0);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(ncBSPVertex), &vertices[meshFaces[meshFaceNumber].firstVertexIndex].position );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, decalTextures[meshFaces[meshFaceNumber].textureIndex] );

    glActiveTexture( GL_TEXTURE1 );
    glBindTexture( GL_TEXTURE_2D, lightmapTextures[meshFaces[meshFaceNumber].lightmapIndex] );

    switch( render_wireframe.GetInteger() ) {
        case 0:
            glDrawElements(	GL_TRIANGLES, meshFaces[meshFaceNumber].numMeshIndices, GL_UNSIGNED_INT,
						&meshIndices[meshFaces[meshFaceNumber].firstMeshIndex]);
        break;
        case 1:
            glDrawElements(	GL_LINES, meshFaces[meshFaceNumber].numMeshIndices, GL_UNSIGNED_INT,
						&meshIndices[meshFaces[meshFaceNumber].firstMeshIndex]);
        break;
        case 2:
            glDrawElements(	GL_POINTS, meshFaces[meshFaceNumber].numMeshIndices, GL_UNSIGNED_INT,
						&meshIndices[meshFaces[meshFaceNumber].firstMeshIndex]);
        break;
        default:
            glDrawElements(	GL_TRIANGLES, meshFaces[meshFaceNumber].numMeshIndices, GL_UNSIGNED_INT,
						&meshIndices[meshFaces[meshFaceNumber].firstMeshIndex]);
        break;
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    
    glBindVertexArray( 0 );
}

#ifdef USE_TESSELATION
void ncBSP::RenderPatch( int patchNumber )
{
    for(int i=0; i<patches[patchNumber].numQuadraticPatches; ++i)
        patches[patchNumber].quadraticPatches[i].Draw();

}
#endif


#ifdef USE_TESSELATION
bool ncBSPBiquadricPatch::Tesselate(int newTesselation)
{
    tesselation=newTesselation;
    _core.Print( LOG_INFO, "Woop\n" );
    float px, py;
    ncBSPVertex temp[3];
    vertices=new ncBSPVertex[(tesselation+1)*(tesselation+1)];
    
    for(int v=0; v<=tesselation; ++v)
    {
        px=(float)v/tesselation;
        
        
        
        vertices[v]=
        controlPoints[0]*((1.0f-px)*(1.0f-px))+
        controlPoints[3]*((1.0f-px)*px*2)+
        controlPoints[6]*(px*px);
    }
    
    _core.Print( LOG_INFO, "Woop1\n" );
    
    for(int u=1; u<=tesselation; ++u)
    {
        py=(float)u/tesselation;
        
        temp[0]=controlPoints[0]*((1.0f-py)*(1.0f-py))+
        controlPoints[1]*((1.0f-py)*py*2)+
        controlPoints[2]*(py*py);
        
        temp[1]=controlPoints[3]*((1.0f-py)*(1.0f-py))+
        controlPoints[4]*((1.0f-py)*py*2)+
        controlPoints[5]*(py*py);
        
        temp[2]=controlPoints[6]*((1.0f-py)*(1.0f-py))+
        controlPoints[7]*((1.0f-py)*py*2)+
        controlPoints[8]*(py*py);
        
        for(int v=0; v<=tesselation; ++v)
        {
            px=(float)v/tesselation;
            
            vertices[u*(tesselation+1)+v]=	temp[0]*((1.0f-px)*(1.0f-px))+
            temp[1]*((1.0f-px)*px*2)+
            temp[2]*(px*px);
        }
    }
    
    //Create indices
    indices=new GLuint[tesselation*(tesselation+1)*2];
    if(!indices)
    {
        _core.Error( ERC_FATAL, "Unable to allocate memory for surface indices." );
        return false;
    }
    _core.Print( LOG_INFO, "Woop2\n" );
    for(int row=0; row<tesselation; ++row)
    {
        for(int point=0; point<=tesselation; ++point)
        {
            //calculate indices
            //reverse them to reverse winding
            indices[(row*(tesselation+1)+point)*2+1]=row*(tesselation+1)+point;
            indices[(row*(tesselation+1)+point)*2]=(row+1)*(tesselation+1)+point;
        }
    }
    _core.Print( LOG_INFO, "Woop3\n" );
    
    //Fill in the arrays for multi_draw_arrays
    trianglesPerRow=new int[tesselation];
    rowIndexPointers=new unsigned int *[tesselation];
    if(!trianglesPerRow || !rowIndexPointers)
    {
        _core.Error( ERC_FATAL, "Unable to allocate memory for indices for multi_draw_arrays");
        return false;
    }
    _core.Print( LOG_INFO, "Woop4\n" );
    for(int row=0; row<tesselation; ++row)
    {
        trianglesPerRow[row]=2*(tesselation+1);
        rowIndexPointers[row]=&indices[row*2*(tesselation+1)];
    }
    
    return true;
}

void ncBSPBiquadricPatch::Draw() {

    
    for(int row=0; row<tesselation; ++row)
    {
        glDrawElements(	GL_TRIANGLE_STRIP, 2*(tesselation+1), GL_UNSIGNED_INT,
                       &indices[row*2*(tesselation+1)]);
        // glMultiDrawElementsEXT(	GL_TRIANGLE_STRIP, trianglesPerRow,
            // GL_UNSIGNED_INT, (const void **)rowIndexPointers,
            // tesselation);
    }
}
#endif