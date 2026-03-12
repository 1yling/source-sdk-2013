//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Automatic reloading of modified materials and textures.
//
//=============================================================================

#include "cbase.h"
#include "c_material_reload_system.h"
#include "filesystem.h"
#include "materialsystem/imaterialsystem.h"
#include "tier1/fileio.h"
#include "tier1/strtools.h"
#include "tier1/utlstring.h"
#include "tier1/utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_autoreload( "mat_autoreload", "1", FCVAR_ARCHIVE, "Automatically reload materials and textures when they are modified on disk." );

CMaterialReloadSystem g_MaterialReloadSystem;

CMaterialReloadSystem::CMaterialReloadSystem() : CAutoGameSystemPerFrame( "MaterialReloadSystem" )
{
}

CMaterialReloadSystem::~CMaterialReloadSystem()
{
}

bool CMaterialReloadSystem::Init()
{
	SetupWatchers();
	return true;
}

void CMaterialReloadSystem::Shutdown()
{
	for ( int i = 0; i < m_Watchers.Count(); ++i )
	{
		delete m_Watchers[i].pWatcher;
	}
	m_Watchers.Purge();
}

void CMaterialReloadSystem::SetupWatchers()
{
	char searchPaths[8192];
	if ( g_pFullFileSystem->GetSearchPath( "GAME", false, searchPaths, sizeof( searchPaths ) ) > 0 )
	{
		CUtlVector<char*> paths;
		V_SplitString( searchPaths, ";", paths );

		for ( int i = 0; i < paths.Count(); ++i )
		{
			// Skip VPKs
			if ( V_stristr( paths[i], ".vpk" ) )
				continue;

			char materialsPath[MAX_PATH];
			V_ComposeFileName( paths[i], "materials", materialsPath, sizeof( materialsPath ) );

			if ( g_pFullFileSystem->IsDirectory( materialsPath, "GAME" ) )
			{
				char osMaterialsPath[MAX_PATH];
				if ( g_pFullFileSystem->GetLocalPath( materialsPath, osMaterialsPath, sizeof( osMaterialsPath ) ) )
				{
					AddWatcherForDirectory( osMaterialsPath );
				}
			}
		}
		for ( int i = 0; i < paths.Count(); ++i )
		{
			delete[] paths[i];
		}
		paths.Purge();
	}
}

void CMaterialReloadSystem::AddWatcherForDirectory( const char *pDirectory )
{
	if ( !pDirectory || !pDirectory[0] )
		return;

	// Check if we already have a watcher for this path
	for ( int i = 0; i < m_Watchers.Count(); ++i )
	{
		if ( !V_stricmp( m_Watchers[i].path.String(), pDirectory ) )
			return;
	}

	CDirWatcher *pWatcher = new CDirWatcher();
	pWatcher->SetDirToWatch( pDirectory );

	int idx = m_Watchers.AddToTail();
	m_Watchers[idx].pWatcher = pWatcher;
	m_Watchers[idx].path = pDirectory;

	Msg( "MaterialReloadSystem: Watching directory: %s\n", pDirectory );
}

void CMaterialReloadSystem::Update( float frametime )
{
	if ( !mat_autoreload.GetBool() )
		return;

	bool bNeedsTextureReload = false;

	for ( int i = 0; i < m_Watchers.Count(); ++i )
	{
		CUtlString changedFile;
		while ( m_Watchers[i].pWatcher->GetChangedFile( &changedFile ) )
		{
			const char *pszFile = changedFile.String();

			if ( V_striEndsWith( pszFile, ".vmt" ) )
			{
				char materialName[MAX_PATH];
				V_strncpy( materialName, pszFile, sizeof( materialName ) );
				V_StripExtension( materialName, materialName, sizeof( materialName ) );
				V_FixSlashes( materialName, '/' );

				Msg( "MaterialReloadSystem: Reloading material: %s\n", materialName );
				materials->ReloadMaterials( materialName );
			}
			else if ( V_striEndsWith( pszFile, ".vtf" ) )
			{
				bNeedsTextureReload = true;
			}
		}
	}

	if ( bNeedsTextureReload )
	{
		Msg( "MaterialReloadSystem: Reloading all textures due to disk changes.\n" );
		materials->ReloadTextures();
	}
}
