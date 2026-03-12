//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Automatic reloading of modified materials and textures.
//
//=============================================================================

#ifndef C_MATERIAL_RELOAD_SYSTEM_H
#define C_MATERIAL_RELOAD_SYSTEM_H

#ifdef _WIN32
#pragma once
#endif

#include "igamesystem.h"
#include "tier1/utlvector.h"
#include "tier1/utlstring.h"

class CDirWatcher;

class CMaterialReloadSystem : public CAutoGameSystemPerFrame
{
public:
	CMaterialReloadSystem();
	~CMaterialReloadSystem();

	// IGameSystem methods
	virtual bool Init();
	virtual void Shutdown();
	virtual void Update( float frametime );

private:
	void SetupWatchers();
	void AddWatcherForDirectory( const char *pDirectory );

	struct WatcherInfo_t
	{
		CDirWatcher *pWatcher;
		CUtlString path;
	};

	CUtlVector<WatcherInfo_t> m_Watchers;
};

#endif // C_MATERIAL_RELOAD_SYSTEM_H
