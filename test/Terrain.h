//
//  Terrain.h
//  Nanocat
//
//  Created by Neko Code on 11/6/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#ifndef __Terrain__
#define __Terrain__

#include "Renderer.h"

/*
    Terrain edit types.
*/
enum ncTerrainEditTypes {
    TERRAIN_EDIT_NEW = 0,
    TERRAIN_EDIT_LOAD,
};

enum ncVBODataType {
    VBO_DATA_TYPE_VECTOR3 = 0,
    VBO_DATA_TYPE_VECTOR2,
    VBO_DATA_TYPE_COLOUR4,
    VBO_DATA_TYPE_UINT,
    
    VBO_DATA_TYPE_PAD = 0xffffffff
};

enum ncTerrainEditDeformModes {
    DEFORM_MODE_GAUSSIAN_SMOOTH = 0,
    DEFORM_MODE_RAISE_UNIFORM,
    DEFORM_MODE_RAISE_CONE,
    DEFORM_MODE_LEVEL,
    
    DEFORM_MODE_PAD = 0xffffffff
};

struct ncTerrainBuffers {
    ncVec3			*NormalFaces;
    ncVec3			*Normals;
    ncVec3			*Positions;
    GLuint			*Indexes;
    ncVec2          *UVCoord;
};

struct ncTerrainVBOHandles {
    GLuint mVertices;
    GLuint mUV;
    GLuint mNormals;
    GLuint mColors;
};

struct ncTerrainRippleParams {
    int		posx;
    int		posz;
    float	mag;
    float	max_mag;
    int		size;
    float	radiation;
};

/*
    Beautiful game terrain.
*/
class ncLODTerrain {
    
public:
    ncLODTerrain( void );
    ~ncLODTerrain( void );
    
    void	Load( ncTerrainEditTypes type, const char *filename_p, ncVec3 pos,
                 float scale = 1.0f, float height = 160.0f );
    void    Render( ncSceneEye eye );
    void	MakeRipple( float x, float z, float mag );
    void    Refresh();
    
    // Some helpers.
    float   GetHeightAt( float xPos, float zPos );
    float   GetHeightFromImageData( Byte *imageData, int nX, int nY );
    int     GetVertexIndexAt( float xPos, float zPos, bool xalligned = false );
    ncVec3  GetRandomPos();

    // Main shader.
    ncGLShader  *m_shader;
    
    // Holds heights from transformed vertex data.
    float       *mHeightData; // Not an image data!
    
    int         m_heightmap_id;
    int         m_texturemap_id[5];
    int         m_detail_id;

private:
    uint                m_vao;
    int					m_map_size;
    int					m_index_size;
    
    int					m_sizex;
    int					m_sizez;
    
    float				m_scale;
    float				m_height;
    float				m_max_height;
    
    int					m_num_leaves;
    float				m_edit_radius;
    bool				m_recalc_lighting;
    bool				m_quick_normals_toggle;

    bool                m_firstFrame = true;
    
    ncMatrix4   m_modelView;
    GLuint      m_bspIndexElementsData, m_bspIndexElementsDataTF;
    GLuint      m_transformFeedbackQuery;
    GLuint      m_transformFeedback;
    
    ncTerrainEditTypes          m_type;
    ncTerrainEditDeformModes    m_deform_mode;
    ncTerrainBuffers            m_buffer;
    ncTerrainVBOHandles         m_vbo_handles;
    ncTerrainVBOHandles         m_vbo_transformHandles;
    ncTerrainRippleParams       m_ripple;
    
    bool		LineIntersect1( float start, float axisdir, float min, float max, float &enter, float &exit );
    bool		LineInTerrainTri( ncVec3 v0, ncVec3 v1, ncVec3 p0, ncVec3 p1, ncVec3 p2, int quad_index );
    void		FindSelectionPoint( ncVec3 origin, ncVec3 line_dir, ncVec3 bl, ncVec3 tr, ncVec3 min_size );
    
    void		Deform( ncTerrainEditDeformModes mode, int index, float radius, float strength );
    void		RemapData( GLuint handle, void *buf_p, ncVBODataType type, int diameter = 0, unsigned int index = 0 );
    
    void		CalculateNormals( int diameter, unsigned int index, bool cheap = false );
    void		LoadHeightmap( const char *heightmap_p );
    
    // Don't use it if you don't know how.
    void        GetTransformationData();
};


#endif
