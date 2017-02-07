#include "cgame/cg_local.h"
#include "mppShared.h"

static MultiPlugin_t	*sys;
static MultiSystem_t	*trap;

static vmCvar_t cvar_allow;

/**************************************************
* mpp
*
* Plugin exported function. This function gets called
* the moment the module is loaded and provides a
* pointer to a shared structure. Store the pointer
* and copy the System pointer safely.
**************************************************/

__declspec(dllexport) void mpp(MultiPlugin_t *pPlugin)
{
	sys = pPlugin;
	trap = sys->System;

	trap->Cvar.Register(&cvar_allow, "privateduel_allow", "1", CVAR_ARCHIVE);
}

/**************************************************
* mppPreSystem
*
* Plugin exported function. This function gets called
* from the game module to perform an action in the
* engine, before going into the engine.
**************************************************/

__declspec(dllexport) int mppPreMain(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11)
{
	if ((cmd == CG_DRAW_ACTIVE_FRAME || CG_CROSSHAIR_PLAYER) &&
	cvar_allow.integer != 0 && sys->snap->ps.duelInProgress) {
		// Fake a mind trick
		int trickedIndex;
		if (sys->cg->clientNum > 47) trickedIndex = 3;
		if (sys->cg->clientNum > 31) trickedIndex = 2;
		if (sys->cg->clientNum > 15) trickedIndex = 1;
		else trickedIndex = 0;
		for (int i = 0; i < sys->cgs->maxclients; i++) {
			if (i == sys->cg->clientNum || i == sys->snap->ps.duelIndex) continue;
			centity_t *cent = sys->mppIsPlayerEntity(i);
			if (cent) {
				int *trick = &cent->currentState.trickedentindex;
				trick += trickedIndex;
				*trick = *trick | (1 << (sys->cg->clientNum % 16));
			}
		}
	}

	return sys->noBreakCode;
}

__declspec(dllexport) int mppPostMain(int cmd, int arg0, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10, int arg11) {
	return mppPreMain(cmd, arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10, arg11);
}

__declspec(dllexport) int mppPreSystem(int *args)
{
	switch (args[0])
	{
		case CG_R_ADDREFENTITYTOSCENE:
		{
			if (cvar_allow.integer != 0 && sys->snap->ps.duelInProgress) {
				refEntity_t *refEntity = (refEntity_t *)args[1];
				centity_t *cent = sys->mppIsPlayerEntity(sys->gameEntity);
				if (cent && cent->currentState.clientNum != sys->cg->clientNum && cent->currentState.clientNum != sys->snap->ps.duelIndex) {
					return qtrue;
				}
			}
			break;
		}
		case CG_S_GETVOICEVOLUME:
		case CG_S_MUTESOUND:
		case CG_S_ADDLOOPINGSOUND:
		case CG_S_UPDATEENTITYPOSITION:
		case CG_S_ADDREALLOOPINGSOUND:
		case CG_S_RESPATIALIZE:
		case CG_S_STARTSOUND:
		case CG_S_ADDLOCALSET:
		{
			if (cvar_allow.integer != 0 && sys->snap->ps.duelInProgress) {
				int entityId;
				if (args[0] == CG_S_STARTSOUND) {
					entityId = args[2];
				}
				else if (args[0] == CG_S_ADDLOCALSET) {
					entityId = args[4];
				}
				else {
					entityId = args[1];
				}
				centity_t *cent = sys->mppIsPlayerEntity(entityId);
				if (cent && cent->currentState.clientNum != sys->cg->clientNum && cent->currentState.clientNum != sys->snap->ps.duelIndex) {
					return qtrue;
				}
			}
			break;
		}
	default:
		break;
	}

	return sys->noBreakCode;
}

__declspec(dllexport) int mppPostSystem(int *args) {
	switch (args[0])
	{
		case CG_CVAR_SET:
		case CG_CVAR_UPDATE:
		{
			trap->Cvar.Update(&cvar_allow);
			break;
		}
		default:
			break;
	}

	return sys->noBreakCode;
}