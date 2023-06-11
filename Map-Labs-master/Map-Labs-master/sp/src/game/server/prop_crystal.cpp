#include "cbase.h"
#include "props.h"
#include "te_effect_dispatch.h"
#include "particle_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PROP_CRYSTAL_EXPLODE_SOUND "NPC_Hunter.FlechetteExplode"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPropCrystal : public CPhysicsProp
{
	DECLARE_CLASS(CPropCrystal, CPhysicsProp);
	DECLARE_SERVERCLASS();

	virtual void	OnBreak(const Vector &vecVelocity, const AngularImpulse &angVel, CBaseEntity *pBreaker);

	void Precache();
};

IMPLEMENT_SERVERCLASS_ST(CPropCrystal, DT_PropCrystal)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(prop_crystal, CPropCrystal);

void CPropCrystal::Precache()
{
	BaseClass::Precache();

	PrecacheParticleSystem("he_crystal_explode");
	PrecacheScriptSound(PROP_CRYSTAL_EXPLODE_SOUND);
}

void CPropCrystal::OnBreak(const Vector &vecVelocity, const AngularImpulse &angVel, CBaseEntity *pBreaker)
{
	BaseClass::OnBreak(vecVelocity, angVel, pBreaker);

	DevMsg("duuude\n");

	//CTakeDamageInfo info( NULL, NULL, GetAbsVelocity() * 4.0f, GetAbsOrigin(), 25, DMG_BLASt );
	EmitSound(PROP_CRYSTAL_EXPLODE_SOUND);
	DispatchParticleEffect("he_crystal_explode", GetAbsOrigin(), GetAbsAngles());
}