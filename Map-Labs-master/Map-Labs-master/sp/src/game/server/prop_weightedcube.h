#ifndef PROP_WEIGHTEDCUBE_H
#define PROP_WEIGHTEDCUBE_H
#ifdef _WIN32
#pragma once
#endif

#include "props.h"
#include "baseanimating.h"
#include "Sprite.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "player_pickup.h"
//#include "env_portal_laser.h"

// resource file names
#define IMPACT_DECAL_NAME	"decals/smscorch1model"

// context think
#define UPDATE_THINK_CONTEXT	"UpdateThinkContext"
;
class CPropWeightedCube : public CPhysicsProp
{
public:
	DECLARE_CLASS(CPropWeightedCube, CPhysicsProp);
	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();

	virtual void Precache();
	virtual void Spawn();

	//Use
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value);
	int ObjectCaps();

	bool CreateVPhysics()
	{
		VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
		return true;
	}

	bool Dissolve(const char* materialName, float flStartTime, bool bNPCOnly, int nDissolveType, Vector vDissolverOrigin, int magnitude);

	void InputDissolve(inputdata_t& data);
	void InputSilentDissolve(inputdata_t& data);
	void InputPreDissolveJoke(inputdata_t& data);
	void InputExitDisabledState(inputdata_t& data);

	//Pickup
	//void Pickup(void);
	void OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason);
	void OnPhysGunDrop(CBasePlayer* pPhysGunUser, PhysGunDrop_t reason);
	//virtual void Activate(void);

	void ToggleLaser(bool state);

	int GetCubeType() { return m_cubeType; }
	void SetCubeType(int type) { m_cubeType = type; }

	void SetActivated(bool active);

	int m_nLensAttachment;
	Vector m_vLensVec;
	QAngle m_aLensAng;

private:
	int	m_cubeType;
	int m_skinType;
	int m_paintPower;
	bool m_useNewSkins;
	bool m_allowFunnel;

	//CEnvPortalLaser* m_pLaser;

	CHandle<CBasePlayer> m_hPhysicsAttacker;

	COutputEvent m_OnOrangePickup;
	COutputEvent m_OnBluePickup;
	COutputEvent m_OnPlayerPickup;

	//COutputEvent m_OnPainted;

	COutputEvent m_OnPhysGunDrop;

	COutputEvent m_OnFizzled;

};
#endif // TRIGGERS_H