//========= H ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef HLSS_DYNAMIC_POINT_ENTITIES_H
#define HLSS_DYNAMIC_POINT_ENTITIES_H
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

enum eHLSSDynamicWeaponType
{
	HLSS_DYNAMIC_WEAPON_NOTHING = -1,
	HLSS_DYNAMIC_WEAPON_PISTOL = 0,
	HLSS_DYNAMIC_WEAPON_SMG1,
	HLSS_DYNAMIC_WEAPON_AR2,
	HLSS_DYNAMIC_WEAPON_RPG,
	HLSS_DYNAMIC_WEAPON_SHOTGUN,
	HLSS_DYNAMIC_WEAPON_GRENADE,
	HLSS_DYNAMIC_WEAPON_357,
	HLSS_DYNAMIC_WEAPON_STUNSTICK,
	HLSS_DYNAMIC_WEAPON_CROWBAR,
	HLSS_DYNAMIC_HEALTH,
	HLSS_DYNAMIC_BATTERY,
	HLSS_DYNAMIC_SMG1_GRENADE,
	HLSS_DYNAMIC_AR2_ALTFIRE,
	HLSS_DYNAMIC_AMMO_SMG1,
	HLSS_DYNAMIC_AMMO_AR2,
	HLSS_DYNAMIC_AMMO_SHOTGUN,
	HLSS_DYNAMIC_MAX_WEAPON_ITEMS,
};

#define HLSS_WEAPON_POINT_THINK_DELAY_INITIAL 0.1f
#define HLSS_WEAPON_POINT_THINK_DELAY 0.2f
#define HLSS_ITEM_DATA_UPDATE_DELAY 1.0f

class CHLSS_DynamicWeaponPoint : public CPointEntity
{
	DECLARE_CLASS(CHLSS_DynamicWeaponPoint, CPointEntity);

public:
	CHLSS_DynamicWeaponPoint();

	virtual void Spawn(void);
	void WeaponPointThink(void);

	int	m_iWeaponType;
	int m_iDifficulty;

	DECLARE_DATADESC();
};

enum eHLSSDynamicAmmoCrateType
{
	HLSS_DYNAMIC_AMMO_CRATE_NOTHING = -1,
	HLSS_DYNAMIC_AMMO_CRATE_PISTOL = 0,
	HLSS_DYNAMIC_AMMO_CRATE_SMG1,
	HLSS_DYNAMIC_AMMO_CRATE_AR2,
	HLSS_DYNAMIC_AMMO_CRATE_RPG,
	HLSS_DYNAMIC_AMMO_CRATE_SHOTGUN,
	HLSS_DYNAMIC_AMMO_CRATE_GRENADE,
	HLSS_DYNAMIC_AMMO_CRATE_357,
	HLSS_DYNAMIC_MAX_AMMO_CRATE_ITEMS
};

class CHLSS_DynamicAmmoCratePoint : public CPointEntity
{
	DECLARE_CLASS(CHLSS_DynamicAmmoCratePoint, CPointEntity);
public:
	CHLSS_DynamicAmmoCratePoint();

	virtual void Spawn(void);
	virtual void Precache(void);
	void WaitThink();
	void AmmoCratePointThink(void);

	int	m_iAmmoType;
	int m_iDifficulty;

	DECLARE_DATADESC();
};

enum eHLSSDynamicItemCrateType
{
	HLSS_DYNAMIC_CRATE_ITEM_NOTHING = -1,
	HLSS_DYNAMIC_CRATE_ITEM_PISTOL_AMMO = 0,
	HLSS_DYNAMIC_CRATE_ITEM_SMG1_AMMO,
	HLSS_DYNAMIC_CRATE_ITEM_AR2_AMMO,
	HLSS_DYNAMIC_CRATE_ITEM_RPG_AMMO,
	HLSS_DYNAMIC_CRATE_ITEM_SHOTGUN_AMMO,
	HLSS_DYNAMIC_CRATE_HEALTHKIT,
	HLSS_DYNAMIC_CRATE_BATTERY,
	HLSS_DYNAMIC_MAX_CRATE_ITEMS,
};

//-----------------------------------------------------------------------------
// A breakable crate that drops items
//-----------------------------------------------------------------------------
class CHLSS_ItemCrate : public CPhysicsProp
{
public:
	CHLSS_ItemCrate();

	DECLARE_CLASS(CHLSS_ItemCrate, CPhysicsProp);
	DECLARE_DATADESC();

	void Precache(void);
	void Spawn(void);

	virtual int	ObjectCaps() { return BaseClass::ObjectCaps() | FCAP_WCEDIT_POSITION; };

	virtual int		OnTakeDamage(const CTakeDamageInfo &info);

	void InputKill(inputdata_t &data);

	virtual void VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);
	virtual void OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);

protected:
	virtual void OnBreak(const Vector &vecVelocity, const AngularImpulse &angVel, CBaseEntity *pBreaker);

private:
	int m_iProbability;

	COutputEvent m_OnCacheInteraction;
};

#endif