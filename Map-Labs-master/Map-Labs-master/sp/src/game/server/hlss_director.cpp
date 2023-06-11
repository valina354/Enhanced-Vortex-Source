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

// global pointer to Alyx for fast lookups
CEntityClassList<CHLSS_Director> g_DirectorList;
template <> CHLSS_Director *CEntityClassList<CHLSS_Director>::m_pClassList = NULL;

ConVar	hlss_director_debug("hlss_director_debug", "0");
ConVar	hlss_director_avarage_sitution_scale("hlss_director_avarage_sitution_scale", "0.3");

LINK_ENTITY_TO_CLASS(hlss_director, CHLSS_Director);

BEGIN_DATADESC(CHLSS_Director)

DEFINE_ARRAY(m_iPlayerOldSituations, FIELD_INTEGER, HLSS_SITUATION_MEMORY),
DEFINE_FIELD(m_iIntervalsBeforeRecording, FIELD_INTEGER),

DEFINE_FIELD(m_iPlayerSituation, FIELD_INTEGER),
DEFINE_FIELD(m_iAvarageSituation, FIELD_INTEGER),
DEFINE_FIELD(m_flNextSituationUpdateTime, FIELD_TIME),
DEFINE_FIELD(m_flLastTookDamageTime, FIELD_TIME),
DEFINE_FIELD(m_flStopTakingDamageTime, FIELD_TIME),
DEFINE_FIELD(m_iLastPlayerHealth, FIELD_INTEGER),
DEFINE_FIELD(m_iPlayerHealthOnDamageStart, FIELD_INTEGER),

DEFINE_FIELD(m_flLastLoadTime, FIELD_TIME),
DEFINE_FIELD(m_flStartTime, FIELD_TIME),
DEFINE_FIELD(m_flStartTimeDifference, FIELD_FLOAT),

DEFINE_ARRAY(m_iAmmoTypes, FIELD_INTEGER, HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS),

DEFINE_FIELD(m_flWaveSituationTime, FIELD_TIME),
DEFINE_FIELD(m_flWaveSituationSpeed, FIELD_FLOAT),
DEFINE_FIELD(m_flWaveSituationSize, FIELD_FLOAT),

DEFINE_FIELD(m_iPlayerScore, FIELD_INTEGER),

DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "StartEnabled"),

DEFINE_FIELD(m_flNextUpdateItemData, FIELD_TIME),

DEFINE_ARRAY(m_bPlayerHas, FIELD_BOOLEAN, HLSS_DYNAMIC_HEALTH),
DEFINE_ARRAY(m_iItemsInMap, FIELD_INTEGER, HLSS_DYNAMIC_MAX_WEAPON_ITEMS),
DEFINE_ARRAY(m_iCratesInMap, FIELD_INTEGER, HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS),
DEFINE_ARRAY(m_flAmmoFullnessScale, FIELD_FLOAT, HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS),
DEFINE_FIELD(m_iWeaponPointsInArea, FIELD_INTEGER),

DEFINE_KEYFIELD(m_flDelayFirstAR2, FIELD_FLOAT, "FirstAR2"),
DEFINE_KEYFIELD(m_flDelayFirstShotgun, FIELD_FLOAT, "FirstShotgun"),
DEFINE_KEYFIELD(m_flDelayFirstSMG1_Grenade, FIELD_FLOAT, "FirstSMG1_Grenade"),
DEFINE_KEYFIELD(m_flDelayFirstAR2_AltFire, FIELD_FLOAT, "FirstAR2_AltFire"),
DEFINE_KEYFIELD(m_flDelayFirstRPGAfterNeeded, FIELD_FLOAT, "FirstRPGNeeded"),
DEFINE_FIELD(m_flNeedRPGStartTime, FIELD_TIME),
DEFINE_FIELD(m_bPlayerNeedsRPG, FIELD_BOOLEAN),
DEFINE_FIELD(m_iPlayerNeedsRPG, FIELD_INTEGER),

DEFINE_ARRAY(m_iNumFriendlyWeapons, FIELD_INTEGER, HLSS_NPC_WEAPON_NUM),
DEFINE_ARRAY(m_iNumHostileWeapons, FIELD_INTEGER, HLSS_NPC_WEAPON_NUM),
DEFINE_KEYFIELD(m_iMaxFriendlyWeapons[0], FIELD_INTEGER, "Max_Friendly_Shotgun"),
DEFINE_KEYFIELD(m_iMaxFriendlyWeapons[1], FIELD_INTEGER, "Max_Friendly_RPG"),
DEFINE_KEYFIELD(m_iMaxFriendlyWeapons[2], FIELD_INTEGER, "Max_Friendly_AR2"),
DEFINE_KEYFIELD(m_iMaxHostileWeapons[0], FIELD_INTEGER, "Max_Hostile_Shotgun"),
DEFINE_KEYFIELD(m_iMaxHostileWeapons[1], FIELD_INTEGER, "Max_Hostile_RPG"),
DEFINE_KEYFIELD(m_iMaxHostileWeapons[2], FIELD_INTEGER, "Max_Hostile_AR2"),

DEFINE_FIELD(m_bAllDead, FIELD_BOOLEAN),
DEFINE_FIELD(m_bAllButFriendliesDead, FIELD_BOOLEAN),

DEFINE_KEYFIELD(m_iszNextName, FIELD_STRING, "FirstPath_Name"),
DEFINE_FIELD(m_hNextPath, FIELD_EHANDLE),
DEFINE_FIELD(m_flFullPlayerPath, FIELD_FLOAT),
DEFINE_FIELD(m_flPlayerPath, FIELD_FLOAT),
DEFINE_FIELD(m_flActualPlayerPath, FIELD_FLOAT),

DEFINE_FIELD(m_flPathSituationStart, FIELD_FLOAT),
DEFINE_FIELD(m_flPathSituationEnd, FIELD_FLOAT),
DEFINE_FIELD(m_flPathSituationSize, FIELD_FLOAT),

DEFINE_FIELD(m_flLastTimeProgressed, FIELD_TIME),
DEFINE_FIELD(m_flNextDistanceToSpawnHostiles, FIELD_FLOAT),
DEFINE_FIELD(m_flNextDistanceToSpawnFriendlies, FIELD_FLOAT),

//	DEFINE_FIELD( m_iSpawnersWithLiveNPCs,			FIELD_INTEGER ),

// Function Pointers
DEFINE_THINKFUNC(DirectorThink),

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_FLOAT, "ResetStartTime", InputResetStartTime),

DEFINE_OUTPUT(m_AllDead, "AllDead"),
DEFINE_OUTPUT(m_AllButFriendliesDead, "AllButFriendliesDead"),
END_DATADESC()

//=========================================================
// Returns a pointer to Director's entity
//=========================================================
CHLSS_Director *CHLSS_Director::GetDirector(void)
{
	return g_DirectorList.m_pClassList;
}


CHLSS_Director::CHLSS_Director(void)
{
	m_flDelayFirstAR2 = HLSS_WEAPONS_GIVE_AR2_TIME;
	m_flDelayFirstShotgun = HLSS_WEAPONS_GIVE_SHOTGUN_TIME;
	m_flDelayFirstSMG1_Grenade = HLSS_WEAPONS_GIVE_SMG1_GRENADE_TIME;
	m_flDelayFirstAR2_AltFire = HLSS_WEAPONS_GIVE_AR2_ALTFIRE_TIME;

	for (int i = 0; i<HLSS_NPC_WEAPON_NUM; i++)
	{
		m_iNumFriendlyWeapons[i] = 0;
		m_iMaxFriendlyWeapons[i] = 3;
		m_iNumHostileWeapons[i] = 0;
		m_iMaxHostileWeapons[i] = 3;
	}

	m_bEnabled = false;

	m_iWeaponPointsInArea = 0; //HLSS_ITEMS_MAX_IN_AREA;

	m_flDelayFirstRPGAfterNeeded = 0;
	m_flNeedRPGStartTime = 0;
	m_bPlayerNeedsRPG = false;
	m_iPlayerNeedsRPG = 0;

	m_flStartTimeDifference = 0;

	m_iszNextName = NULL_STRING;
	m_hNextPath = NULL;
	m_flFullPlayerPath = -1;
	m_flPlayerPath = 0;
	m_flActualPlayerPath = 0;

	m_iAvarageSituation = 50;

	m_flPathSituationStart = 0;
	m_flPathSituationEnd = 0;
	m_flPathSituationSize = 0;

	m_flLastTimeProgressed = 0;
	m_flNextDistanceToSpawnHostiles = -1;
	m_flNextDistanceToSpawnFriendlies = -1;

	g_DirectorList.Insert(this);
}

CHLSS_Director::~CHLSS_Director()
{
	g_DirectorList.Remove(this);
}

void CHLSS_Director::Spawn()
{
	SetSolid(SOLID_NONE);
	Precache();

	m_flNextSituationUpdateTime = gpGlobals->curtime + HLSS_SITUATION_UPDATE_DELAY;

	m_flLastTookDamageTime = gpGlobals->curtime;
	m_flStopTakingDamageTime = 0;

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (pPlayer)
	{
		m_iLastPlayerHealth = pPlayer->GetHealth();
		m_iPlayerHealthOnDamageStart = pPlayer->GetHealth();
	}
	else
	{
		m_iLastPlayerHealth = 100;
		m_iPlayerHealthOnDamageStart = 100;
	}

	m_flLastLoadTime = gpGlobals->curtime;
	m_flStartTime = gpGlobals->curtime;

	m_iPlayerScore = 0;

	m_iIntervalsBeforeRecording = HLSS_SITUATION_INTERVALS_BEFORE_RECORD;
	m_iPlayerSituation = HLSS_START_SITUATION;
	m_iPlayerOldSituations[HLSS_SITUATION_MEMORY - 1] = clamp(m_iPlayerSituation + random->RandomInt(-1, 1), 0, 50);
	for (int i = HLSS_SITUATION_MEMORY - 1; i>0; i--)
	{
		m_iPlayerOldSituations[i - 1] = clamp(m_iPlayerOldSituations[i] + random->RandomInt(-1, 1), 0, 50);
	}

	m_flWaveSituationSpeed = random->RandomFloat(HLSS_WAVE_MAX_SPEED, HLSS_WAVE_MIN_SPEED);
	m_flWaveSituationTime = gpGlobals->curtime + m_flWaveSituationSpeed;
	m_flWaveSituationSize = HLSS_WAVE_SITUATION_BASE + (cos(random->RandomFloat(0, M_PI_F) * HLSS_WAVE_SITUATION_DIFFERENCE));

	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_PISTOL] = GetAmmoDef()->Index("Pistol");
	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_SMG1] = GetAmmoDef()->Index("SMG1");
	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_SHOTGUN] = GetAmmoDef()->Index("Buckshot");
	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_AR2] = GetAmmoDef()->Index("AR2");
	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_RPG] = GetAmmoDef()->Index("RPG_Round");
	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_GRENADE] = GetAmmoDef()->Index("Grenade");
	m_iAmmoTypes[HLSS_DYNAMIC_AMMO_CRATE_357] = GetAmmoDef()->Index("357");

	UpdateItemData(true);
	//UPDATE as soon as possible
	m_flNextUpdateItemData = gpGlobals->curtime; // + random->RandomFloat(0, HLSS_ITEM_DATA_UPDATE_DELAY);

	m_hNextPath = (CHLSS_Player_Path *)gEntList.FindEntityByName(NULL, m_iszNextName);

	SetThink(&CHLSS_Director::DirectorThink);
	SetNextThink(gpGlobals->curtime);

}

void CHLSS_Director::Precache()
{
	//TERO: ammo crate stuff

	PrecacheScriptSound("AmmoCrate.Open");
	PrecacheScriptSound("AmmoCrate.Close");

	PrecacheModel("models/items/ammocrate_bullets.mdl");	// Small rounds
	PrecacheModel("models/items/ammocrate_smg1.mdl");		// Medium rounds
	PrecacheModel("models/items/ammocrate_ar2.mdl");		// Large rounds
	PrecacheModel("models/items/ammocrate_rockets.mdl");	// RPG rounds
	PrecacheModel("models/items/ammocrate_buckshot.mdl");	// Buckshot
	PrecacheModel("models/items/ammocrate_grenade.mdl");	// Grenades

	UTIL_PrecacheOther("item_item_crate");
}


void CHLSS_Director::OnRestore()
{
	m_flLastLoadTime = gpGlobals->curtime;
}

void CHLSS_Director::InputEnable(inputdata_t &inputdata)
{
	if (!m_bEnabled)
	{
		m_bAllDead = false;
		m_bAllButFriendliesDead = false;

		SetNextThink(gpGlobals->curtime);
		m_bEnabled = true;

		DevMsg("Start time difference %f\n", m_flStartTimeDifference);

		m_flStartTime = gpGlobals->curtime + m_flStartTimeDifference;
	}
}

void CHLSS_Director::InputResetStartTime(inputdata_t &inputdata)
{
	m_flStartTime = gpGlobals->curtime + inputdata.value.Float();
}

void CHLSS_Director::InputDisable(inputdata_t &inputdata)
{
	if (m_bEnabled)
	{
		m_bEnabled = false;

		m_flStartTimeDifference = m_flStartTime - gpGlobals->curtime;
	}
}

void CHLSS_Director::DirectorThink()
{
	if (m_bEnabled)
	{
		//TERO: these two shouldn't be called every frame if the spawner is disabled. Instead, they will be called
		//		via the GetItemToSpawn and GetAmmoCrateToSpawn
		UpdatePlayerPath();
		UpdatePlayerSituation();
		UpdateItemData();

		SetNextThink(gpGlobals->curtime);
	}
}

void CHLSS_Director::UpdatePlayerSituation()
{
	if (m_flNextSituationUpdateTime > gpGlobals->curtime)
	{
		return;
	}

	m_flNextSituationUpdateTime = gpGlobals->curtime + HLSS_SITUATION_UPDATE_DELAY;


	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_director: no player\n");
		return;
	}

	float flPathReachedRatio = 0;
	if (m_flFullPlayerPath > 0)
	{
		flPathReachedRatio = m_flPlayerPath / m_flFullPlayerPath;
	}

	float hostileDiff = 0.0f;
	float hostileMax = 0.0f;
	float friendlyDiff = 0.0f;
	float friendlyMax = 0.0f;

	int disposition = D_ER;
	int difficulty;

	int iDistAttackSize;

	CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "hlss_dynamic_npc_spawner");
	CHLSS_Dynamic_NPC_Spawner *pSpawner = dynamic_cast <CHLSS_Dynamic_NPC_Spawner*>(pEnt);

	while (pSpawner)
	{
		if (pSpawner->m_bEnabled)
		{
			disposition = CBaseCombatCharacter::GetDefaultRelationshipDispositionBetweenClasses(pSpawner->m_iClassify, CLASS_PLAYER);

			if (disposition == D_HT)
			{
				difficulty = pSpawner->m_iNPCdifficulty * pSpawner->m_iNPCdifficulty;

				iDistAttackSize = pSpawner->m_iNumToSpawnInDistance;
				if (iDistAttackSize < 0)
				{
					iDistAttackSize = 0;
				}

				//TERO: we take the burst size into account too so that NPCs that will soon be spawned are counted
				hostileDiff += (float)((pSpawner->m_iNumberOfNPCs + pSpawner->m_iAttackSize + iDistAttackSize) * difficulty);
				hostileMax += (float)(pSpawner->m_iMaxNumberOfNPCs * difficulty);
			}
			else if (disposition == D_LI)
			{
				difficulty = (100 - pSpawner->m_iNPCdifficulty) * (100 - pSpawner->m_iNPCdifficulty);

				//TERO: we take the burst size into account too so that NPCs that will soon be spawned are counted
				//		maybe we shouldn't be doing this for friendly NPCs, though
				friendlyDiff += (float)((pSpawner->m_iNumberOfNPCs + pSpawner->m_iAttackSize) * difficulty);
				friendlyMax += (float)(pSpawner->m_iMaxNumberOfNPCs * difficulty);
			}
		}

		pEnt = gEntList.FindEntityByClassname(pEnt, "hlss_dynamic_npc_spawner");
		pSpawner = dynamic_cast <CHLSS_Dynamic_NPC_Spawner*>(pEnt);
	}

	float flHostileScale = (hostileMax != 0) ? (hostileDiff / hostileMax) : 0.0f;
	float flFriendlyScale = (friendlyMax != 0) ? (friendlyDiff / friendlyMax) : 0.0f;
	float flNPCScale = ((friendlyDiff + hostileDiff) != 0) ? (((flFriendlyScale * friendlyDiff) - (flHostileScale * hostileDiff)) / (hostileDiff + friendlyDiff)) : 0.0f;

	//float flAfterLoad = clamp(((gpGlobals->curtime - m_flLastLoadTime) / HLSS_EASY_TIME_AFTER_LOAD), 0.0f, 1.0f);

	int iHealth = pPlayer->GetHealth();

	//TERO: lets start recording the damage take
	if (iHealth < m_iLastPlayerHealth && m_flStopTakingDamageTime == 0)
	{
		m_iLastPlayerHealth = iHealth;
		m_iPlayerHealthOnDamageStart = iHealth;
		m_flStopTakingDamageTime = gpGlobals->curtime;
	}

	int iDamage = clamp(m_iPlayerHealthOnDamageStart - iHealth, 0, 100);

	if (m_flStopTakingDamageTime != 0 && m_flStopTakingDamageTime + 1.0f < gpGlobals->curtime)
	{
		if (iHealth < m_iLastPlayerHealth)
		{
			m_iLastPlayerHealth = iHealth;
			m_flStopTakingDamageTime = gpGlobals->curtime;

			m_flLastTookDamageTime = gpGlobals->curtime;
		}
		else
		{
			m_iLastPlayerHealth = iHealth;
			m_flStopTakingDamageTime = 0;
		}
	}

	float flDamageTime = 1.0f - clamp(((gpGlobals->curtime - m_flLastTookDamageTime) / HLSS_DAMAGE_TIME_EASY), 0.0f, 1.0f);

	if (flDamageTime >= 0.6f)
	{
		flDamageTime = 0.8f + ((flDamageTime - 0.6f) *  0.5f);
	}
	else
	{
		flDamageTime *= ((1.0f / 0.6f) * 0.8f);
	}

	float flNewSituation = HLSS_SITUATION_BASE + ((float)g_pGameRules->GetSkillLevel()) * HLSS_SITUATION_GAME_DIFFICULTY;

	flNewSituation -= (clamp((flDamageTime * 1.2f * (((float)iDamage + 50.0f) / 100.0f)), 0.0f, 1.0f) * HLSS_SITUATION_DAMAGE);

	//TERO: we shoul add int ammo
	float flHealthScalar = HLSS_SITUATION_HEALTH;

	if (iHealth < 25)
	{
		flHealthScalar = HLSS_SITUATION_HEALTH_LOW;
	}

	flNewSituation += ((((float)iHealth) - 50.0f) / 50.0f) * flHealthScalar;

	//TERO: NPC situation
	flNewSituation += (flNPCScale * HLSS_SITUATION_NPC); //(flFriendlyScale - flHostileScale) * 40.0f;

	float flWeapons = 0;

	//TERO: this doesn't take into count grenades or smg1 grenades, etc.
	for (int i = HLSS_DYNAMIC_WEAPON_PISTOL; i<HLSS_DYNAMIC_WEAPON_STUNSTICK; i++)
	{
		//if ((pPlayer->Weapon_OwnsThisType( STRING(iszWeaponNames[i]) ) != NULL))
		if ((pPlayer->Weapon_OwnsThisType(m_lpzItemNames[i]) != NULL))
		{
			if (m_iAmmoTypes[i] != -1)
			{
				int iMax = GetAmmoDef()->MaxCarry(m_iAmmoTypes[i]);
				int iCount = pPlayer->GetAmmoCount(m_iAmmoTypes[i]);

				if (iMax != 0)
				{
					flWeapons += (float)iCount / (float)iMax;
				}
			}
		}
	}

	flWeapons = clamp(flWeapons * HLSS_MAX_WEAPONS_CARRY_SCALE, 0.0f, 1.0f);

	flNewSituation += flWeapons * HLSS_SITUATION_WEAPONS;


	//********************
	//SITUATION FROM SCORE
	//********************

	float flScore = clamp((float)(HLSS_SITUATION_SCORE_MAX - m_iPlayerScore) / HLSS_SITUATION_SCORE_MAX, 0.0f, 1.0f) * HLSS_SITUATION_SCORE;

	//Warning("Player score %d, situation from score %f\n", m_iPlayerScore, flScore);

	flNewSituation += flScore;

	//*******************
	//SITUATION DEBUGGING
	//*******************

	//flNewSituation = ((flNewSituation * 0.25f) + (((float)m_iPlayerSituation) * 0.75)); //TERO: this is the old way of considering the old situations

	if (hlss_director_debug.GetInt() == 1)
	{
		Msg("hlss_dynamic_npc_spawner: Hostile scale %f, Friendly scale %f, NPC situation %f, Weapon sit. %f\n", flHostileScale, flFriendlyScale, flNPCScale * HLSS_SITUATION_NPC, flWeapons * HLSS_SITUATION_WEAPONS);
		Msg("hlss_dynamic_npc_spawner: score sit %f damage sit. %f, health sit. %f\n", flScore, -(clamp((flDamageTime * 1.2f * (((float)iDamage + 50.0f) / 100.0f)), 0.0f, 1.0f) * HLSS_SITUATION_DAMAGE), ((((float)iHealth) - 50.0f) / 100.0f) * flHealthScalar);
		DevMsg("hlss_dynamic_npc_spawner: situation before clamping %f!!!\n", flNewSituation);
	}

	//DevMsg("hlss_dynamic_npc_spawner: Weapons sit. addition: %f NPC sit. addition: %f\n", flWeapons, flNPCScale * HLSS_SITUATION_NPC);


#ifdef HLSS_DIRECTOR_AFTERLOAD
	flNewSituation = clamp(flNewSituation, 0.0f, 50.0f + (flAfterLoad * 50.0f));
#endif



	//*****************************
	// OLD SITUATIONS
	//*****************************

	m_iIntervalsBeforeRecording--;

	//TERO: if we have had enough intervals, lets push the old situations back
	if (m_iIntervalsBeforeRecording <= 0)
	{
		for (int i = 1; i<HLSS_SITUATION_MEMORY; i++)
		{
			m_iPlayerOldSituations[i - 1] = m_iPlayerOldSituations[i];
		}

		m_iIntervalsBeforeRecording = HLSS_SITUATION_INTERVALS_BEFORE_RECORD;
	}

	//TERO: the new situation should always be recorded, though
	m_iPlayerOldSituations[HLSS_SITUATION_MEMORY - 1] = (int)flNewSituation;

	int iTotalNumberOfIncreases = 0;
	int	iTotalNumberOfDecreases = 0;
	float flAvarageSituation = (float)m_iPlayerOldSituations[0];

	int iSituationDifference;

	for (int i = 1; i<HLSS_SITUATION_MEMORY; i++)
	{
		iSituationDifference = m_iPlayerOldSituations[i] - m_iPlayerOldSituations[i - 1];

		if (iSituationDifference > 0) //m_iPlayerOldSituations[i-1] < m_iPlayerOldSituations[i])
		{
			iTotalNumberOfIncreases += iSituationDifference;
		}

		else if (iSituationDifference < 0) //m_iPlayerOldSituations[i-1] > m_iPlayerOldSituations[i])
		{
			iTotalNumberOfDecreases -= iSituationDifference;
		}

		flAvarageSituation += (float)m_iPlayerOldSituations[i];

		if (hlss_director_debug.GetInt() == 3)
		{
			Vector vecForward, vecRight, vecUp;
			pPlayer->GetVectors(&vecForward, &vecRight, &vecUp);

			Vector vecOrigin = pPlayer->GetAbsOrigin() + (vecForward * 128.0f);

			NDebugOverlay::Line(vecOrigin - (vecRight * 64.0f), vecOrigin + (vecRight * 64.0f), 0, 0, 255, true, HLSS_SITUATION_UPDATE_DELAY);
			NDebugOverlay::Line(vecOrigin - (vecRight * 64.0f) + (vecUp * 128.0f), vecOrigin + (vecRight * 64.0f) + (vecUp * 128.0f), 0, 0, 255, true, HLSS_SITUATION_UPDATE_DELAY);

			NDebugOverlay::Line(vecOrigin - (vecRight * 64.0f) + (vecUp * 128.0f), vecOrigin - (vecRight * 64.0f), 0, 0, 255, true, HLSS_SITUATION_UPDATE_DELAY);
			NDebugOverlay::Line(vecOrigin + (vecRight * 64.0f) + (vecUp * 128.0f), vecOrigin + (vecRight * 64.0f), 0, 0, 255, true, HLSS_SITUATION_UPDATE_DELAY);

			Vector vecStart = ((float)((float)((i - 1) - HLSS_SITUATION_MEMORY_HALF) / HLSS_SITUATION_MEMORY) * 128.0f * vecRight) + ((m_iPlayerOldSituations[i - 1] * 1.28f) * vecUp);
			Vector vecEnd = ((float)((float)(i - HLSS_SITUATION_MEMORY_HALF) / HLSS_SITUATION_MEMORY) * 128.0f * vecRight) + ((m_iPlayerOldSituations[i] * 1.28f) * vecUp);
			NDebugOverlay::Line(vecOrigin + vecStart, vecOrigin + vecEnd, 255, 0, 0, true, HLSS_SITUATION_UPDATE_DELAY);
		}
	}

	flAvarageSituation = (flAvarageSituation / (float)HLSS_SITUATION_MEMORY);
	m_iAvarageSituation = (int)flAvarageSituation;

	float flAvarageScale = clamp(hlss_director_avarage_sitution_scale.GetFloat(), 0.0f, 1.0f);

	if (hlss_director_debug.GetInt() == 4)
	{
		DevMsg("Avarage Situation %d, current situation %f, avarage scale %f, ", m_iAvarageSituation, flNewSituation, flAvarageScale);
	}

	flNewSituation = (flAvarageScale * flAvarageSituation) + ((1.0f - flAvarageScale) * flNewSituation);

	if (hlss_director_debug.GetInt() == 4)
	{
		DevMsg("new situation %f\n", flNewSituation);
	}

	if (iTotalNumberOfIncreases > HLSS_SITUATION_MEMORY_HALF)
	{
		if (hlss_director_debug.GetInt() == 1)
		{
			DevWarning("hlss_dynamic_npc_spanwer: situation increases in the %d recordings during %f seconds: %d\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY * HLSS_SITUATION_INTERVALS_BEFORE_RECORD), iTotalNumberOfIncreases);
			DevMsg("hlss_dynamic_npc_spanwer: situation decreases in the %d recordings during %f seconds: %d\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY * HLSS_SITUATION_INTERVALS_BEFORE_RECORD), iTotalNumberOfDecreases);
		}

	}
	else if (iTotalNumberOfDecreases > HLSS_SITUATION_MEMORY_HALF)
	{
		if (hlss_director_debug.GetInt() == 1)
		{
			DevMsg("hlss_dynamic_npc_spanwer: situation increases in the %d recordings during %f seconds: %d\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY * HLSS_SITUATION_INTERVALS_BEFORE_RECORD), iTotalNumberOfIncreases);
			DevWarning("hlss_dynamic_npc_spanwer: situation decreases in the %d recordings during %f seconds: %d\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY * HLSS_SITUATION_INTERVALS_BEFORE_RECORD), iTotalNumberOfDecreases);
		}

		//TERO: I am decreasing the situation here even more to help the player
		flNewSituation -= HLSS_SITUATION_LONG_DECREASE_HELP;
	}
	else
	{
		if (hlss_director_debug.GetInt() == 1)
		{
			DevMsg("hlss_dynamic_npc_spanwer: situation increases in the %d recordings during %f seconds: %d\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY * HLSS_SITUATION_INTERVALS_BEFORE_RECORD), iTotalNumberOfIncreases);
			DevMsg("hlss_dynamic_npc_spanwer: situation decreases in the %d recordings during %f seconds: %d\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY * HLSS_SITUATION_INTERVALS_BEFORE_RECORD), iTotalNumberOfDecreases);
		}
	}

	//DevMsg("hlss_dynamic_npc_spawner: avarage situation in the %d recordings during %f seconds: %f\n", HLSS_SITUATION_MEMORY, ((float)HLSS_SITUATION_MEMORY * HLSS_SITUATION_UPDATE_DELAY), iAvarageSituation);

	/*if (iTotalNumberOfIncreases > iTotalNumberOfDecreases)
	{
	//TERO: the situation has grown in the last 4 seconds, lets ease up a little bit (Game Difficulty variable?)
	flNewSituation -= (iTotalNumberOfIncreases * HLSS_SITUATION_MORE_INCREASES_HELP);
	}
	else if ( iTotalNumberOfIncreases < iTotalNumberOfDecreases)
	{
	//TERO: the situation has gone down in the last 4 seconds, lets help the player by putting the situation even more down (Game Difficulty variable?)
	flNewSituation -= (iTotalNumberOfDecreases * HLSS_SITUATION_MORE_DECREASES_HELP);
	}*/




	float flWave = 1.0f - ((m_flWaveSituationTime - gpGlobals->curtime) / m_flWaveSituationSpeed);

	if (flWave >= 1.0f)
	{
		m_flWaveSituationSpeed = random->RandomFloat(HLSS_WAVE_MAX_SPEED, HLSS_WAVE_MIN_SPEED);
		m_flWaveSituationTime = gpGlobals->curtime + m_flWaveSituationSpeed;
		m_flWaveSituationSize = HLSS_WAVE_SITUATION_BASE + (cos(random->RandomFloat(0, M_PI_F * 0.5f) * HLSS_WAVE_SITUATION_DIFFERENCE));
		flWave = 0.0f;
	}
	else
	{
		flWave = 2.0f * M_PI_F * clamp(flWave, 0.0f, 1.0f);
		flWave = sin(flWave) * m_flWaveSituationSize;
	}

	//DevMsg("hlss_dynamic_npc_spawner: Additional Situation by Wave %f\n", flWave);

	flWave = (clamp((gpGlobals->curtime - m_flStartTime) / HLSS_SITUATION_INCREASE_THROUGH_TIME_TIME, 0.0f, 1.0f) * HLSS_SITUATION_INCREASE_THROUGH_TIME_MAX);

	if (hlss_director_debug.GetInt() == 1)
	{
		DevMsg("hlss_dynamic_npc_spawner: Additional Situation by time %f\n", flWave);
	}

	flNewSituation += flWave;



	if (m_flPlayerPath > m_flPathSituationEnd || m_flPathSituationEnd == 0)
	{
		m_flPathSituationStart = m_flPlayerPath;
		m_flPathSituationEnd = m_flPathSituationStart + random->RandomFloat(1024.0f, 2048.0f);
		m_flPathSituationSize = random->RandomFloat(20.0f, 40.0f);

		if (hlss_director_debug.GetInt() == 1)
		{
			DevMsg("hlss_director: new path situation, start %f, end %f, dist %f, size %f", m_flPathSituationStart, m_flPathSituationEnd, (m_flPathSituationEnd - m_flPathSituationStart), m_flPathSituationSize);
		}
		flWave = 0.0;
	}
	else
	{
		flWave = (m_flPathSituationEnd - m_flPlayerPath) / (m_flPathSituationEnd - m_flPathSituationStart);
		if (hlss_director_debug.GetInt() == 1)
		{
			DevMsg("hlss_director: path situation ration %f, distance %f", flWave, m_flPathSituationEnd - m_flPlayerPath);
		}
	}

	flWave = sin(M_PI_F * flWave) * m_flPathSituationSize;

	if (hlss_director_debug.GetInt() == 1)
	{
		DevMsg(", situation addition %f\n", flWave);
	}

	flNewSituation += flWave;


	//m_iPlayerSituation = (int)clamp(flNewSituation,0,100);

	//TERO: I disabled this because it was making the wait for a map too long
#ifdef HLSS_DIRECTOR_AFTERLOAD
	m_iPlayerSituation = (int)clamp(flNewSituation, 0.0f, 50.0f + (flAfterLoad * 50.0f));
#else
	m_iPlayerSituation = (int)flNewSituation;
#endif

	if (hlss_director_debug.GetInt() == 1)
	{
		DevMsg("hlss_dynamic_npc_spanwer: new player situation: %d\n", m_iPlayerSituation);
	}
}

// Item entity names
const char *CHLSS_Director::m_lpzCrateItemNames[HLSS_DYNAMIC_MAX_CRATE_ITEMS] =
{
	"item_ammo_pistol",
	"item_ammo_smg1",
	"item_ammo_ar2_large",
	"item_rpg_round",
	"item_box_buckshot",
	"item_healthkit",
	"item_battery",
};

int CHLSS_Director::GetItemToSpawnForCrate()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_dynamic_npc_spawner: no player\n");
		return HLSS_DYNAMIC_CRATE_ITEM_NOTHING;
	}

	//TERO: if we get asked while the dynamic spawner is disabled
	if (!m_bEnabled)
	{
		UpdateItemData();
		UpdatePlayerSituation();
	}

	int iItemToSpawn = HLSS_DYNAMIC_CRATE_ITEM_NOTHING;

	int iDamageTimePunishment = (int)((4.0f - clamp(gpGlobals->curtime - m_flStopTakingDamageTime, 0, 4.0f)) * 5.0f);

	if (pPlayer->m_iHealth <= (75 - iDamageTimePunishment - (m_iItemsInMap[HLSS_DYNAMIC_HEALTH] * 34)) &&
		random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_HEALTHKIT_PROBABILITY)
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_HEALTHKIT;
	}
	else if (random->RandomInt(0, 3) == 0 && m_bPlayerHas[HLSS_DYNAMIC_WEAPON_RPG] && m_flAmmoFullnessScale[HLSS_DYNAMIC_AMMO_CRATE_RPG] < 1.0f && m_iPlayerSituation < 60)
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_ITEM_RPG_AMMO;
	}
	else if (random->RandomInt(0, 2) == 0 && m_bPlayerHas[HLSS_DYNAMIC_WEAPON_AR2] && m_iItemsInMap[HLSS_DYNAMIC_AMMO_AR2] < 5)
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_ITEM_AR2_AMMO;
	}
	else if (random->RandomInt(0, 2) == 0 && (m_bPlayerHas[HLSS_DYNAMIC_WEAPON_SHOTGUN] || random->RandomInt(0, 1) == 0) && m_iItemsInMap[HLSS_DYNAMIC_AMMO_SHOTGUN] < 5)
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_ITEM_SHOTGUN_AMMO;
	}
	else if (random->RandomInt(0, 2) == 0 && m_iItemsInMap[HLSS_DYNAMIC_AMMO_SMG1] < 5)
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_ITEM_SMG1_AMMO;
	}
	else if (random->RandomInt(0, 2) == 0 && m_iItemsInMap[HLSS_DYNAMIC_BATTERY] < 4 && m_iPlayerSituation < 50)
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_BATTERY;
	}
	else
	{
		iItemToSpawn = HLSS_DYNAMIC_CRATE_ITEM_PISTOL_AMMO;
	}

	return iItemToSpawn;
}

//---------------------------------------------
//
//---------------------------------------------
int CHLSS_Director::GetItemToSpawn()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_dynamic_npc_spawner: no player\n");
		return HLSS_DYNAMIC_WEAPON_NOTHING;
	}

	//TERO: if we get asked while the dynamic spawner is disabled
	if (!m_bEnabled)
	{
		UpdateItemData();
		UpdatePlayerSituation();
	}

	int i;
	int iItemToSpawn = HLSS_DYNAMIC_WEAPON_NOTHING;

	//bool bPlayerNeedsRPG = false;

	int iDamageTimePunishment = (int)((4.0f - clamp(gpGlobals->curtime - m_flStopTakingDamageTime, 0, 4.0f)) * 5.0f);

	if (pPlayer->m_iHealth <= (75 - iDamageTimePunishment - (m_iItemsInMap[HLSS_DYNAMIC_HEALTH] * 34)) &&
		random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_HEALTHKIT_PROBABILITY)
	{
		iItemToSpawn = HLSS_DYNAMIC_HEALTH;
	}
	else if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_PISTOL] && m_iItemsInMap[HLSS_DYNAMIC_WEAPON_PISTOL] < 1)
	{
		iItemToSpawn = HLSS_DYNAMIC_WEAPON_PISTOL;
	}
	else if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_STUNSTICK] && (m_iItemsInMap[HLSS_DYNAMIC_WEAPON_STUNSTICK] < 1))
	{
		iItemToSpawn = HLSS_DYNAMIC_WEAPON_STUNSTICK;
	}
	else if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_RPG] &&
		m_iItemsInMap[HLSS_DYNAMIC_WEAPON_RPG] < 1 &&
		random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_RPG_PROBABILITY &&
		CanSpawnRPG())
	{
		iItemToSpawn = HLSS_DYNAMIC_WEAPON_RPG;
	}
	else if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_AR2] &&
		m_iItemsInMap[HLSS_DYNAMIC_WEAPON_AR2] < 2 &&
		m_iPlayerSituation < 25 &&
		random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_AR2_PROBABILITY &&
		CanSpawnAR2(false))
	{
		iItemToSpawn = HLSS_DYNAMIC_WEAPON_AR2;
	}
	else if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_SHOTGUN] &&
		m_iItemsInMap[HLSS_DYNAMIC_WEAPON_SHOTGUN] < 3 &&
		m_iPlayerSituation < 40 &&
		random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_SHOTGUN_PROBABILITY &&
		CanSpawnShotgun(false))
	{
		iItemToSpawn = HLSS_DYNAMIC_WEAPON_SHOTGUN;
	}
	else if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_SMG1] && m_iItemsInMap[HLSS_DYNAMIC_WEAPON_SMG1] < 2)
	{
		iItemToSpawn = HLSS_DYNAMIC_WEAPON_SMG1;
	}
	else
	{

#define HLSS_DYNAMIC_ITEMS_RANDOM_MAX 8

		int iAmmoToSpawn = HLSS_DYNAMIC_AMMO_SMG1;

		if (m_bPlayerHas[HLSS_DYNAMIC_WEAPON_AR2])
		{
			iAmmoToSpawn = HLSS_DYNAMIC_AMMO_AR2;
		}
		else if (m_bPlayerHas[HLSS_DYNAMIC_WEAPON_SHOTGUN])
		{
			iAmmoToSpawn = HLSS_DYNAMIC_AMMO_SHOTGUN;
		}

		int iRandomItems[HLSS_DYNAMIC_ITEMS_RANDOM_MAX];
		for (i = 0; i<HLSS_DYNAMIC_ITEMS_RANDOM_MAX; i++)
		{
			iRandomItems[i] = HLSS_DYNAMIC_WEAPON_NOTHING;
		}

		if (pPlayer->m_iHealth <= (75 - (m_iItemsInMap[HLSS_DYNAMIC_HEALTH] * 20)) &&
			random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_HEALTHKIT_SMALL_PROBABILITY)
		{
			iRandomItems[0] = HLSS_DYNAMIC_HEALTH;
		}
		else
		{
			iRandomItems[0] = HLSS_DYNAMIC_BATTERY;
		}

		if (m_iItemsInMap[HLSS_DYNAMIC_BATTERY] < 3 && m_iPlayerSituation < 50)
		{
			iRandomItems[1] = HLSS_DYNAMIC_BATTERY;
		}

		if (m_iPlayerSituation < 70)
		{
			iRandomItems[2] = HLSS_DYNAMIC_WEAPON_GRENADE;
		}

		if (random->RandomInt(0, 4) == 0)
		{
			//TERO: sometimes randomly spawn pistol or smg1
			iRandomItems[3] = random->RandomInt(HLSS_DYNAMIC_WEAPON_PISTOL, HLSS_DYNAMIC_WEAPON_SMG1);
		}

		if (random->RandomInt(0, 2) == 0 && m_iPlayerSituation < 75)
		{
			//TERO: sometimes randomly spawn a grenade, ar2 altfire or battery
			iRandomItems[4] = HLSS_DYNAMIC_BATTERY;
			iRandomItems[5] = HLSS_DYNAMIC_BATTERY;

			if ((m_bPlayerHas[HLSS_DYNAMIC_WEAPON_AR2] || m_iItemsInMap[HLSS_DYNAMIC_WEAPON_AR2] > 0) && m_iItemsInMap[HLSS_DYNAMIC_AR2_ALTFIRE] < 3 &&
				CanSpawnAR2_AltFire())
			{
				iRandomItems[4] = HLSS_DYNAMIC_AR2_ALTFIRE;
			}

			if (m_iItemsInMap[HLSS_DYNAMIC_SMG1_GRENADE] < 3 && CanSpawnSMG1_Grenade() && random->RandomInt(0, m_iWeaponPointsInArea) < HLSS_GRENADE_PROBABILITY)
			{
				iRandomItems[5] = HLSS_DYNAMIC_SMG1_GRENADE;
			}
		}
		else
		{
			iRandomItems[4] = iAmmoToSpawn;
			iRandomItems[5] = iAmmoToSpawn;
		}

		if (random->RandomInt(0, 2))
		{
			iRandomItems[7] = iAmmoToSpawn;
		}

		if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_CROWBAR] && m_iItemsInMap[HLSS_DYNAMIC_WEAPON_CROWBAR] < 1)
		{
			iRandomItems[6] = HLSS_DYNAMIC_WEAPON_CROWBAR;
			//iRandomItems[7] = HLSS_DYNAMIC_WEAPON_CROWBAR;
		}

		if (!m_bPlayerHas[HLSS_DYNAMIC_WEAPON_357] && m_iItemsInMap[HLSS_DYNAMIC_WEAPON_357])
		{
			iRandomItems[7] = HLSS_DYNAMIC_WEAPON_357;
		}

		return iRandomItems[random->RandomInt(0, HLSS_DYNAMIC_ITEMS_RANDOM_MAX - 1)];

		/*int j = random->RandomInt(0,3);

		for (i=0; i<4; i++)
		{
		if (iRandomItems[i] != -1)
		{
		return iRandomItems[j];
		}

		j++;
		if (j>3)
		j = 0;
		}*/
	}

	return iItemToSpawn;
}

const float flNPCWeaponsScaleByGameDifficulty[3] = { 1.2f, 1.0f, 0.8f };

bool CHLSS_Director::CanSpawnAR2(bool bNPC, int disposition)
{
	if (m_flDelayFirstAR2 < 0)
		return false;

	if (bNPC)
	{
		if (m_flStartTime + (m_flDelayFirstAR2 * flNPCWeaponsScaleByGameDifficulty[g_pGameRules->GetSkillLevel() - 1]) < gpGlobals->curtime)
		{
			if (disposition == D_LI)
			{
				return ((m_iNumFriendlyWeapons[HLSS_NPC_WEAPON_AR2] < m_iMaxHostileWeapons[HLSS_NPC_WEAPON_AR2]) ||
					m_iMaxFriendlyWeapons[HLSS_NPC_WEAPON_AR2] == -1);
			}
			else //if (disposition == D_HT)
			{
				return ((m_iNumHostileWeapons[HLSS_NPC_WEAPON_AR2] < m_iMaxHostileWeapons[HLSS_NPC_WEAPON_AR2]) ||
					m_iMaxHostileWeapons[HLSS_NPC_WEAPON_AR2] == -1);
			}
		}

		return false;
	}

	return (m_flStartTime + m_flDelayFirstAR2 < gpGlobals->curtime);
}

bool CHLSS_Director::CanSpawnShotgun(bool bNPC, int disposition)
{
	if (m_flDelayFirstShotgun < 0)
		return false;

	if (bNPC)
	{
		if (m_flStartTime + (m_flDelayFirstShotgun * flNPCWeaponsScaleByGameDifficulty[g_pGameRules->GetSkillLevel() - 1]) < gpGlobals->curtime)
		{
			if (disposition == D_LI)
			{
				return ((m_iNumFriendlyWeapons[HLSS_NPC_WEAPON_SHOTGUN] < m_iMaxHostileWeapons[HLSS_NPC_WEAPON_SHOTGUN]) ||
					m_iMaxFriendlyWeapons[HLSS_NPC_WEAPON_SHOTGUN] == -1);
			}
			else //if (disposition == D_HT)
			{
				return ((m_iNumHostileWeapons[HLSS_NPC_WEAPON_SHOTGUN] < m_iMaxHostileWeapons[HLSS_NPC_WEAPON_SHOTGUN]) ||
					m_iMaxHostileWeapons[HLSS_NPC_WEAPON_SHOTGUN] == -1);
			}
		}

		return false;
	}

	return (m_flStartTime + m_flDelayFirstShotgun < gpGlobals->curtime);
}

bool CHLSS_Director::CanSpawnSMG1_Grenade()
{
	if (m_flDelayFirstSMG1_Grenade < 0)
		return false;

	return (m_flStartTime + m_flDelayFirstSMG1_Grenade < gpGlobals->curtime);
}

bool CHLSS_Director::CanSpawnAR2_AltFire()
{
	if (m_flDelayFirstAR2_AltFire < 0)
		return false;

	return (m_flStartTime + m_flDelayFirstAR2_AltFire < gpGlobals->curtime);
}

bool CHLSS_Director::CanSpawnRPG(int disposition)
{
	return false;

	if (disposition == D_HT)
	{
		return ((m_iNumHostileWeapons[HLSS_NPC_WEAPON_RPG] < m_iMaxHostileWeapons[HLSS_NPC_WEAPON_RPG]) || m_iMaxHostileWeapons[HLSS_NPC_WEAPON_RPG] == -1);
	}

	return m_bPlayerNeedsRPG;
}

int CHLSS_Director::GetAmmoCrateToSpawn()
{
	int iAmmoCrate = -1;
	int iSituation = (m_iAvarageSituation * 0.6) + (m_iPlayerSituation * 0.4) - random->RandomInt(10, 20);
	float flFullnessScale = 0.5f + (((float)(100 - m_iPlayerSituation) / 100.0f) * 0.25f);
	//bool bPlayerNeedsRPG = false;

	//TERO: if we get asked while the dynamic spawner is disabled
	if (!m_bEnabled)
	{
		UpdateItemData();
		UpdatePlayerSituation();
	}

	/*for (int i=0; i<HLSS_DYNAMIC_NPC_SPAWNER_NUMBER_OF_TYPES; i++)
	{
	if ( m_bRequiresFriendlyRPG[i] && m_iNumberOfNPCs[i] > 0)
	{
	bPlayerNeedsRPG	= true;
	}
	}*/

	for (int k = 0; k<HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS; k++)
	{
		//TERO: note that bPlayerHas[k] is boolean for the weapons, not crates
		//		luckily the indexes are same for the weapons and crates, phew	
		if ((m_bPlayerHas[k] || m_iItemsInMap[k] > 0) && m_flAmmoFullnessScale[k] < flFullnessScale && m_iCratesInMap[k] < 2)
		{
			if ((k != HLSS_DYNAMIC_WEAPON_AR2 || (iSituation < 20 && m_flStartTime + HLSS_WEAPONS_GIVE_AR2_TIME < gpGlobals->curtime)) &&
				(k != HLSS_DYNAMIC_WEAPON_SHOTGUN || (iSituation < 40 && m_flStartTime + HLSS_WEAPONS_GIVE_SHOTGUN_TIME < gpGlobals->curtime)) &&
				(k != HLSS_DYNAMIC_WEAPON_RPG || CanSpawnRPG()) &&
				(k != HLSS_DYNAMIC_WEAPON_GRENADE || (iSituation < 70 && random->RandomInt(0, 2) == 0)))
			{
				iAmmoCrate = k;
				flFullnessScale = m_flAmmoFullnessScale[k];
			}
		}
	}

	DevMsg("hlss_director: returning %d\n", iAmmoCrate);

	return iAmmoCrate;
}

void CHLSS_Director::UpdateItemData(bool bForce)
{
	//UPDATE THE REST

	if (m_flNextUpdateItemData > gpGlobals->curtime && !bForce)
	{
		return;
	}

	m_flNextUpdateItemData = gpGlobals->curtime + HLSS_ITEM_DATA_UPDATE_DELAY;

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_dynamic_npc_spawner: no player\n");
		return;
	}

	CBaseEntity *pEnt;

	float flDist;
	bool  bOwnedByPlayer;

	//*****************************
	//TERO: WEAPONS OWNED BY PLAYER
	//*****************************

	for (int i = 0; i<HLSS_DYNAMIC_HEALTH; i++)
	{
		//STRING(iszWeaponNames[i])
		m_bPlayerHas[i] = (pPlayer->Weapon_OwnsThisType(m_lpzItemNames[i]) != NULL);
	}
	m_bPlayerHas[HLSS_DYNAMIC_WEAPON_GRENADE] = true;

	//********************
	//TERO: WEAPONS IN MAP
	//********************

	CBaseEntity *pItem;

	for (int i = 0; i<HLSS_DYNAMIC_MAX_WEAPON_ITEMS; i++)
	{
		m_iItemsInMap[i] = 0;

		//STRING(iszWeaponNames[i])
		pItem = gEntList.FindEntityByClassname(NULL, m_lpzItemNames[i]);

		while (pItem)
		{
			flDist = (pItem->GetAbsOrigin() - pPlayer->EyePosition()).Length();

			if (pItem->GetOwnerEntity() && (pItem->GetOwnerEntity()->IsPlayer() || pItem->GetOwnerEntity()->IsNPC()) && i < HLSS_DYNAMIC_HEALTH)
			{
				bOwnedByPlayer = true;
			}
			else
			{
				//TERO: lets not consider old items
				if (pItem->m_flSimulationTime + 120.0f < gpGlobals->curtime)
				{
					bOwnedByPlayer = true;
					//DevMsg("Item %s, sim time %f, not considered\n", m_lpzItemNames[i], gpGlobals->curtime - pItem->m_flSimulationTime);
				}
				else
				{
					bOwnedByPlayer = false;
				}
			}

			//TERO: if player is close a weapon, he should be able to get it, no need to spawn another
			if (flDist < HLSS_MIN_WEAPON_SHOULD_FIND_DISTANCE && i < HLSS_DYNAMIC_HEALTH)
			{
				m_bPlayerHas[i] = true;
			}

			if (flDist < HLSS_MIN_WEAPON_DISTANCE && !bOwnedByPlayer)
			{
				m_iItemsInMap[i]++;
			}

			//STRING(iszWeaponNames[i])
			pItem = gEntList.FindEntityByClassname(pItem, m_lpzItemNames[i]);
		}
	}

	//*******************************
	//TERO: WEAPON POINTS IN THE AREA
	//*******************************

	m_iWeaponPointsInArea = 0;

	pItem = gEntList.FindEntityByClassname(NULL, "hlss_dynamic_weapon_point");

	while (pItem && m_iWeaponPointsInArea < HLSS_ITEMS_MAX_IN_AREA)
	{
		flDist = (pItem->GetAbsOrigin() - pPlayer->EyePosition()).Length();

		if (flDist < HLSS_ITEMS_AREA_DISTANCE)
		{
			m_iWeaponPointsInArea++;
		}

		pItem = gEntList.FindEntityByClassname(pItem, "hlss_dynamic_weapon_point");
	}


	//***********************************
	//TERO: ITEM_ITEM_CRATES WITH MEDKITS
	//***********************************

	pItem = gEntList.FindEntityByClassname(NULL, "item_item_crate");

	while (pItem)
	{
		flDist = (pItem->GetAbsOrigin() - pPlayer->EyePosition()).Length();

		if (flDist < HLSS_MIN_WEAPON_DISTANCE) //pItem->GetOwnerEntity() == this && 
		{
			//DevMsg("item_item_crate with medkits found\n");
			m_iItemsInMap[HLSS_DYNAMIC_HEALTH] += 3;		//NOTE: that we assume all item crates owned by the dynamic spawner are healthkit crates
		}

		pItem = gEntList.FindEntityByClassname(pItem, "item_item_crate");
	}

	//TERO: NOTE, you have to make sure the item_item_crate is removed after being broken
	//DevMsg("Number of medkits in map %d, 75 - medkit * 34 = %d\n", m_iItemsInMap[HLSS_DYNAMIC_HEALTH], 75 - (m_iItemsInMap[HLSS_DYNAMIC_HEALTH]*34));

	//*****************************
	//TERO: CRATES IN MAP
	//*****************************

	for (int i = 0; i<HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS; i++)
	{
		m_iCratesInMap[i] = 0;

		if (m_iAmmoTypes[i] == -1)
		{
			Warning("hlss_dynamic_npc_spawner: incorrect ammo type %d\n", m_iAmmoTypes[i]);
		}
		else
		{
			int iMax = GetAmmoDef()->MaxCarry(m_iAmmoTypes[i]);
			int iCount = pPlayer->GetAmmoCount(m_iAmmoTypes[i]);

			if (iMax != 0)
			{
				m_flAmmoFullnessScale[i] = (((float)iCount) / ((float)iMax));
			}
			else
			{
				m_flAmmoFullnessScale[i] = 1.0f;
			}
		}
	}

	pEnt = gEntList.FindEntityByClassname(NULL, "hlss_dynamic_ammo_crate_point");

	while (pEnt)
	{
		CHLSS_DynamicAmmoCratePoint *pAmmoCrate;
		pAmmoCrate = dynamic_cast <CHLSS_DynamicAmmoCratePoint*>(pEnt);

		if (pAmmoCrate && m_iPlayerSituation <= pAmmoCrate->m_iDifficulty)
		{
			if (pAmmoCrate->m_iAmmoType >= 0 && pAmmoCrate->m_iAmmoType < HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS &&
				(pAmmoCrate->GetAbsOrigin() - pPlayer->EyePosition()).Length() < HLSS_MIN_AMMO_DISTANCE)
			{
				m_iCratesInMap[pAmmoCrate->m_iAmmoType]++;
			}
		}

		pEnt = gEntList.FindEntityByClassname(pEnt, "hlss_dynamic_ammo_crate_point");
	}


	if (m_flDelayFirstRPGAfterNeeded >= 0)
	{
		//UPDATE RPG DATA
		if (m_iPlayerNeedsRPG > 0)
		{
			m_bPlayerNeedsRPG = true;
		}

		//TERO: update the time
		if (m_bPlayerNeedsRPG)
		{
			if (m_flDelayFirstRPGAfterNeeded != 0)
			{
				if (m_flNeedRPGStartTime == 0)
				{
					m_flNeedRPGStartTime = gpGlobals->curtime + m_flDelayFirstRPGAfterNeeded;
					m_bPlayerNeedsRPG = false;
				}
				else if (m_flNeedRPGStartTime > gpGlobals->curtime)
				{
					m_bPlayerNeedsRPG = false;
				}
			}
		}
		else
		{
			m_flNeedRPGStartTime = 0;
		}
	}
}

void CHLSS_Director::UpdatePlayerPath()
{
	if (!m_hNextPath)
	{
		return;
	}

	if (m_flFullPlayerPath == -1)
	{
		CHLSS_Player_Path *pPath = m_hNextPath;

		m_flFullPlayerPath = 0;

		while (pPath)
		{
			m_flFullPlayerPath += pPath->m_flDistanceToNext;

			pPath = pPath->m_hNextPath;
		}
	}

	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

	if (!pPlayer)
	{
		Warning("hlss_director: no player\n");
		return;
	}

	/*int contents = enginetrace->GetPointContents_Collideable( m_hNextPath->GetCollideable(), pPlayer->WorldSpaceCenter() );
	if( (contents & CONTENTS_SOLID) )
	{
	m_hNextPath = m_hNextPath->m_hNextPath;
	//DevWarning("hlss_director: reached a brush\n");

	if (!m_hNextPath)
	{
	m_flPlayerPath = m_flFullPlayerPath;
	m_flPlayerPath = m_flFullPlayerPath;

	DevMsg("hlss_director: path done\n");
	return;
	}
	}*/

	float flPlayerPath = (pPlayer->GetLocalOrigin() - m_hNextPath->GetLocalOrigin()).Length();

	CHLSS_Player_Path *pPath = m_hNextPath;

	while (pPath)
	{
		flPlayerPath += pPath->m_flDistanceToNext;
		pPath = pPath->m_hNextPath;
	}

	if (flPlayerPath > m_flFullPlayerPath)
	{
		m_flLastTimeProgressed = gpGlobals->curtime;
		flPlayerPath = m_flFullPlayerPath;
	}

	flPlayerPath = m_flFullPlayerPath - flPlayerPath;

	if (hlss_director_debug.GetInt() == 5)
	{
		DevMsg("hlss_director: path distance now %f/%f, so far reached %f/%f\n", flPlayerPath, m_flFullPlayerPath, m_flPlayerPath, m_flFullPlayerPath);
	}

	m_flActualPlayerPath = flPlayerPath;

	if (flPlayerPath > m_flPlayerPath)
	{
		m_flLastTimeProgressed = gpGlobals->curtime;
		m_flPlayerPath = flPlayerPath;
	}
}

float CHLSS_Director::GetNextDistanceToSpawn(int disposition)
{
	float flAdditionalDist = ((float)m_iPlayerSituation / 100.0f) * 1024.0f;

	if (disposition == D_LI)
	{
		if (m_flPlayerPath > m_flNextDistanceToSpawnFriendlies)
		{
			if (m_iPlayerSituation > 80 && m_iAvarageSituation > 80)
			{
				m_flNextDistanceToSpawnFriendlies = m_flPlayerPath + random->RandomFloat(1024.0f, 2048.0f + flAdditionalDist);
			}
			else
			{
				m_flNextDistanceToSpawnFriendlies = m_flPlayerPath + random->RandomFloat(512.0f, 1024.0f + flAdditionalDist);
			}
		}

		return m_flNextDistanceToSpawnFriendlies;
	}

	if (m_flPlayerPath > m_flNextDistanceToSpawnHostiles)
	{
		if ((m_iPlayerSituation < 80 && m_iAvarageSituation < 80) || m_flNextDistanceToSpawnHostiles == -1)
		{
			m_flNextDistanceToSpawnHostiles = m_flPlayerPath + random->RandomFloat(1024.0f, 3072.0f - flAdditionalDist);
		}
		else
		{
			m_flNextDistanceToSpawnHostiles = m_flPlayerPath + random->RandomFloat(512.0f, 2048.0f - flAdditionalDist);
		}
	}

	return m_flNextDistanceToSpawnHostiles;
}