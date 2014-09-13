//
//  Nanocat engine.
//
//  Static world loader & renderer..
//
//  Created by Neko Code on 6/15/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef bspmngr_included
#define bspmngr_included

#include "ShaderLoader.h"
#include "ncBitMessage.h"
#include "FileSystem.h"
#include "GameMath.h"
#include "Renderer.h"

#define MAX_BSP_SCALE 2.0
#define DIR_ENTRIES 17
//#define USE_TESSELATION

/* 
    Don't change its order or it will blow up. 
*/
enum ncBSPDirectoryType {
	SW_ENTITIES = 0,
	SW_TEXTURES,
	SW_PLANES,
	SW_NODES,
	SW_LEAVES,
	SW_LEAFFACES,
	SW_LEAFBRUSHES,
	SW_MODELS,
	SW_BRUSHES,
	SW_BRUSHSIDES,
	SW_VERTICES,
	SW_MESHINDICES,
	SW_FXS,
	SW_FACES,
	SW_LIGHTMAPS,
	SW_LIGHTVOLS,
	SW_VISIBLEDATA
};

struct ncBSPLoadVertex {
    ncVec3 position;
    ncVec2 decal;
    ncVec2 light;
    
    ncVec3 normal;
    
    GLubyte color[4];
};

class ncBSPVertex {
public:
    ncVec3 position;
    ncVec3 normal;
    
    ncVec2 decal;
    ncVec2 light;
    
    ncBSPVertex operator+(const ncBSPVertex & rhs) const
    {
        ncBSPVertex result;
        result.position=position+rhs.position;
        result.decal.x=decal.x+rhs.decal.x;
        result.decal.y=decal.y+rhs.decal.y;
        result.light.x=light.x+rhs.light.x;
        result.light.y=light.y+rhs.light.y;
        
        return result;
    }
    
    ncBSPVertex operator*( const float rhs ) const {
        ncBSPVertex result;
        result.position=position*rhs;
        result.decal.x=decal.x*rhs;
        result.decal.y=decal.y*rhs;
        result.light.x=light.x*rhs;
        result.light.y=light.y*rhs;
        
        return result;
    }
};

enum ncBSPFaceType {
    SW_POLYGON = 1,
    SW_PATCH,
    SW_MESH,
    SW_BILLBOARD
};

struct ncBSPDirectoryEntry {
    ncBSPFaceType faceType;
    int typeFaceNumber;
};

struct ncBSPLoadFace {
    int texture;
    int effect;
    int type;
    int firstVertexIndex;
    int vertexCount;
    int firstMeshIndex;
    int numMeshIndices;
    int lightmapIndex;
    int lightmapStart[2];
    int lightmapSize[2];
    
    ncVec3 lightmapOrigin;
    ncVec3 sTangent, tTangent;
    ncVec3 normal;
    
    int patchSize[2];
};

struct ncBSPPolygonFace {
    int firstVertexIndex;
    int vertexCount;
    int textureIndex;
    int lightmapIndex;
};

struct ncBSPMeshFace {
    int firstVertexIndex;
    int vertexCount;
    int textureIndex;
    int lightmapIndex;
    int firstMeshIndex;
    int numMeshIndices;
};

class ncBSPBiquadricPatch
{
public:
    bool Tesselate(int newTesselation);
    void Draw();
    
    ncBSPVertex controlPoints[9];
    
    int tesselation;
    ncBSPVertex *vertices;
    GLuint *indices;

    int *trianglesPerRow;
    unsigned int **rowIndexPointers;
    
    ncBSPBiquadricPatch() : vertices(NULL)
    {
        
    }
    
    ~ncBSPBiquadricPatch()
    {
        if( vertices )
            delete [] vertices;
        vertices = NULL;
        
        if( indices )
            delete [] indices;
        indices = NULL;
    }
};

struct ncBSPPatch {
    int textureIndex;
    int lightmapIndex;
    int width, height;
    
    int numQuadraticPatches;
    ncBSPBiquadricPatch *quadraticPatches;
};

struct ncBSPLoadTexture {
    char name[64];
    int flags, contents;
};

struct ncBSPLoadLightmap {
    GLubyte lightmapData[128 * 128 * 3];
};

struct ncBSPLoadLeaf {
    int cluster;
    int area;
    int mins[3];
    int maxs[3];
    int firstLeafFace;
    int numFaces;
    int firstLeafBrush;
    int numBrushes;
};

struct ncBSPLeaf {
    int cluster;
    ncVec3 boundingBoxVertices[8];
    int firstLeafFace;
    int numFaces;
};

struct ncBSPNode {
    int planeIndex;
    int front, back;
    int mins[3];
    int maxs[3];
};

struct ncBSPVisibilityData {
    int numClusters;
    int bytesPerCluster;
    byte *bitset;
};

class ncBSP {
public:
    bool Load( const char *filename );
    void Unload( void );
    void Render( bool reflection, ncSceneEye oc );
    void Initialize( void );
    
    // Other.
    int CalculateLeaf( const ncVec3 cameraPosition );
    int IsClusterVisible( int cameraCluster, int testCluster );
    void CalculateVisibleData( const ncVec3  cameraPosition );
    
    bool Initialized;
    bool InUse;
    
    ncBitset visibleFaces;
    ncGLShader bspShader;
    
private:
    // BSP header.
    struct bspheader_t {
        char string[4];
        int version;
        filechunk_t dirEntry[DIR_ENTRIES];
    };
    
    int vertexCount;
    int facesCount;
    int polygonCount;
    int *meshIndices;
    int meshCount;
    int patchCount;
    int decalTextureCount;
    int lightmapCount;
    int nodeCount;
    int planeCount;
    int leafCount;
    int *leafFaces;
    
    ncBSPVertex         *vertices;
    ncBSPDirectoryEntry *bspDirectory;
    ncBSPPolygonFace    *polygonFaces;
    ncBSPMeshFace       *meshFaces;
    ncBSPPatch          *patches;
    ncBSPLeaf           *leaves;
    ncPlane             *planes;
    ncBSPNode           *nodes;
    ncBSPVisibilityData visibilityData;
    
    GLuint  *decalTextures;
    GLuint  *lightmapTextures;
    GLuint  whiteTexture;
    
    GLuint vaoID[2];
    
    GLuint lightVBO;
    GLuint decalVBO;
    GLuint verticeVBO;
    GLuint normalVBO;
    
    // Rendering functions.
    void RenderFace( int faceNumber, bool reflection, ncSceneEye oc );
    void RenderPolygon( int polygonFaceNumber );
    void RenderMesh( int meshFaceNumber );
    void RenderPatch( int patchNumber );
    
    // Vertex array/buffer object setup.
    bool SetupObjects( void );
    
    // BSP build functions.
    bool LoadMeshes( FILE *file );
    bool LoadLightmaps( FILE *file );
    bool LoadData( FILE *file );
    bool LoadVertices( FILE *file );
    bool LoadFaces( FILE *file );
    bool LoadTextures( FILE *file );
    
    bspheader_t header;
};

extern ncBSP _bspmngr;

#endif