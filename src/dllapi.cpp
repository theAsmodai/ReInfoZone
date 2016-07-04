/*
 * Copyright (c) 2001-2003 Will Day <willday@hpgx.net>
 *
 *    This file is part of Metamod.
 *
 *    Metamod is free software; you can redistribute it and/or modify it
 *    under the terms of the GNU General Public License as published by the
 *    Free Software Foundation; either version 2 of the License, or (at
 *    your option) any later version.
 *
 *    Metamod is distributed in the hope that it will be useful, but
 *    WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Metamod; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *    In addition, as a special exception, the author gives permission to
 *    link the code of this program with the Half-Life Game Engine ("HL
 *    Engine") and Modified Game Libraries ("MODs") developed by Valve,
 *    L.L.C ("Valve").  You must obey the GNU General Public License in all
 *    respects for all of the code used other than the HL Engine and MODs
 *    from Valve.  If you modify this file, you may extend this exception
 *    to your version of the file, but you are not obligated to do so.  If
 *    you do not wish to do so, delete this exception statement from your
 *    version.
 *
 */

#include "precompiled.h"

void Think(edict_t* ed)
{
	checkHudTimer(ed);
	RETURN_META(MRES_IGNORED);
}

void ClientDisconnect(edict_t* ed)
{
	if (g_game.isMapActive()) {
		auto player = g_players.getPlayer(ed);
		player->disconnect();
	}
	RETURN_META(MRES_IGNORED);
}

void ClientPutInServer(edict_t* ed)
{
	auto player = g_players.getPlayer(ed);
	player->putInServer();
	RETURN_META(MRES_IGNORED);
}

void ClientCommand(edict_t* ed)
{
	bool handled = g_game.atCommand(g_players.getPlayer(ed), CMD_ARGV(0));
	RETURN_META(handled ? MRES_SUPERCEDE : MRES_IGNORED);
}

void ClientUserInfoChanged(edict_t* ed, char* infobuffer)
{
	if (ed) {
		auto player = g_players.getPlayer(ed);
		if (player && player->isInGame())
			player->loadOptions(infobuffer);
	}
	RETURN_META(MRES_IGNORED);
}

void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
	pcv_mp_logecho = g_engfuncs.pfnCVarGetPointer("mp_logecho");
	pcv_sv_accelerate = g_engfuncs.pfnCVarGetPointer("sv_accelerate");

	gmsgSayText = GET_USER_MSG_ID(PLID, "SayText", NULL);
	gmsgTextMsg = GET_USER_MSG_ID(PLID, "TextMsg", NULL);
	gmsgTeamInfo = GET_USER_MSG_ID(PLID, "TeamInfo", NULL);
	gmsgSendAudio = GET_USER_MSG_ID(PLID, "SendAudio", NULL);
	gmsgShowMenu = GET_USER_MSG_ID(PLID, "ShowMenu", NULL);

	g_sModelIndexRadio = g_engfuncs.pfnPrecacheModel("sprites/radio.spr");
#ifndef NDEBUG
	g_sModelIndexDot = g_engfuncs.pfnPrecacheModel("sprites/dot.spr");
	g_sModelIndexLedglow = g_engfuncs.pfnPrecacheModel("sprites/ledglow.spr");
#endif

	g_game.mapStart(pEdictList, size_t(clientMax));

	RETURN_META(MRES_IGNORED);
}

void ServerDeactivate()
{
	g_game.mapEnd();
	RETURN_META(MRES_IGNORED);
}

void CmdStart(const edict_t* ed, const usercmd_s* cmd, unsigned int random_seed)
{
	auto player = g_players.getPlayer(ed);

	if (player->isInGame()) {	
		if (cmd->buttons & IN_USE) {
			if (player->usePressed())
				const_cast<usercmd_s *>(cmd)->buttons &= ~IN_USE;
		}

		if (iz_smooth_positioning->string[0] != '0')
			player->updateZone();
	}

	RETURN_META(MRES_IGNORED);
}

static DLL_FUNCTIONS gPostFunctionTable =
{
	NULL,					// pfnGameInit
	NULL,					// pfnSpawn
	&Think,					// pfnThink
	NULL,					// pfnUse
	NULL,					// pfnTouch
	NULL,					// pfnBlocked
	NULL,					// pfnKeyValue
	NULL,					// pfnSave
	NULL,					// pfnRestore
	NULL,					// pfnSetAbsBox

	NULL,					// pfnSaveWriteFields
	NULL,					// pfnSaveReadFields

	NULL,					// pfnSaveGlobalState
	NULL,					// pfnRestoreGlobalState
	NULL,					// pfnResetGlobalState

	NULL,					// pfnClientConnect
	&ClientDisconnect,		// pfnClientDisconnect
	NULL,					// pfnClientKill
	&ClientPutInServer,		// pfnClientPutInServer
	NULL,					// pfnClientCommand
	&ClientUserInfoChanged,	// pfnClientUserInfoChanged
	&ServerActivate,		// pfnServerActivate
	&ServerDeactivate,		// pfnServerDeactivate

	NULL,					// pfnPlayerPreThink
	NULL,					// pfnPlayerPostThink

	NULL,					// pfnStartFrame
	NULL,					// pfnParmsNewLevel
	NULL,					// pfnParmsChangeLevel

	NULL,					// pfnGetGameDescription
	NULL,					// pfnPlayerCustomization

	NULL,					// pfnSpectatorConnect
	NULL,					// pfnSpectatorDisconnect
	NULL,					// pfnSpectatorThink

	NULL,					// pfnSys_Error

	NULL,					// pfnPM_Move
	NULL,					// pfnPM_Init
	NULL,					// pfnPM_FindTextureType

	NULL,					// pfnSetupVisibility
	NULL,					// pfnUpdateClientData
	NULL,					// pfnAddToFullPack
	NULL,					// pfnCreateBaseline
	NULL,					// pfnRegisterEncoders
	NULL,					// pfnGetWeaponData
	NULL,					// pfnCmdStart
	NULL,					// pfnCmdEnd
	NULL,					// pfnConnectionlessPacket
	NULL,					// pfnGetHullBounds
	NULL,					// pfnCreateInstancedBaselines
	NULL,					// pfnInconsistentFile
	NULL,					// pfnAllowLagCompensation
};

C_DLLEXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	if(!pFunctionTable) {
		UTIL_LogPrintf("GetEntityAPI2 called with null pFunctionTable");
		return FALSE;
	}
	if(*interfaceVersion != INTERFACE_VERSION) {
		UTIL_LogPrintf("GetEntityAPI2 version mismatch; requested=%d ours=%d", *interfaceVersion, INTERFACE_VERSION);
		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}
	pFunctionTable->pfnClientCommand = &ClientCommand;
	pFunctionTable->pfnCmdStart = &CmdStart;
	return TRUE;
}

C_DLLEXPORT int GetEntityAPI2_Post(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion)
{
	if (!pFunctionTable) {
		UTIL_LogPrintf("GetEntityAPI2 called with null pFunctionTable");
		return FALSE;
	}
	if (*interfaceVersion != INTERFACE_VERSION) {
		UTIL_LogPrintf("GetEntityAPI2 version mismatch; requested=%d ours=%d", *interfaceVersion, INTERFACE_VERSION);
		//! Tell metamod what version we had, so it can figure out who is out of date.
		*interfaceVersion = INTERFACE_VERSION;
		return FALSE;
	}
	memcpy(pFunctionTable, &gPostFunctionTable, sizeof(DLL_FUNCTIONS));
	return TRUE;
}
