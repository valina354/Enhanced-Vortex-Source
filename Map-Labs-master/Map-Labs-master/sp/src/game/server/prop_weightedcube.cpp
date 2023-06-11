//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Defines a weighted storage cube.
//
//=====================================================================================//
#include "cbase.h"					// for pch
#include "prop_weightedcube.h"
#include "prop_laser_catcher.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define	CUBE_MODEL			"models/props/metal_box.mdl"
#define	REFLECTION_MODEL	"models/props/reflection_cube.mdl"
#define	SPHERE_MODEL		"models/props/sphere.mdl"
#define	ANTIQUE_MODEL		"models/props/p2/metal_box.mdl"

#define FIZZLE_SOUND		"Prop.Fizzled"

enum CubeType
{
	Standard = 0,
	Companion = 1,
	Reflective,
	Sphere,
	Antique,
};

enum SkinOld
{
	OLDStandard = 0,
	OLDCompanion = 1,
	OLDStandardActivated,
	OLDReflective,
	OLDSphere,
	OLDAntique
};

enum SkinType
{
	Clean = 0,
	Rusted = 1,
};

enum PaintPower
{
	Bounce = 0,
	Stick = 1,
	Speed,
	Portal,
	None,
};


LINK_ENTITY_TO_CLASS(prop_weighted_cube, CPropWeightedCube);


BEGIN_DATADESC(CPropWeightedCube)


// Save/load
DEFINE_USEFUNC(Use),

//DEFINE_KEYFIELD(m_oldSkin, FIELD_INTEGER, "skin"),
DEFINE_KEYFIELD(m_cubeType, FIELD_INTEGER, "CubeType"),
DEFINE_KEYFIELD(m_skinType, FIELD_INTEGER, "SkinType"),
DEFINE_KEYFIELD(m_paintPower, FIELD_INTEGER, "PaintPower"),
DEFINE_KEYFIELD(m_useNewSkins, FIELD_BOOLEAN, "NewSkins"),
DEFINE_KEYFIELD(m_allowFunnel, FIELD_BOOLEAN, "allowfunnel"),

DEFINE_INPUTFUNC(FIELD_VOID, "Dissolve", InputDissolve),
DEFINE_INPUTFUNC(FIELD_VOID, "SilentDissolve", InputSilentDissolve),
DEFINE_INPUTFUNC(FIELD_VOID, "PreDissolveJoke", InputPreDissolveJoke),
DEFINE_INPUTFUNC(FIELD_VOID, "ExitDisabledState", InputExitDisabledState),

// Output
//DEFINE_OUTPUT(m_OnPainted, "OnPainted"),

DEFINE_OUTPUT(m_OnFizzled, "OnFizzled"),
DEFINE_OUTPUT(m_OnOrangePickup, "OnOrangePickup"),
DEFINE_OUTPUT(m_OnBluePickup, "OnBluePickup"),
DEFINE_OUTPUT(m_OnPlayerPickup, "OnPlayerPickup"),
DEFINE_OUTPUT(m_OnPhysGunDrop, "OnPhysGunDrop"),

DEFINE_THINKFUNC(Think),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: Precache
// Input  :  - 
//-----------------------------------------------------------------------------
void CPropWeightedCube::Precache()
{
	PrecacheModel(CUBE_MODEL);
	PrecacheModel(REFLECTION_MODEL);
	PrecacheModel(SPHERE_MODEL);
	PrecacheModel(ANTIQUE_MODEL);
	PrecacheScriptSound(FIZZLE_SOUND);

	BaseClass::Precache();

}

int CPropWeightedCube::ObjectCaps()
{
	int caps = BaseClass::ObjectCaps();

	caps |= FCAP_IMPULSE_USE;

	return caps;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  :  - 
//-----------------------------------------------------------------------------
void CPropWeightedCube::Spawn()
{
	Precache();

	if (m_useNewSkins)
	{
		m_nSkin = 0;
		switch (m_cubeType)
		{
		case Standard:
			SetModel(CUBE_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 3;
			if (m_paintPower == Stick)
				m_nSkin = 8;
			if (m_paintPower == Speed)
				m_nSkin = 9;
			break;
		case Companion:
			SetModel(CUBE_MODEL);
			m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 10;
			if (m_paintPower == Speed)
				m_nSkin = 11;
			break;
		case Reflective:
			SetModel(REFLECTION_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 2;
			if (m_paintPower == Speed)
				m_nSkin = 3;
			break;
		case Sphere:
			SetModel(SPHERE_MODEL);
			break;
		case Antique:
			SetModel(ANTIQUE_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 3;
			if (m_paintPower == Stick)
				m_nSkin = 8;
			if (m_paintPower == Speed)
				m_nSkin = 9;
			break;
		}
	}
	else
	{
		switch (m_nSkin)
		{
		case OLDStandard:
			SetModel(CUBE_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 3;
			if (m_paintPower == Stick)
				m_nSkin = 8;
			if (m_paintPower == Speed)
				m_nSkin = 9;
			break;
		case OLDCompanion:
			SetModel(CUBE_MODEL);
			m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 10;
			if (m_paintPower == Speed)
				m_nSkin = 11;
			break;
		case OLDReflective:
			SetModel(REFLECTION_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 1;
			if (m_paintPower == Stick)
				m_nSkin = 2;
			if (m_paintPower == Speed)
				m_nSkin = 3;
			break;
		case OLDSphere:
			SetModel(SPHERE_MODEL);
			break;
		case OLDAntique:
			SetModel(ANTIQUE_MODEL);
			if (m_skinType == Rusted)
				m_nSkin = 3;
			if (m_paintPower == Stick)
				m_nSkin = 8;
			if (m_paintPower == Speed)
				m_nSkin = 9;
			break;
		}
	}

	SetSolid(SOLID_VPHYSICS);

	// In order to pick it up, needs to be physics.
	CreateVPhysics();

	SetUse(&CPropWeightedCube::Use);

	BaseClass::Spawn();

}

void CPropWeightedCube::ToggleLaser(bool state)
{
	//if (m_cubeType != Reflective) return;

	//if (m_pLaser == nullptr) {
	//	m_pLaser = dynamic_cast<CEnvPortalLaser*>(CreateEntityByName("env_portal_laser"));
	//	m_pLaser->KeyValue("damage", "100");
	//	m_pLaser->KeyValue("width", "2");
	//	m_pLaser->KeyValue("texture", "sprites/laserbeam.spr");
	//	m_pLaser->KeyValue("renderamt", "100");
	//	m_pLaser->KeyValue("TextureScroll", "35");
	//	m_pLaser->SetParent(this);
	//	m_pLaser->SetParentAttachment("SetLaserAttachmentParent", "focus", false);
	//	DispatchSpawn(m_pLaser);
	//	m_pLaser->Activate();
	//	m_pLaser->TurnOff();
	//}

	//if (state == true) {
	//	m_pLaser->TurnOn();
	//}
	//else if (state == false) {
	//	m_pLaser->TurnOff();
	//	if (dynamic_cast<CEnvLaserTarget*>(m_pLaser->m_pHitObject) != nullptr) {
	//		CEnvLaserTarget* pTarget = dynamic_cast<CEnvLaserTarget*>(m_pLaser->m_pHitObject);
	//		CPropLaserCatcher* pCatcher = dynamic_cast<CPropLaserCatcher*>(pTarget->GetParent());
	//		pCatcher->TurnOff(m_pLaser);
	//	}
	//}
}

void CPropWeightedCube::InputPreDissolveJoke(inputdata_t& data)
{
	// Sets some VO to do before fizzling.
}

void CPropWeightedCube::InputExitDisabledState(inputdata_t& data)
{
	// Exits the disabled state of a reflector cube.
}


void CPropWeightedCube::InputDissolve(inputdata_t& data)
{
	Dissolve(NULL, gpGlobals->curtime, false, 0, GetAbsOrigin(), 1);
	EmitSound(FIZZLE_SOUND);
}

void CPropWeightedCube::InputSilentDissolve(inputdata_t& data)
{
	Dissolve(NULL, gpGlobals->curtime, false, 0, GetAbsOrigin(), 1);
}

bool CPropWeightedCube::Dissolve(const char* materialName, float flStartTime, bool bNPCOnly, int nDissolveType, Vector vDissolverOrigin, int magnitude)
{
	m_OnFizzled.FireOutput(this, this);
	return BaseClass::Dissolve(materialName, flStartTime, bNPCOnly, nDissolveType, vDissolverOrigin, magnitude);
}

void CPropWeightedCube::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	CBasePlayer* pPlayer = ToBasePlayer(pActivator);
	if (pPlayer)
	{
		pPlayer->PickupObject(this);
	}
}

void CPropWeightedCube::OnPhysGunPickup(CBasePlayer* pPhysGunUser, PhysGunPickup_t reason)
{
	m_hPhysicsAttacker = pPhysGunUser;

	if (reason == PICKED_UP_BY_CANNON || reason == PICKED_UP_BY_PLAYER)
	{
		m_OnPlayerPickup.FireOutput(pPhysGunUser, this);
	}
}

void CPropWeightedCube::OnPhysGunDrop(CBasePlayer* pPhysGunUser, PhysGunDrop_t reason)
{
	m_OnPhysGunDrop.FireOutput(pPhysGunUser, this);
}

void CPropWeightedCube::SetActivated(bool active)
{
	if (m_useNewSkins)
	{
		if (active)
		{
			m_nSkin = 0;
		}
		else
		{
			m_nSkin = 2;
		}

		switch (m_cubeType)
		{
		case Standard:

			if (m_skinType == Rusted)
			{
				if (active)
				{
					m_nSkin = 5;
				}
				else
				{
					m_nSkin = 3;
				}

			}
			else
			{
				if (active)
				{
					m_nSkin = 2;
				}
				else
				{
					m_nSkin = 0;
				}

			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 10;
				}
				else
				{
					m_nSkin = 6;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 11;
				}
				else
				{
					m_nSkin = 7;
				}
			}

			break;
		case Companion:

			if (active)
			{
				m_nSkin = 4;
			}
			else
			{
				m_nSkin = 1;
			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 10;
				}
				else
				{
					m_nSkin = 6;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 9;
				}
				else
				{
					m_nSkin = 7;
				}
			}
			break;
		case Reflective:

			if (m_skinType == Rusted)
			{
				if (active)
				{
					m_nSkin = 5;
				}
				else
				{
					m_nSkin = 1;
				}

			}
			else
			{
				if (active)
				{
					m_nSkin = 6;
				}
				else
				{
					m_nSkin = 0;
				}

			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 7;
				}
				else
				{
					m_nSkin = 2;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 8;
				}
				else
				{
					m_nSkin = 3;
				}

			}

			break;
		case Sphere:

			break;
		case Antique:

			if (m_skinType == Rusted)
			{
				if (active)
				{
					m_nSkin = 5;
				}
				else
				{
					m_nSkin = 3;
				}

			}
			else
			{
				if (active)
				{
					m_nSkin = 2;
				}
				else
				{
					m_nSkin = 0;
				}

			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 10;
				}
				else
				{
					m_nSkin = 6;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 11;
				}
				else
				{
					m_nSkin = 7;
				}
			}

			break;
		}
	}
	else
	{
		switch (m_nSkin)
		{
		case OLDStandard:
			if (m_skinType == Rusted)
			{
				if (active)
				{
					m_nSkin = 5;
				}
				else
				{
					m_nSkin = 3;
				}

			}
			else
			{
				if (active)
				{
					m_nSkin = 2;
				}
				else
				{
					m_nSkin = 0;
				}

			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 10;
				}
				else
				{
					m_nSkin = 6;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 11;
				}
				else
				{
					m_nSkin = 7;
				}
			}
			break;
		case OLDCompanion:
			if (active)
			{
				m_nSkin = 4;
			}
			else
			{
				m_nSkin = 1;
			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 10;
				}
				else
				{
					m_nSkin = 6;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 9;
				}
				else
				{
					m_nSkin = 7;
				}
			}
			break;
		case OLDReflective:
			if (m_skinType == Rusted)
			{
				if (active)
				{
					m_nSkin = 5;
				}
				else
				{
					m_nSkin = 1;
				}

			}
			else
			{
				if (active)
				{
					m_nSkin = 6;
				}
				else
				{
					m_nSkin = 0;
				}

			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 7;
				}
				else
				{
					m_nSkin = 2;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 8;
				}
				else
				{
					m_nSkin = 3;
				}

			}
			break;
		case OLDSphere:

			break;
		case OLDAntique:
			if (m_skinType == Rusted)
			{
				if (active)
				{
					m_nSkin = 5;
				}
				else
				{
					m_nSkin = 3;
				}

			}
			else
			{
				if (active)
				{
					m_nSkin = 2;
				}
				else
				{
					m_nSkin = 0;
				}

			}

			if (m_paintPower == Bounce)
			{
				if (active)
				{
					m_nSkin = 10;
				}
				else
				{
					m_nSkin = 6;
				}
			}

			if (m_paintPower == Speed)
			{
				if (active)
				{
					m_nSkin = 11;
				}
				else
				{
					m_nSkin = 7;
				}
			}
			break;
		}
	}
}
