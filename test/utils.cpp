//
//  Nanocat engine.
//
//  Utilities..
//
//  Created by Neko Code on 4/25/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Core.h"
#include "ConsoleCommand.h"
#include "FileSystem.h"
#include "NCString.h"
#include "Models.h"
#include "LocalGame.h"

#include "Utils.h"

/*
    Make mouse ray from screen coordinates.
*/
ncVec3 ncUtils::RayFromMousePos( ncMatrix4 m_modelView, int x, int y )
{
    float value_fov = DEG2RAD( GameView_FieldOfView.GetFloat() );
    float value_aspect = Render_AspectRatio.GetFloat();
    
    float modifier_x;
    float modifier_y;
    //mathematical handling of the difference between
    //your mouse position and the 'center' of the window
    
    float point[3];
    //the untransformed ray will be put here
    
    float point_dist = 100.0;
    //it'll be put this far on the Z plane
    
    float camera_origin[3];
    //this is where the camera sits, in 3dspace
    
    float point_xformed[3];
    //this is the transformed point
    
    
    
    float final_point[3];
    
    //for the wuDrawSprite call.
    
    //These lines are the biggest part of this function.
    //This is where the mouse position is turned into a mathematical
    //'relative' of 3d space. The conversion to an actual point
    modifier_x = tan( value_fov * 0.5 )
    * (( 1.0 - x / g_mainRenderer->renderHalfWidth ) * ( value_aspect ) );
    modifier_y = tan( value_fov * 0.5 )
    * -( 1.0 - y / g_mainRenderer->renderHalfWidth );
    modifier_y *= -1.0f;
    //These 3 take our modifier_x/y values and our 'casting' distance
    //to throw out a point in space that lies on the point_dist plane.
    //If we were using completely untransformed, untranslated space,
    //this would be fine - but we're not :)
    point[0] = modifier_x * point_dist;
    point[1] = modifier_y * point_dist;
    point[2] = point_dist;
    
    //Next we make an openGL call to grab our MODELVIEW_MATRIX -
    //This is the matrix that rasters 3d points to 2d space - which is
    //kinda what we're doing, in reverse
    //glGetFloatv(GL_MODELVIEW_MATRIX, pulledMatrix);
    //Some folks would then invert the matrix - I invert the results.
    
    //First, to get the camera_origin, we transform the 12, 13, 14
    //slots of our pulledMatrix - this gets us the actual viewing
    //position we are 'sitting' at when the function is called
    camera_origin[0] = -(
                         m_modelView.m[0] * m_modelView.m[12] +
                         m_modelView.m[1] * m_modelView.m[13] +
                         m_modelView.m[2] * m_modelView.m[14]);
    camera_origin[1] = -(
                         m_modelView.m[4] * m_modelView.m[12] +
                         m_modelView.m[5] * m_modelView.m[13] +
                         m_modelView.m[6] * m_modelView.m[14]);
    camera_origin[2] = -(
                         m_modelView.m[8] * m_modelView.m[12] +
                         m_modelView.m[9] * m_modelView.m[13] +
                         m_modelView.m[10] * m_modelView.m[14]);
    
    //Second, we transform the position we generated earlier - the '3d'
    //mouse position - by our viewing matrix.
    point_xformed[0] = -(
                         m_modelView.m[0] * point[0] +
                         m_modelView.m[1] * point[1] +
                         m_modelView.m[2] * point[2]);
    point_xformed[1] = -(
                         m_modelView.m[4] * point[0] +
                         m_modelView.m[5] * point[1] +
                         m_modelView.m[6] * point[2]);
    point_xformed[2] = -(
                         m_modelView.m[8] * point[0] +
                         m_modelView.m[9] * point[1] +
                         m_modelView.m[10] * point[2]);
    
    //That's pretty much that. Using the camera origin and the
    //transformed ray, you can now do ray-triangle collision
    //detection, which i'll leave up to you.
    
    //so you can see this function in action, i've included a second
    //function to draw a sprite (billboard)
    final_point[0] = point_xformed[0] + camera_origin[0];
    final_point[1] = point_xformed[1] + camera_origin[1];
    final_point[2] = point_xformed[2] + camera_origin[2];
    
    
    ncVec3 finalWoot = ncVec3( final_point[0], final_point[1], final_point[2] );
    
    //g_modelManager->Spawn( MODEL_DEFAULT, "cat.sm1", "model", finalWoot, finalWoot, false );
    return finalWoot;
}


/* 
    GL projection function.
    From GLU library.
*/

static void __gluMultMatrixVecf(const GLfloat matrix[16], const GLfloat in[4],
                                GLfloat out[4])
{
    int i;
    
    for( i = 0; i < 4; i++ ) {
        out[i] =
        in[0] * matrix[0*4+i] +
        in[1] * matrix[1*4+i] +
        in[2] * matrix[2*4+i] +
        in[3] * matrix[3*4+i];
    }
}

/*
 ** Invert 4x4 matrix.
 ** Contributed by David Moore (See Mesa bug #6748)
 */
static int __gluInvertMatrixf(const GLfloat m[16], GLfloat invOut[16])
{
    float inv[16], det;
    int i;
    
    inv[0] =   m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
    + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[4] =  -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
    - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[8] =   m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
    + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
    - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[1] =  -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
    - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[5] =   m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
    + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[9] =  -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
    - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[13] =  m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
    + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[2] =   m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
    + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[6] =  -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
    - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[10] =  m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
    + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
    - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[3] =  -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
    - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
    inv[7] =   m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
    + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
    inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
    - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
    inv[15] =  m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
    + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
    
    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return GL_FALSE;
    
    det = 1.0 / det;
    
    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;
    
    return GL_TRUE;
}

static void __gluMultMatricesf(const GLfloat a[16], const GLfloat b[16],
                               GLfloat r[16])
{
    int i, j;
    
    for (i = 0; i < 4; i++) {
        for (j = 0; j < 4; j++) {
            r[i*4+j] =
            a[i*4+0]*b[0*4+j] +
            a[i*4+1]*b[1*4+j] +
            a[i*4+2]*b[2*4+j] +
            a[i*4+3]*b[3*4+j];
        }
    }
}

GLint ncUtils::gluProject(GLfloat objx, GLfloat objy, GLfloat objz,
           const GLfloat modelMatrix[16],
           const GLfloat projMatrix[16],
           const GLint viewport[4],
           GLfloat *winx, GLfloat *winy, GLfloat *winz)
{
    float in[4];
    float out[4];
    
    in[0]=objx;
    in[1]=objy;
    in[2]=objz;
    in[3]=1.0;
    __gluMultMatrixVecf(modelMatrix, in, out);
    __gluMultMatrixVecf(projMatrix, out, in);
    if (in[3] == 0.0) return(GL_FALSE);
    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];
    /* Map x, y and z to range 0-1 */
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;
    
    /* Map x,y to viewport */
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];
    
    *winx=in[0];
    *winy=in[1];
    *winz=in[2];
    
    
    return(GL_TRUE);
}

GLint ncUtils::gluUnProject(GLfloat winx, GLfloat winy, GLfloat winz,
             const GLfloat modelMatrix[16],
             const GLfloat projMatrix[16],
             const GLint viewport[4],
             GLfloat *objx, GLfloat *objy, GLfloat *objz)
{
    float finalMatrix[16];
    float in[4];
    float out[4];
    
    __gluMultMatricesf(modelMatrix, projMatrix, finalMatrix);
    if (!__gluInvertMatrixf(finalMatrix, finalMatrix)) return(GL_FALSE);
    
    in[0]=winx;
    in[1]=winy;
    in[2]=winz;
    in[3]=1.0;
    
    /* Map x and y from window coordinates */
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];
    
    /* Map to range -1 to 1 */
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;
    
    __gluMultMatrixVecf(finalMatrix, in, out);
    if (out[3] == 0.0) return(GL_FALSE);
    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];
    *objx = out[0];
    *objy = out[1];
    *objz = out[2];
    
    
    return(GL_TRUE);
}


/*
    Model convertation tool.
*/
void model_convert( void )
{
    if( c_CommandManager->ArgCount() < 2 ) {
        g_Core->Print( LOG_NONE, "Model convert utility.\n" );
        g_Core->Print( LOG_INFO, "Usage: cm <type> <name>" );
        g_Core->Print( LOG_NONE, "Types available: \n" );
        g_Core->Print( LOG_NONE, "\"1\" - convert .obj to .sm\n" );

        return;
    }

    switch( atoi(c_CommandManager->Arguments(0) ) ) {
        case 1:
            g_Core->Print( LOG_INFO, "Processing \"%s.obj\" ...\n", c_CommandManager->Arguments(1) );
            
            break;

        default:
            g_Core->Print( LOG_ERROR, "model_convert: unknown type\n" );
            break;
    }
}

/*
    Wavefront .obj to sm
*/
typedef struct { int v[3], t[3], n[3]; } polygon_type;
void ncUtils::OBJtoSM( const NString filename ) {
   int         vertex_num  = 0,
    normal_num  = 0,
    polygon_num = 0,
    uv_num      = 0,
    i;

    char        l_char;
    FILE        *l_file;


    // load the file
    l_file = c_FileSystem->OpenRead( NC_TEXT("%s/%s/%s.obj", Filesystem_Path.GetString(), MODEL_FOLDER, filename) );

    if( !l_file  ){
        g_Core->Print( LOG_ERROR, "Couldn't find \"%s.obj\"\n", filename );
        return;
    }

    static char mat_name[32];
    static ncVec3 vertex[1000000];
    static ncVec3 normal_coord[1000000];
    static ncVec2 uv_coord[1000000];
    static polygon_type polygon[1000000];


    while( 1 ) {
        char l_header[128];

        int res = fscanf(l_file, "%s", l_header);
        if (res == EOF)
            break; // End of the file

        if ( !strcmp( l_header, "v" ) )
        {
            fscanf( l_file, "%f %f %f", &vertex[vertex_num].x, &vertex[vertex_num].y,&vertex[vertex_num].z );
            vertex_num++;
        }
        else if ( !strcmp( l_header, "vn" ) ) {
           // fscanf( l_file, "%f %f %f", normal_coord[normal_num].x, &normal_coord[normal_num].y, &normal_coord[normal_num].z );
            normal_num++;
        }
        else if ( !strcmp( l_header, "vt" ) ) {
            fscanf( l_file, "%f %f", &uv_coord[uv_num].x, &
                   uv_coord[uv_num].y );
            uv_num++;
        }
        else if ( !strcmp( l_header, "f" ) )
        {
            for ( i = 0; i < 3; i++ )
            {
                fscanf( l_file, "%c", &l_char );
                fscanf( l_file, "%d", &polygon[polygon_num].v[i] ); // read vertex

                fscanf( l_file, "%c", &l_char );
                fscanf( l_file, "%d", &polygon[polygon_num].t[i] ); // read texture coords ( uv )

                fscanf( l_file, "%c", &l_char );
                fscanf( l_file, "%d", &polygon[polygon_num].n[i] ); // read normals
            }
            polygon_num++;
        }
        else if( !strcmp( l_header, "mat" ) )
            fscanf( l_file, "%s", &mat_name );
    }

    fclose (l_file);

	ncVec3     *tmp_m_vertices;
	ncVec3      *tmp_normals;
    ncVec2      *tmp_uv;

	GLuint      *tmp_faces;

    tmp_m_vertices        = new ncVec3[polygon_num * 3];
    tmp_normals         = new ncVec3[polygon_num * 3];
    tmp_uv              = new ncVec2[polygon_num * 3];
	tmp_faces           = new uint[polygon_num * 3];

	uint num_index = -1;

    int c;
	for ( c = 0; c < polygon_num; c++ ) {
    	for ( i = 0; i < 3; i++ ) {
            num_index++;

            tmp_m_vertices[num_index].x = vertex[ polygon[c].v[i] - 1].x;
            tmp_m_vertices[num_index].y = vertex[ polygon[c].v[i] - 1].y;
            tmp_m_vertices[num_index].z = vertex[ polygon[c].v[i] - 1].z;

            tmp_normals[num_index].x = normal_coord[ polygon[c].n[i] - 1].x;
            tmp_normals[num_index].y = normal_coord[ polygon[c].n[i] - 1].y;
            tmp_normals[num_index].z = normal_coord[ polygon[c].n[i] - 1].z;

            tmp_uv[num_index].x    = uv_coord[ polygon[c].t[i] - 1].x;
            tmp_uv[num_index].y   = uv_coord[ polygon[c].t[i] - 1].y;

            tmp_faces[c * 3 + i] = num_index;

    	}
	}

    ncSM1Header     *header;
    ncSM1Header     outheader;

    FILE            *g_file;
    int             len;
    ncFileChunk     *chunk1, *chunk2, *chunk3;

    header = &outheader;
    memset( header, 0, sizeof(ncSM1Header) );

    header->id          = SM1HEADER;
    header->poly_num    = polygon_num;

    g_stringHelper->SPrintf( header->material, strlen(mat_name) + 1, mat_name );

    g_file  = c_FileSystem->OpenWrite( NC_TEXT("%s/%s.sm1", Filesystem_Path.GetString(), filename) );

    c_FileSystem->Write(g_file, header, sizeof(ncSM1Header));

    len = (num_index+1) * sizeof(ncVec3);

    chunk1              = &header->chunk[0];
    chunk1->offset      = ftell(g_file);
    chunk1->length      = len;
    header->chunk[0]    = *chunk1;

    g_Core->Print(LOG_DEVELOPER, "Getting vertex data ( chunk ofs: %i len: %i )\n", header->chunk[0].offset, header->chunk[0].length);
    c_FileSystem->Write( g_file, tmp_m_vertices, (len+3)&~3 );

    len = (num_index+1) * sizeof(ncVec3);

    chunk2              = &header->chunk[1];
    chunk2->offset      = ftell(g_file);
    chunk2->length      = len;
    header->chunk[1]    = *chunk2;

    g_Core->Print(LOG_DEVELOPER, "Getting normal data ( chunk2 ofs: %i len: %i )\n", header->chunk[1].offset, header->chunk[1].offset);
    c_FileSystem->Write( g_file, tmp_normals, (len+3)&~3 );

    len = (num_index+1) * sizeof(ncVec2);

    chunk3 = &header->chunk[2];
    chunk3->offset      = ftell(g_file);
    chunk3->length      = len;
    header->chunk[2]    = *chunk3;

    g_Core->Print( LOG_DEVELOPER, "Getting UV coordinates ( chunk3 ofs: %i len: %i )\n", header->chunk[2].offset, header->chunk[2].length );
    c_FileSystem->Write( g_file, tmp_uv, (len+2)&~2 );

    // write all changes
    fseek(g_file, 0, SEEK_SET);
    c_FileSystem->Write( g_file, header, sizeof(ncSM1Header) );
    fclose(g_file);

    // - -------------------------------------------------------------------

    free(tmp_m_vertices); tmp_m_vertices    = NULL;
	free(tmp_normals);  tmp_normals     = NULL;
	free(tmp_faces);    tmp_faces       = NULL;
    free(tmp_uv);       tmp_uv          = NULL;

    g_Core->Print( LOG_DEVELOPER, "Model \"%s.obj\" successfully converted to *.sm format ( '%i' polygon faces )\n", filename, polygon_num );
}

