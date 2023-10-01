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

#include <stdio.h>
#include "server_account.h"
#include "utils/module.h"

ServerAccount g_ServerAccount;
PLUGIN_EXPOSE(ServerAccount, g_ServerAccount);
IVEngineServer2* engine = nullptr;
ICvar* icvar = nullptr;
CSteam3ServerS1* g_pSteam3Server = nullptr;

SH_DECL_HOOK0(ISource2ServerConfig, GetGameDescription, SH_NOATTRIB, 0, const char*);
int iLogOnAnonymousHookID = -1;
SH_DECL_HOOK0_void(ISteamGameServer, LogOnAnonymous, SH_NOATTRIB, 0);

bool ServerAccount::Load(PluginId id, ISmmAPI* ismm, char* error, size_t maxlen, bool late)
{
	PLUGIN_SAVEVARS();

	if (late)
	{
		V_strncpy(error, "Late load is not supported", maxlen);
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);

		return false;
	}

	GET_V_IFACE_ANY(GetEngineFactory, engine, IVEngineServer2, INTERFACEVERSION_VENGINESERVER);
	GET_V_IFACE_CURRENT(GetEngineFactory, icvar, ICvar, CVAR_INTERFACE_VERSION);
	GET_V_IFACE_CURRENT(GetServerFactory, g_pSource2ServerConfig, ISource2ServerConfig, INTERFACEVERSION_SERVERCONFIG);
	
	CModule libengine(engine);

	CMemory fn = libengine.FindPatternSIMD(WIN_LINUX("40 53 48 83 EC 20 33 D2 48 8D 0D", "E8 ? ? ? ? 48 89 C7 E8 ? ? ? ? 5D 48 8B 00 C3"));
	if (!fn)
	{
		V_strncpy(error, "Failed to find function to get Steam3Server", maxlen);
		ConColorMsg(Color(255, 0, 0, 255), "[%s] %s\n", GetLogTag(), error);
		
		return false;
	}

	g_pSteam3Server = fn.WIN_LINUX(Offset(0xB).ResolveRelativeAddress().RCast<CSteam3ServerS1*>(),FollowNearCall().RCast<CSteam3ServerS1*(*)()>()());

	SH_ADD_HOOK(ISource2ServerConfig, GetGameDescription, g_pSource2ServerConfig, SH_MEMBER(this, &ServerAccount::GetGameDescription), true);

	g_pCVar = icvar;
	ConVar_Register(FCVAR_RELEASE | FCVAR_GAMEDLL);

	return true;
}

bool ServerAccount::Unload(char *error, size_t maxlen)
{
	SH_REMOVE_HOOK(ISource2ServerConfig, GetGameDescription, g_pSource2ServerConfig, SH_MEMBER(this, &ServerAccount::GetGameDescription), true);
	if (iLogOnAnonymousHookID != -1)
	{
		SH_REMOVE_HOOK_ID(iLogOnAnonymousHookID);
	}
	
	ConVar_Unregister();
	
	return true;
}

void ServerAccount::SetAccount(const char* pszToken)
{
	m_sAccountToken = pszToken;
}

const char* ServerAccount::GetGameDescription()
{
	if (g_pSteam3Server->SteamGameServer())
	{
		iLogOnAnonymousHookID = SH_ADD_VPHOOK(ISteamGameServer, LogOnAnonymous, g_pSteam3Server->SteamGameServer(), SH_MEMBER(this, &ServerAccount::LogOnAnonymous), false);
		
		SH_REMOVE_HOOK(ISource2ServerConfig, GetGameDescription, g_pSource2ServerConfig, SH_MEMBER(this, &ServerAccount::GetGameDescription), true);
	}

	RETURN_META_VALUE(MRES_IGNORED, nullptr);
}

void ServerAccount::LogOnAnonymous()
{
	if (m_sAccountToken.IsEmpty())
	{
		Warning("****************************************************\n");
		Warning("*                                                  *\n");
		Warning("*  No Steam account token was specified.           *\n");
		Warning("*  Logging into anonymous game server account.     *\n");
		Warning("*  Connections will be restricted to LAN only.     *\n");
		Warning("*                                                  *\n");
		Warning("*  To create a game server account go to           *\n");
		Warning("*  http://steamcommunity.com/dev/managegameservers *\n");
		Warning("*                                                  *\n");
		Warning("****************************************************\n");
	}
	else
	{
		Msg("Logging into Steam gameserver account with logon token '%.8sxxxxxxxxxxxxxxxxxxxxxxxx'\n", m_sAccountToken.String());
		
		META_IFACEPTR(ISteamGameServer)->LogOn(m_sAccountToken);

		RETURN_META(MRES_SUPERCEDE);
	}
}

CON_COMMAND_F(sv_setsteamaccount, "token\nSet game server account token to use for logging in to a persistent game server account", FCVAR_NONE)
{
	if(g_pSteam3Server->SteamGameServer() && g_pSteam3Server->SteamGameServer()->BLoggedOn())
	{
		Warning( "Warning: Game server already logged into steam.  You need to use the sv_setsteamaccount command earlier.\n" );
		return;
	}

	if (args.ArgC() != 2)
	{
		Warning("Usage: sv_setsteamaccount <login_token>\n");
		return;
	}

	g_ServerAccount.SetAccount(args[1]);
}

///////////////////////////////////////
const char* ServerAccount::GetLicense()
{
	return "GPL";
}

const char* ServerAccount::GetVersion()
{
	return "1.0.0";
}

const char* ServerAccount::GetDate()
{
	return __DATE__;
}

const char *ServerAccount::GetLogTag()
{
	return "ServerAccount";
}

const char* ServerAccount::GetAuthor()
{
	return "Phoenix (˙·٠●Феникс●٠·˙)";
}

const char* ServerAccount::GetDescription()
{
	return "Adds sv_setsteamaccount command";
}

const char* ServerAccount::GetName()
{
	return "Server account";
}

const char* ServerAccount::GetURL()
{
	return "https://github.com/komashchenko/ServerAccount";
}
