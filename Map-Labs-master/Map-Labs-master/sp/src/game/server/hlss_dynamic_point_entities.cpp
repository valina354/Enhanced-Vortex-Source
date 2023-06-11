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
#include "hlss_dynamic_point_entities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Item entity names
const char *CHLSS_Director::m_lpzItemNames[HLSS_DYNAMIC_MAX_WEAPON_ITEMS] =
{
	"weapon_pistol",
	"weapon_smg1",
	"weapon_ar2",
	"weapon_rpg",
	"weapon_shotgun",
	"weapon_frag",
	"weapon_357",
	"weapon_stunstick",
	"weapon_crowbar",
	"item_healthkit",
	"item_battery",
	"item_ammo_smg1_grenade",
	"item_ammo_ar2_altfire",
	"item_ammo_smg1",
	"item_ammo_ar2_large",
	"item_box_buckshot",
};

//---------------------------------------------------------
//---------------------------------------------------------
LINK_ENTITY_TO_CLASS(hlss_dynamic_weapon_point, CHLSS_DynamicWeaponPoint);

BEGIN_DATADESC(CHLSS_DynamicWeaponPoint)
DEFINE_FIELD(m_iWeaponType, FIELD_INTEGER),
DEFINE_KEYFIELD(m_iDifficulty, FIELD_INTEGER, "difficulty"),
DEFINE_THINKFUNC(WeaponPointThink),

END_DATADESC()

CHLSS_DynamicWeaponPoint::CHLSS_DynamicWeaponPoint()
{
	m_iWeaponType = HLSS_DYNAMIC_WEAPON_NOTHING;
	m_iDifficulty = 100;
}

void CHLSS_DynamicWeaponPoint::WeaponPointThink()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_dynamic_weapon_point: no player\n");
	}

	if (pPlayer && pPlayer->FVisible(GetAbsOrigin()))
	{

#ifdef HLSS_USE_EHANDLE_DIRECTOR
		CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "hlss_director");
		CHLSS_Director *pSpawner = dynamic_cast <CHLSS_Director*>(pEnt);
#else
		CHLSS_Director *pSpawner = CHLSS_Director::GetDirector();
#endif

		if (pSpawner)
		{
			if (pSpawner->m_iPlayerSituation > m_iDifficulty)
			{
				pSpawner->m_iWeaponPointsInArea--;
				UTIL_Remove(this);
				return;
			}

			int iItemToSpawn = pSpawner->GetItemToSpawn();

			//DevMsg("hlss_dynamic_weapon_point: item to spawn %d\n", iItemToSpawn);

			if (iItemToSpawn != HLSS_DYNAMIC_WEAPON_NOTHING)
			{
				if (iItemToSpawn == HLSS_DYNAMIC_HEALTH && pPlayer->m_iHealth <= 25 && pSpawner->m_iItemsInMap[HLSS_DYNAMIC_HEALTH] < 3)
				{
					CBaseEntity *pCrate = CBaseEntity::CreateNoSpawn("item_item_crate", GetAbsOrigin(), GetAbsAngles(), NULL);
					//CItem_ItemCrate *pCrate = (CItem_ItemCrate *)CBaseEntity::Crate("item_item_crate", GetAbsOrigin(), GetAbsAngles(), NULL);

					if (pCrate)
					{
						//char *szItemClass = "item_healthkit";

						pCrate->KeyValue("CrateType", "0");
						pCrate->KeyValue("ItemClass", "item_healthkit");
						pCrate->KeyValue("ItemCount", "3");

						DispatchSpawn(pCrate);

						pSpawner->m_iItemsInMap[HLSS_DYNAMIC_HEALTH] += 3;
						pSpawner->m_iWeaponPointsInArea--;

						UTIL_Remove(this);
						return;
					}
				}
				else
				{
					//STRING(pSpawner->iszWeaponNames[iItemToSpawn])
					CBaseEntity *pItem = CBaseEntity::CreateNoSpawn(CHLSS_Director::m_lpzItemNames[iItemToSpawn], GetAbsOrigin(), GetAbsAngles(), NULL);

					if (pItem)
					{
						//DevMsg("hlss_dynamic_npc_spawner: creating an item of type: %s\n", pSpawner->iszWeaponNames[iItemToSpawn]);

						pItem->Precache();
						DispatchSpawn(pItem);

						pSpawner->m_iItemsInMap[iItemToSpawn]++;
						if (iItemToSpawn < HLSS_DYNAMIC_HEALTH)
						{
							pSpawner->m_bPlayerHas[iItemToSpawn] = true;
						}
						pSpawner->m_iWeaponPointsInArea--;

						UTIL_Remove(this);
					}
				}
			}
		}
	}

	SetNextThink(gpGlobals->curtime + HLSS_WEAPON_POINT_THINK_DELAY);
}

void CHLSS_DynamicWeaponPoint::Spawn()
{
	BaseClass::Spawn();

	SetThink(&CHLSS_DynamicWeaponPoint::WeaponPointThink);
	SetNextThink(gpGlobals->curtime); // + HLSS_WEAPON_POINT_THINK_DELAY_INITIAL );
}

//---------------------------------------------------------
//---------------------------------------------------------
LINK_ENTITY_TO_CLASS(hlss_dynamic_ammo_crate_point, CHLSS_DynamicAmmoCratePoint);

BEGIN_DATADESC(CHLSS_DynamicAmmoCratePoint)
DEFINE_FIELD(m_iAmmoType, FIELD_INTEGER),
DEFINE_KEYFIELD(m_iDifficulty, FIELD_INTEGER, "difficulty"),
DEFINE_THINKFUNC(WaitThink),
DEFINE_THINKFUNC(AmmoCratePointThink),
END_DATADESC()

CHLSS_DynamicAmmoCratePoint::CHLSS_DynamicAmmoCratePoint()
{
	m_iAmmoType = HLSS_DYNAMIC_AMMO_CRATE_NOTHING;
	m_iDifficulty = 100;
}

void CHLSS_DynamicAmmoCratePoint::AmmoCratePointThink()
{
	CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
	if (!pPlayer)
	{
		Warning("hlss_dynamic_ammo_crate: no player\n");
	}

	if (pPlayer && pPlayer->FVisible(GetAbsOrigin()))
	{

#ifdef HLSS_USE_EHANDLE_DIRECTOR
		CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "hlss_director");
		CHLSS_Director *pSpawner = dynamic_cast <CHLSS_Director*>(pEnt);
#else
		CHLSS_Director *pSpawner = CHLSS_Director::GetDirector();
#endif

		if (pSpawner)
		{
			if (pSpawner->m_iPlayerSituation > m_iDifficulty)
			{
				UTIL_Remove(this);
				return;
			}

			int iAmmoCrate = pSpawner->GetAmmoCrateToSpawn();

			if (iAmmoCrate > HLSS_DYNAMIC_AMMO_CRATE_NOTHING && iAmmoCrate <= HLSS_DYNAMIC_WEAPON_357)
			{
				CBaseEntity *pCrate = CBaseEntity::CreateNoSpawn("item_ammo_crate", GetAbsOrigin(), GetAbsAngles(), NULL);

				if (pCrate)
				{
					//DevMsg("hlss_dynamic_npc_spawner: creating an ammo crate of type %d for ammo %d\n", iAmmoCrate, m_iAmmoTypes[i]);

					char cAmmo[1];

					Q_snprintf(cAmmo, 1, "%d", iAmmoCrate);

					pCrate->KeyValue("AmmoType", cAmmo);

					DispatchSpawn(pCrate);

					m_iAmmoType = iAmmoCrate;

					pSpawner->m_iCratesInMap[iAmmoCrate]++;
					pSpawner->m_flAmmoFullnessScale[iAmmoCrate] = 1.0f;

					SetThink(NULL);
					return;
				}
			}
			else
			{
				UTIL_Remove(this);
			}
		}
	}

	SetNextThink(gpGlobals->curtime + HLSS_WEAPON_POINT_THINK_DELAY);
}

void CHLSS_DynamicAmmoCratePoint::Spawn()
{
	BaseClass::Spawn();
	/*Precache();

	SetModel( HLSS_DYNAMIC_AMMO_CRATE_MODEL );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();*/

	SetThink(&CHLSS_DynamicAmmoCratePoint::WaitThink);
	SetNextThink(gpGlobals->curtime + HLSS_WEAPON_POINT_THINK_DELAY);	//TERO: remove 2.0f after you have figured out how to spawn weapons first
}

void CHLSS_DynamicAmmoCratePoint::Precache()
{
	PrecacheModel("models/items/ammocrate_pistol.mdl");		// Small rounds
	PrecacheModel("models/items/ammocrate_smg1.mdl");			// Medium rounds
	PrecacheModel("models/items/ammocrate_ar2.mdl");			// Large rounds
	PrecacheModel("models/items/ammocrate_rockets.mdl");		// RPG rounds
	PrecacheModel("models/items/ammocrate_buckshot.mdl");		// Buckshot
	PrecacheModel("models/items/ammocrate_grenade.mdl");		// Grenades
	PrecacheModel("models/items/ammocrate_smg1.mdl");			// 357
	PrecacheModel("models/items/ammocrate_smg1.mdl");			// Crossbow

	PrecacheScriptSound("AmmoCrate.Open");
	PrecacheScriptSound("AmmoCrate.Close");

	BaseClass::Precache();
}

void CHLSS_DynamicAmmoCratePoint::WaitThink()
{
	SetThink(&CHLSS_DynamicAmmoCratePoint::AmmoCratePointThink);
	SetNextThink(gpGlobals->curtime + HLSS_WEAPON_POINT_THINK_DELAY);
}




LINK_ENTITY_TO_CLASS(hlss_item_crate, CHLSS_ItemCrate);


//-----------------------------------------------------------------------------
// Save/load: 
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CHLSS_ItemCrate)
DEFINE_KEYFIELD(m_iProbability, FIELD_INTEGER, "probability"),

DEFINE_INPUTFUNC(FIELD_VOID, "Kill", InputKill),
DEFINE_OUTPUT(m_OnCacheInteraction, "OnCacheInteraction"),

END_DATADESC()

CHLSS_ItemCrate::CHLSS_ItemCrate(void)
{
	m_iProbability = 100;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLSS_ItemCrate::Precache(void)
{
	// Set this here to quiet base prop warnings
	PrecacheModel("models/items/item_item_crate.mdl");
	SetModel("models/items/item_item_crate.mdl");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLSS_ItemCrate::Spawn(void)
{
	DisableAutoFade();
	SetModelName(AllocPooledString("models/items/item_item_crate.mdl"));

	Precache();
	SetModel("models/items/item_item_crate.mdl");
	AddEFlags(EFL_NO_ROTORWASH_PUSH);
	BaseClass::Spawn();

	m_iProbability = clamp(m_iProbability, 0, 100);

	if (random->RandomInt(0, 100) > m_iProbability)
	{
		UTIL_Remove(this);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CHLSS_ItemCrate::InputKill(inputdata_t &data)
{
	UTIL_Remove(this);
}


//-----------------------------------------------------------------------------
// Item crates blow up immediately
//-----------------------------------------------------------------------------
int CHLSS_ItemCrate::OnTakeDamage(const CTakeDamageInfo &info)
{
	if (info.GetDamageType() & DMG_AIRBOAT)
	{
		CTakeDamageInfo dmgInfo = info;
		dmgInfo.ScaleDamage(10.0);
		return BaseClass::OnTakeDamage(dmgInfo);
	}

	return BaseClass::OnTakeDamage(info);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLSS_ItemCrate::VPhysicsCollision(int index, gamevcollisionevent_t *pEvent)
{
	float flDamageScale = 1.0f;
	if (FClassnameIs(pEvent->pEntities[!index], "prop_vehicle_airboat") ||
		FClassnameIs(pEvent->pEntities[!index], "prop_vehicle_jeep"))
	{
		flDamageScale = 100.0f;
	}

	m_impactEnergyScale *= flDamageScale;
	BaseClass::VPhysicsCollision(index, pEvent);
	m_impactEnergyScale /= flDamageScale;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLSS_ItemCrate::OnBreak(const Vector &vecVelocity, const AngularImpulse &angImpulse, CBaseEntity *pBreaker)
{
	// FIXME: We could simply store the name of an entity to put into the crate 
	// as a string entered in by worldcraft. Should we?	I'd do it for sure
	// if it was easy to get a dropdown with all entity types in it.

	m_OnCacheInteraction.FireOutput(pBreaker, this);

#ifdef HLSS_USE_EHANDLE_DIRECTOR
	CBaseEntity *pEnt = gEntList.FindEntityByClassname(NULL, "hlss_director");
	CHLSS_Director *pDirector = dynamic_cast <CHLSS_Director*>(pEnt);
#else
	CHLSS_Director *pDirector = CHLSS_Director::GetDirector();
#endif

	if (!pDirector)
	{
		return;
	}

	int iItemIndex = pDirector->GetItemToSpawnForCrate();

	if (iItemIndex == HLSS_DYNAMIC_CRATE_ITEM_NOTHING)
	{
		return;
	}

	for (int i = 0; i < 3; ++i)
	{
		CBaseEntity *pSpawn = CreateEntityByName(CHLSS_Director::m_lpzCrateItemNames[iItemIndex]);

		if (!pSpawn)
			return;

		// Give a little randomness...
		Vector vecOrigin;
		CollisionProp()->RandomPointInBounds(Vector(0.25, 0.25, 0.25), Vector(0.75, 0.75, 0.75), &vecOrigin);
		pSpawn->SetAbsOrigin(vecOrigin);

		QAngle vecAngles;
		vecAngles.x = random->RandomFloat(-20.0f, 20.0f);
		vecAngles.y = random->RandomFloat(0.0f, 360.0f);
		vecAngles.z = random->RandomFloat(-20.0f, 20.0f);
		pSpawn->SetAbsAngles(vecAngles);

		Vector vecActualVelocity;
		vecActualVelocity.Random(-10.0f, 10.0f);
		//		vecActualVelocity += vecVelocity;
		pSpawn->SetAbsVelocity(vecActualVelocity);

		QAngle angVel;
		AngularImpulseToQAngle(angImpulse, angVel);
		pSpawn->SetLocalAngularVelocity(angVel);

		// If we're creating an item, it can't be picked up until it comes to rest
		// But only if it wasn't broken by a vehicle
		CItem *pItem = dynamic_cast<CItem*>(pSpawn);
		if (pItem && !pBreaker->GetServerVehicle())
		{
			pItem->ActivateWhenAtRest();
		}

		pSpawn->Spawn();
	}
}

void CHLSS_ItemCrate::OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason)
{
	BaseClass::OnPhysGunPickup(pPhysGunUser, reason);

	m_OnCacheInteraction.FireOutput(pPhysGunUser, this);

	if (reason == PUNTED_BY_CANNON)
	{
		Vector vForward;
		AngleVectors(pPhysGunUser->EyeAngles(), &vForward, NULL, NULL);
		Vector vForce = Pickup_PhysGunLaunchVelocity(this, vForward, PHYSGUN_FORCE_PUNTED);
		AngularImpulse angular = AngularImpulse(0, 0, 0);

		IPhysicsObject *pPhysics = VPhysicsGetObject();

		if (pPhysics)
		{
			pPhysics->AddVelocity(&vForce, &angular);
		}

		TakeDamage(CTakeDamageInfo(pPhysGunUser, pPhysGunUser, GetHealth(), DMG_GENERIC));
	}
}