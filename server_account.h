/**
 * vim: set ts=4 sw=4 tw=99 noet :
 * ======================================================
 * Server account
 * Written by Phoenix (˙·٠●Феникс●٠·˙) 2023.
 * ======================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 * 
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from 
 * the use of this software.
 */

#ifndef _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
#define _INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_

#include <ISmmPlugin.h>
#include "steam/isteamgameserver.h"

#ifdef _WIN32
#define WIN_LINUX(win, linux) win
#else
#define WIN_LINUX(win, linux) linux
#endif

class ServerAccount final : public ISmmPlugin, public IMetamodListener
{
public:
	bool Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late);
	bool Unload(char* error, size_t maxlen);
	void SetAccount(const char* pszToken);
	
private:
	const char* GetAuthor();
	const char* GetName();
	const char* GetDescription();
	const char* GetURL();
	const char* GetLicense();
	const char* GetVersion();
	const char* GetDate();
	const char* GetLogTag();

private: // Hooks
	const char* GetGameDescription();
	void LogOnAnonymous();

	CUtlString m_sAccountToken;
};

class CSteam3ServerS1 : public CSteamGameServerAPIContext
{
};

#endif //_INCLUDE_METAMOD_SOURCE_STUB_PLUGIN_H_
