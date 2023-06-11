//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	The Pit Monster - Xen Movement Limiter
//
//=============================================================================//


#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "explode.h"

#include "tier0/memdbgon.h"

ConVar	sk_pitmonster_dmg("sk_pitmonster_dmg", "0");
ConVar	hlss_pitmonster_nodmg("hlss_pitmonster_nodmg", "1");

class CPitMonster : public CBaseCombatCharacter//, virtual public CBaseAnimating
{
	DECLARE_CLASS(CPitMonster, CBaseCombatCharacter);

public:
	CPitMonster();
	~CPitMonster();

	Class_T Classify(void) { return CLASS_BEE; }

	void	Spawn(void);
	void	Precache(void);

	void	SeekThink(void);

	bool	CreateVPhysics(void);

	virtual float	GetDamage() { return m_flDamage; }
	virtual void	SetDamage(float flDamage) { m_flDamage = flDamage; }

	CHandle<CBaseEntity>		m_hTarget;

	void	InputSetTarget(inputdata_t &inputdata);
	void	InputDie(inputdata_t &inputdata);
	void	InputPlayDamage(inputdata_t &inputdate);

protected:
	float					m_flDamage;

private:

	float					m_flAttackTime;
	float					m_flLastDamageTime;
	float					m_flNextRandomTime;
	int						m_iTentacleAttachment;
	bool					m_bDead;
	bool					m_bDamageGivenYet;
	Vector					m_vRandomVector;

	enum Pitmonster_animations
	{
		PMAnim_idle = 0,
		PMAnim_pain,
		PMAnim_attack,
	};

	int						m_iAnimation;

	DECLARE_DATADESC();
};

BEGIN_DATADESC(CPitMonster)
DEFINE_FIELD(m_hTarget, FIELD_EHANDLE),
DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
DEFINE_FIELD(m_flAttackTime, FIELD_TIME),
DEFINE_FIELD(m_flLastDamageTime, FIELD_TIME),
DEFINE_FIELD(m_flNextRandomTime, FIELD_TIME),
DEFINE_FIELD(m_iTentacleAttachment, FIELD_INTEGER),
DEFINE_FIELD(m_vRandomVector, FIELD_VECTOR),
DEFINE_FIELD(m_iAnimation, FIELD_INTEGER),
DEFINE_FIELD(m_bDamageGivenYet, FIELD_BOOLEAN),

// Function Pointers
DEFINE_FUNCTION(SeekThink),

DEFINE_INPUTFUNC(FIELD_STRING, "SetTarget", InputSetTarget),
DEFINE_INPUTFUNC(FIELD_VOID, "Die", InputDie),
DEFINE_INPUTFUNC(FIELD_VOID, "PlayDamage", InputPlayDamage),

END_DATADESC()
LINK_ENTITY_TO_CLASS(npc_pitmonster, CPitMonster);

CPitMonster::CPitMonster()
{
}

CPitMonster::~CPitMonster()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPitMonster::InputSetTarget(inputdata_t &inputdata)
{
	// Find the specified entity
	string_t iszEntityName = inputdata.value.StringID();
	if (iszEntityName == NULL_STRING)
	{
		m_hTarget = NULL;

		if (!m_bDead && m_iAnimation == PMAnim_attack)
		{
			m_iAnimation = PMAnim_idle;
			ResetSequence(LookupSequence("idle"));
		}

		return;
	}

	CBaseEntity *pEntity = gEntList.FindEntityByName(NULL, iszEntityName, NULL, inputdata.pActivator, inputdata.pCaller);
	if (!pEntity)
	{
		m_hTarget = NULL;

		if (!m_bDead && m_iAnimation == PMAnim_attack)
		{
			m_iAnimation = PMAnim_idle;
			ResetSequence(LookupSequence("idle"));
		}

		Warning("pitmonster %s couldn't find SetTarget entity named %s\n", STRING(GetEntityName()), STRING(iszEntityName));
		return;
	}

	m_hTarget = pEntity;
}

void CPitMonster::InputDie(inputdata_t &inputdata)
{
	ResetSequence(LookupSequence("die1"));
	m_bDead = true;
}

void CPitMonster::InputPlayDamage(inputdata_t &inputdate)
{
	if (m_flLastDamageTime + 2 < gpGlobals->curtime)
	{
		ResetSequence(LookupSequence("damage1"));
		m_flLastDamageTime = gpGlobals->curtime;

		m_iAnimation = PMAnim_pain;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CPitMonster::CreateVPhysics(void)
{
	// Create the object in the physics system
	VPhysicsInitNormal(SOLID_BBOX, FSOLID_NOT_STANDABLE, false);

	return true;
}

void CPitMonster::Precache(void)
{
	PrecacheScriptSound("NPC_Vortigaunt.Claw");
	PrecacheScriptSound("NPC_Vortigaunt.Swing");
	PrecacheModel("models/props_xen/monster.mdl");
}

void CPitMonster::Spawn(void)
{
	Precache();

	SetSolid(SOLID_VPHYSICS);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetModel("models/props_xen/monster.mdl");

	UTIL_SetSize(this, Vector(-160, -160, 0), Vector(160, 160, 160));

	SetDamage(200);

	SetMoveType(MOVETYPE_NONE); //, MOVECOLLIDE_FLY_BOUNCE 
	SetCollisionGroup(COLLISION_GROUP_NPC);
	SetThink(&CPitMonster::SeekThink);

	SetNextThink(gpGlobals->curtime);

	m_takedamage = DAMAGE_YES;
	m_iHealth = m_iMaxHealth = 3;
	m_bloodColor = BLOOD_COLOR_YELLOW;

	m_hTarget = UTIL_GetLocalPlayer();


	// Start our animation cycle. Use the random to avoid everything thinking the same frame
	//SetContextThink( &CPropCombineBall::AnimThink, gpGlobals->curtime + random->RandomFloat( 0.0f, 0.1f), s_pAnimThinkContext );

	int nSequence = LookupSequence("idle1");

	m_iAnimation = PMAnim_idle;

	SetCycle(0);
	m_flAnimTime = gpGlobals->curtime;
	SetAnimatedEveryTick(true);
	ResetSequence(nSequence);
	ResetClientsideFrame();

	m_flAttackTime = 0;
	m_flLastDamageTime = 0;
	m_flNextRandomTime = 0;
	m_iTentacleAttachment = LookupAttachment("spike");

	m_bDead = false;

	SetPoseParameter("tentacle_x", 0);
	SetPoseParameter("tentacle_z", 0);
	SetPoseParameter("tentacle_y", 80);

	m_vRandomVector = Vector(0, 0, 80);
}

void CPitMonster::SeekThink(void)
{


	if (!m_bDead)
	{
		float paraX = GetPoseParameter("tentacle_z");
		float paraZ = GetPoseParameter("tentacle_y");
		float paraY = GetPoseParameter("tentacle_x");

		float wantedX = 0;
		float wantedY = 0;
		float wantedZ = 80;

		if (m_hTarget != NULL)
		{

			//TERO: Here we should check if we can go straight to the enemy

			CBaseEntity *pEnemy = m_hTarget;

			if (pEnemy)
			{
				//trace_t tr;
				//UTIL_TraceLine( GetAbsOrigin()+Vector(0,0,180), pEnemy->GetAbsOrigin(), MASK_SOLID_BRUSHONLY,  this, COLLISION_GROUP_NONE, &tr);

				wantedX = pEnemy->GetAbsOrigin().x - GetAbsOrigin().x + 15;
				wantedY = pEnemy->GetAbsOrigin().y - GetAbsOrigin().y + 10;
				wantedZ = pEnemy->GetAbsOrigin().z - GetAbsOrigin().z - 50;

				if (m_flLastDamageTime + 1.6f < gpGlobals->curtime)
				{
					if (m_iAnimation == PMAnim_pain)
					{
						m_iAnimation = PMAnim_idle;
						ResetSequence(LookupSequence("idle"));
					}

					Vector vecOrigin;
					QAngle vecAngles;

					GetAttachment(m_iTentacleAttachment, vecOrigin, vecAngles);
					Vector vecDist = pEnemy->GetAbsOrigin() - vecOrigin;
					vecDist.z = 0;
					float enemyDist = VectorNormalize(vecDist);

					DevMsg("enemydist:    %f  ", enemyDist);

					//See if we are attacking and if we should hurt someone
					if (!m_bDamageGivenYet && (gpGlobals->curtime - m_flAttackTime) > 0.11 && (gpGlobals->curtime - m_flAttackTime) < 0.59 && hlss_pitmonster_nodmg.GetBool())
					{
						CBaseEntity *pHurt = CheckTraceHullAttack(vecOrigin, vecOrigin + vecDist * 4, Vector(-20, -20, -20), Vector(20, 20, 20), sk_pitmonster_dmg.GetInt(), DMG_SLASH);
						if (pHurt)
						{
							m_bDamageGivenYet = true;

							pHurt->ViewPunch(QAngle(5, 0, -18));
							// Play a random attack hit sound
							EmitSound("NPC_Vortigaunt.Claw");
						}
					}

					//See if we can do a new attack
					if (enemyDist < 250 && wantedZ < 355)
					{

						SetPoseParameter("tentacle_z", UTIL_Approach(wantedX, paraX, 2));
						SetPoseParameter("tentacle_y", UTIL_Approach(wantedZ, paraZ, 2));
						SetPoseParameter("tentacle_x", UTIL_Approach(wantedY, paraY, 2));

						if (m_flAttackTime + 0.66 < gpGlobals->curtime && m_flLastDamageTime + 3.0f < gpGlobals->curtime)
						{
							m_flAttackTime = gpGlobals->curtime;

							//Lets save this random pose so that we will play that when done with the attack
							m_vRandomVector.x = wantedX;
							m_vRandomVector.y = wantedY;
							m_vRandomVector.z = wantedZ;

							m_flNextRandomTime = gpGlobals->curtime + 2;

							ResetSequence(LookupSequence("attack1"));
							m_bDamageGivenYet = false;
							m_iAnimation = PMAnim_attack;

							EmitSound("NPC_Vortigaunt.Swing");
						}
					}
					else
					{
						//Lets pick a new random pose since our enemy is too far away		

						if (m_flNextRandomTime < gpGlobals->curtime)
						{
							m_vRandomVector.x = random->RandomInt(-140, 140);
							m_vRandomVector.y = random->RandomInt(-140, 140);
							m_vRandomVector.z = 80;

							m_flNextRandomTime = gpGlobals->curtime + random->RandomFloat(0.1, 1.8);
						}

						SetPoseParameter("tentacle_z", UTIL_Approach(m_vRandomVector.x, paraX, 0.5));
						SetPoseParameter("tentacle_y", UTIL_Approach(m_vRandomVector.z, paraZ, 0.5));
						SetPoseParameter("tentacle_x", UTIL_Approach(m_vRandomVector.y, paraY, 0.5));
					}

				}

				DevMsg("pitmonster X: %f  ", wantedX);
				DevMsg("pitmonster Y: %f  ", wantedY);
				DevMsg("pitmonster Z: %f\n", wantedZ);

			}
		}
		else //has no enemy
		{
			if (m_flLastDamageTime + 1.6f < gpGlobals->curtime)
			{
				if (m_iAnimation == PMAnim_pain)
				{
					m_iAnimation = PMAnim_idle;
					ResetSequence(LookupSequence("idle"));
				}
			}


			if (m_flNextRandomTime < gpGlobals->curtime)
			{
				m_vRandomVector.x = random->RandomInt(-140, 140);
				m_vRandomVector.y = random->RandomInt(-140, 140);
				m_vRandomVector.z = 80;

				m_flNextRandomTime = gpGlobals->curtime + random->RandomFloat(1, 2);
			}

			SetPoseParameter("tentacle_z", UTIL_Approach(m_vRandomVector.x, paraX, 2));
			SetPoseParameter("tentacle_y", UTIL_Approach(m_vRandomVector.z, paraZ, 2));
			SetPoseParameter("tentacle_x", UTIL_Approach(m_vRandomVector.y, paraY, 2));
		}
	}
	else //We are so dead, ugh!
	{
		float paraX = GetPoseParameter("tentacle_x");
		float paraZ = GetPoseParameter("tentacle_y");
		float paraY = GetPoseParameter("tentacle_z");

		SetPoseParameter("tentacle_x", UTIL_Approach(0, paraX, 10));
		SetPoseParameter("tentacle_y", UTIL_Approach(80, paraZ, 10));
		SetPoseParameter("tentacle_z", UTIL_Approach(0, paraY, 10));
	}

	SetSimulationTime(gpGlobals->curtime);

	StudioFrameAdvance();

	// Think as soon as possible
	SetNextThink(gpGlobals->curtime); //TÄHÄN JOKU PITEMPI AIKA
}
