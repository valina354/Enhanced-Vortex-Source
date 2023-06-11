//========= Copyright ? 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: An entity that creates NPCs in the game. There are two types of NPC
//			makers -- one which creates NPCs using a template NPC, and one which
//			creates an NPC via a classname.
//
//=============================================================================//

#include "cbase.h"
#include "datacache/imdlcache.h"
#include "entityapi.h"
#include "entityoutput.h"
#include "ai_basenpc.h"
#include "TemplateEntities.h"
#include "ndebugoverlay.h"
#include "mapentities.h"
#include "IEffects.h"
#include "props.h"
#include "ai_network.h"
#include "hl2_gamerules.h"
#include "ammodef.h"
#include "items.h"
#include "ai_behavior_follow.h"
#include "hlss_dynamic_npc_spawner.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CHLSS_Dynamic_Spawn_Brush)

DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "StartEnabled"),
DEFINE_FIELD(m_iNumNodes, FIELD_INTEGER),

DEFINE_KEYFIELD(m_iszGroupName, FIELD_STRING, "GroupName"),

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),

DEFINE_OUTPUT(m_OnSpawn, "OnSpawn"),
END_DATADESC()

CHLSS_Dynamic_Spawn_Brush::CHLSS_Dynamic_Spawn_Brush(void)
{
	m_iNumNodes = 0;

	m_bEnabled = false;

	m_iszGroupName = NULL_STRING;
}


void CHLSS_Dynamic_Spawn_Brush::Spawn(void)
{
	Precache();

	// Bind to our bmodel.
	SetModel(STRING(GetModelName()));
	AddSolidFlags(FSOLID_VOLUME_CONTENTS);
	AddEffects(EF_NODRAW);

	BaseClass::Spawn();

	m_pAINetwork = g_pBigAINet;

	m_iNumNodes = GetNetwork()->GetNodeCountInBrush(this);
}

void CHLSS_Dynamic_Spawn_Brush::InputEnable(inputdata_t &inputdata)
{
	//BUG? attack brush doesn't have think function
	SetNextThink(gpGlobals->curtime);

	m_bEnabled = true;
}

void CHLSS_Dynamic_Spawn_Brush::InputDisable(inputdata_t &inputdata)
{
	m_bEnabled = false;
}

bool CHLSS_Dynamic_Spawn_Brush::CanSpawnHere(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC)
{
	if (!m_bEnabled)
		return false;

	return true;
}

void CHLSS_Dynamic_Spawn_Brush::OnSuccesfulSpawn(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC)
{
	m_OnSpawn.FireOutput(this, this);
}

//------------------------------------------------------------------------------
// ATTACK BRUSH STARTS HERE
//------------------------------------------------------------------------------


LINK_ENTITY_TO_CLASS(hlss_dynamic_attack_brush, CHLSS_Dynamic_Attack_Brush);

BEGIN_DATADESC(CHLSS_Dynamic_Attack_Brush)
DEFINE_FIELD(m_iCurrentClassify, FIELD_INTEGER),
END_DATADESC()

CHLSS_Dynamic_Attack_Brush::CHLSS_Dynamic_Attack_Brush(void)
{
	m_iCurrentClassify = CLASS_NONE;
}


void CHLSS_Dynamic_Attack_Brush::OnSuccesfulSpawn(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC)
{
	pNPC->SetState(NPC_STATE_ALERT);

	Activity movementActivity = ACT_RUN;
	bool bIsFlying = (pNPC->GetMoveType() == MOVETYPE_FLY) || (pNPC->GetMoveType() == MOVETYPE_FLYGRAVITY);
	if (bIsFlying)
	{
		movementActivity = ACT_FLY;
	}

	pNPC->ScheduledFollowPath(SCHED_ALERT_WALK, pPlayer, movementActivity);

	int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pNPC->Classify(), CLASS_PLAYER);

	if (disposition == D_HT)
	{
		pNPC->UpdateEnemyMemory(pPlayer, pPlayer->EyePosition());
	}
	else if (disposition == D_LI)
	{
		CAI_FollowBehavior *pBehavior;
		if (pNPC->GetBehavior(&pBehavior))
		{
			pBehavior->SetFollowTarget(pPlayer);
		}
	}

	m_iCurrentClassify = pNPC->Classify();

	BaseClass::OnSuccesfulSpawn(pPlayer, pNPC);
}

void CHLSS_Dynamic_Attack_Brush::ReleaseBrush()
{
	m_iCurrentClassify = CLASS_NONE;
}

bool CHLSS_Dynamic_Attack_Brush::CanSpawnHere(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC)
{
	if (!BaseClass::CanSpawnHere(pPlayer, pNPC))
	{
		return false;
	}

	if (m_iCurrentClassify != CLASS_NONE &&
		CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pNPC->Classify(), m_iCurrentClassify) != D_LI)
	{
		return false;
	}

	if (HasSpawnFlags(SF_HLSS_BRUSH_CHECK_VISIBILITY) && pPlayer->FVisible(GetLocalOrigin()))
	{
		//DevMsg("hlss_dynamic_attack_bruch %s origin is visible to player\n", STRING(GetEntityName()));
		return false;
	}

	return true;
}

//------------------------------------------------------------------------------
// IDLE BRUSH STARTS HERE
//------------------------------------------------------------------------------

LINK_ENTITY_TO_CLASS(hlss_dynamic_idle_brush, CHLSS_Dynamic_Idle_Brush);

BEGIN_DATADESC(CHLSS_Dynamic_Idle_Brush)
DEFINE_KEYFIELD(m_iszIdlePath, FIELD_STRING, "idle_path"),

DEFINE_THINKFUNC(IdleBrushThink),

DEFINE_INPUTFUNC(FIELD_VOID, "CheckSpawn", InputCheckSpawn),
END_DATADESC()

CHLSS_Dynamic_Idle_Brush::CHLSS_Dynamic_Idle_Brush()
{
	m_iszIdlePath = NULL_STRING;
}

void CHLSS_Dynamic_Idle_Brush::Spawn()
{
	BaseClass::Spawn();

	SetThink(&CHLSS_Dynamic_Idle_Brush::IdleBrushThink);
	SetNextThink(gpGlobals->curtime + HLSS_IDLE_BRUSH_THINK_DELAY);
}

void CHLSS_Dynamic_Idle_Brush::IdleBrushThink()
{
	if (!m_bEnabled)
	{
		return;
	}

	SetNextThink(gpGlobals->curtime + HLSS_IDLE_BRUSH_THINK_DELAY);

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_dynamic_idle_brush: no player\n");
		return;
	}

	NDebugOverlay::Box(GetLocalOrigin(), Vector(10, 10, 10), Vector(-10, -10, -10), 0, 0, 255, 0, 5);

	if (HasSpawnFlags(SF_HLSS_BRUSH_SPAWN_CHECK_WHEN_ORIGIN_VISIBLE) && pPlayer->FVisible(this))
	{
		CheckSpawn();
	}
}

bool CHLSS_Dynamic_Idle_Brush::CanSpawnHere(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC)
{
	if (!BaseClass::CanSpawnHere(pPlayer, pNPC))
	{
		return false;
	}

	return true;
}

void CHLSS_Dynamic_Idle_Brush::OnSuccesfulSpawn(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC)
{
	pNPC->SetState(NPC_STATE_ALERT);

	CBaseEntity *pTarget = NULL;

	if (m_iszIdlePath != NULL_STRING)
	{
		pTarget = gEntList.FindEntityByName(NULL, STRING(m_iszIdlePath));
	}

	if (pTarget)
	{
		Activity movementActivity = ACT_WALK;
		bool bIsFlying = (pNPC->GetMoveType() == MOVETYPE_FLY) || (pNPC->GetMoveType() == MOVETYPE_FLYGRAVITY);
		if (bIsFlying)
		{
			movementActivity = ACT_FLY;
		}

		pNPC->ScheduledFollowPath(SCHED_ALERT_WALK, pTarget, movementActivity);
	}

	BaseClass::OnSuccesfulSpawn(pPlayer, pNPC);
}

void CHLSS_Dynamic_Idle_Brush::InputCheckSpawn(inputdata_t &inputdata)
{
	CheckSpawn();
}

void CHLSS_Dynamic_Idle_Brush::CheckSpawn()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
	{
		return;
	}

	//
	CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "hlss_dynamic_npc_spawner");
	CHLSS_Dynamic_NPC_Spawner *pSpawner = dynamic_cast <CHLSS_Dynamic_NPC_Spawner*>(pEnt);

	int iNumCands = 0;
#define iMaxCands 5

	CHLSS_Dynamic_NPC_Spawner *pSpawnerCandidate[iMaxCands];
	int iIdleNPCs[iMaxCands];

	for (int j = 0; j<iMaxCands; j++)
	{
		pSpawnerCandidate[j] = NULL;
		iIdleNPCs[j] = 0;
	}

	while (pEnt)
	{
		if (pSpawner &&
			pSpawner->m_iNumberOfNPCs < pSpawner->m_iMaxNumberOfNPCs &&
			pSpawner->AllowedGroup(this))
		{
			int iIdle = pSpawner->GetNumberOfIdleNPCsToSpawn();

			if (iIdle > 0)
			{
				int iSlot = -1;

				if (iNumCands < iMaxCands)
				{
					iSlot = iNumCands;
				}
				else
				{
					for (int j = 0; j<iMaxCands; j++)
					{
						if (iIdleNPCs[j] < iIdle && (iSlot == -1 || iIdleNPCs[j] < iIdleNPCs[iSlot]))
						{
							iSlot = j;
						}
					}
				}

				if (iSlot != -1)
				{
					iIdleNPCs[iSlot] = iIdle;
					pSpawnerCandidate[iSlot] = pSpawner;

					iNumCands++;
				}
			}
		}

		pEnt = gEntList.FindEntityByClassname(pEnt, "hlss_dynamic_npc_spawner");
		pSpawner = dynamic_cast <CHLSS_Dynamic_NPC_Spawner*>(pEnt);
	}

	DevMsg("hlss_dynamic_idle_brush: candidates found %d\n", iNumCands);

	if (iNumCands > 0)
	{
		int iNodesLeft = m_iNumNodes;

		int iRandom = random->RandomInt(0, iNumCands - 1);

		DevMsg("hlss_dynamic_idle_brush: picking candidate %d to spawn first\n", iRandom);

		Class_T iFirstClassify = pSpawnerCandidate[iRandom]->m_iClassify;

		for (int j = 0; (j<iNumCands && iNodesLeft > 0); j++)
		{
			int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(iFirstClassify, pSpawnerCandidate[iRandom]->m_iClassify);

			if (disposition == D_LI)
			{
				DevMsg("Trying to spawn %d NPCs from spawner %s\n", iIdleNPCs[iRandom], STRING(pSpawnerCandidate[iRandom]->GetEntityName()));

				CAI_BaseNPC *pNPC = pSpawnerCandidate[iRandom]->CreateNPC();

				if (!pNPC)
				{
					continue;
				}

				int iPlayerNode = GetNetwork()->NearestNodeToPoint(pNPC, pPlayer->GetAbsOrigin(), false);
				if (iPlayerNode == NO_NODE)
				{
					UTIL_RemoveImmediate(pNPC);
					continue;
				}


				for (int i = 0; (i<iIdleNPCs[iRandom] && iNodesLeft > 0); i++)
				{
					if (pSpawnerCandidate[iRandom]->SpawnInsideBrush(pPlayer, iPlayerNode, pNPC, this))
					{
						iNodesLeft--;

						//TERO: get next NPC
						if (i < (iIdleNPCs[iRandom] - 1))
						{
							pNPC = pSpawnerCandidate[iRandom]->CreateNPC();
						}
					}
					else
					{
						DevMsg("spawning NPCs with spawner %s failed at %d/%d\n", STRING(pSpawnerCandidate[iRandom]->GetEntityName()), i, iIdleNPCs[iRandom]);
						UTIL_RemoveImmediate(pNPC);
						break;
					}
				}
			}
			else
			{
				DevMsg("Cannot to spawn %d NPCs from spawner %s, disposition is not LIKE to our first one.\n", iIdleNPCs[iRandom], STRING(pSpawnerCandidate[iRandom]->GetEntityName()));
			}

			iRandom++;

			if (iRandom >= iNumCands)
			{
				iRandom = 0;
			}
		}
	}

	if (HasSpawnFlags(SF_HLSS_BRUSH_REMOVE_AFTER_CHECK_SPAWN))
	{
		UTIL_Remove(this);
	}
	else
	{
		m_bEnabled = false;
	}
}


//---------------------------------------------------------------------------

//---------------------------------------------------------------------------

BEGIN_DATADESC(CHLSS_Player_Path)
DEFINE_KEYFIELD(m_iszNextName, FIELD_STRING, "NextPath_Name"),
DEFINE_FIELD(m_hNextPath, FIELD_EHANDLE),
DEFINE_FIELD(m_flDistanceToNext, FIELD_FLOAT),
DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "StartEnabled"),
DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
END_DATADESC()

LINK_ENTITY_TO_CLASS(hlss_player_path, CHLSS_Player_Path);

CHLSS_Player_Path::CHLSS_Player_Path(void)
{
	m_iszNextName = NULL_STRING;
	m_hNextPath = NULL;
	m_flDistanceToNext = 0;
}


void CHLSS_Player_Path::Spawn(void)
{
	Precache();

	// Bind to our bmodel.
	SetModel(STRING(GetModelName()));
	AddSolidFlags(FSOLID_VOLUME_CONTENTS);
	AddEffects(EF_NODRAW);

	AddSolidFlags(FSOLID_NOT_SOLID);
	AddSolidFlags(FSOLID_TRIGGER);

	//	SetTouch( &CHLSS_Player_Path::Touch );


	BaseClass::Spawn();

	if (m_iszNextName != NULL_STRING)
	{
		m_hNextPath = (CHLSS_Player_Path *)gEntList.FindEntityByName(NULL, m_iszNextName);

		if (m_hNextPath)
		{
			m_flDistanceToNext = (GetLocalOrigin() - m_hNextPath->GetLocalOrigin()).Length();
		}
		else
		{
			Warning("Couldn't find next path %s for %s\n", m_iszNextName, GetDebugName());
		}
	}
}

void CHLSS_Player_Path::Touch(CBaseEntity *pOther)
{
	if (!m_bEnabled)
		return;

	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();

	if (pOther->IsPlayer() && pDirector)
	{
		pDirector->m_hNextPath = m_hNextPath;
		//DevWarning("hlss_director: reached a brush\n");

		if (!pDirector->m_hNextPath)
		{
			pDirector->m_flPlayerPath = pDirector->m_flFullPlayerPath;
			pDirector->m_flPlayerPath = pDirector->m_flFullPlayerPath;

			//DevMsg("hlss_director: path done\n");
			return;
		}
	}
}

void CHLSS_Player_Path::InputEnable(inputdata_t &inputdata)
{
	if (!m_bEnabled)
	{
		m_bEnabled = true;

		CHLSS_Player_Path *pNext = m_hNextPath;

		while (pNext)
		{
			pNext->m_bEnabled = true;
			pNext = m_hNextPath;
		}
	}
}

void CHLSS_Player_Path::InputDisable(inputdata_t &inputdata)
{
	if (m_bEnabled)
	{
		m_bEnabled = false;

		CHLSS_Player_Path *pNext = m_hNextPath;

		while (pNext)
		{
			pNext->m_bEnabled = false;
			pNext = m_hNextPath;
		}
	}
}