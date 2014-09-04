//
//  Nanocat engine.
//
//  Main game asset manager.
//
//  Created by Neko Vision on 1/22/14.
//  Copyright (c) 2014 Neko Vision. All rights reserved.
//

#include "assetmanager.h"
#include "model.h"
#include "core.h"
#include "material.h"
#include "server.h"

ncAssetManager _assetmanager;

/*
    Initialize asset system.
*/
void ncAssetManager::Initialize( void ) {

}

/*
    Load chosen asset.
*/
void ncAssetManager::Load( assettype_t type, const char *name ) {

    // wtf
    if( server_dedi.GetInteger() )
        return;

    if( !name ) {
        _core.Error( ERC_ASSET, "AssetLoad: empty asset name given." );
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
            _core.Error( ERC_ASSET, "AssetLoad: unknown asset type given." );
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

    _core.Print( LOG_WARN, "Could not find \"%s\", trying to load it.\n", name );
    Load( ASSET_SHADER, name );

    for( i = 0; i <  _shaderManager.shaderCount; i++ ) {
        if( !strcmp( _shaderManager.shaders[i].s_name, name ) ) {
            //_core.Print("found %s (index: %i)\n", shader, i );
            *shader = _shaderManager.shaders[i];
            return;
        }
    }
}


/*
    Check if chosen asset exists.
*/
bool ncAssetManager::Exists( assettype_t type, char *name ) {
    if( !name )
        return false;

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
    }

    return false;
}
