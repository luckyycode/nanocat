//
//  Nanocat engine.
//
//  Game model loader & renderer..
//
//  Created by Neko Vision on 3/4/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "Models.h"
#include "Core.h"
#include "AssetManager.h"
#include "ConsoleCommand.h"
#include "Camera.h"
#include "FileSystem.h"
#include "NCString.h"
#include "MaterialLoader.h"
#include "GameWorld.h"
#include "Renderer.h"
#include "LevelEnvironment.h"

ncGLShader            sky;
ncGLShader            fxmodel;

int                 sky_id;

ncModelLoader _modelLoader;

#define MODEL_GAME_FORMAT   "sm1"

void model_spawnit( void ) {
    _modelLoader.Spawn(MODEL_DEFAULT, _commandManager.Arguments(0), "model", _camera.g_vEye, _camera.g_vEye, false);
}

/*
    Initialize, load models.
*/
void ncModelLoader::Initialize( void ) {
    int             i;
    DIR             *dir;
    struct dirent   *ent;

    _core.Print( LOG_INFO, "Loading models....\n" );

    _gameworld.cmodel_count = 0;
    _gameworld.spawned_models = 0;

    _gmodel = (ncPrecachedModel*)malloc(sizeof(ncPrecachedModel) * MAX_MODELS);
    _model = (ncModel*)malloc(sizeof(ncModel) * MAX_MODELS);

    // Load models now
    i = 0;

    // Find all model files in model folder.
    if ( ( dir = opendir( _stringhelper.STR( "%s/%s", Filesystem_Path.GetString(), MODEL_FOLDER ) ) ) != NULL ) {
        while ( ( ent = readdir (dir) ) != NULL ) {
            // TEMP
            if( !strcmp( _filesystem.GetFileExtension( ent->d_name ), MODEL_GAME_FORMAT ) )
            {
                i++;
                Load( (char*)_stringhelper.STR("%s", _filesystem.GetFileName( ent->d_name ) ) );
            }
        }
        closedir (dir);
    }
    else {
        // Failed to get to the directory.
        _core.Error( ERC_FATAL, "Couldn't load files from '%s' folder ( possibly this folder does not exist. )\n", MODEL_FOLDER );
    }

    _core.Print( LOG_INFO, "%i models loaded.\n", i );

    // Load mesh shader.
    _assetmanager.FindShader( "model", &fxmodel );

    glUseProgram( fxmodel.shader_id );
    glUniform1i( glGetUniformLocation( fxmodel.shader_id, "decal" ), 0 );
    glUseProgram( 0 );

    _commandManager.Add( "sm", model_spawnit );
}

/*
    Spawn model.
*/
void ncModelLoader::Spawn( ncModelType type, const char *name, const char *shadername,
                 ncVec3 pos, ncVec3 rot, bool followsplayer ) {
    int i;

    for( i = 0; i < _gameworld.cmodel_count; i++ )
    {
        if( !strcmp( _model[i].m_name, name ))
        {
            _gmodel[_gameworld.spawned_models].g_model         = &_model[i];

            _gmodel[_gameworld.spawned_models].position        = pos;
            _gmodel[_gameworld.spawned_models].rotation        = rot;

            _gmodel[_gameworld.spawned_models].in_use          = true;
            _gmodel[_gameworld.spawned_models].follow_player   = followsplayer;

            // replace previous ( if exists )
            if ( _gmodel[_gameworld.spawned_models].type == MODEL_SKY )
                sky_id = _gameworld.spawned_models;

            if( _gmodel[_gameworld.spawned_models].type != MODEL_SKY )
                _assetmanager.FindShader( shadername, &_gmodel[_gameworld.spawned_models].g_shader );

            _gmodel[_gameworld.spawned_models].type            = type;
            _gameworld.spawned_models++;
            
            _levelenvironment.PassShader( &_gmodel[_gameworld.spawned_models].g_shader.shader_id );

            return;
        }
    }

    _core.Print( LOG_WARN, "Couldn't find \"%s\"\n", name );
}

/*
    Find model by the name.
*/
ncModel ncModelLoader::Find( char *name ) {
    int i;

    for( i = 0; i < _gameworld.cmodel_count; i++ ) {
        if( !strcmp( _model[i].m_name, name )) {
            return _model[i];
        }
    }

    // This is stupid, but okay. Zero is taken by
    // default model.
    return _model[0];
}

/*
    Load the model file.
*/
void ncModelLoader::Load( const char *filename ) {
    ncSM1Header     *head;

    _stringhelper.SPrintf( _model[_gameworld.cmodel_count].m_name, strlen(filename) + 1, filename );
    _filesystem.Load( _stringhelper.STR("%s/%s/%s", Filesystem_Path.GetString(), MODEL_FOLDER, filename), (void**)&head );

    // check the model header
    if (head->id != SM1HEADER) {
#ifdef WARN_LEVEL_ERROR
        _core.Error( ERC_ASSET, "Could not load \"%s\"", filename );
#else
        _core.Print(LOG_INFO, "Could not load \"%s\" - corrupted.\n");
#endif
        return;
    }

    _model[_gameworld.cmodel_count].material = _materials.Find( head->material );

    int            length, ofs;
    ncVec3           *_verts;
    ncVec3           *_normals;
    ncVec2           *_uvs;

    // Load vertices.
    length      = head->chunk[0].length;
    ofs         = head->chunk[0].offset;

    _verts = (ncVec3*)malloc( length * sizeof( ncVec3 ) );
    memcpy (_verts, (Byte*)head + ofs, length);

    // Load normals.
    length      = head->chunk[1].length;
    ofs         = head->chunk[1].offset;

    _normals = (ncVec3*)malloc( length * sizeof( ncVec3 ) );
    memcpy( _normals, (Byte*)head + ofs, length );

    // Load uvs.
    length      = head->chunk[2].length;
    ofs         = head->chunk[2].offset;

    _uvs = (ncVec2*)malloc( length * sizeof( ncVec2 ) );
    memcpy( _uvs, (Byte*)head + ofs, length );

	_model[_gameworld.cmodel_count]._faces = 3 * head->poly_num;

    // - -------------------------------------------------------------------
    // Now we need to setup vertex array object and
    // vertex buffer object.
    glGenVertexArrays( 1, &_model[_gameworld.cmodel_count].m_vao );
    glBindVertexArray( _model[_gameworld.cmodel_count].m_vao );

    // Vertices.
	glGenBuffers(1, &_model[_gameworld.cmodel_count].m_vbo);
    glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _model[_gameworld.cmodel_count].m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _model[_gameworld.cmodel_count]._faces * 3, _verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)NULL );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals.
    glGenBuffers(1, &_model[_gameworld.cmodel_count].m_normals);
    glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, _model[_gameworld.cmodel_count].m_normals);
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, (void*)NULL );
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _model[_gameworld.cmodel_count]._faces * 3, _normals, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Texture coordinates.
    glGenBuffers(1, &_model[_gameworld.cmodel_count].m_uv);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, _model[_gameworld.cmodel_count].m_uv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _model[_gameworld.cmodel_count]._faces * 2, _uvs, GL_STATIC_DRAW);
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)NULL );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray( 0 );

    free( _verts );
    free( _normals );
    free( _uvs );

    _gameworld.cmodel_count++;
}

/*
    Render the sky.
*/
void model_sky_render( void ) {

}

/*
    Render models.
*/
void ncModelLoader::Render( bool reflection, ncSceneEye eye ) {
    int i;

    if( !_gameworld.Active )
        return;

    glUseProgram( fxmodel.shader_id  );

    for( i = 0; i < _gameworld.spawned_models; i++ ) {
        if( !_gmodel[i].in_use )
            continue;

        // Ignore the sky model to prevent bothering.
        if( i == sky_id )
            continue;
        
        
        float defaultScale = 0.1f;
        
        ncVec3 reflectedView = ncVec3( defaultScale, -defaultScale, defaultScale );
        ncVec3 defaultView = ncVec3( defaultScale, defaultScale, defaultScale );

        ncMatrix4 model, pos;

        model.Identity();
        pos.Identity();

        pos.Translate( _gmodel[i].position );
        
        if( reflection )
            pos.Scale( reflectedView );
        else
            pos.Scale( defaultView );

        model = model * _camera.ViewMatrix;
        pos = model * pos;
        
        float ipd = 10.64;
        ncVec3 offset = ncVec3( ipd / 2.0f, 0.0f, 0.0f );
        ncVec3 minus_offset = ncVec3( -(ipd / 2.0f), 0.0f, 0.0f );
        
        ncMatrix4 ls = ncMatrix4();
        ncMatrix4 rs = ncMatrix4();
        ls.Translate( offset );
        rs.Translate( minus_offset );
        
        pos = eye == EYE_LEFT ? ls * pos : rs * pos;

        glUniformMatrix4fv( glGetUniformLocation( fxmodel.shader_id, "ProjMatrix" ), 1, false, _camera.ProjectionMatrix.m );
        glUniformMatrix4fv( glGetUniformLocation( fxmodel.shader_id, "ModelMatrix" ), 1, false, pos.m );
        
        glUniform3f( glGetUniformLocation( fxmodel.shader_id, "cameraPos" ), _camera.g_vEye.x, _camera.g_vEye.y, _camera.g_vEye.z );
        
        _levelenvironment.PassShader( &fxmodel.shader_id );
        
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _gmodel[i].g_model->material.texture.tex_id );

        glUniform1f( glGetUniformLocation( _gmodel[i].g_shader.shader_id, "time"), (float) _core.Time / 1000.0 );

        glBindVertexArray( _gmodel[i].g_model->m_vao );

        switch ( render_wireframe.GetInteger() ) {
            case 0:  glDrawArrays( GL_TRIANGLES, 0, _gmodel[i].g_model->_faces );
                break;
            case 1:  glDrawArrays( GL_LINES, 0, _gmodel[i].g_model->_faces );
                break;
            case 2:  glDrawArrays( GL_POINTS, 0, _gmodel[i].g_model->_faces );
                break;
            default: glDrawArrays( GL_TRIANGLES, 0, _gmodel[i].g_model->_faces );
                break;
        }

        glBindVertexArray( 0 );

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    glUseProgram(0);
}

