//=================== Half-Life 2: Short Stories Mod 2009 =====================//
//
// Purpose:	Pull objects to us
//
//=============================================================================//

#include "cbase.h"
#include "hlss_debris_maker.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define HLSS_DEBRISMAKER_DOOR_Y_ANGLE_ACCEPTABLE 30

//---------------------------------------------------------
//---------------------------------------------------------
LINK_ENTITY_TO_CLASS(hlss_debris_maker, CHLSS_Debris_Maker);

BEGIN_DATADESC(CHLSS_Debris_Maker)
DEFINE_FIELD(m_hDebrisTarget, FIELD_EHANDLE),
DEFINE_FIELD(m_bNormalDoor, FIELD_BOOLEAN),

DEFINE_THINKFUNC(DebrisMakerThink),
END_DATADESC()

CHLSS_Debris_Maker::CHLSS_Debris_Maker()
{
	m_hDebrisTarget = NULL;
	m_bNormalDoor = true;
}

void CHLSS_Debris_Maker::Spawn(void)
{
	BaseClass::Spawn();

	SetThink(&CHLSS_Debris_Maker::DebrisMakerThink);
	SetNextThink(gpGlobals->curtime + 1.0f);
}

void CHLSS_Debris_Maker::DebrisMakerThink(void)
{
	if (!m_hDebrisTarget)
	{
		UTIL_Remove(this);
		return;
	}

	IPhysicsObject *pPhysicsObject = m_hDebrisTarget->VPhysicsGetObject();
	if (pPhysicsObject)
	{
		if (pPhysicsObject->IsAsleep() && ShouldMakeDebris(m_hDebrisTarget)) // || (vecVelocity.Length() < 60 || vecVelocity == vec3_origin) && (angImpulse.Length() < 60 ))
		{
			DevMsg("hlss_debris_maker: making prop %s into debris\n", m_hDebrisTarget->GetDebugName());

			m_hDebrisTarget->SetCollisionGroup(COLLISION_GROUP_DEBRIS);
			UTIL_Remove(this);
			return;
		}
	}
	else
	{
		UTIL_Remove(this);
		return;
	}

	SetNextThink(gpGlobals->curtime + 1.0f);
}

bool CHLSS_Debris_Maker::ShouldMakeDebris(CBaseEntity *pEntity)
{
	if (m_bNormalDoor)
	{
		QAngle angAngle = pEntity->GetAbsAngles();
		float flAngle = UTIL_AngleDiff(angAngle.x, 0);

		//DevMsg("Ent is normal door, y angle %f, x angle %f, z angle %f\n", UTIL_AngleDiff( angAngle.y, 0 ),  UTIL_AngleDiff( angAngle.x, 0 ),  UTIL_AngleDiff( angAngle.z, 0 ));

		if ((((flAngle - 90.0f) < HLSS_DEBRISMAKER_DOOR_Y_ANGLE_ACCEPTABLE) &&
			((flAngle - 90.0f) > -HLSS_DEBRISMAKER_DOOR_Y_ANGLE_ACCEPTABLE)) ||
			(((flAngle + 90.0f) < HLSS_DEBRISMAKER_DOOR_Y_ANGLE_ACCEPTABLE) &&
			((flAngle + 90.0f) > -HLSS_DEBRISMAKER_DOOR_Y_ANGLE_ACCEPTABLE)))
		{
			//DevMsg("correct angles\n");
			return true;
		}
		else
		{
			return false;
		}
	}

	return true;
}

void CHLSS_Debris_Maker::Create(CPhysicsProp *pProp)
{
	if (!pProp)
		return;

	CHLSS_Debris_Maker *pDebrisMaker = (CHLSS_Debris_Maker *)CBaseEntity::Create("hlss_debris_maker", pProp->GetAbsOrigin(), QAngle(0, 0, 0), pProp);

	//pBee->SetOwnerEntity( Instance( pentOwner ) );
	if (pDebrisMaker)
	{
		pDebrisMaker->Spawn();
		pDebrisMaker->m_hDebrisTarget = pProp;
		pDebrisMaker->SetParent(pProp);

		//DevMsg("Created debris maker for entity %s\n", pProp->GetDebugName());
	}

}