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

ConVar	hlss_dynamic_npc_spawner_debug("hlss_dynamic_npc_spawner_debug", "0");
ConVar	hlss_dynamic_npc_spawner_use_closes_to_path("hlss_dynamic_npc_spawner_use_closes_to_path", "1");

static void DispatchActivate(CBaseEntity *pEntity)
{
	bool bAsyncAnims = mdlcache->SetAsyncLoad(MDLCACHE_ANIMBLOCK, false);
	pEntity->Activate();
	mdlcache->SetAsyncLoad(MDLCACHE_ANIMBLOCK, bAsyncAnims);
}

LINK_ENTITY_TO_CLASS(hlss_dynamic_npc_spawner, CHLSS_Dynamic_NPC_Spawner);

BEGIN_DATADESC(CHLSS_Dynamic_NPC_Spawner)
DEFINE_KEYFIELD(m_iszTemplateName, FIELD_STRING, "Template_Name"),
DEFINE_KEYFIELD(m_flMinimumSpawnAttackDelay, FIELD_FLOAT, "Template_Delay"),
DEFINE_KEYFIELD(m_iMaxNumberOfNPCs, FIELD_INTEGER, "Template_Max"),
DEFINE_KEYFIELD(m_iNPCdifficulty, FIELD_INTEGER, "Template_Difficulty"),
DEFINE_KEYFIELD(m_flFirstSpawnDelay, FIELD_FLOAT, "Template_FirstDelay"),
DEFINE_KEYFIELD(m_iMaxAttackSize, FIELD_INTEGER, "Template_MaxAttack"),

DEFINE_FIELD(m_iszTemplateData, FIELD_STRING),
DEFINE_FIELD(m_flNextSpawnTime, FIELD_TIME),
DEFINE_FIELD(m_flNextSpawnAttackTime, FIELD_TIME),
DEFINE_FIELD(m_iAttackSize, FIELD_INTEGER),
DEFINE_FIELD(m_iNumberOfNPCs, FIELD_INTEGER),
DEFINE_FIELD(m_iClassify, FIELD_INTEGER),

DEFINE_FIELD(m_bPanicEvent, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_iPanicEventMaxNPCs, FIELD_INTEGER, "PanicEvent_Max"),
DEFINE_KEYFIELD(m_flPanicEventMaxFullDelay, FIELD_FLOAT, "PanicEvent_Delay"),
DEFINE_KEYFIELD(m_iPanicEventAbsoluteMaxNPCs, FIELD_INTEGER, "PanicEvent_Absolute"),

DEFINE_FIELD(m_bRequiresFriendlyRPG, FIELD_BOOLEAN),
DEFINE_FIELD(m_bRequiresHostileRPG, FIELD_BOOLEAN),

DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "StartEnabled"),

DEFINE_FIELD(m_bAllDead, FIELD_BOOLEAN),

#ifdef HLSS_USE_EHANDLE_DIRECTOR
DEFINE_FIELD(m_hDirector, FIELD_EHANDLE),
#endif

DEFINE_FIELD(m_hAttackBrush, FIELD_EHANDLE),
DEFINE_FIELD(m_hIgnoreBrush, FIELD_EHANDLE),
DEFINE_FIELD(m_flAttackBrushPlayerPath, FIELD_EHANDLE),
DEFINE_FIELD(m_flNextAttackBrushSearch, FIELD_TIME),

DEFINE_KEYFIELD(m_iszGroupName, FIELD_STRING, "GroupName"),

DEFINE_FIELD(m_flNextDistanceToCreateNPCs, FIELD_FLOAT),
DEFINE_FIELD(m_iNumToSpawnInDistance, FIELD_INTEGER),

// Function Pointers
DEFINE_THINKFUNC(DynamicSpawnerThink),

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),

DEFINE_INPUTFUNC(FIELD_INTEGER, "SetMaxAttack", InputSetMaxAttack),
DEFINE_INPUTFUNC(FIELD_INTEGER, "SetMaxNPCs", InputSetMaxNPCs),

DEFINE_INPUTFUNC(FIELD_VOID, "StartPanicEvent", InputStartPanicEvent),
DEFINE_INPUTFUNC(FIELD_VOID, "StopPanicEvent", InputStopPanicEvent),

DEFINE_OUTPUT(m_AllDead, "AllDead"),
END_DATADESC()

CHLSS_Dynamic_NPC_Spawner::CHLSS_Dynamic_NPC_Spawner(void)
{
	m_flMinimumSpawnAttackDelay = HLSS_DYNAMIC_NPC_SPANWER_DEFAULT_BURST_DELAY;
	m_iAttackSize = 0;
	m_iMaxAttackSize = 8;
	m_iMaxNumberOfNPCs = 4;

	m_iNPCdifficulty = 50;

	m_iClassify = CLASS_NONE;

	m_iszTemplateData = NULL_STRING;
	m_iszTemplateName = NULL_STRING;

	m_bRequiresFriendlyRPG = false;
	m_bRequiresHostileRPG = false;

	m_flFirstSpawnDelay = 0;

	m_bPanicEvent = false;

	m_bEnabled = false;

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	m_hDirector = NULL;
#endif

	m_hAttackBrush = NULL;
	m_hIgnoreBrush = NULL;

	m_flNextAttackBrushSearch = 0;

	m_pAINetwork = NULL;

	m_iszGroupName = NULL_STRING;

	m_flNextDistanceToCreateNPCs = -1;
	m_iNumToSpawnInDistance = 0;
}

void CHLSS_Dynamic_NPC_Spawner::Spawn()
{
	SetSolid(SOLID_NONE);
	Precache();

	m_flNextSpawnTime = 0;
	m_flNextSpawnAttackTime = gpGlobals->curtime + m_flMinimumSpawnAttackDelay;

	m_iNumberOfNPCs = 0;

	m_iNPCdifficulty = clamp(m_iNPCdifficulty, 0, 100);
	m_pAINetwork = g_pBigAINet;

	m_bAllDead = false;

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	FindDirector();
#endif

	SetThink(&CHLSS_Dynamic_NPC_Spawner::DynamicSpawnerThink);
	SetNextThink(gpGlobals->curtime);
}

void CHLSS_Dynamic_NPC_Spawner::OnRestore()
{
	//TERO: not sure if this should be done here
	m_pAINetwork = g_pBigAINet;
}

void CHLSS_Dynamic_NPC_Spawner::Precache()
{
	if (!m_iszTemplateData)
	{
		//
		// This must be the first time we're activated, not a load from save game.
		// Look up the template in the template database.
		//
		if (!m_iszTemplateName)
		{
			Warning("hlss_dynamic_npc_spawner %s has no template NPC!\n", STRING(GetEntityName()));
			UTIL_Remove(this);
		}
		else
		{
			m_iszTemplateData = Templates_FindByTargetName(STRING(m_iszTemplateName));
			if (m_iszTemplateData == NULL_STRING)
			{
				DevWarning("hlss_dynamic_npc_spanwer %s: template NPC %s not found!\n", STRING(GetEntityName()), STRING(m_iszTemplateName));
				UTIL_Remove(this);
				return;
			}
		}
	}

	Assert(m_iszTemplateData != NULL_STRING);

	// If the mapper marked this as "preload", then instance the entity preache stuff and delete the entity
	//if ( !HasSpawnFlags(SF_NPCMAKER_NOPRELOADMODELS) )
	if (m_iszTemplateData != NULL_STRING)
	{

		CBaseEntity *pEntity = NULL;
		MapEntity_ParseEntity(pEntity, STRING(m_iszTemplateData), NULL);
		if (pEntity != NULL)
		{
			pEntity->Precache();

			m_iClassify = pEntity->Classify();

			if (FClassnameIs(pEntity, "npc_combinegunship") ||
				FClassnameIs(pEntity, "npc_strider"))
			{
				if (CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pEntity->Classify(), CLASS_PLAYER) == D_LI)
				{
					m_bRequiresHostileRPG = true;
				}
				else if (CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pEntity->Classify(), CLASS_PLAYER) == D_HT)
				{
					m_bRequiresFriendlyRPG = true;
				}
			}

			UTIL_RemoveImmediate(pEntity);
		}
	}
}

// Item entity names
const char *CHLSS_Dynamic_NPC_Spawner::m_lpzNPCWeaponNames[HLSS_NPC_WEAPON_NUM] =
{
	"weapon_shotgun",
	"weapon_rpg",
	"weapon_ar2",
};


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pVictim - 
//-----------------------------------------------------------------------------
void CHLSS_Dynamic_NPC_Spawner::DeathNotice(CBaseEntity *pVictim)
{
	m_iNumberOfNPCs--;

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	CHLSS_Director *pDirector = m_hDirector;
#else
	CHLSS_Director *pDirector = CHLSS_Director::GetDirector(); //m_hDirector;
#endif

	if (pDirector)
	{
		if (CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pVictim->Classify(), CLASS_PLAYER) == D_HT &&
			pVictim->MyNPCPointer() && pVictim->MyNPCPointer()->m_bKilledByPlayer)
		{
			pDirector->m_iPlayerScore += clamp(((m_iNPCdifficulty * m_iNPCdifficulty) / 1000 + 0.5), 1, 10);
		}

		if (m_bRequiresFriendlyRPG)
		{
			pDirector->m_iPlayerNeedsRPG--;
		}

		//Tell the director about wapons
		if (pVictim->MyNPCPointer() && pVictim->MyNPCPointer()->CapabilitiesGet() & bits_CAP_USE_WEAPONS)
		{
			int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pVictim->MyNPCPointer()->Classify(), CLASS_PLAYER);

			if (disposition == D_LI)
			{
				for (int i = 0; i<HLSS_NPC_WEAPON_NUM; i++)
				{
					//TERO: if an NPC has picked upn a weapon, we don't want to have negative numbers
					if (pVictim->MyNPCPointer()->Weapon_OwnsThisType(m_lpzNPCWeaponNames[i]) && pDirector->m_iNumFriendlyWeapons[i] > 0)
					{
						pDirector->m_iNumFriendlyWeapons[i]--;
					}
				}
			}
			else
			{
				for (int i = 0; i<HLSS_NPC_WEAPON_NUM; i++)
				{
					//TERO: if an NPC has picked up a weapon, we don't want to have negative numbers
					if (pVictim->MyNPCPointer()->Weapon_OwnsThisType(m_lpzNPCWeaponNames[i]) && pDirector->m_iNumHostileWeapons[i] > 0)
					{
						pDirector->m_iNumHostileWeapons[i]++;;
					}
				}
			}
		}
	}
}

void CHLSS_Dynamic_NPC_Spawner::InputEnable(inputdata_t &inputdata)
{
	if (!m_bEnabled)
	{
		SetNextThink(gpGlobals->curtime);
		m_bEnabled = true;
		m_bAllDead = false;
	}
}

void CHLSS_Dynamic_NPC_Spawner::InputDisable(inputdata_t &inputdata)
{
	ClearAttackBrush();
	m_bEnabled = false;
}

void CHLSS_Dynamic_NPC_Spawner::InputSetMaxAttack(inputdata_t &inputdata)
{
	m_iMaxAttackSize = inputdata.value.Int();
}

void CHLSS_Dynamic_NPC_Spawner::InputSetMaxNPCs(inputdata_t &inpudata)
{
	m_iMaxNumberOfNPCs = inpudata.value.Int();
}

void CHLSS_Dynamic_NPC_Spawner::InputStartPanicEvent(inputdata_t &inpudata)
{
	m_bPanicEvent = true;
}

void CHLSS_Dynamic_NPC_Spawner::InputStopPanicEvent(inputdata_t &inpudata)
{
	m_bPanicEvent = false;
}

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifdef HLSS_USE_EHANDLE_DIRECTOR
bool CHLSS_Dynamic_NPC_Spawner::FindDirector()
{
	m_hDirector = (CHLSS_Director *)gEntList.FindEntityByClassname(NULL, "hlss_director");

	return (m_hDirector != NULL);
}
#endif

void CHLSS_Dynamic_NPC_Spawner::DynamicSpawnerThink()
{
	SetNextThink(gpGlobals->curtime);

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	if (!m_hDirector && !FindDirector())
	{
		Warning("DynamicSpawnerThink: no m_hDirector!\n");
		return;
	}

	CHLSS_Director *pDirector = m_hDirector;
#else
	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();
#endif

	if (!pDirector)
	{
		Warning("DynamicSpawnerThink: no pDirector!\n");
		return;
	}

	if (m_bEnabled && pDirector->m_bEnabled)
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		if (!pPlayer)
		{
			Warning("hlss_dynamic_npc_spawner: no player\n");
			return;
		}

		//TERO: if it's panic event for this NPC, ignore all burst shit
		if (m_bPanicEvent)
		{

#ifdef DEBUG_DNS_ENTITY
			DevMsg("hlss_dynamic_npc_spawner: panic event\n");
#endif

			if (m_iNumberOfNPCs < m_iPanicEventMaxNPCs)
			{
				if (m_flNextSpawnAttackTime < gpGlobals->curtime)
				{
					SpawnAttack();

					if (m_iNumberOfNPCs >= m_iPanicEventMaxNPCs)
					{
						m_flNextSpawnAttackTime = gpGlobals->curtime + m_flPanicEventMaxFullDelay;
					}
					else
					{
						m_flNextSpawnAttackTime = gpGlobals->curtime + 0.2f;
					}
					m_flNextSpawnTime = 0;
					m_iAttackSize = 0;
				}
			}
			else if (m_iNumberOfNPCs < m_iPanicEventAbsoluteMaxNPCs && m_flNextSpawnAttackTime < gpGlobals->curtime)
			{
				SpawnAttack();
				m_flNextSpawnAttackTime = gpGlobals->curtime + m_flPanicEventMaxFullDelay;
				m_flNextSpawnTime = 0;
				m_iAttackSize = 0;
			}
		}
		else if (pDirector->m_flFullPlayerPath != -1)
		{
			if (pDirector->m_flPlayerPath >= m_flNextDistanceToCreateNPCs)
			{
				if (m_flNextDistanceToCreateNPCs != -1 && m_iNumToSpawnInDistance < 0)
				{
					m_iNumToSpawnInDistance = GetDistanceAttackSize();

					if (m_iNumToSpawnInDistance > 0)
					{
						DevWarning("%s STARTING NEW ATTACK WITH SIZE %d, situation %d\n", GetDebugName(), m_iNumToSpawnInDistance, CHLSS_Director::GetDirector()->m_iPlayerSituation);
					}
					else
					{
						DevWarning("%s NO NEW ATTACK, situation %d\n", GetDebugName(), CHLSS_Director::GetDirector()->m_iPlayerSituation);
					}
				}
				else if (m_flNextDistanceToCreateNPCs != -1 && m_iNumToSpawnInDistance != 0)
				{
					if (hlss_dynamic_npc_spawner_debug.GetInt() == 2)
					{
						DevMsg("%s now spawning, left: %d\n", m_iszTemplateName, m_iNumToSpawnInDistance);
					}

					if (m_iNumberOfNPCs < m_iMaxNumberOfNPCs)
					{
						if (SpawnAttack())
						{
							//TERO: maybe not the best way to do this
							/*if (pDirector->m_iAvarageSituation < 50 && pDirector->m_iPlayerSituation < 50)
							{
							m_flNumToSpawnInDistance-=0.5f;
							}
							else
							{
							m_flNumToSpawnInDistance-=1.0f;
							}*/

							m_iNumToSpawnInDistance--;
						}
					}
				}
				// We don't want to get the next one before the player has killed atleast some of the NPCs
				// This could allow the player to cheat by simply running away from all the enemies, though,
				// so the m_iNumberOfNPCs doesn't have to be full zero.
				else //if ( m_iNumberOfNPCs < (m_iMaxNumberOfNPCs/2) )
				{
					int iMaxNPCs = (int)((float)m_iMaxNumberOfNPCs*0.5f + 0.6f);

					if (hlss_dynamic_npc_spawner_debug.GetInt() == 2)
					{
						DevMsg("%s NPCs left alive %d/%d where max is %d\n", m_iszTemplateName, m_iNumberOfNPCs, iMaxNPCs, m_iMaxNumberOfNPCs);
					}

					// -1 helps with aliengrunts when m_iMaxNumberOfNPCs is 1
					// since player can run away from them easily
					if ((m_iNumberOfNPCs - 1) < iMaxNPCs)
					{
						ClearAttackBrush();
						GetNextDistanceToSpawn();
					}
				}
				//DEBUG:
				/*else
				{
				DevMsg("NPCs left alive: %d/%d\n", m_iNumberOfNPCs, m_iMaxNumberOfNPCs);
				}*/

				m_flNextSpawnTime = 0;
			}
			else
			{
				if (hlss_dynamic_npc_spawner_debug.GetInt() == 1)
				{
					DevMsg("%s distance to spawn %f/%f. ", GetDebugName(), pDirector->m_flPlayerPath, m_flNextDistanceToCreateNPCs);
					DevMsg("Last time progressed %f. ", gpGlobals->curtime - pDirector->m_flLastTimeProgressed);
					DevMsg("Delay: %f\n", pDirector->m_flLastTimeProgressed + m_flFirstSpawnDelay - gpGlobals->curtime);
				}

				if (m_iAttackSize > 0)
				{
					if (hlss_dynamic_npc_spawner_debug.GetInt() == 1)
					{
						DevMsg("Left in attack %d\n", m_iAttackSize);
					}

					CheckAttackSpawning();
				}
				else if (pDirector->m_flLastTimeProgressed + m_flFirstSpawnDelay < gpGlobals->curtime)
				{
					GetNextAttack();
				}
				else
				{
					m_flNextSpawnTime = 0;
				}
			}

		}//end have path
		else if (m_flFirstSpawnDelay == 0 || pDirector->m_flStartTime + m_flFirstSpawnDelay + (((50.0f - pDirector->m_iPlayerSituation) / 100.0f) * m_flMinimumSpawnAttackDelay * HLSS_NPC_FIRST_SPAWN_BURST_SCALE) < gpGlobals->curtime)
		{
			CheckAttackSpawning();
			GetNextAttack();
		}
	}
	else
	{

		//TERO: here we check if there's still some alive NPCs,
		//		we continue to set SetNextThink( gpGlobals->curtime );
		//		until there is no NPCs alive left and we fire the appropriate outout

		if (!m_bAllDead)
		{
			if (m_iNumberOfNPCs > 0)
			{
				SetNextThink(gpGlobals->curtime);
			}
			else
			{
				m_AllDead.FireOutput(this, this);
				m_bAllDead = true;
			}
		}
	}
}

void CHLSS_Dynamic_NPC_Spawner::GetNextDistanceToSpawn()
{
	DevMsg("Old distance to create NPCs: %f", m_flNextDistanceToCreateNPCs);

	m_iNumToSpawnInDistance = -1;

	int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(m_iClassify, CLASS_PLAYER);
	m_flNextDistanceToCreateNPCs = CHLSS_Director::GetDirector()->GetNextDistanceToSpawn(disposition);

	m_flNextDistanceToCreateNPCs += random->RandomFloat(0, ((float)m_iNPCdifficulty) * 5.12f); //TERO: difficulty / 100 * 512

	DevMsg("New: %f\n", m_flNextDistanceToCreateNPCs);
}

int	CHLSS_Dynamic_NPC_Spawner::GetDistanceAttackSize()
{
	int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(m_iClassify, CLASS_PLAYER);

	int iAttack, iMax;
	int iPlayerSituation = CHLSS_Director::GetDirector()->m_iPlayerSituation;

	if (disposition == D_LI)
	{
		if (m_iNPCdifficulty < 50 && m_iNPCdifficulty < iPlayerSituation)
		{
			iMax = 0;
		}
		else
		{
			float scale = ((100.0f - (float)iPlayerSituation) / 100.0f) + (m_iNPCdifficulty / 200.0f); //THIS
			scale = clamp(scale, 0.0f, 1.0f);

			iMax = (int)((scale * (float)(m_iMaxAttackSize - m_iNumberOfNPCs)) + 0.6f);
			iMax = max(0, iMax);
		}

		iAttack = min(random->RandomInt(3, iMax), iMax);
	}
	else
	{
		if (m_iNPCdifficulty > 50 && m_iNPCdifficulty > (iPlayerSituation + 10))
		{
			iMax = 0;
		}
		else
		{
			float scale = ((iPlayerSituation + 10) / 100.0f) + ((100.0f - m_iNPCdifficulty) / 200.0f); //THIS
			scale = clamp(scale, 0.0f, 1.0f);

			iMax = (int)((scale * (float)(m_iMaxAttackSize - m_iNumberOfNPCs)) + 0.6f);
			iMax = max(0, iMax);
		}

		iAttack = min(random->RandomInt(3, iMax), iMax);
	}

	return iAttack;
}

void CHLSS_Dynamic_NPC_Spawner::CheckAttackSpawning()
{
	if (m_flNextSpawnTime != 0 && m_flNextSpawnTime < gpGlobals->curtime &&
		m_iNumberOfNPCs < m_iMaxNumberOfNPCs)
	{
		if (SpawnAttack())
		{
			m_iAttackSize--;
		}

		//TERO: the equation will make the delay between 0.5 and 1.0
		m_flNextSpawnTime = gpGlobals->curtime + (1.0f - ((float)CHLSS_Director::GetDirector()->m_iPlayerSituation / 200.0f)); //HLSS_DYNAMIC_NPC_SPANWER_DEFAULT_DELAY;

		//if we have finished our burst
		if (m_iAttackSize <= 0)
		{
			ClearAttackBrush();
			m_flNextSpawnTime = 0;
		}
	}
#ifdef DEBUG_DNS_ENTITY
	//DEBUG:
	else if (m_flNextSpawnTime != 0)
	{
		DevMsg("hlss_dynamic_npc_spawner: Spawning next in: %f, DNS is %d/%d full \n", m_flNextSpawnTime - gpGlobals->curtime, m_iNumberOfNPCs, m_iMaxNumberOfNPCs);
	}
#endif

}

void CHLSS_Dynamic_NPC_Spawner::GetNextAttack()
{
	//TERO: we don't need to check the burst shit now
	if (m_flNextSpawnTime != 0)
		return;

	int iPlayerSituation = CHLSS_Director::GetDirector()->m_iPlayerSituation;


	int iAttack, iMax;
	float flAdditionalTime, flAdditionalDelay;

	int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(m_iClassify, CLASS_PLAYER);

	//TERO: lets calculate the additional time for the NPC burst here
	if (disposition == D_LI)
	{
		//between -0.5 and 0.5 of the minimun burst delay
		flAdditionalTime = ((iPlayerSituation - 50.0f) / 100.0f) * m_flMinimumSpawnAttackDelay;

		flAdditionalDelay = random->RandomFloat(1.0f, 2.0f);
	}
	else
	{
		flAdditionalTime = ((50.0f - iPlayerSituation) / 100.0f) * m_flMinimumSpawnAttackDelay;
		//flAdditionalTime = random->RandomFloat(1.0f, 2.0f);

		flAdditionalDelay = random->RandomFloat(1.0f, 2.0f);
	}

	//TERO: we use flAdditionalTime to make it longer if player has too little health
	if (m_flNextSpawnAttackTime + flAdditionalTime < gpGlobals->curtime)
	{
		//TERO: the equation will make the burst delay between 1.0 and 1.5 of the minimum delay
		m_flNextSpawnAttackTime = gpGlobals->curtime + (m_flMinimumSpawnAttackDelay * (1.5f - (iPlayerSituation / 200.0f)));

		if (disposition == D_LI)
		{
			if (m_iNPCdifficulty < 50 && m_iNPCdifficulty < iPlayerSituation)
			{
				//DevMsg("hlss_dynamic_npc_spawner: difficulty below 50, no spawning\n");
				iMax = 0;
			}
			else
			{
				float scale = ((100.0f - (float)iPlayerSituation) / 100.0f) + (m_iNPCdifficulty / 200.0f); //THIS
				scale = clamp(scale, 0.0f, 1.0f);

				iMax = (int)((scale * (float)(m_iMaxAttackSize - m_iNumberOfNPCs)) + 0.6f);
				iMax = max(0, iMax);
			}

			iAttack = min(random->RandomInt(3, iMax), iMax);
		}
		else
		{
			int iAdditionalSit = clamp((gpGlobals->curtime - CHLSS_Director::GetDirector()->m_flStartTime) / HLSS_SITUATION_INCREASE_THROUGH_TIME_TIME, 0.0f, 1.0f) * 10;

			if (m_iNPCdifficulty > 50 && m_iNPCdifficulty > iPlayerSituation + iAdditionalSit)
			{
				iMax = 0;
			}
			else
			{
				float scale = ((iPlayerSituation + iAdditionalSit) / 100.0f) + ((100.0f - m_iNPCdifficulty) / 200.0f); //THIS
				scale = clamp(scale, 0.0f, 1.0f);

				iMax = (int)((scale * (float)(m_iMaxAttackSize - m_iNumberOfNPCs)) + 0.6f);
				iMax = max(0, iMax);
			}

			iAttack = min(random->RandomInt(3, iMax), iMax);
		}

		ClearAttackBrush();

		//TERO: since there are no slots available, lets add some more delay
		if (iAttack <= 0)
		{
			m_flNextSpawnAttackTime += flAdditionalDelay;
		}
		else
		{
			m_flNextSpawnTime = gpGlobals->curtime + HLSS_DYNAMIC_NPC_SPANWER_DEFAULT_DELAY;

			m_iAttackSize = iAttack;

#ifdef DEBUG_DNS_ENTITY
			DevMsg("hlss_dynamic_npc_spawner: %s starting attack of size %d/%d\n", STRING(GetEntityName()), m_iAttackSize, m_iMaxAttackSize);
#endif

		}
	}
#ifdef DEBUG_DNS_ENTITY
	//DEBUG:
	else
	{
		DevMsg("hlss_dynamic_npc_spawner: %s next attack starts in %f\n", STRING(GetEntityName()), m_flNextSpawnAttackTime + flAdditionalTime - gpGlobals->curtime);
	}
#endif

}

bool CHLSS_Dynamic_NPC_Spawner::SpawnInsideBrush(CBasePlayer *pPlayer, int iPlayerNode, CAI_BaseNPC *pNPC, CHLSS_Dynamic_Spawn_Brush* pBrush)
{
	if (!pBrush)
	{
		return false;
	}

	//booleans: first one is for visibility check, second one for closest node
	CHLSS_DNS_Node_Filter pNodeFilter(pPlayer, pNPC, GetNetwork(), iPlayerNode, 2048.0f, 128.0f, true, false);

	int iNode = GetNetwork()->NodeInsideBrush(pNPC, pBrush, &pNodeFilter, pBrush->m_iNumNodes); //NULL);

	if (iNode != NO_NODE)
	{
		Vector vecNode = GetNetwork()->GetNodePosition(pNPC, iNode);

		pNPC->SetAbsOrigin(vecNode);

		pBrush->OnSuccesfulSpawn(pPlayer, pNPC);
		return true;
	}

	return false;
}

bool CHLSS_Dynamic_NPC_Spawner::AllowedGroup(CHLSS_Dynamic_Spawn_Brush *pBrush)
{
	if (HasSpawnFlags(SF_HLSS_SPAWNER_ONLY_ALLOW_FROM_GROUP) || pBrush->HasSpawnFlags(SF_HLSS_BRUSH_ONLY_ALLOW_FROM_GROUP))
	{
		if (IDENT_STRINGS(m_iszGroupName, pBrush->m_iszGroupName))
		{
			//DevMsg("Strings match %s and %s\n", STRING( m_iszGroupName ), STRING( pBrush->m_iszGroupName ));
			return true;
		}
		else
		{
			//DevMsg("Strings do NOT match %s and %s\n", STRING( m_iszGroupName ), STRING( pBrush->m_iszGroupName ));
			return false;
		}
	}

	return true;
}

CHLSS_Dynamic_Attack_Brush* CHLSS_Dynamic_NPC_Spawner::FindAttackBrush(CBasePlayer *pPlayer, int iPlayerNode, CAI_BaseNPC *pNPC)
{
	//return (gEntList.FindEntityByClassname( NULL, "hlss_dynamic_attack_brush" ));

	int i, j;
	int numCands = 0;

#define DESTINATION_CANDIDATE_NUMS 5

	CHLSS_Dynamic_Attack_Brush *pDestCandidate[DESTINATION_CANDIDATE_NUMS];
	CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "hlss_dynamic_attack_brush");

	float flCandDist[DESTINATION_CANDIDATE_NUMS];
	float flBestDist = HLSS_MAX_DISTANCE_FROM_PLAYER;
	float flDist;

#ifdef HLSS_USE_DOT_PRODUCT_FOR_BRUSH_CANDIDATE
	float flDot;
	float flCandDot[DESTINATION_CANDIDATE_NUMS];

	Vector forward, vecDist;
	pPlayer->GetVectors(&forward, NULL, NULL);
#endif

	for (j = 0; j<DESTINATION_CANDIDATE_NUMS; j++)
	{
		pDestCandidate[j] = NULL;
		flCandDist[j] = HLSS_MAX_DISTANCE_FROM_PLAYER;
	}

	//TERO: we don't have a m_iLastDestinationGroup[i] yet, we should find one
	while (pEnt && numCands < DESTINATION_CANDIDATE_NUMS)	//TERO: we are commenting out this because we are finding the nearest cands
	{
		CHLSS_Dynamic_Attack_Brush *pDestination;

		pDestination = dynamic_cast <CHLSS_Dynamic_Attack_Brush*>(pEnt);

		if (pDestination &&
			pDestination->CanSpawnHere(pPlayer, pNPC) &&
			pDestination != m_hIgnoreBrush &&
			AllowedGroup(pDestination))
		{
			pNPC->SetAbsOrigin(pPlayer->GetAbsOrigin());
			bool bGotPath = pNPC->GetNavigator()->SetGoal(pDestination->GetLocalOrigin());

			flDist = pNPC->GetNavigator()->GetPathDistanceToGoal();

#ifdef HLSS_USE_DOT_PRODUCT_FOR_BRUSH_CANDIDATE
			vecDist = pPlayer->GetLocalOrigin() - pDestination->GetLocalOrigin();
			VectorNormalize(vecDist);
			flDot = DotProduct(vecDist, forward);
#endif

#ifdef DEBUG_DNS_ENTITY
			NDebugOverlay::Box(pDestination->GetLocalOrigin(), Vector(10, 10, 10), Vector(-10, -10, -10), 0, 0, 255, 0, 5);
			DevMsg("hlss_dynamic_npc_spawner: pNPC->GetNavigator()->GetPathDistanceToGoal() = %f\n", pNPC->GetNavigator()->GetPathDistanceToGoal());
#endif

			if (bGotPath && flDist >= HLSS_MIN_DISTANCE_FROM_PLAYER && flDist <= flBestDist + HLSS_IGNORE_FUTHER_CANDS_DIFFERENCE)
			{
				int iSlot = -1;

				if (numCands < DESTINATION_CANDIDATE_NUMS)
				{
					iSlot = numCands;
				}
				else
				{
					for (j = 0; j<DESTINATION_CANDIDATE_NUMS; j++)
					{

#ifdef HLSS_USE_DOT_PRODUCT_FOR_BRUSH_CANDIDATE
						if ((flCandDist[j] > flDist) ||
							(flCandDist[j] + 128.0f > flDist && flCandDot[j] > flDot))
						{
							if (iSlot == -1 || (flCandDist[j] > flCandDist[iSlot] &&
								(flCandDist[j] + 128.0f > flCandDist[iSlot] && flCandDot[j] > flCandDot[iSlot])))
							{
								iSlot = j;

								DevMsg("replacing candidate new/old: dist %f/%f, dot %f/%f\n", flDist, flCandDist[iSlot], flDot, flCandDot[iSlot]);
							}
						}
#else
						if (flCandDist[j] > flDist && (iSlot == -1 || flCandDist[j] > flCandDist[iSlot]))
						{
							iSlot = j;
						}
#endif
					}
				}

				if (iSlot != -1)
				{
					flCandDist[iSlot] = flDist;
					pDestCandidate[iSlot] = pDestination;

#ifdef HLSS_USE_DOT_PRODUCT_FOR_BRUSH_CANDIDATE
					flCandDot[iSlot] = flDot;
#endif

					numCands++;

					if (flDist < flBestDist)
					{
						flBestDist = flDist;
					}
				}
			}
		}

		pEnt = gEntList.FindEntityByClassname(pEnt, "hlss_dynamic_attack_brush");
	}

	if (numCands < 1)
	{
		return NULL;
	}

	if (numCands > DESTINATION_CANDIDATE_NUMS)
	{
		numCands = DESTINATION_CANDIDATE_NUMS;
	}

	i = random->RandomInt(0, numCands - 1);

	if (hlss_dynamic_npc_spawner_use_closes_to_path.GetBool())
	{
		CHLSS_Director *pDirector = CHLSS_Director::GetDirector();

		if (pDirector->m_flFullPlayerPath != -1 && pDirector->m_hNextPath != NULL)
		{
			DevMsg("hlss_dynamic_npc_spawner: choosing closest one to path, max 4096.0f\n");

			int iPathNode = GetNetwork()->NearestNodeToPoint(pNPC, pDirector->m_hNextPath->GetLocalOrigin(), false);

			if (iPathNode != NO_NODE)
			{
				Vector vecPath = GetNetwork()->GetNodePosition(pNPC, iPathNode);
				float flBestDist = 4096.0f;
				float flDist;
				bool bGotPath;

				pNPC->SetAbsOrigin(vecPath);

				for (j = 0; j<numCands; j++)
				{
					//flDist = (vecPath - pDestCandidate[j]->GetLocalOrigin()).Length();

					bGotPath = pNPC->GetNavigator()->SetGoal(pDestCandidate[j]->GetLocalOrigin());
					flDist = pNPC->GetNavigator()->GetPathDistanceToGoal();

					if (bGotPath && flDist < flBestDist)
					{
						flBestDist = flDist;
						i = j;
					}
				}

				DevMsg("best one %d with dist %f\n", i, flBestDist);
			}
		}
	}
#ifdef DEBUG_DNS_ENTITY
	else
	{
		DevMsg("hlss_dynamic_npc_spawner: picking random candidate %d\n", i);
	}
#endif

	pNPC->GetNavigator()->ClearGoal();

	return pDestCandidate[i];
}

bool CHLSS_Dynamic_NPC_Spawner::ShouldLookForNewAttackBrush()
{
	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();

	if (pDirector->m_flFullPlayerPath != -1 &&
		(pDirector->m_flActualPlayerPath < (m_flAttackBrushPlayerPath - 512.0f) ||
		pDirector->m_flActualPlayerPath >(m_flAttackBrushPlayerPath + 512.0f)))
	{
		return true;
	}

	return false;
}

bool CHLSS_Dynamic_NPC_Spawner::SpawnAttack()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
	{
		return false;
	}

	//TERO: lets create an NPC
	CAI_BaseNPC *pNPC = CreateNPC();

	if (!pNPC)
	{
		return false;
	}

	Vector vecSpawnPos = pNPC->GetAbsOrigin();

	int iPlayerNode = GetNetwork()->NearestNodeToPoint(pNPC, pPlayer->GetAbsOrigin(), false);
	if (iPlayerNode == NO_NODE)
	{
		UTIL_RemoveImmediate(pNPC);
		return false;
	}

	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();

	if (m_flNextAttackBrushSearch < gpGlobals->curtime && (!m_hAttackBrush || ShouldLookForNewAttackBrush()))
	{
		if (pDirector->m_flFullPlayerPath != -1)
		{
			DevWarning("Actual path %f, attack brush path %f, diff %f\n", pDirector->m_flActualPlayerPath, m_flAttackBrushPlayerPath, pDirector->m_flActualPlayerPath - m_flAttackBrushPlayerPath);
		}

		//DevMsg("hlss_dynamic_npc_spawner: no brush yet, trying to find one\n");
		m_hAttackBrush = FindAttackBrush(pPlayer, iPlayerNode, pNPC);
		m_flAttackBrushPlayerPath = pDirector->m_flActualPlayerPath;

		m_flNextAttackBrushSearch = gpGlobals->curtime + 1.0f;

		pNPC->SetAbsOrigin(vecSpawnPos);
	}

	if (m_hAttackBrush)
	{
		//DevMsg("hlss_dynamic_npc_spawner: found brush trying to spawn\n");
		if (SpawnInsideBrush(pPlayer, iPlayerNode, pNPC, m_hAttackBrush))
		{
			return true;
		}
		else
		{
			ClearAttackBrush();
		}
	}

	UTIL_RemoveImmediate(pNPC);
	return false;
}

void CHLSS_Dynamic_NPC_Spawner::ClearAttackBrush()
{
	CHLSS_Dynamic_Attack_Brush *pBrush = m_hAttackBrush;

	if (pBrush)
	{
		pBrush->ReleaseBrush();
	}
	m_hIgnoreBrush = m_hAttackBrush;
	m_hAttackBrush = NULL;
}

CAI_BaseNPC* CHLSS_Dynamic_NPC_Spawner::CreateNPC()
{

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	CHLSS_Director *pDirector = m_hDirector;
#else
	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();
#endif

	if (!pDirector)
	{
		return NULL;
	}

	CAI_BaseNPC	*pNPC = NULL;
	CBaseEntity *pEntity = NULL;
	MapEntity_ParseEntity(pEntity, STRING(m_iszTemplateData), NULL);
	if (pEntity != NULL)
	{
		pNPC = (CAI_BaseNPC *)pEntity;
	}

	if (!pNPC)
	{
		Warning("NULL Ent in hlss_dynamic_npc_spawner!\n");
		return NULL;
	}

	pNPC->KeyValue("additionalequipment", "0");

	pNPC->AddSpawnFlags(SF_NPC_FADE_CORPSE);
	pNPC->RemoveSpawnFlags(SF_NPC_TEMPLATE);

	DispatchSpawn(pNPC);
	pNPC->SetOwnerEntity(this);
	DispatchActivate(pNPC);

	m_iClassify = pNPC->Classify();
	m_iNumberOfNPCs++;

	if (pNPC->RemoveTimeDelay() > 0)
	{
		pNPC->m_flRemoveTime = gpGlobals->curtime + pNPC->RemoveTimeDelay();
	}

	int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pNPC->Classify(), CLASS_PLAYER);

	if (pNPC->CapabilitiesGet() & bits_CAP_USE_WEAPONS)
	{
		int iRandomizedSkill = clamp(pDirector->m_iPlayerSituation + random->RandomInt(-15, 15), 0, 100);

		if (disposition == D_LI && pDirector->CanSpawnRPG(D_LI))
		{
			pDirector->m_iNumFriendlyWeapons[HLSS_NPC_WEAPON_RPG]++;
			pNPC->GiveWeapon(MAKE_STRING("weapon_rpg"));
		}
		else if (disposition == D_HT && pDirector->CanSpawnRPG(D_HT))
		{
			pDirector->m_iNumHostileWeapons[HLSS_NPC_WEAPON_RPG]++;
			pNPC->GiveWeapon(MAKE_STRING("weapon_rpg"));
		}
		else if (iRandomizedSkill >= 90 && pDirector->CanSpawnAR2(true, disposition))
		{
			pNPC->GiveWeapon(MAKE_STRING("weapon_ar2"));

			if (disposition == D_LI)
			{
				pDirector->m_iNumFriendlyWeapons[HLSS_NPC_WEAPON_AR2]++;
			}
			else
			{
				pDirector->m_iNumHostileWeapons[HLSS_NPC_WEAPON_AR2]++;
			}
		}
		else if (iRandomizedSkill >= 70 && pDirector->CanSpawnShotgun(true, disposition))
		{
			pNPC->GiveWeapon(MAKE_STRING("weapon_shotgun"));

			if (disposition == D_LI)
			{
				pDirector->m_iNumFriendlyWeapons[HLSS_NPC_WEAPON_SHOTGUN]++;
			}
			else
			{
				pDirector->m_iNumHostileWeapons[HLSS_NPC_WEAPON_SHOTGUN]++;
			}
		}
		else
		{
			pNPC->GiveWeapon(MAKE_STRING("weapon_smg1"));
		}
	}

	return pNPC;
}

int CHLSS_Dynamic_NPC_Spawner::GetNumberOfIdleNPCsToSpawn()
{
#ifdef HLSS_USE_EHANDLE_DIRECTOR
	CHLSS_Director *pDirector = m_hDirector;
#else
	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();
#endif

	if (!pDirector)
	{
		return 0;
	}

	if (!m_bEnabled || (m_flFirstSpawnDelay != 0 && pDirector->m_flStartTime + m_flFirstSpawnDelay + (((50.0f - pDirector->m_iPlayerSituation) / 100.0f) * m_flMinimumSpawnAttackDelay * HLSS_NPC_FIRST_SPAWN_BURST_SCALE) > gpGlobals->curtime))
	{
		return 0;
	}

	int iMax = 0;

	int disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(m_iClassify, CLASS_PLAYER);

	if (disposition == D_LI)
	{
		if (m_iNPCdifficulty < 50 && m_iNPCdifficulty < pDirector->m_iPlayerSituation)
		{
			iMax = 0;
		}
		else
		{
			float scale = ((100 - pDirector->m_iPlayerSituation) / 100.0f) + (m_iNPCdifficulty / 300.0f);
			scale = clamp(scale, 0.3f, 1.0f); //TERO: 0.3 used to be zero

			iMax = (int)(scale * (float)(m_iMaxNumberOfNPCs - m_iNumberOfNPCs));
			iMax = max(0, iMax);

		}
	}
	else
	{
		//TERO: if NPC is possibly hostile, we don't spawn it until situation is 50
		if (m_iNPCdifficulty > 50 && m_iNPCdifficulty > pDirector->m_iPlayerSituation)
		{
			iMax = 0;
		}
		else
		{
			float scale = (pDirector->m_iPlayerSituation / 100.0f) + ((100.0f - m_iNPCdifficulty) / 300.0f);
			scale = clamp(scale, 0.3f, 1.0f); //TERO: 0.3 used to be zero

			iMax = (int)(scale * (float)(m_iMaxNumberOfNPCs - m_iNumberOfNPCs));
			iMax = max(0, iMax);
		}
	}

	return random->RandomInt(0, iMax);
}