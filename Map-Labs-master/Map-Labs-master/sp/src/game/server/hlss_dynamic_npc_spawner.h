//========= Copyright ? 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HLSS_DYNAMIC_NPC_SPAWNER_H
#define HLSS_DYNAMIC_NPC_SPAWNER_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ai_network.h"
#include "ai_node.h"
#include "ai_pathfinder.h"
#include "ai_navigator.h"
#include "ai_waypoint.h"
#include "ai_route.h"
#include "monstermaker.h"
#include "hlss_dynamic_point_entities.h"

class CHLSS_Director;
class CHLSS_Dynamic_NPC_Spawner;
class CHLSS_Dynamic_Spawn_Brush;
class CHLSS_Dynamic_Attack_Brush;
class CHLSS_Dynamic_Idle_Brush;

#define HLSS_DYNAMIC_NPC_SPANWER_DEFAULT_BURST_DELAY 30.0f
#define HLSS_DYNAMIC_NPC_SPANWER_DEFAULT_DELAY 0.5f
#define HLSS_MAX_SPAWN_ATTEMPTS 20
#define HLSS_MIN_DISTANCE_FROM_PLAYER 1024.0f
#define HLSS_MAX_DISTANCE_FROM_PLAYER 4096.0f
#define HLSS_IGNORE_FUTHER_CANDS_DIFFERENCE 1024.0f

#define HLSS_EASY_TIME_AFTER_LOAD 30.0f
#define HLSS_DAMAGE_TIME_EASY 30.0f
#define HLSS_SITUATION_UPDATE_DELAY 0.2f
#define HLSS_SITUATION_MEMORY 80
#define HLSS_SITUATION_MEMORY_HALF 40

#define HLSS_MIN_WEAPON_DISTANCE 2048.0f
#define HLSS_MIN_WEAPON_SHOULD_FIND_DISTANCE 128.0f
#define HLSS_MIN_AMMO_DISTANCE 1024.0f


//TERO: WAVE SITUATION
#define HLSS_WAVE_MIN_SPEED 60.0f
#define HLSS_WAVE_MAX_SPEED 20.0f
#define HLSS_WAVE_SITUATION_BASE	10.0f
#define HLSS_WAVE_SITUATION_DIFFERENCE 10.0f

#define HLSS_WAVE_SITUATION_BASE 10.0f

const int hlss_situation_decrease_help[3] = { 1.0f, 0.8f, 0.6f };
const int hlss_situation_increase_help[3] = { 0.7f, 0.6f, 0.5f };

//TERO: start situation used to 50
#define HLSS_START_SITUATION random->RandomInt(0,50)
#define HLSS_SITUATION_GAME_DIFFICULTY 5.0f
#define HLSS_SITUATION_NPC 60.0f
#define HLSS_SITUATION_DAMAGE 10.0f
#define HLSS_SITUATION_HEALTH 30.0f
#define HLSS_SITUATION_HEALTH_LOW 40.0f
#define HLSS_SITUATION_WEAPONS 20.0f
#define HLSS_SITUATION_INCREASE_THROUGH_TIME_MAX 40.0f
#define HLSS_SITUATION_INCREASE_THROUGH_TIME_TIME 420.0f //60 * 7 = 7mins
#define HLSS_SITUATION_BASE 20.0f
#define HLSS_SITUATION_LONG_DECREASE_HELP 10.0f
#define HLSS_SITUATION_MORE_DECREASES_HELP hlss_situation_decrease_help[g_pGameRules->GetSkillLevel()-1] //1.0f
#define HLSS_SITUATION_MORE_INCREASES_HELP hlss_situation_increase_help[g_pGameRules->GetSkillLevel()-1]
#define HLSS_SITUATION_INTERVALS_BEFORE_RECORD 8
#define HLSS_SITUATION_HOSTILE_SCALE 1.4

#define HLSS_GIVE_NO_HEALTHKIT_DAMAGE_TIME 4.0f
#define HLSS_GIVE_NO_HEALTHKIT_PUNISHMENT 20

//TERO: You can only carry two weapons in HE, pistol and primary --> 0.5f
//		In normal HL2 or PD you can carry 5 of the guns			 --> 0.20f
#define HLSS_MAX_WEAPONS_CARRY_SCALE 0.50f	

#define HLSS_WEAPONS_GIVE_SHOTGUN_TIME 60.0f
#define HLSS_WEAPONS_GIVE_AR2_TIME 120.0f
#define HLSS_WEAPONS_GIVE_SMG1_GRENADE_TIME 90.0f
#define HLSS_WEAPONS_GIVE_AR2_ALTFIRE_TIME 180.0f

#define HLSS_ITEMS_MAX_IN_AREA 5
#define HLSS_ITEMS_AREA_DISTANCE 1024.0f
#define HLSS_HEALTHKIT_PROBABILITY 4
#define HLSS_HEALTHKIT_SMALL_PROBABILITY 3
#define HLSS_RPG_PROBABILITY 3
#define HLSS_AR2_PROBABILITY 4
#define HLSS_SHOTGUN_PROBABILITY 5
#define HLSS_GRENADE_PROBABILITY 5

#define HLSS_NPC_FIRST_SPAWN_BURST_SCALE 0.5f

#define HLSS_SITUATION_SCORE_MAX 100
#define HLSS_SITUATION_SCORE 20.0f


//TERO: DEBUG!
//#define DEBUG_DNS_ENTITY


enum eHLSS_NPCWeaponTypes
{
	HLSS_NPC_WEAPON_NOTHING = -2,
	HLSS_NPC_WEAPON_SMG1 = -1,
	HLSS_NPC_WEAPON_SHOTGUN = 0,
	HLSS_NPC_WEAPON_RPG,
	HLSS_NPC_WEAPON_AR2,
	HLSS_NPC_WEAPON_NUM,
};

//-----------------------------------------------------------------------------------------
// NPC SPAWNER
//-----------------------------------------------------------------------------------------

class CHLSS_Dynamic_NPC_Spawner : public CBaseEntity
{
public:
	DECLARE_CLASS(CHLSS_Dynamic_NPC_Spawner, CBaseEntity);

	CHLSS_Dynamic_NPC_Spawner();

	virtual void Precache();
	virtual void Spawn();

	virtual void OnRestore();

	bool		 SpawnInsideBrush(CBasePlayer *pPlayer, int iPlayerNode, CAI_BaseNPC *pNPC, CHLSS_Dynamic_Spawn_Brush* pBrush);
	CHLSS_Dynamic_Attack_Brush* FindAttackBrush(CBasePlayer *pPlayer, int iPlayerNode, CAI_BaseNPC *pNPC);
	bool		 SpawnAttack();
	void		 ClearAttackBrush();

	int			 GetNumberOfIdleNPCsToSpawn();

	CAI_BaseNPC*	 CreateNPC();

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	bool		 FindDirector();
#endif

	void		 DynamicSpawnerThink(void);

	void		 CheckAttackSpawning();
	void		 GetNextAttack();

	virtual void DeathNotice(CBaseEntity *pChild);// NPC maker children use this to tell the NPC maker that they have died.

	DECLARE_DATADESC();

	string_t m_iszTemplateName;		// The name of the NPC that will be used as the template.
	string_t m_iszTemplateData;		// The keyvalue data blob from the template NPC that will be used to spawn new ones.

	float	 m_flFirstSpawnDelay;
	float	 m_flNextSpawnTime;
	float	 m_flNextSpawnAttackTime;
	float	 m_flMinimumSpawnAttackDelay;

	bool	 m_bPanicEvent;
	int		 m_iPanicEventMaxNPCs;
	float	 m_flPanicEventMaxFullDelay;
	int		 m_iPanicEventAbsoluteMaxNPCs;

	int		 m_iNumberOfNPCs;
	int		 m_iMaxNumberOfNPCs;
	int		 m_iAttackSize;
	int		 m_iMaxAttackSize;

	int		m_iNPCdifficulty;

	bool	 m_bRequiresFriendlyRPG;
	bool	 m_bRequiresHostileRPG;

	bool	 m_bEnabled;

	Class_T	 m_iClassify;

	void	InputEnable(inputdata_t &inputdata);
	void	InputDisable(inputdata_t &inputdata);

	void	InputSetMaxAttack(inputdata_t &inputdata);
	void	InputSetMaxNPCs(inputdata_t &inpudata);

	void	InputStartPanicEvent(inputdata_t &inpudata);
	void	InputStopPanicEvent(inputdata_t &inputdata);

	COutputEvent m_AllDead;
	bool	m_bAllDead;

	CHandle<CHLSS_Dynamic_Attack_Brush> m_hAttackBrush;
	CHandle<CHLSS_Dynamic_Attack_Brush> m_hIgnoreBrush;

	float	m_flNextAttackBrushSearch;
	float	m_flAttackBrushPlayerPath;

	bool	ShouldLookForNewAttackBrush();

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	CHandle<CHLSS_Director> m_hDirector;
#endif

	static const char *m_lpzNPCWeaponNames[HLSS_NPC_WEAPON_NUM];

	string_t m_iszGroupName;

#define SF_HLSS_SPAWNER_ONLY_ALLOW_FROM_GROUP		( 1 << 0  )

	bool	AllowedGroup(CHLSS_Dynamic_Spawn_Brush *pBrush);


	float	m_flNextDistanceToCreateNPCs;
	int		m_iNumToSpawnInDistance;

	void	GetNextDistanceToSpawn();
	int		GetDistanceAttackSize();

private:
	CAI_Network*		m_pAINetwork;
	CAI_Network *		GetNetwork()						{ return m_pAINetwork; }
};

class CHLSS_Player_Path;

//-----------------------------------------------------------------------------------------
// DIRECTOR
//-----------------------------------------------------------------------------------------

class CHLSS_Director : public CBaseEntity
{
public:
	DECLARE_CLASS(CHLSS_Director, CBaseEntity);

	CHLSS_Director *m_pNext;

	CHLSS_Director();
	~CHLSS_Director();

	static CHLSS_Director *GetDirector();

	virtual void Precache();
	virtual void Spawn();

	virtual void OnRestore();

	void		 DirectorThink(void);

	void		 UpdatePlayerSituation();

	void	InputEnable(inputdata_t &inputdata);
	void	InputDisable(inputdata_t &inputdata);
	void	InputResetStartTime(inputdata_t &inputdata);

	int		GetItemToSpawn();
	int		GetItemToSpawnForCrate();
	int		GetAmmoCrateToSpawn();
	void	UpdateItemData(bool bForce = false);
	void	UpdatePlayerPath();

	//VARIABLES
	int		 m_iPlayerOldSituations[HLSS_SITUATION_MEMORY];
	int		 m_iIntervalsBeforeRecording;

	int		 m_iPlayerSituation;
	int		 m_iAvarageSituation;
	float	 m_flNextSituationUpdateTime;

	float	 m_flLastTookDamageTime;
	float	 m_flStopTakingDamageTime;
	int		 m_iLastPlayerHealth;
	int		 m_iPlayerHealthOnDamageStart;

	float	 m_flLastLoadTime;
	float	 m_flStartTime;
	float	 m_flStartTimeDifference;

	static const char *m_lpzItemNames[HLSS_DYNAMIC_MAX_WEAPON_ITEMS];
	int		 m_iAmmoTypes[HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS];

	float	 m_flWaveSituationTime;
	float	 m_flWaveSituationSpeed;
	float	 m_flWaveSituationSize;

	int		 m_iPlayerScore;

	bool	 m_bEnabled;

	float	m_flNextUpdateItemData;

	bool	m_bPlayerHas[HLSS_DYNAMIC_HEALTH];
	int		m_iItemsInMap[HLSS_DYNAMIC_MAX_WEAPON_ITEMS];
	int		m_iCratesInMap[HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS];
	float	m_flAmmoFullnessScale[HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS];
	int		m_iWeaponPointsInArea;

	float	m_flDelayFirstAR2;
	float	m_flDelayFirstShotgun;
	float	m_flDelayFirstSMG1_Grenade;
	float	m_flDelayFirstAR2_AltFire;
	float	m_flDelayFirstRPGAfterNeeded;
	float	m_flNeedRPGStartTime;
	bool	m_bPlayerNeedsRPG;
	int		m_iPlayerNeedsRPG;

	bool	CanSpawnAR2(bool bNPC, int disposition = D_LI);
	bool	CanSpawnShotgun(bool bNPC, int disposition = D_LI);
	bool	CanSpawnSMG1_Grenade();
	bool	CanSpawnAR2_AltFire();
	bool	CanSpawnRPG(int disposition = D_LI);

	int m_iNumFriendlyWeapons[HLSS_NPC_WEAPON_NUM];
	int m_iMaxFriendlyWeapons[HLSS_NPC_WEAPON_NUM];
	int m_iNumHostileWeapons[HLSS_NPC_WEAPON_NUM];
	int m_iMaxHostileWeapons[HLSS_NPC_WEAPON_NUM];

	COutputEvent m_AllDead;
	COutputEvent m_AllButFriendliesDead;
	bool	m_bAllDead;
	bool	m_bAllButFriendliesDead;

	static const char *m_lpzCrateItemNames[HLSS_DYNAMIC_MAX_CRATE_ITEMS];

	string_t m_iszNextName;
	CHandle<CHLSS_Player_Path> m_hNextPath;
	float m_flFullPlayerPath;
	float m_flPlayerPath;
	float m_flActualPlayerPath;

	float m_flPathSituationStart;
	float m_flPathSituationEnd;
	float m_flPathSituationSize;

	float m_flLastTimeProgressed;
	float GetNextDistanceToSpawn(int disposition);
	float m_flNextDistanceToSpawnHostiles;
	float m_flNextDistanceToSpawnFriendlies;

public:

	DECLARE_DATADESC();
};

//-----------------------------------------------------------------------------------------
// BASE CLASS SPAWN BRUSH
//-----------------------------------------------------------------------------------------

class CHLSS_Dynamic_Spawn_Brush : public CBaseEntity //CBaseNPCMaker
{
public:
	DECLARE_CLASS(CHLSS_Dynamic_Spawn_Brush, CBaseEntity);

	CHLSS_Dynamic_Spawn_Brush();

	virtual void Spawn();

	virtual bool CanSpawnHere(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC);
	virtual void OnSuccesfulSpawn(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC);

	void	InputEnable(inputdata_t &inputdata);
	void	InputDisable(inputdata_t &inputdata);

	DECLARE_DATADESC();

	int m_iNumNodes;
	bool m_bEnabled;

	COutputEvent m_OnSpawn;

#define SF_HLSS_BRUSH_ONLY_ALLOW_FROM_GROUP		( 1 << 0  )

	string_t m_iszGroupName;

	CAI_Network*		m_pAINetwork;
	CAI_Network *		GetNetwork()						{ return m_pAINetwork; }
};

//-----------------------------------------------------------------------------------------
// ATTACK BRUSH
//-----------------------------------------------------------------------------------------

class CHLSS_Dynamic_Attack_Brush : public CHLSS_Dynamic_Spawn_Brush //CBaseNPCMaker
{
public:
	DECLARE_CLASS(CHLSS_Dynamic_Attack_Brush, CHLSS_Dynamic_Spawn_Brush);

	CHLSS_Dynamic_Attack_Brush();

	virtual bool CanSpawnHere(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC);
	virtual void OnSuccesfulSpawn(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC);
	void ReleaseBrush();

	DECLARE_DATADESC();

	Class_T m_iCurrentClassify;

#define SF_HLSS_BRUSH_CHECK_VISIBILITY ( 1 << 1  )
};

//-----------------------------------------------------------------------------------------
// IDLE BRUSH
//-----------------------------------------------------------------------------------------

#define HLSS_IDLE_BRUSH_THINK_DELAY 0.2f

class CHLSS_Dynamic_Idle_Brush : public CHLSS_Dynamic_Spawn_Brush //CBaseNPCMaker
{
public:
	DECLARE_CLASS(CHLSS_Dynamic_Idle_Brush, CHLSS_Dynamic_Spawn_Brush);

	CHLSS_Dynamic_Idle_Brush();

	virtual bool CanSpawnHere(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC);
	virtual void OnSuccesfulSpawn(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC);

	virtual void Spawn();
	void IdleBrushThink();

	void	CheckSpawn();

	void	InputCheckSpawn(inputdata_t &inputdata);

	string_t m_iszIdlePath;

	DECLARE_DATADESC();

#define SF_HLSS_BRUSH_REMOVE_AFTER_CHECK_SPAWN			( 1 << 1  )
#define SF_HLSS_BRUSH_SPAWN_CHECK_WHEN_ORIGIN_VISIBLE	( 1 << 2  )

};

//-----------------------------------------------------------------------------------------
// FILTER
//-----------------------------------------------------------------------------------------

class CHLSS_DNS_Node_Filter : public INearestNodeFilter
{
public:
	CHLSS_DNS_Node_Filter(CBasePlayer *pPlayer, CAI_BaseNPC *pNPC, CAI_Network *pAINetwork, int iPlayerNode, float flMaxDistance,
		float flMinDistance, bool bCheckPlayerVisibility = true, bool bGetClosestNode = true)
		: m_pPlayer(pPlayer),
		m_pNPC(pNPC),
		m_pAINetwork(pAINetwork),
		m_iPlayerNode(iPlayerNode),
		m_flMaxDistance(flMaxDistance),
		m_flMinDistance(flMinDistance),
		m_bCheckPlayerVisibility(bCheckPlayerVisibility),
		m_bGetClosestNode(bGetClosestNode)
	{
		m_vecTopOfHull = m_pNPC->GetHullMaxs(); //vecHullMaxs;
		m_vecTopOfHull.x = 0;
		m_vecTopOfHull.y = 0;
	}

	bool IsValid(CAI_Node *pNode)
	{
		Vector vecNode = pNode->GetPosition(m_pNPC->GetHullType()); //pNode->GetOrigin();

		//NDebugOverlay::Box( vecNode, Vector(10,10,10), Vector(-10,-10,-10), 255, 0, 0, 0, 5 );

		float flDist = (vecNode - m_pPlayer->EyePosition()).Length();

		if (flDist > m_flMaxDistance) //|| flDist < m_flMinDistance )
		{
			//DevMsg("node is too far away\n");
			return false;
		}

		if (flDist < m_flMinDistance)
		{
			//DevMsg("node is too close %f / %f\n", flDist, m_flMinDistance);
			return false;
		}

		if (m_iPlayerNode != NO_NODE && !GetNetwork()->IsConnected(pNode->GetId(), m_iPlayerNode))
		{
			//DevMsg("Is not connected to player's closest node\n");
			return false;
		}

		if (m_bCheckPlayerVisibility && (m_pPlayer->FVisible(vecNode) || m_pPlayer->FVisible(vecNode + m_vecTopOfHull)))
		{
			//DevMsg("playey can see node\n");
			return false;
		}

		trace_t tr;
		UTIL_TraceHull(vecNode,
			vecNode + Vector(0, 0, 1),
			m_pNPC->GetHullMins(),
			m_pNPC->GetHullMaxs(),
			MASK_NPCSOLID,
			NULL,
			COLLISION_GROUP_NONE,
			&tr);

		if (tr.fraction != 1.0)
		{
			return false;
		}


		//AI_Waypoint_t *	m_pRoute = m_pNPC->GetPathfinder()->BuildNodeRoute( vecNode, m_pPlayer->GetAbsOrigin(), bits_BUILD_IGNORE_NPCS, 32.0f );
		/*AI_Waypoint_t *m_pRoute = m_pNPC->GetPathfinder()->BuildRoute( vecNode, m_pPlayer->GetAbsOrigin(), m_pPlayer, 32.0f );

		if (m_pRoute)
		{
		m_pNPC->GetNavigator()->GetPath()->ComputeRouteGoalDistances( m_pRoute );
		DevMsg("flPathDistGoal %f\n", m_pRoute->flPathDistGoal);

		}*/

		if (m_bGetClosestNode)
		{
			m_flMaxDistance = flDist;
		}
		return true;
	};

	bool ShouldContinue()
	{
		if (m_flMaxDistance <= m_flMinDistance)
		{
			return false;
		}

		return true;
	}

	int m_iPlayerNode;
	float m_flMinDistance;
	float m_flMaxDistance;

	bool m_bCheckPlayerVisibility;
	bool m_bGetClosestNode;

	CAI_BaseNPC *m_pNPC;
	Vector m_vecTopOfHull;

	CBasePlayer *m_pPlayer;

private:
	CAI_Network*		m_pAINetwork;
	CAI_Network *		GetNetwork()						{ return m_pAINetwork; }
};

class CHLSS_Player_Path : public CBaseEntity //CBaseNPCMaker
{
public:
	DECLARE_CLASS(CHLSS_Player_Path, CBaseEntity);

	CHLSS_Player_Path();

	virtual void Spawn();
	virtual void Touch(CBaseEntity *pOther);

	void	InputEnable(inputdata_t &inputdata);
	void	InputDisable(inputdata_t &inputdata);

	DECLARE_DATADESC();

	COutputEvent m_OnTrigger;

	string_t	m_iszNextName;
	CHandle<CHLSS_Player_Path> m_hNextPath;
	float		m_flDistanceToNext;
	bool		m_bEnabled;
};

#endif // HLSS_DYNAMIC_NPC_SPAWNER_H