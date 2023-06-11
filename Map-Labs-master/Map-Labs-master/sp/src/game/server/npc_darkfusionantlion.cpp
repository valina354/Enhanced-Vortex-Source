#include "cbase.h"
#include "npc_antlion.h"
#include "te_effect_dispatch.h"
#include "tier1/KeyValues.h"

// Dark Fusion Antlion class definition
class CNPC_DarkFusionAntlion : public CNPC_Antlion
{
	DECLARE_CLASS(CNPC_DarkFusionAntlion, CNPC_Antlion);
	DECLARE_DATADESC();

public:
	void Spawn() override;
	void Precache() override;
	void Event_Killed(const CTakeDamageInfo& info) override;
	int OnTakeDamage_Alive(const CTakeDamageInfo& info) override;

private:
	void Explode();
	void CreatePurpleEffect();
	void CreateBloodEffect(const Vector& position);

	bool m_bExploded;
};

LINK_ENTITY_TO_CLASS(npc_darkfusionantlion, CNPC_DarkFusionAntlion);

BEGIN_DATADESC(CNPC_DarkFusionAntlion)
DEFINE_FIELD(m_bExploded, FIELD_BOOLEAN),
END_DATADESC()

void CNPC_DarkFusionAntlion::Spawn()
{
	Precache();
	SetModel("models/antlion.mdl");
	SetHealth(100);
	SetSize(Vector(-40, -40, 0), Vector(40, 40, 80));  // Increase in size
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_STEP);

	// Create the purple effect
	CreatePurpleEffect();

	BaseClass::Spawn();
}

void CNPC_DarkFusionAntlion::Precache()
{
	PrecacheModel("models/antlion.mdl");

	// Additional precache for the exploding effect
	PrecacheModel("models/effects/purple_acid.mdl");

	// Precache the particle effect
	PrecacheParticleSystem("dark_fusion_antlion_effect");

	// Precache the blood effect
	PrecacheParticleSystem("blue_blood_antlion");

	// Precache the louder sound
	PrecacheScriptSound("NPC_Antlion.MeleeAttack");

	BaseClass::Precache();
}

void CNPC_DarkFusionAntlion::Event_Killed(const CTakeDamageInfo& info)
{
	if (!m_bExploded)
	{
		// Explode on death with dark purple acid effect
		Explode();
		m_bExploded = true;
	}

	BaseClass::Event_Killed(info);
}

int CNPC_DarkFusionAntlion::OnTakeDamage_Alive(const CTakeDamageInfo& info)
{
	// Increase the damage dealt by the Dark Fusion Antlion
	CTakeDamageInfo modifiedInfo = info;
	modifiedInfo.ScaleDamage(1.5f);  // Increase damage by 50%

	return BaseClass::OnTakeDamage_Alive(modifiedInfo);
}

void CNPC_DarkFusionAntlion::Explode()
{
	// Spawn and configure the explosion effect
	CBaseEntity* pEffect = CreateEntityByName("env_explosion");
	if (pEffect)
	{
		pEffect->SetAbsOrigin(GetAbsOrigin());
		DispatchSpawn(pEffect);
		pEffect->KeyValue("iMagnitude", "100");
		pEffect->KeyValue("spawnflags", "64");  // No sound
		pEffect->KeyValue("model", "models/effects/purple_acid.mdl");
		pEffect->Activate();
		pEffect->Spawn();
	}
}

void CNPC_DarkFusionAntlion::CreatePurpleEffect()
{
	// Create the particle effect
	const char* pszEffectName = "dark_fusion_antlion_effect";
	CEffectData data;
	data.m_vOrigin = GetAbsOrigin();
	data.m_vAngles = vec3_angle;
	data.m_fFlags = 0;
	DispatchEffect(pszEffectName, data);
}

void CNPC_DarkFusionAntlion::CreateBloodEffect(const Vector& position)
{
	// Create the blood effect
	const char* pszEffectName = "blue_blood_antlion";
	CEffectData data;
	data.m_vOrigin = position;
	data.m_vAngles = vec3_angle;
	data.m_fFlags = 0;
	DispatchEffect(pszEffectName, data);
}
