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

ncAssetManager _assetmanager;

/*
    Initialize asset system.
*/
void ncAssetManager::Initialize( void ) {
    _core.Print( LOG_INFO, "Asset manager initializing...\n" );
}

/*
    Load chosen asset.
*/
void ncAssetManager::Load( AssetType type, const char *name ) {

    // wtf, probably wrong place to do this.
    if( Server_Dedicated.GetInteger() )
        return;

    if( !name ) {
        _core.Error( ERC_ASSET, "ncAssetLoader:Load - Empty asset name given." );
        return;
    }

    switch( type ) {
        case ASSET_MATERIAL:
            _materials.Load( name );
            break;
        case ASSET_SHADER:
            _shaderManager.Load( name );
            break;
        case ASSET_MODEL:
            _modelLoader.Load( name );
            break;
        case ASSET_SOUND:
            // a long story
            break;

        default:
            _core.Error( ERC_ASSET, "ncAssetLoader::Load - Unknown asset type given." );
            break;
    }
}


/*
    Find shader.
*/
void ncAssetManager::FindShader( const char *name, ncGLShader *shader ) {
    int i;

    for( i = 0; i < _shaderManager.shaderCount; i++ ) {
        if( !strcmp( _shaderManager.shaders[i].s_name, name ) ) {
            *shader = _shaderManager.shaders[i];
            return;
        }
    }

    _core.Print( LOG_WARN, "FindShader: Could not find \"%s\" shader, trying to load it...\n", name );
    
    // Well, load it now.
    Load( ASSET_SHADER, name );

    for( i = 0; i <  _shaderManager.shaderCount; i++ ) {
        if( !strcmp( _shaderManager.shaders[i].s_name, name ) ) {
            *shader = _shaderManager.shaders[i];
            return;
        }
    }
    
    // No way.
    _core.Error( ERC_ASSET, "ncAssetManager::FindShader - Couldn't find %s shader.\n", name );
}


/*
    Check if chosen asset exists.
*/
bool ncAssetManager::Exists( AssetType type, char *name ) {
    if( !name ) {
        _core.Print( LOG_WARN, "ncAssetManager::Exists - empty name given.\n" );
        return false;
    }
    
    int i;

    switch( type ) {
        case ASSET_SHADER:
            for ( i = 0; i <  _shaderManager.shaderCount; i++ ) {
                if ( !strcmp(  _shaderManager.shaders[i].s_name, name ) )
                    return true;
            }
            break;
        case ASSET_IMAGE:
            
            break;
        case ASSET_MATERIAL:
            
            break;
        case ASSET_MODEL:
            
            break;
        case ASSET_SOUND:
            
            break;
        default:
            _core.Print( LOG_WARN, "ncAssetManager::Exists - unknown asset type given.\n" );
            return false;
    }

    return false;
}
