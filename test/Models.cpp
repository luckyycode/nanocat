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

ncGLShader            *sky;
ncGLShader            *fxmodel;

int                 sky_id;

ncModelLoader local_modelLoader;
ncModelLoader *g_modelManager = &local_modelLoader;

#define MODEL_GAME_FORMAT   "sm1"

void model_spawnit( void ) {
    g_modelManager->Spawn(MODEL_DEFAULT, c_CommandManager->Arguments(0), "model", g_playerCamera->g_vEye, g_playerCamera->g_vEye, false);
}

/*
    Initialize, load models.
*/
void ncModelLoader::Initialize( void ) {
    int             i;
    DIR             *dir;
    struct dirent   *ent;

    g_Core->Print( LOG_INFO, "Loading models....\n" );

    g_gameWorld->cmodel_count = 0;
    g_gameWorld->spawned_models = 0;

    _gmodel = new ncPrecachedModel[MAX_MODELS];
    _model = new ncModel[MAX_MODELS];
    
    // Load models now
    i = 0;

    // Find all model files in model folder.
    if ( ( dir = opendir( _stringhelper.STR( "%s/%s", Filesystem_Path.GetString(), MODEL_FOLDER ) ) ) != NULL ) {
        while ( ( ent = readdir (dir) ) != NULL ) {
            // TEMP
            if( !strcmp( c_FileSystem->GetFileExtension( ent->d_name ), MODEL_GAME_FORMAT ) )
            {
                i++;
                Load( (char*)_stringhelper.STR("%s", c_FileSystem->GetFileName( ent->d_name ) ) );
            }
        }
        closedir (dir);
    }
    else {
        // Failed to get to the directory.
        g_Core->Error( ERR_FATAL, "Couldn't load files from '%s' folder ( possibly this folder does not exist. )\n", MODEL_FOLDER );
    }

    g_Core->Print( LOG_INFO, "%i models loaded.\n", i );

    // Load mesh shader.
    //f_AssetManager->FindShaderByName( "model", &fxmodel );

    //glUseProgram( fxmodel.Id );
    //glUniform1i( glGetUniformLocation( fxmodel.Id, "decal" ), 0 );
    //glUseProgram( 0 );

    c_CommandManager->Add( "sm", model_spawnit );
}

/*  
    Remove all active spawned models.
*/
void ncModelLoader::RemoveSpawnedModels( void ) {
    g_Core->Print( LOG_DEVELOPER, "Removing all spawned models...\n" );
}

/*
    Spawn model.
*/
void ncModelLoader::Spawn( ncModelType type, const NString name, const NString shadername,
                 ncVec3 pos, ncVec3 rot, bool followsplayer ) {
    int i;

    for( i = 0; i < g_gameWorld->cmodel_count; i++ )
    {
        if( !strcmp( _model[i].m_name, name ))
        {
            int num  = g_gameWorld->spawned_models;
            _gmodel[num].g_model         = &_model[i];

            _gmodel[num].position        = pos;
            _gmodel[num].rotation        = rot;

            _gmodel[num].in_use          = true;
            _gmodel[num].follow_player   = followsplayer;

            // replace previous ( if exists )
            if ( _gmodel[num].type == MODEL_SKY )
                sky_id = num;

            if( _gmodel[num].type != MODEL_SKY )
                _gmodel[num].g_shader = f_AssetManager->FindShaderByName( shadername );

            _gmodel[num].type            = type;
            g_gameWorld->spawned_models++;
            
            //g_LevelEnvironment->PassShader( &_gmodel[num].g_shader->GetId() );

            return;
        }
    }

    g_Core->Print( LOG_WARN, "Couldn't find \"%s\"\n", name );
}

/*
    Find model by the name.
*/
ncModel ncModelLoader::Find( NString name ) {
    int i;

    for( i = 0; i < g_gameWorld->cmodel_count; i++ ) {
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
void ncModelLoader::Load( const NString filename ) {
    ncSM1Header     *head;

    _stringhelper.SPrintf( _model[g_gameWorld->cmodel_count].m_name, strlen(filename) + 1, filename );
    c_FileSystem->Load( _stringhelper.STR("%s/%s/%s", Filesystem_Path.GetString(), MODEL_FOLDER, filename), (void**)&head );

    // check the model header
    if (head->id != SM1HEADER) {
        g_Core->Error( ERR_ASSET, "Could not load \"%s\"", filename );
        return;
    }

    _model[g_gameWorld->cmodel_count].material = g_materialManager->Find( head->material );

    int            length, ofs;
    ncVec3           *_verts;
    ncVec3           *_normals;
    ncVec2           *_uvs;

    // Load m_vertices.
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

	_model[g_gameWorld->cmodel_count]._faces = 3 * head->poly_num;

    // - -------------------------------------------------------------------
    // Now we need to setup vertex array object and
    // vertex buffer object.
    glGenVertexArrays( 1, &_model[g_gameWorld->cmodel_count].m_vao );
    glBindVertexArray( _model[g_gameWorld->cmodel_count].m_vao );

    // Vertices.
	glGenBuffers(1, &_model[g_gameWorld->cmodel_count].m_vbo);
    glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, _model[g_gameWorld->cmodel_count].m_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _model[g_gameWorld->cmodel_count]._faces * 3, _verts, GL_STATIC_DRAW);
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, (void*)NULL );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Normals.
    glGenBuffers(1, &_model[g_gameWorld->cmodel_count].m_normals);
    glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, _model[g_gameWorld->cmodel_count].m_normals);
	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, 0, (void*)NULL );
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _model[g_gameWorld->cmodel_count]._faces * 3, _normals, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Texture coordinates.
    glGenBuffers(1, &_model[g_gameWorld->cmodel_count].m_uv);
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, _model[g_gameWorld->cmodel_count].m_uv);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * _model[g_gameWorld->cmodel_count]._faces * 2, _uvs, GL_STATIC_DRAW);
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, (void*)NULL );
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray( 0 );

    free( _verts );
    free( _normals );
    free( _uvs );

    g_gameWorld->cmodel_count++;
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

    if( !g_gameWorld->Active )
        return;

    fxmodel->Use();

    for( i = 0; i < g_gameWorld->spawned_models; i++ ) {
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

        model = model * g_playerCamera->ViewMatrix;
        pos = model * pos;
        
        float ipd = 10.64;
        ncVec3 offset = ncVec3( ipd / 2.0f, 0.0f, 0.0f );
        ncVec3 minus_offset = ncVec3( -(ipd / 2.0f), 0.0f, 0.0f );
        
        ncMatrix4 ls = ncMatrix4();
        ncMatrix4 rs = ncMatrix4();
        ls.Translate( offset );
        rs.Translate( minus_offset );
        
        pos = eye == EYE_LEFT ? ls * pos : rs * pos;

        fxmodel->SetUniform( "ProjMatrix", 1, false, g_playerCamera->ProjectionMatrix.m );
        fxmodel->SetUniform( "ModelMatrix", 1, false, pos.m );
        fxmodel->SetUniform( "cameraPos", g_playerCamera->g_vEye );
        
        //g_LevelEnvironment->PassShader( &fxmodel->Id );
        
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, _gmodel[i].g_model->material->Image.TextureID );

        fxmodel->SetUniform( "time", (float)g_Core->Time / 1000.0f );

        glBindVertexArray( _gmodel[i].g_model->m_vao );

        if( Render_Wireframe.GetInteger() == 0 ) {
            glDrawArrays( GL_TRIANGLES, 0, _gmodel[i].g_model->_faces );
        } else if( Render_Wireframe.GetInteger() == 1 ) {
            glDrawArrays( GL_LINES, 0, _gmodel[i].g_model->_faces );
        } else if ( Render_Wireframe.GetInteger() == 2 ) {
            glDrawArrays( GL_POINTS, 0, _gmodel[i].g_model->_faces );
        } else {
            glDrawArrays( GL_TRIANGLES, 0, _gmodel[i].g_model->_faces );
        }

        glBindVertexArray( 0 );

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    glUseProgram(0);
}

