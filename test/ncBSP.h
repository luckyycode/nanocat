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

#define MAX_BSP_SCALE 1.0
#define DIR_ENTRIES 17

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
	SW_VISIBLEDATA,
    SW_MAXLUMPS
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

    ncVec2 decal;
    ncVec2 light;
    
    ncVec3 normal;
    
    Byte color[4];

    
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
/*
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
};*/

enum ncBSPFaceType {
    SW_POLYGON = 1,
    SW_PATCH = 2,
    SW_MESH = 3,
    SW_BILLBOARD = 4,
    SW_FACE = 5
};

struct ncBSPEntityDirString
{
    int  size;
    NString ents;
};

struct ncBSPEntities
{
    char* entities;
};

struct ncBSPLightVol
{
    unsigned char ambient[3];
    unsigned char directional[3];
    unsigned char dir[2];
};

struct ncBSPDirectoryEntry {
    ncBSPFaceType faceType;
    int typeFaceNumber;
};

typedef struct {
    int brushside;
    int n_brushsides;
    int texture;
} ncBSPBrush;

typedef struct {
    int plane;
    int texture;
} ncBSPBrushSide;

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


class ncBSPBiquadPatch
{
public:
    ncBSPBiquadPatch() : m_tesselationLevel(0),
    m_trianglesPerRow(NULL),
    m_rowIndexPointers(NULL)
    {
    }
    
    ~ncBSPBiquadPatch()
    {
        delete [] m_trianglesPerRow;
        delete [] m_rowIndexPointers;
    }
    
    void Tesselate(int tessLevel);      // perform tesselation
    void Render(void);
    void GenerateArrays(void);
    
    uint patch_vao[1];
    
    ncBSPVertex controlPoints[9];
    
    int                          m_tesselationLevel;
    ncBSPVertex                 *m_m_vertices;
    unsigned int    *m_indices;
    int*                         m_trianglesPerRow;
    unsigned int**               m_rowIndexPointers;
};

struct ncBSPPatch {
    int textureIdx;   // surface texture index
    int lightmapIdx;  // surface lightmap index
    int width;
    int height;
    
    ncBSPBiquadPatch *quadraticPatches;
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


// BSP header.
class ncBSPHeader {
public:
    char string[4];
    int version;
    
    ncFileChunk dirEntry[DIR_ENTRIES];
};

// BSP manager class.
class ncBSP {
public:
    bool Load( const NString filename );
    void Unload( void );
    void Render( bool reflection, ncSceneEye oc );
    void Initialize( void );
    
    // Visibility calculation.
    int CalculateLeaf( const ncVec3 cameraPosition );
    int IsClusterVisible( int cameraCluster, int testCluster );
    void CalculateVisibleData( const ncVec3  cameraPosition );
    
    bool Initialized;
    bool InUse = false; // IsRendering?
    
    ncBitset visibleFaces;
    ncGLShader *bspShader;

private:
    FILE *mainFile;
    
    int vertexCount;
    int facesCount;
    int polygonCount;
    int meshCount;
    int patchCount;
    int decalTextureCount;
    int lightmapCount;
    int nodeCount;
    int planeCount;
    int leafCount;
    int m_iNumLeafBrushes;
    
    int *leafFaces;
    int *meshIndices;
    int *m_pLeafBrushes;
    
    ncBSPLoadLeaf       *m_leafData;
    ncBSPVertex         *m_vertices;
    ncBSPDirectoryEntry *f_bspType;
    ncBSPLoadFace       *m_polygonFaces;
    ncBSPMeshFace       *m_meshFaces;
    ncBSPPatch          *m_patches;
    ncBSPLeaf           *m_leaves;
    ncPlane             *m_planes;
    ncBSPNode           *m_nodes;
    ncBSPLoadFace       *m_pFaces;
    
    ncBSPVisibilityData m_visibilityData;
    
    ncBSPBrush          *m_pBrushes;
    ncBSPBrushSide      *m_pBrushSides;
    
    GLuint  *m_decalTextures;
    GLuint  *m_normalTextures;
    GLuint  *m_lightmapTextures;
    
    GLuint  m_defaultLightTexture;
    
    NString m_EntityData; // Raw string.
    
    /*
     
     Vertex buffer objects:
     
     0) Vertice data
     1) Light UV data
     2) Decal UV data
     3) Normal data
     
     Index array:
     
     0) Mesh index array
     1) Face index array
     2) Patch index array
     
     Vertex array objects:
     
     0) Polygon vao
     1) Mesh vao
     2) Patch vao
     3) Face vao
     
     */
    
#define BSP_FACEVBO_COUNT 4
#define BSP_MESHVBO_COUNT 4
#define BSP_INDEXELEMENTS_COUNT 3
    
#define BSP_VERTEXARRAY_COUNT 4
    
#define BSPSHADER_MAXUNIFORMS 7
    
    GLuint m_bspFaceVBOs[BSP_FACEVBO_COUNT];
    GLuint m_bspMeshVBOs[BSP_MESHVBO_COUNT];
    GLuint m_bspIndexElementsData[BSP_INDEXELEMENTS_COUNT];
    
    GLuint m_bspVertexArray[BSP_VERTEXARRAY_COUNT];
    
    enum ncBSPShaderUniforms {
        SHMVP_UNIFORM = 0, // Model + View + Projection
        SHMV_UNIFORM, // Model view
        SHCAMPOS_UNIFORM, // Camera position
        SHLIGHTMAP_UNIFORM,
        SHDECALMAP_UNIFORM,
        SHNORMALMAP_UNIFORM,
        SHDEPTHMAP_UNIFORM
    };
    
    // Shader uniforms.
    GLuint g_bspUniforms[BSPSHADER_MAXUNIFORMS];

    // Rendering functions.
    void RenderFace( int faceNumber, bool reflection, ncSceneEye oc );
    void RenderPolygon( int polygonFaceNumber );
    void RenderMesh( int meshFaceNumber );
    void RenderPatch( int patchNumber );
    // Render faces.
    void RenderFaces( int faceNumber );
    
    // Vertex array/buffer object setup.
    bool SetupObjects( void );
    bool CleanupGraphics( void );
    
    // Helper functions.
    void ComputeTangentData( void );
    
    // BSP build functions.
    void LoadEntityString( void );
    void LoadMeshes( void );
    void LoadBrushes( void );
    void LoadLightmaps( void );
    void LoadLightVols( void );
    void LoadData( void );
    void LoadVertices( void );
    void LoadFaces( void );
    void LoadTextures( void );
    
    // Global functions.
    bool IsValid( ncBSPHeader *header );

    ncBSPHeader header;
    
    const ncVec3 bsp_position = ncVec3( 0.0, 0.0, 0.0 );
};

extern ncBSP *g_staticWorld;


#endif