//
//  Nanocat engine.
//
//  Main game asset manager..
//
//  Created by Neko Vision on 1/22/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "AssetManager.h"
#include "Models.h"
#include "Core.h"
#include "MaterialLoader.h"
#include "MultiplayerServer.h"
#include "CoreFont.h"
#include "GameWorld.h"
#include "ncBSP.h"
#include "Terrain.h"

ncAssetManager local_assetManager;
ncAssetManager *f_AssetManager = &local_assetManager;

/*
    Initialize asset system & load initialize..
*/
void ncAssetManager::Initialize( void ) {
    g_Core->Print( LOG_INFO, "Asset manager initializing...\n" );
    g_Core->Print( LOG_INFO, "Loading assets...\n" );
    
    // Initialize shader system.
    g_shaderManager->Initialize();
    
    // Some core shaders.
    // Load shaders first because some another assets may require them.
    //f_AssetManager->Load( ASSET_SHADER, "oculusvr" ); // Not really needed, already passes in 2d screen shader.
    f_AssetManager->Load( ASSET_SHADER, "passthru" );
    f_AssetManager->Load( ASSET_SHADER, "font" );
    // Not really core ones, user could make a custom ones.
    f_AssetManager->Load( ASSET_SHADER, "terrain" );
    f_AssetManager->Load( ASSET_SHADER, "water" );
    f_AssetManager->Load( ASSET_SHADER, "level" );
    
    // Initialize materials and load textures.
    g_materialManager->Initialize();
    
    // Load material pack.
    f_AssetManager->Load( ASSET_MATERIAL, "main" );
}

/*
    Load chosen asset.
*/
void ncAssetManager::Load( ncAssetType type, const NString name ) {

    // Asset loading is called from renderer initialization, buuuut...
    // wtf, probably wrong place to do this.
    if( Server_Dedicated.GetInteger() )
        return;

    if( !name ) {
        g_Core->Error( ERR_ASSET, "ncAssetLoader:Load - Empty asset name given." );
        return;
    }

    // There's no image asset, it's called material.
    switch( type ) {
        case ASSET_MATERIAL:
            g_materialManager->Load( name );
            break;
        case ASSET_SHADER:
            g_shaderManager->Load( name );
            break;
        case ASSET_MODEL:
            g_modelManager->Load( name );
            break;
        case ASSET_SOUND:
            // a long story
            // ...
            break;

        default:
            g_Core->Error( ERR_ASSET, "ncAssetLoader::Load - Unknown asset type given." );
            break;
    }
}


/*
    Find shader by name.
*/
ncGLShader *ncAssetManager::FindShaderByName( const NString name ) {
    int i;
    
    for( i = 0; i < g_shaderManager->shaderCount; i++ ) {
        if( !strcmp( g_shaderManager->shaders[i].GetName(), name ) ) {
            return &g_shaderManager->shaders[i];
        }
    }
    
    g_Core->Print( LOG_WARN, "FindShaderByName: Could not find \"%s\" shader, trying to load it again...\n", name );
    
    // Well, load it now.
    Load( ASSET_SHADER, name );
    
    for( i = 0; i <  g_shaderManager->shaderCount; i++ ) {
        if( !strcmp( g_shaderManager->shaders[i].GetName(), name ) ) {
            return &g_shaderManager->shaders[i];
        }
    }
    
    // No way.
    g_Core->Error( ERR_ASSET, "ncAssetManager::FindShaderByName - Couldn't find %s shader.\n", name );
    
    return NULL;
}



/*
    Find shader by name.
*/
void ncAssetManager::FindShaderByName( const NString name, ncGLShader *shader ) {
    int i;

    for( i = 0; i < g_shaderManager->shaderCount; i++ ) {
        if( !strcmp( g_shaderManager->shaders[i].GetName(), name ) ) {
            shader = &g_shaderManager->shaders[i];
            return;
        }
    }

    g_Core->Print( LOG_WARN, "FindShaderByName: Could not find \"%s\" shader, trying to load it again...\n", name );
    
    // Well, load it now.
    Load( ASSET_SHADER, name );

    // Check now.
    for( i = 0; i <  g_shaderManager->shaderCount; i++ ) {
        if( !strcmp( g_shaderManager->shaders[i].GetName(), name ) ) {
            *shader = g_shaderManager->shaders[i];
            return;
        }
    }
    
    // No way.
    g_Core->Error( ERR_ASSET, "ncAssetManager::FindShaderByName - Couldn't find %s shader.\n", name );
}


/*
    Check if chosen asset exists.
*/
bool ncAssetManager::Exists( ncAssetType type, NString name ) {
    if( !name ) {
        g_Core->Print( LOG_WARN, "ncAssetManager::Exists - empty name given.\n" );
        NC_ASSERTWARN( name );
        return false;
    }
    
    switch( type ) {
        case ASSET_SHADER:
            for ( int i = 0; i <  g_shaderManager->shaderCount; i++ ) {
                if ( !strcmp(  g_shaderManager->shaders[i].GetName(), name ) ) {
                    return true;
                }
            }
            break;
        // Not really necessary.
        case ASSET_MATERIAL:
            for( int i = 0; i < g_materialManager->MaterialCount; i++ ) {
                if( !strcmp( g_materialManager->m_Materials[i].Name, name ) ) {
                    return true;
                }
            }
            break;
        case ASSET_MODEL:
            
            break;
        case ASSET_SOUND:
            
            break;
        default:
            g_Core->Print( LOG_WARN, "ncAssetManager::Exists - unknown asset type given.\n" );
            return false;
    }

    return false;
}
