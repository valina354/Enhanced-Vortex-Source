#include "cbase.h"
#include "item_jetpack.h"
#include "player.h"

LINK_ENTITY_TO_CLASS(item_jetpack, CItemJetpack);

BEGIN_DATADESC(CItemJetpack)
DEFINE_FUNCTION(ActivateJetpack),
END_DATADESC()

CItemJetpack::CItemJetpack()
{
}

void CItemJetpack::Spawn()
{
	Precache();
	SetModel("models/jetpack.mdl");
	SetSolid(SOLID_BBOX);
	SetCollisionGroup(COLLISION_GROUP_DEBRIS);
	UTIL_SetSize(this, -Vector(16, 16, 16), Vector(16, 16, 16));
	SetThink(NULL);
	SetUse(&CItemJetpack::ActivateJetpack);
}

void CItemJetpack::Precache()
{
	PrecacheModel("models/jetpack.mdl");
}

bool CItemJetpack::KeyValue(const char* szKeyName, const char* szValue)
{
	// Implement any key-value pairs you need to set up the jetpack's properties
	// Example: if (FStrEq(szKeyName, "speed")) { speed = atof(szValue); return true; }
	return BaseClass::KeyValue(szKeyName, szValue);
}

void CItemJetpack::ActivateJetpack(CBasePlayer* pPlayer)
{
	if (!pPlayer)
		return;

	if (pPlayer->IsOnGround())
	{
		Vector forward;
		AngleVectors(pPlayer->EyeAngles(), &forward);

		// Check for the key combination: Space + Ctrl + Forward
		if (pPlayer->m_afButtonPressed & IN_JUMP && pPlayer->m_afButtonPressed & IN_DUCK &&
			pPlayer->m_afButtonPressed & IN_FORWARD)
		{
			// Enable jetpack functionality here
			// Example: pPlayer->m_bHasJetpack = true;
		}
	}
}
