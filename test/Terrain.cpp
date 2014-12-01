//
//  Nanocat engine.
//
//  Beautiful terrain creator & renderer..
//
//  Created by Neko Code on 2/8/14.
//  Copyright (c) 2014 Neko Code. All rights reserved.
//

#include "Core.h"
#include "AssetManager.h"
#include "ShaderLoader.h"
#include "Camera.h"
#include "MaterialLoader.h"
#include "Renderer.h"
#include "GameWorld.h"
#include "Terrain.h"
#include "Input.h"
#include "LocalGame.h"
#include "Models.h"
#include "BeautifulEnvironment.h"
#include "Utils.h"

/*
 
        Beautiful Large Terrain Renderer.
 
*/


/*
 *      Prepare new terrain.
 */
ncLODTerrain::ncLODTerrain() {
    m_buffer.Positions = NULL;
    m_buffer.Indexes = NULL;
    m_buffer.Normals = NULL;
    m_buffer.UVCoord = NULL;
}

ncLODTerrain::~ncLODTerrain() {
    ifarrdelete( mHeightData );
    
    ifarrdelete( m_buffer.NormalFaces );
    ifarrdelete( m_buffer.Normals );
    ifarrdelete( m_buffer.Positions );
    ifarrdelete( m_buffer.UVCoord );
    ifarrdelete( m_buffer.Indexes );
}

/*
 *      Create new terrain.
 */
void ncLODTerrain::Load( ncTerrainEditTypes type, const char *filename_p, ncVec3 pos, float scale, float height ) {
    
    m_type = type;
    
    m_quick_normals_toggle = false;
    
    m_deform_mode = DEFORM_MODE_RAISE_CONE;
    m_edit_radius = 30.0f * scale;
    
    m_height = height;
    m_scale = scale;
    m_recalc_lighting = true;
    m_num_leaves = 0;
    
    // Model view for current terrain.
    m_modelView.Identity();
    m_modelView.Translate( pos );
    
    m_shader = f_AssetManager->FindShaderByName( "terrain" );
    
    // Diffuse textures.
    m_texturemap_id[0] = g_materialManager->Find("snowy")->Image.TextureID;
    m_texturemap_id[1] = g_materialManager->Find("rocky")->Image.TextureID;
    m_texturemap_id[2] = g_materialManager->Find("sandy")->Image.TextureID;
    m_texturemap_id[3] = g_materialManager->Find("grassy")->Image.TextureID;
    m_texturemap_id[4] = g_materialManager->Find("angry")->Image.TextureID;
    
    // Heightmap texture.
    m_heightmap_id = g_materialManager->Find( filename_p )->Image.TextureID;
    
    // Detail texture.
    m_detail_id = g_materialManager->Find("detail1")->Image.TextureID;

    // Setup shader.
    m_shader->Use();
    for( int i = 0; i < 5; i++ )
        m_shader->SetUniform( NC_TEXT("gSampler[%i]", i), i );
    
    m_shader->SetUniform( "cat", 5 );
    m_shader->Next();
    
    LoadHeightmap( filename_p );
}

/*
 *      Code from NeHe tutorial code.
 */

float ncLODTerrain::GetHeightFromImageData( Byte *imageData, int nX, int nY )
{
    int nPos = ( ( nX % m_sizex )  + ( ( nY % m_sizex ) * m_sizex ) ) * 3;
    
    float flR = (float)imageData[ nPos ];
    float flG = (float)imageData[ nPos + 1 ];
    float flB = (float)imageData[ nPos + 2 ];
    
    // The Luminance Algorithm.
    return ( 0.299f * flR + 0.587f * flG + 0.114f * flB );
}

/*
 *      Load terrain.
 *      Load terrain from simple image or bsp-like file.
 */
void ncLODTerrain::LoadHeightmap( const char *heightmap_p ) {

    ncImage *tex;
    
    // Heightmap image data.
    m_heightmap_id = g_materialManager->Find( heightmap_p )->Image.TextureID;
    tex = &g_materialManager->Find( heightmap_p )->Image;
    
    m_sizex = tex->Width;
    m_sizez = tex->Heigth;
    
    // Not that important for rendering and something like this.

    m_map_size = ( m_sizez * m_sizex );
    m_index_size = m_map_size * 2;
    
    // Create buffers now.
    m_buffer.Positions = (ncVec3 *)malloc( m_map_size * sizeof( ncVec3 ) );
    m_buffer.Indexes = (GLuint *)malloc( m_index_size * sizeof( GLuint ) );
    m_buffer.UVCoord = (ncVec2 *)malloc( m_map_size * sizeof( ncVec2 ) );
    
    float center_x = (m_sizex * 0.5f);
    float center_z = (m_sizez * 0.5f);
    
    int id = 0;
    int index = 0;
    int heightid;
    
    m_max_height = 255.0f * ( m_height * m_scale );
    
    for( int z = 0; z < m_sizez; z++ ) {
        for( int x = 0; x < m_sizex; x++ ) {
 
            int i = (z * m_sizex ) + x;
            heightid = i * (32 / 8);
            
            // Vertex data.
            m_buffer.Positions[id].x = (GLfloat)(-center_x + x) * m_scale;
            m_buffer.Positions[id].y = 0.0f; // We generate height value in vertex shader.
            m_buffer.Positions[id].z = (GLfloat)(-center_z + z) * m_scale;

            m_buffer.UVCoord[id].x = (GLfloat)( (GLfloat)x / m_sizex );
            m_buffer.UVCoord[id].y = (GLfloat)( (GLfloat)z / m_sizez );
            
            id++;
            
            // Indices.
            for( int row = 0; row < 2; row++ ) {
                // First vertex index.
                if( row && !x && z ) {
                    m_buffer.Indexes[index] = m_buffer.Indexes[index-1];
                    index++;
                }
                if( z != m_sizez - 1 ) {
                    m_buffer.Indexes[index] = ((z + row) * m_sizex) + x;
                    index++;
                }
            }
            
        }
        
        // Last vertex index.
        if( z != m_sizez-1 ) {
            m_buffer.Indexes[index] = m_buffer.Indexes[index-1];
            index++;
        }
    }
    
    // Normals.
    int faces = ((m_sizex * 2) * m_sizez);
    m_buffer.NormalFaces = new ncVec3[faces];
    m_buffer.Normals = new ncVec3[m_map_size];
    
    CalculateNormals( 0, 0 );
    
    delete [] m_buffer.NormalFaces;
    
    // Vertex objects.
    glGenVertexArrays( 1, &m_vao );
    glBindVertexArray( m_vao );
    
    // Generate vertex buffer objects.
    glGenBuffers( 1, &m_vbo_handles.mVertices );
    glGenBuffers( 1, &m_vbo_handles.mUV );
    glGenBuffers( 1, &m_vbo_handles.mNormals );
    
    glGenBuffers( 1, &m_vbo_transformHandles.mVertices );
    glGenBuffers( 1, &m_vbo_transformHandles.mUV );
    glGenBuffers( 1, &m_vbo_transformHandles.mNormals );
    
    glGenBuffers( 1, &m_bspIndexElementsData );
    glGenBuffers( 1, &m_bspIndexElementsDataTF );

    mHeightData = new float[m_map_size];

    // Transform feedback object.
    glGenBuffers( 1, &m_transformFeedback );
    
    glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, m_transformFeedback );
    glBufferData( GL_TRANSFORM_FEEDBACK_BUFFER, m_map_size * sizeof( ncVec3 ), NULL, GL_DYNAMIC_COPY );
    glBindBuffer( GL_TRANSFORM_FEEDBACK_BUFFER, 0 );

    // Quires.
    glGenQueries(1, &m_transformFeedbackQuery);
    
    glBindVertexArray( 0 );

    Refresh();
    
    g_Core->Print( LOG_INFO, "Terrain file \"%s\" successfully loaded!\n", heightmap_p );
}

/*
 *      Generate normals.
 */
void ncLODTerrain::CalculateNormals( int diameter, unsigned int index, bool cheap )
{
    if( cheap ) {
        // Rough estimate face normals.
        ncVec3 *Positions = m_buffer.Positions;
        ncVec3 *face_p = m_buffer.NormalFaces;
        
        if( !diameter ) {
            for( int i = 0; i < m_map_size; i++ ) {
                // Three vertices forming plane.
                ncVec3 p0 = Positions[i];
                ncVec3 p1 = Positions[i+1];
                ncVec3 p2 = Positions[i+m_sizex];
                
                // Calculate vectors.
                ncVec3 v0 = p1 - p0;
                ncVec3 v1 = p2 - p0;
                
                // Cross and normalize.
                face_p[i].Cross( v1, v0 );
                face_p[i].Normalize();
                
                m_buffer.Normals[i] = face_p[i];
            }
        }
        else {
            int min, max, rad;
            rad = diameter / 2;
            min = index - (rad * m_sizex) - rad;
            max = index + (rad * m_sizex) + rad;
            
            int diff = max - min;
            
            // Corrent out-of-bound values.
            if( max > m_map_size-m_sizex ) {
                max = m_map_size-m_sizex;
                min = max - diff;
            }
            
            if( min < 0 ) {
                min = 0;
                max = diff;
            }
            
            int c = 0;
            for( int i = min; i < max; i++ ) {
                ncVec3 p0 = Positions[i];
                ncVec3 p1 = Positions[i + 1];
                ncVec3 p2 = Positions[i + m_sizex];
                
                ncVec3 v0 = p1 - p0;
                ncVec3 v1 = p2 - p0;

                face_p[i].Cross( v1, v0 );
                face_p[i].Normalize();
                
                m_buffer.Normals[i] = face_p[i];
                
                c++; // wow, such c++, much similar
                if( c >= diameter ) {
                    c = 0;
                    i += m_sizex-diameter;
                }
            }
        }
        
        return;
    }
    
    // Full average vertex normals, expensive!
    if( m_recalc_lighting ) {
        m_recalc_lighting = false;
        
        ncVec3 *Positions = m_buffer.Positions;
        ncVec3 *face_p = m_buffer.NormalFaces;
        
        // Face normals.
        int faces = ((m_sizex*2) * m_sizez)-m_sizex;
        for( int i = 0; i < faces-m_sizex; i+=2 )
        {
            int n = (i/2);
            
            ncVec3 p0 = Positions[n];
            ncVec3 p1 = Positions[n+1];
            ncVec3 p2 = Positions[n+m_sizex];
            ncVec3 p3 = Positions[n+m_sizex+1];
            
            ncVec3 v0 = p1 - p0;
            ncVec3 v1 = p2 - p0;
            ncVec3 v2 = p2 - p1;
            ncVec3 v3 = p3 - p1;
            
            face_p[i].Cross( v1, v0 );
            face_p[i+1].Cross( v2, v3 );
        }
        
        // Vertex (averaged) normals.
        for( int z = 0; z < m_sizez; z++ )
        {
            for( int x = 0; x < m_sizex; x++ )
            {
                int i = ((m_sizex * z) + x);
                
                ncVec3 norm( 0.0f, 0.0f, 0.0f );
                int total_faces = 0;
                
                if( z > 0 ) {
                    int face = (i * 2)-(m_sizex * 2);
                    if( x > 0 ) {
                        total_faces += 2;
                        norm = norm + face_p[face-2];
                        norm = norm + face_p[face-1];
                    }
                    
                    if( x < m_sizex-1 ) {
                        total_faces += 2;
                        norm = norm + face_p[face];
                        norm = norm + face_p[face+1];
                    }
                }
                
                if( z < m_sizez ) {
                    int face = (i * 2);
                    
                    if( x > 0 ) {
                        total_faces += 2;
                        norm = norm + face_p[face-2];
                        norm = norm + face_p[face-1];
                    }
                    
                    if( x < m_sizex - 1 ) {
                        total_faces += 2;
                        norm = norm + face_p[face];
                        norm = norm + face_p[face+1];
                    }
                }
                
                if( total_faces ) // "Sanity" check.
                {
                    // Normalise result.
                    m_buffer.Normals[i] = norm / total_faces;
                    m_buffer.Normals[i].Normalize();
                }
            }
        }
    }
    
}

/*
 *      Update vertex buffer object data using mapping.
 */
void ncLODTerrain::RemapData( GLuint handle, void *buf_p, ncVBODataType type, int diameter, unsigned int index ) {
    
    if( !diameter || diameter > m_sizex ) {
        // Remap the whole buffer.
        int size;
        switch( type )
        {
            case VBO_DATA_TYPE_VECTOR3:
                size = sizeof(ncVec3);
                break;
            case VBO_DATA_TYPE_VECTOR2:
                size = sizeof(ncVec2);
                break;
            default:
                size = 4;
                break;
        }
        glBindBuffer( GL_ARRAY_BUFFER, handle );
        ncVec3 *vbo_data_p = (ncVec3 *)glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
        memcpy( vbo_data_p, buf_p, size * m_map_size );
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
    else {
        int min, max, rad;
        rad = diameter / 2;
        min = index - (rad * m_sizex) - rad;
        max = index + (rad * m_sizex) + rad;
        int diff = max - min;
        
        // Correct out-of-bounds values.
        if( max > m_map_size ) {
            max = m_map_size;
            min = max - diff;
        }
        
        if( min < 0 ) {
            min = 0;
            max = diff;
        }
        
        glBindBuffer( GL_ARRAY_BUFFER, handle );
        switch( type )
        {
            case VBO_DATA_TYPE_VECTOR3:
            {
                ncVec3 *vbo_data_p = (ncVec3 *)glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
                vbo_data_p += min;
                ncVec3 *vbuf_p = (ncVec3*)buf_p + min;
                for( int i = 0; i < diameter; i++ )
                {
                    for( int j = 0; j < diameter; j++ )
                    {
                        if( !vbo_data_p )
                            continue;
                        // oops
                        vbo_data_p[j] = vbuf_p[j];
                    }
                    vbuf_p += m_sizex;
                    vbo_data_p += m_sizex;
                }
                //memcpy( vbo_data_p, vbuf_p, sizeof( Vector ) * ( max - min ) );
            }
                break;
                
            case VBO_DATA_TYPE_VECTOR2:
            {
                ncVec2 *vbo_data_p = (ncVec2 *)glMapBuffer( GL_ARRAY_BUFFER, GL_READ_WRITE );
                vbo_data_p += min;
                ncVec2 *vbuf_p = (ncVec2*)buf_p + min;
                for( int i = 0; i < diameter; i++ )
                {
                    for( int j = 0; j < diameter; j++ )
                    {
                        vbo_data_p[j] = vbuf_p[j];
                    }
                    vbuf_p += m_sizex;
                    vbo_data_p += m_sizex;
                }
                //memcpy( vbo_data_p, vbuf_p, sizeof( Vector ) * ( max - min ) );
            }
                break;
                
        }
        glUnmapBuffer( GL_ARRAY_BUFFER );
    }
}

/*
 *      Check line intersection.
 */
bool ncLODTerrain::LineIntersect1( float start, float axisdir, float min, float max, float &enter, float &exit )
{
    // Intersection params.
    float t0, t1;
    t0 = (min - start) / axisdir;
    t1 = (max - start) / axisdir;
    
    // sort intersections
    if( t0 > t1 ) {
        float t = t0;
        t0 = t1;
        t1 = t;
    }
    
    // Reduce interval.
    if( t0 > enter )
        enter = t0;
    
    if( t1 < exit )
        exit = t1;
    
    // Ray misses the box.
    if( exit < enter )
        return false;
    
    return true;
}


/*
 *      Get vertex index at X,Z point.
 */
int ncLODTerrain::GetVertexIndexAt( float xPos, float zPos, bool xalligned ) {
    xPos += m_sizex / 2;
    zPos += m_sizex / 2;
    
    if(xPos < 0)
        xPos = 0;
    else if(xPos >= m_sizex)
        xPos = m_sizex - 1;
    
    if(zPos < 0)
        zPos = 0;
    else if(zPos >= m_sizex)
        zPos = m_sizex - 1;
    
    int vertexIndex = 0;
    
    if( xalligned ) {
        vertexIndex = (int)(xPos) * m_sizex + (int)(zPos);
    } else {
        vertexIndex = (int)(xPos) + (int)(zPos) * m_sizex;
    }
    
    return vertexIndex;
}

/*
 *  Get point height at x,z
 */
GLfloat ncLODTerrain::GetHeightAt(GLfloat xPos, GLfloat zPos) {
    int vertexIndex = GetVertexIndexAt( xPos, zPos );
    
    g_Core->Print( LOG_INFO, "pls: %f\n", mHeightData[vertexIndex] );
    return mHeightData[vertexIndex];
}

/*
 *      Refresh terrain.
 */
void ncLODTerrain::Refresh() {
    m_shader->Use();
    
    m_shader->SetUniform( "u_map_dims", (float)m_sizex * m_scale, (float)m_sizez * m_scale );
    m_shader->SetUniform( "u_map_scale", m_scale * m_height );

    m_shader->Next();
}

/*
 *      Render terrain.
 */
void ncLODTerrain::Render( ncSceneEye eye ) {
    
    ncMatrix4 pos;
    pos.Identity();
    
    ncMatrix4 projectionModelView = g_playerCamera->ProjectionMatrix * g_playerCamera->ViewMatrix * pos;
    
    m_shader->Use();

    glBindVertexArray( m_vao );
    
    m_shader->SetUniform( "ModelMatrix", 1, false, pos.m );
    m_shader->SetUniform( "ProjMatrix", 1, false, g_playerCamera->ProjectionMatrix.m );
    m_shader->SetUniform( "ViewMatrix", 1, false, g_playerCamera->ViewMatrix.m );
    m_shader->SetUniform( "MVP", 1, false, projectionModelView.m );
    
    m_shader->SetUniform( "cameraPos", g_playerCamera->g_vEye );

    m_modelView = pos * g_playerCamera->ViewMatrix;
    
    glEnableVertexAttribArray( 0 );
    glEnableVertexAttribArray( 1 );
    glEnableVertexAttribArray( 2 );

    // Update vertex transformation data if needed..
    if( m_firstFrame ) {
        GetTransformationData();
        m_firstFrame = false;
    } else { // Or just render as you need.
    
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo_handles.mVertices );
        glBufferData( GL_ARRAY_BUFFER, sizeof( ncVec3 ) * m_map_size, m_buffer.Positions, GL_DYNAMIC_DRAW );
        glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(ncVec3), (void*)0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo_handles.mUV );
        glBufferData( GL_ARRAY_BUFFER, sizeof( ncVec2 ) * m_map_size, m_buffer.UVCoord, GL_DYNAMIC_DRAW );
        glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(ncVec2), (void*)0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
        glBindBuffer( GL_ARRAY_BUFFER, m_vbo_handles.mNormals );
        glBufferData( GL_ARRAY_BUFFER, sizeof( ncVec3 ) * m_map_size, m_buffer.Normals, GL_DYNAMIC_DRAW );
        glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(ncVec3), (void*)0 );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_bspIndexElementsDataTF);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_index_size * sizeof(GLuint), m_buffer.Indexes, GL_DYNAMIC_DRAW);
        
        glDrawElements( GL_TRIANGLE_STRIP, m_index_size, GL_UNSIGNED_INT, NULL );
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    }
    
    glDisableVertexAttribArray( 2 );
    glDisableVertexAttribArray( 1 );
    glDisableVertexAttribArray( 0 );
    
    glBindVertexArray( 0 );
    m_shader->Next();
    
    //g_playerCamera->g_vEye.y = GetHeightAt( g_playerCamera->g_vEye.z, g_playerCamera->g_vEye.z );
}

/*
 *      Get random position on terrain.
 */
ncVec3 ncLODTerrain::GetRandomPos() {

    int randomXZY = rand() % (m_sizex * m_sizez);
    
    float ny = mHeightData[randomXZY];
    float nx = m_buffer.Positions[randomXZY].x;
    float nz = m_buffer.Positions[randomXZY].z;
    
    return ncVec3( nx, ny, nz );
}

/*
 *      Get data from vertex shader. 
 *      Super-duper useful!
 */
void ncLODTerrain::GetTransformationData() {
    int queryResult;
    
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_transformHandles.mVertices );
    glBufferData( GL_ARRAY_BUFFER, sizeof( ncVec3 ) * m_map_size, m_buffer.Positions, GL_DYNAMIC_DRAW );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(ncVec3), (void*)0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_transformHandles.mUV );
    glBufferData( GL_ARRAY_BUFFER, sizeof( ncVec2 ) * m_map_size, m_buffer.UVCoord, GL_DYNAMIC_DRAW );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof(ncVec2), (void*)0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    glBindBuffer( GL_ARRAY_BUFFER, m_vbo_transformHandles.mNormals );
    glBufferData( GL_ARRAY_BUFFER, sizeof( ncVec3 ) * m_map_size, m_buffer.Normals, GL_DYNAMIC_DRAW );
    glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof(ncVec3), (void*)0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    
    // We don't need color data.
    glEnable( GL_RASTERIZER_DISCARD );
    
    glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_transformFeedback );
    
    glBeginQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_transformFeedbackQuery );
    glBeginTransformFeedback( GL_POINTS );
    
    glDrawArrays( GL_POINTS, 0, m_map_size );
    
    glEndTransformFeedback();
    glEndQuery( GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN );
    
    glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0 );
    glDisable( GL_RASTERIZER_DISCARD );
    
    // Get data now.
    glBindBufferBase( GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_transformFeedback );
    
    // We get only height value here, not the whole vertex data.
    glGetBufferSubData( GL_TRANSFORM_FEEDBACK_BUFFER, 0, m_map_size * sizeof(float), mHeightData );
    glGetQueryObjectiv(m_transformFeedbackQuery, GL_QUERY_RESULT, &queryResult);
    
    g_Core->Print( LOG_DEVELOPER, "%i terrain primitives written.\n", queryResult );
}

/*
 *      Make terrain deformation.
 */
void ncLODTerrain::Deform( ncTerrainEditDeformModes mode, int index, float radius, float strength )
{
    ncVec3 center = m_buffer.Positions[index];
    center.y = 0.0f;
    float index_rad = radius / m_scale;
    
    for( int z = -index_rad; z < index_rad; z++ )
    {
        for( int x = -index_rad; x < index_rad; x++ )
        {
            int i = index + (z * m_sizex) + x;
            // Skip out of bounds indices.
            if( i < 0 || i >= m_map_size )
                continue;
            
            ncVec3 pos = m_buffer.Positions[i];
            // Ignore height component.
            pos.y = 0.0f;
            // Get distance squared from
            // deformation center point to current vertice.
            float dist = center.Distance( pos );
            if( dist < radius * radius )
            {
                switch( mode )
                {
                    case DEFORM_MODE_RAISE_UNIFORM:
                        m_buffer.Positions[i].y += strength;
                        break;
                        
                    case DEFORM_MODE_RAISE_CONE:
                    {
                        float real_dist = sqrtf( dist );
                        float normal_inv_dist = real_dist == 0.0f ? 1.0f : 1.0f - ( real_dist / radius );
                        m_buffer.Positions[i].y += normal_inv_dist * ( strength  );
                    }
                        break;
                        
                    case DEFORM_MODE_GAUSSIAN_SMOOTH:
                    {
                        int gx, gz;
                        float r = (index*(1.0f/m_sizex));
                        int ri = r;
                        r = r - ri;
                        gx = (r*m_sizex)+x;
                        gz = (index*(1.0f/m_sizex))+z;
                        float h = 0.0f;
                        int total_verts = 0;
                        
                        if( gz > 0 )
                        {
                            int vert = i-m_sizex;
                            if( gx > 0 )
                            {
                                total_verts+=2;
                                h += m_buffer.Positions[vert-2].y;
                                h += m_buffer.Positions[vert-1].y;
                            }
                            
                            if( gx < m_sizex-1 )
                            {
                                total_verts+=2;
                                h += m_buffer.Positions[vert].y;
                                h += m_buffer.Positions[vert+1].y;
                            }
                        }
                        
                        if( gz < m_sizez )
                        {
                            int vert = i;
                            if( gx > 0 )
                            {
                                total_verts+=2;
                                h += m_buffer.Positions[vert-2].y;
                                h += m_buffer.Positions[vert-1].y;
                            }
                            
                            if( gx < m_sizex-1 )
                            {
                                total_verts+=2;
                                h += m_buffer.Positions[vert].y;
                                h += m_buffer.Positions[vert+1].y;
                            }
                        }
                        
                        if( total_verts )
                            // Normalise result.
                            m_buffer.Positions[i].y += ( ( h / total_verts ) - m_buffer.Positions[i].y ) * ( strength );
                    }
                        break;
                }
            }
        }
    }
    
    int diam = (int)index_rad * 2;
    
    RemapData( m_vbo_handles.mVertices, m_buffer.Positions, VBO_DATA_TYPE_VECTOR3, diam, index );
    
}

/*
 *      Add ripple while editing.
 */
void ncLODTerrain::MakeRipple( float x, float z, float mag ) {
    m_ripple.mag = mag;
    m_ripple.max_mag = mag;
    m_ripple.size = (int)(mag * 2.2f);
    m_ripple.posx = (int)( m_sizex * x );
    m_ripple.posz = (int)( m_sizez * z );
    m_ripple.radiation = 0.0f;
    
    m_buffer.Positions[(m_ripple.posz*m_sizex)+m_ripple.posx].y -= m_ripple.mag;
}

/*
 
 void ncLODTerrain::find_selection_point( ncVec3 origin, ncVec3 line_dir, ncVec3 bl, ncVec3 tr, ncVec3 min_size )
 {
 // recursive quadtree search method!
 if( m_selection_hit )
 return;
 
 ncVec3 dims = (tr - bl);
 if( dims.x <= min_size.x &&
 dims.z <= min_size.z )
 {
 // line in poly...
 //printf( "Leaf found: %d\n", m_num_leaves++ );
 
 ncVec3 proj_pos = origin + ( line_dir * 100.0f );
 ncVec4 mapbl = m_modelView * ncVec4( (float)(-(m_sizex*m_scale) * 0.5f), 0.0f, (float)(-(m_sizez*m_scale) * 0.5f), 1.0 );
 ncVec3 map_bl = ncVec3( mapbl.x, mapbl.y, mapbl.z );
 
 // find vert buffer index
 int x = ( bl.x - map_bl.x ) / m_scale;
 int z = ( bl.z - map_bl.z ) / m_scale;
 if( z >= m_sizex-1 )
 return;
 
 int i = ( z * m_sizex ) + x;
 
 g_modelManager->Spawn( MODEL_DEFAULT, "cat.sm1", "model", proj_pos, proj_pos, false );
 
 // get quad at world space location
 ncVec3 pos00 =  m_modelView * m_buffer.Positions[i];
 ncVec3 pos10 =  m_modelView * m_buffer.Positions[i+1];
 ncVec3 pos01 =  m_modelView * m_buffer.Positions[i+m_sizex];
 ncVec3 pos11 =  m_modelView * m_buffer.Positions[i+m_sizex+1];
 
 // check tris in the quad found
 if( line_in_terrain_tri( origin, proj_pos, pos00, pos01, pos10, i ) ||
 line_in_terrain_tri( origin, proj_pos, pos01, pos11, pos10, i ) )
 {
 m_selection_hit = true;
 g_Core->Print( LOG_INFO, "Selection HIT!\n" );
 }
 
 return;
 }
 
 // to be reduced
 float enter = -9999999.0f;
 float exit = 9999999.0f;
 // SAT xz line in quadrant
 if( line_intersect_1d( origin.x, line_dir.x, bl.x, tr.x, enter, exit ) &&
 line_intersect_1d( origin.z, line_dir.z, bl.z, tr.z, enter, exit ) )
 {
 // line passes through this quad, check sub-quadrants
 ncVec3 mid = tr-(dims*0.5f);
 find_selection_point( origin, line_dir, bl,								mid,							min_size );
 find_selection_point( origin, line_dir, ncVec3( mid.x, 0.0f, bl.z ),	ncVec3( tr.x, 0.0f, mid.z ),	min_size );
 find_selection_point( origin, line_dir, ncVec3( bl.x, 0.0f, mid.z ),	ncVec3( mid.x, 0.0f, tr.z ),	min_size );
 find_selection_point( origin, line_dir, mid,							tr,								min_size );
 }
 }
 */
