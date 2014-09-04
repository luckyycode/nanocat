//
//  Nanocat engine.
//
//  Utilities.
//
//  Created by Neko Code on 4/25/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "core.h"
#include "command.h"
#include "files.h"
#include "ncstring.h"
#include "model.h"

/*
static const char hex_chars[] = "0123456789abcdef";

static inline char *hex_encode(const char *data, unsigned int data_size)
{
    char *ret;
    char buf2[3];
    buf2[2] = '\0';
    
    for(unsigned int i = 0; i < data_size; i++)
    {
        unsigned char c = (unsigned char) data[i];
        buf2[0] = hex_chars[(c & 0xf0) >> 4];
        buf2[1] = hex_chars[c & 0x0f];
        //ret.append(buf2);
    }
    
    return ret;
}

static inline std::string hex_encode(const std::string &data)
{
    return hex_encode(data.c_str(), data.size());
}

static inline bool hex_digit_decode(char hexdigit, unsigned char &value)
{
    if(hexdigit >= '0' && hexdigit <= '9')
        value = hexdigit - '0';
    else if(hexdigit >= 'A' && hexdigit <= 'F')
        value = hexdigit - 'A' + 10;
    else if(hexdigit >= 'a' && hexdigit <= 'f')
        value = hexdigit - 'a' + 10;
    else
        return false;
    return true;
}
*/


/*
    Model convertation tool.
*/
void model_convert( void )
{
    if( _commandManager.ArgCount() < 2 ) {
        _core.Print( LOG_NONE, "Model convert utility.\n" );
        _core.Print( LOG_INFO, "Usage: cm <type> <name>" );
        _core.Print( LOG_NONE, "Types available: \n" );
        _core.Print( LOG_NONE, "\"1\" - convert .obj to .sm\n" );

        return;
    }

    switch( atoi(_commandManager.Arguments(0) ) ) {
        case 1:
            _core.Print( LOG_INFO, "Processing \"%s.obj\" ...\n", _commandManager.Arguments(1) );
            model_obj2sm( _commandManager.Arguments(1) );
            break;

        default:
            _core.Print( LOG_ERROR, "model_convert: unknown type\n" );
            break;
    }
}

/*
    Wavefront .obj to sm
*/
void model_obj2sm( const char *filename )
{
 /*   int         vertex_num  = 0,
    normal_num  = 0,
    polygon_num = 0,
    uv_num      = 0,
    i;

    char        l_char;
    FILE        *l_file;


    // load the file
    l_file = _filesystem.OpenRead( _stringhelper.STR("%s/%s/%s.obj", filesystem_path->string, MODEL_FOLDER, filename) );

    if( !l_file  ){
        _core.Print( LOG_ERROR, "Couldn't find \"%s.obj\"\n", filename );
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
            fscanf( l_file, "%f %f %f", &vertex[vertex_num].GetX(), &vertex[vertex_num].GetY() ,&vertex[vertex_num].GetZ() );
            vertex_num++;
        }
        else if ( !strcmp( l_header, "vn" ) ) {
            fscanf( l_file, "%f %f %f", normal_coord[normal_num].GetX(), &normal_coord[normal_num].GetY(), &normal_coord[normal_num].GetZ() );
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

	ncVec3     *tmp_vertices;
	ncVec3      *tmp_normals;
    ncVec2      *tmp_uv;

	GLuint      *tmp_faces;

	tmp_vertices        = (ncVec3*)malloc( sizeof(ncVec3) * polygon_num * 3 );
	tmp_normals         = (ncVec3*)malloc( sizeof(ncVec3) * polygon_num * 3 );
    tmp_uv              = (ncVec2*)malloc( sizeof(ncVec2) * polygon_num * 3 );
	tmp_faces           = (uint*)malloc( sizeof(uint) * polygon_num * 3 );

	uint num_index = -1;

    int c;
	for ( c = 0; c < polygon_num; c++ ) {
    	for ( i = 0; i < 3; i++ ) {
            num_index++;

            tmp_vertices[num_index].x = vertex[ polygon[c].v[i] - 1].x;
            tmp_vertices[num_index].y = vertex[ polygon[c].v[i] - 1].y;
            tmp_vertices[num_index].z = vertex[ polygon[c].v[i] - 1].z;

            tmp_normals[num_index].x = normal_coord[ polygon[c].n[i] - 1].x;
            tmp_normals[num_index].y = normal_coord[ polygon[c].n[i] - 1].y;
            tmp_normals[num_index].z = normal_coord[ polygon[c].n[i] - 1].z;

            tmp_uv[num_index].x    = uv_coord[ polygon[c].t[i] - 1].x;
            tmp_uv[num_index].y   = uv_coord[ polygon[c].t[i] - 1].y;

            tmp_faces[c * 3 + i] = num_index;

    	}
	}

    sm1header_t     *header;
    sm1header_t     outheader;

    FILE            *g_file;
    int             len;
    filechunk_t     *chunk1, *chunk2, *chunk3;

    header = &outheader;
    memset( header, 0, sizeof(sm1header_t) );

    header->id          = SM1HEADER;
    header->poly_num    = polygon_num;

    _stringhelper.SPrintf( header->material, strlen(mat_name) + 1, mat_name );

    g_file  = _filesystem.OpenWrite( _stringhelper.STR("%s/%s.sm1", filesystem_path->string, filename) );

    _filesystem.Write(g_file, header, sizeof(sm1header_t));

    len = (num_index+1) * sizeof(ncVec3);

    chunk1              = &header->chunk[0];
    chunk1->offset      = ftell(g_file);
    chunk1->length      = len;
    header->chunk[0]    = *chunk1;

    _core.Print(LOG_DEVELOPER, "Getting vertex data ( chunk ofs: %i len: %i )\n", header->chunk[0].offset, header->chunk[0].length);
    _filesystem.Write( g_file, tmp_vertices, (len+3)&~3 );

    len = (num_index+1) * sizeof(ncVec3);

    chunk2              = &header->chunk[1];
    chunk2->offset      = ftell(g_file);
    chunk2->length      = len;
    header->chunk[1]    = *chunk2;

    _core.Print(LOG_DEVELOPER, "Getting normal data ( chunk2 ofs: %i len: %i )\n", header->chunk[1].offset, header->chunk[1].offset);
    _filesystem.Write( g_file, tmp_normals, (len+3)&~3 );

    len = (num_index+1) * sizeof(ncVec2);

    chunk3 = &header->chunk[2];
    chunk3->offset      = ftell(g_file);
    chunk3->length      = len;
    header->chunk[2]    = *chunk3;

    _core.Print( LOG_DEVELOPER, "Getting UV coordinates ( chunk3 ofs: %i len: %i )\n", header->chunk[2].offset, header->chunk[2].length );
    _filesystem.Write( g_file, tmp_uv, (len+2)&~2 );

    // write all changes
    fseek(g_file, 0, SEEK_SET);
    _filesystem.Write( g_file, header, sizeof(sm1header_t) );
    fclose(g_file);

    // - -------------------------------------------------------------------

    free(tmp_vertices); tmp_vertices    = NULL;
	free(tmp_normals);  tmp_normals     = NULL;
	free(tmp_faces);    tmp_faces       = NULL;
    free(tmp_uv);       tmp_uv          = NULL;

    _core.Print( LOG_DEVELOPER, "Model \"%s.obj\" successfully converted to *.sm format ( '%i' polygon faces )\n", filename, polygon_num );*/
}

