//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"
#include "npc_basescanner.h"
#include "decals.h"
#include "te_effect_dispatch.h"
#include "gib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar	sk_wpnscanner_health( "sk_wpnscanner_health","0");
ConVar	sk_wpnscanner_proj_dmg( "sk_wpnscanner_proj_dmg","0");
ConVar	sk_wpnscanner_proj_speed( "sk_wpnscanner_proj_speed","0");

#define BOLT_SPREAD 1.5

// Creation.
struct baseprojectilecreate_t
{
	Vector vecOrigin;
	Vector vecVelocity;
	CBaseEntity *pOwner;
	string_t iszModel;
	float flDamage;
	int iDamageType;
	float flDamageScale;
};

//=============================================================================
//
// Generic projectile
//
class CBaseScannerProjectile : public CBaseAnimating
{
	DECLARE_CLASS(CBaseScannerProjectile, CBaseAnimating);
public:
	DECLARE_DATADESC();

	void	Spawn(void);
	void	Precache(void);

	static CBaseScannerProjectile *Create(baseprojectilecreate_t &pCreate);

	void			SetDamage(float flDamage) { m_flDamage = flDamage; }
	void			SetDamageScale(float &flScale) { m_flDamageScale = flScale; }
	void			SetDamageType(int iType) { m_iDamageType = iType; }

private:
	// Damage
	virtual float	GetDamage() { return m_flDamage; }
	virtual float	GetDamageScale(void) { return m_flDamageScale; }
	virtual int		GetDamageType(void) { return m_iDamageType; }

	unsigned int	PhysicsSolidMaskForEntity(void) const;

	virtual void	ProjectileTouch(CBaseEntity *pOther);
	void			FlyThink(void);

protected:
	float			m_flDamage;
	int				m_iDamageType;
	float			m_flDamageScale;
};

BEGIN_DATADESC(CBaseScannerProjectile)
DEFINE_FIELD(m_flDamage, FIELD_FLOAT),
DEFINE_FIELD(m_iDamageType, FIELD_INTEGER),
DEFINE_FIELD(m_flDamageScale, FIELD_FLOAT),

DEFINE_FUNCTION(ProjectileTouch),
DEFINE_THINKFUNC(FlyThink),
END_DATADESC()

LINK_ENTITY_TO_CLASS(proj_scanner_base, CBaseScannerProjectile);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseScannerProjectile::Spawn(void)
{
	Precache();

	SetModel(STRING(GetModelName()));

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	AddFlag(FL_OBJECT);

	UTIL_SetSize(this, -Vector(1.0f, 1.0f, 1.0f), Vector(1.0f, 1.0f, 1.0f));

	// Setup attributes.
	SetGravity(0.001f);
	m_takedamage = DAMAGE_NO;

	// Setup the touch and think functions.
	SetTouch(&CBaseScannerProjectile::ProjectileTouch);
	SetThink(&CBaseScannerProjectile::FlyThink);
	SetNextThink(gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseScannerProjectile::Precache(void)
{
	BaseClass::Precache();

	PrecacheModel(STRING(GetModelName()));
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseScannerProjectile *CBaseScannerProjectile::Create(baseprojectilecreate_t &pCreate)
{
	CBaseScannerProjectile *pProjectile = static_cast<CBaseScannerProjectile*>(CBaseEntity::CreateNoSpawn("proj_scanner_base", pCreate.vecOrigin, vec3_angle, pCreate.pOwner));
	if (!pProjectile)
		return NULL;

	pProjectile->SetModelName(pCreate.iszModel);
	pProjectile->SetDamage(pCreate.flDamage);
	pProjectile->SetDamageType(pCreate.iDamageType);
	pProjectile->SetDamageScale(pCreate.flDamageScale);
	pProjectile->SetAbsVelocity(pCreate.vecVelocity);

	// Setup the initial angles.
	QAngle angles;
	VectorAngles(-pCreate.vecVelocity, angles);
	pProjectile->SetAbsAngles(angles);

	// Spawn & Activate
	DispatchSpawn(pProjectile);
	pProjectile->Activate();

	return pProjectile;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
unsigned int CBaseScannerProjectile::PhysicsSolidMaskForEntity(void) const
{
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_HITBOX;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CBaseScannerProjectile::ProjectileTouch(CBaseEntity *pOther)
{
	// Verify a correct "other."
	Assert(pOther);
	if (!pOther->IsSolid() || pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS))
		return;

	// Handle hitting skybox (disappear).
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();
	trace_t *pNewTrace = const_cast<trace_t*>(pTrace);

	if (pTrace->surface.flags & SURF_SKY)
	{
		UTIL_Remove(this);
		return;
	}

	CTakeDamageInfo info;
	info.SetAttacker(GetOwnerEntity());
	info.SetInflictor(this);
	info.SetDamage(GetDamage());
	info.SetDamageType(GetDamageType());
	CalculateMeleeDamageForce(&info, GetAbsVelocity(), GetAbsOrigin(), GetDamageScale());

	Vector dir;
	AngleVectors(GetAbsAngles(), &dir);

	pOther->DispatchTraceAttack(info, dir, pNewTrace);
	ApplyMultiDamage();

	UTIL_Remove(this);
}

//-----------------------------------------------------------------------------
// Purpose: Orient the projectile along its velocity
//-----------------------------------------------------------------------------
void CBaseScannerProjectile::FlyThink(void)
{
	QAngle angles;
	VectorAngles(-(GetAbsVelocity()), angles);
	SetAbsAngles(angles);
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Scanner with a gun
//-----------------------------------------------------------------------------
class CNPC_WpnScanner : public CNPC_BaseScanner
{
	DECLARE_CLASS( CNPC_WpnScanner, CNPC_BaseScanner );
public: 
	void		Spawn( void );
	void		Precache( void );

	int			TranslateSchedule( int scheduleType );
	void		StartTask( const Task_t *pTask );
	void		RunTask( const Task_t *pTask );

	float		MinGroundDist( void );
	void		MoveToAttack(float flInterval);

	char			*GetEngineSound(void);

	virtual void	Gib(void);
	void		Event_Killed(const CTakeDamageInfo &info);

private:
	string_t	m_iszProjectileModel;
	int			m_iMuzzleAttachment;

private:
	DEFINE_CUSTOM_AI;

	/*
	// Custom interrupt conditions
	enum
	{
		COND_CSCANNER_HAVE_INSPECT_TARGET = BaseClass::NEXT_CONDITION,

		NEXT_CONDITION,
	};
	*/

	// Custom schedules
	enum
	{
		SCHED_WPNSCANNER_ATTACK = BaseClass::NEXT_SCHEDULE,

		NEXT_SCHEDULE,
	};

	// Custom tasks
	enum
	{
		TASK_WPNSCANNER_ATTACK = BaseClass::NEXT_TASK,

		NEXT_TASK,
	};
};

LINK_ENTITY_TO_CLASS( npc_wpnscanner, CNPC_WpnScanner );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_WpnScanner::Spawn( void )
{
	Precache();
	SetModel( STRING( GetModelName() ) );

	SetHealth( sk_wpnscanner_health.GetFloat() );
	SetMaxHealth( GetHealth() );

	CapabilitiesAdd( bits_CAP_INNATE_MELEE_ATTACK1 );

	m_iMuzzleAttachment = LookupAttachment( "light" );

	BaseClass::Spawn();

	m_flAttackNearDist = 250;
	m_flAttackFarDist = 700;
	m_flAttackRange = 750;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CNPC_WpnScanner::Precache()
{
	if( !GetModelName() )
	{
		SetModelName(MAKE_STRING("models/shield_scanner.mdl"));
	}

	PrecacheModel("models/gibs/Shield_Scanner_Gib1.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib2.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib3.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib4.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib5.mdl");
	PrecacheModel("models/gibs/Shield_Scanner_Gib6.mdl");

	PrecacheScriptSound("NPC_WPNScanner.Engine");
	PrecacheScriptSound("NPC_WPNScanner.Fire");

	PrecacheModel( STRING( GetModelName() ) );

	m_iszProjectileModel = MAKE_STRING( "models/energy_bolt.mdl" );
	PrecacheModel( STRING(m_iszProjectileModel) );

	BaseClass::Precache();
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
void CNPC_WpnScanner::Gib(void)
{
	if (IsMarkedForDeletion())
		return;

	CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib1.mdl");
	CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib2.mdl");
	CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib3.mdl");
	CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib4.mdl");
	CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib5.mdl");
	CGib::SpawnSpecificGibs(this, 1, 500, 250, "models/gibs/Shield_Scanner_Gib6.mdl");

	BaseClass::Gib();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
char *CNPC_WpnScanner::GetEngineSound(void)
{
	return "NPC_WPNScanner.Engine";
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pInflictor - 
//			pAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
void CNPC_WpnScanner::Event_Killed(const CTakeDamageInfo &info)
{
	// Copy off the takedamage info that killed me, since we're not going to call
	// up into the base class's Event_Killed() until we gib. (gibbing is ultimate death)
	m_KilledInfo = info;

	if (GetEnemy() != NULL && (info.GetDamageType() & DMG_DISSOLVE) == false)
	{
		Vector vecDelta = GetLocalOrigin() - GetEnemy()->GetLocalOrigin();
		if ((vecDelta.z > 120) && (vecDelta.Length() > 360))
		{
			// If I'm divebombing, don't take any more damage. It will make Event_Killed() be called again.
			// This is especially bad if someone machineguns the divebombing scanner. 
			AttackDivebomb();
			return;
		}
	}

	Gib();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : Type - 
//-----------------------------------------------------------------------------
int CNPC_WpnScanner::TranslateSchedule( int scheduleType ) 
{
	switch ( scheduleType )
	{
	case SCHED_IDLE_STAND:
		return SCHED_SCANNER_PATROL;

	case SCHED_SCANNER_ATTACK:
		return SCHED_WPNSCANNER_ATTACK;
	}
	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_WpnScanner::StartTask( const Task_t *pTask )
{
	switch ( pTask->iTask )
	{
	case TASK_WPNSCANNER_ATTACK:
		{
			// Windup
			break;
		}

	default:
		{
			BaseClass::StartTask(pTask);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CNPC_WpnScanner::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_WPNSCANNER_ATTACK:
	{
		CBaseEntity *pEnemy = GetEnemy();
		if (!pEnemy)
		{
			TaskFail(FAIL_NO_ENEMY);
			return;
		}

		if (m_flNextAttack > gpGlobals->curtime)
			return;
		m_flNextAttack = gpGlobals->curtime + 0.2;

		EmitSound("NPC_WPNScanner.Fire");

		Vector vecFirePos;
		QAngle vecAngles;
		GetAttachment(m_iMuzzleAttachment, vecFirePos, vecAngles);
		Vector vecTarget = GetEnemy()->BodyTarget(vecFirePos);
		Vector vecToTarget = (vecTarget - vecFirePos);
		VectorNormalize(vecToTarget);

		VectorAngles(vecToTarget, vecAngles);
		Vector vecRight;
		Vector vecUp;
		AngleVectors(vecAngles, &vecToTarget, &vecRight, &vecUp);

		// Add some inaccuracy
		float x, y, z;
		do {
			x = random->RandomFloat(-BOLT_SPREAD, BOLT_SPREAD) + random->RandomFloat(-BOLT_SPREAD, BOLT_SPREAD);
			y = random->RandomFloat(-BOLT_SPREAD, BOLT_SPREAD) + random->RandomFloat(-BOLT_SPREAD, BOLT_SPREAD);
			z = x*x + y*y;
		} while (z > 1);

		vecToTarget = vecToTarget + x * VECTOR_CONE_20DEGREES * vecRight + y * VECTOR_CONE_20DEGREES * vecUp;
		vecToTarget *= sk_wpnscanner_proj_speed.GetFloat();

		baseprojectilecreate_t newProj;
		newProj.vecOrigin = vecFirePos;
		newProj.vecVelocity = vecToTarget;
		newProj.pOwner = this;
		newProj.iszModel = m_iszProjectileModel;
		newProj.flDamage = sk_wpnscanner_proj_dmg.GetFloat();
		newProj.iDamageType = DMG_ENERGYBEAM;
		newProj.flDamageScale = g_pGameRules->GetDamageMultiplier();
		CBaseScannerProjectile::Create(newProj);

		break;
	}

	default:
	{
		BaseClass::RunTask(pTask);
	}
	}
}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
float CNPC_WpnScanner::MinGroundDist( void )
{
	if ( m_nFlyMode == SCANNER_FLY_ATTACK  )
		return 16;

	return BaseClass::MinGroundDist();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flInterval - 
//-----------------------------------------------------------------------------
void CNPC_WpnScanner::MoveToAttack(float flInterval)
{
	if (GetEnemy() == NULL)
		return;

	if ( flInterval <= 0 )
		return;

	Vector vTargetPos = GetEnemyLKP();
	Vector idealPos = IdealGoalForMovement( vTargetPos, GetAbsOrigin(), GetGoalDistance(), 128 );

	//NDebugOverlay::Box( idealPos, -Vector(4,4,4), Vector(4,4,4), 0,255,0,8, 0.1 );

	MoveToTarget( flInterval, idealPos );

	//FIXME: Re-implement?

	/*
	// ---------------------------------------------------------
	//  Add evasion if I have taken damage recently
	// ---------------------------------------------------------
	if ((m_flLastDamageTime + SCANNER_EVADE_TIME) > gpGlobals->curtime)
	{
	vFlyDirection = vFlyDirection + VelocityToEvade(GetEnemyCombatCharacterPointer());
	}
	*/
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_wpnscanner, CNPC_WpnScanner )

	DECLARE_TASK( TASK_WPNSCANNER_ATTACK )

	//DECLARE_CONDITION(COND_CSCANNER_HAVE_INSPECT_TARGET)

	//=========================================================
	// > SCHED_WPNSCANNER_ATTACK
	//=========================================================
	DEFINE_SCHEDULE
	(
		SCHED_WPNSCANNER_ATTACK,

		"	Tasks"
		"		TASK_SCANNER_SET_FLY_ATTACK			0"
		"		TASK_SET_ACTIVITY					ACTIVITY:ACT_IDLE"
		"		TASK_WPNSCANNER_ATTACK				0"
		"		TASK_WAIT_RANDOM					0.5"
		""
		"	Interrupts"
		"		COND_NEW_ENEMY"
		"		COND_ENEMY_DEAD"
		"		COND_ENEMY_OCCLUDED"
		"		COND_TOO_FAR_TO_ATTACK"
		"		COND_NOT_FACING_ATTACK"
		"		COND_SCANNER_GRABBED_BY_PHYSCANNON"
	)

AI_END_CUSTOM_NPC()
