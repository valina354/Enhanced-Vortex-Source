//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: TE120 Physconcussion gun
//
//=============================================================================//

#ifndef WEAPON_PHYSCONCUSSION_H
#define WEAPON_PHYSCONCUSSION_H
#ifdef _WIN32
#pragma once
#endif

#include "basehlcombatweapon.h"
#include "beam_shared.h"
#include "Sprite.h"
#include "hl2_gamerules.h"
#include "player_pickup.h"

#define	NUM_BEAMS	4
#define	NUM_SPRITES	6

//TE120--
//----------------------------------------------------
// Definitions for gc gun overheat mechanic
//----------------------------------------------------
#define MIN_ENERGY_REQUIRED	25.0
#define MAX_ENERGY_REQUIRED	50.0
#define	MIN_READY_DELAY	0.65
#define	MAX_READY_DELAY	1.0
//TE120--

//-----------------------------------------------------------------------------

struct thrown_objects_conc_t
{
	float				fTimeThrown;
	EHANDLE				hEntity;

	DECLARE_SIMPLE_DATADESC();
};


//-----------------------------------------------------------------------------
// Do we have the super-phys gun?
//-----------------------------------------------------------------------------
bool PlayerHasMegaPhysConcussion();
void UTIL_PhysconcussionTraceHull( const Vector &vecAbsStart, const Vector &vecAbsEnd, const Vector &vecAbsMins, const Vector &vecAbsMaxs, CBaseEntity *pTraceOwner, trace_t *pTrace );
void UTIL_PhysconcussionTraceLine( const Vector &vecAbsStart, const Vector &vecAbsEnd, CBaseEntity *pTraceOwner, trace_t *pTrace );

// We want to test against brushes alone
class CTraceFilterOnlyBrushes : public CTraceFilterSimple
{
public:
	DECLARE_CLASS( CTraceFilterOnlyBrushes, CTraceFilterSimple );
	CTraceFilterOnlyBrushes( int collisionGroup ) : CTraceFilterSimple( NULL, collisionGroup ) {}
	virtual TraceType_t GetTraceType() const { return TRACE_WORLD_ONLY; }
};

class CWeaponPhysConcussion : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponPhysConcussion, CBaseHLCombatWeapon );

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CWeaponPhysConcussion( void );

	void	Drop( const Vector &vecVelocity );
	void	Precache();
	virtual void	Spawn();
	virtual void	OnRestore();
	virtual void	StopLoopingSounds();
	virtual void	UpdateOnRemove(void);
	void	PrimaryAttack();
	void	WeaponIdle();
	void	ItemPreFrame();
	void	ItemPostFrame();
	void	ItemBusyFrame();

	virtual float GetMaxAutoAimDeflection() { return 0.90f; }

	void	ForceDrop( void );
	bool	DropIfEntityHeld( CBaseEntity *pTarget ); // Drops its held entity if it matches the entity passed in
	//CGrabControllerConcussion &GetGrabController() { return m_grabController; }

	bool	CanHolster( void );
	bool	Holster( CBaseCombatWeapon *pSwitchingTo = NULL );
	bool	Deploy( void );

	bool	HasAnyAmmo( void ) { return true; }

	void	InputBecomeMegaConcussion( inputdata_t &inputdata );

	void	BeginUpgrade();

	virtual void SetViewModel( void );
	virtual const char *GetShootSound( int iIndex ) const;

	void	RecordThrownObject( CBaseEntity *pObject );
	void	PurgeThrownObjects();
	bool	IsAccountableForObject( CBaseEntity *pObject );

	bool	ShouldDisplayHUDHint() { return true; }

	virtual void HandleFireOnEmpty();

	CBaseEntity *FindObjectInCone( const Vector &vecOrigin, const Vector &vecDir, float flCone );

	// Punt objects - this is pointing at an object in the world and applying a force to it.
	void	PuntNonVPhysics( CBaseEntity *pEntity, const Vector &forward, trace_t &tr );
	void	PuntVPhysics( CBaseEntity *pEntity, const Vector &forward, trace_t &tr );
	void	PuntRagdoll( CBaseEntity *pEntity, const Vector &forward, trace_t &tr );

	bool	EntityAllowsPunts( CBaseEntity *pEntity );
	bool	IsOpen();

protected:
	enum FindObjectResult_t
	{
		OBJECT_FOUND = 0,
		OBJECT_NOT_FOUND,
		OBJECT_BEING_DETACHED,
	};

	void	CalcClip();
	void	DoMegaEffect( int effectType, Vector *pos = NULL );
	void	DoEffect( int effectType, Vector *pos = NULL );

	void	OpenElements( void );
	void	CloseElements( void );

	// Pickup and throw objects.
	bool	CanPickupObject( CBaseEntity *pTarget );
	void	CheckForTarget( void );
	FindObjectResult_t FindObject( void );
	void	FindObjectTrace( CBasePlayer *pPlayer, trace_t *pTraceResult );
	CBaseEntity *MegaPhysConcussionFindObjectInCone( const Vector &vecOrigin, const Vector &vecDir, float flCone, float flCombineBallCone, bool bOnlyCombineBalls );
	bool	AttachObject( CBaseEntity *pObject, const Vector &vPosition );
	void	UpdateObject( void );
	void	DetachObject( bool playSound = true, bool wasLaunched = false );
	void	LaunchObject( const Vector &vecDir, float flForce );
	void	StartEffects( void );	// Initialize all sprites and beams
	void	StopEffects( bool stopSound = true );	// Hide all effects temporarily
	void	DestroyEffects( void );	// Destroy all sprites and beams

	// Velocity-based throw common to punt and launch code.
	void	ApplyVelocityBasedForce( CBaseEntity *pEntity, const Vector &forward, const Vector &vecHitPos, PhysGunForce_t reason );

	// Physgun effects
	void	DoEffectClosed( void );
	void	DoMegaEffectClosed( void );

	void	DoEffectReady( void );
	void	DoMegaEffectReady( void );

	void	DoMegaEffectHolding( void );
	void	DoEffectHolding( void );

	void	DoMegaEffectLaunch( Vector *pos );
	void	DoEffectLaunch( Vector *pos );

	void	DoEffectNone( void );
	void	DoEffectIdle( void );

	// Trace length
	float	TraceLength();

	// Do we have the super-phys gun?
	inline bool IsMegaPhysConcussion()
	{
		return PlayerHasMegaPhysConcussion();
	}

	// Sprite scale factor
	float	SpriteScaleFactor();

	float	GetLoadPercentage();
	CSoundPatch *GetMotorSound( void );

	void	DryFire( void );
	void	PrimaryFireEffect( void );

	// What happens when the physgun picks up something
	void	Physgun_OnPhysGunPickup( CBaseEntity *pEntity, CBasePlayer *pOwner, PhysGunPickup_t reason );

	// Wait until we're done upgrading
	void	WaitForUpgradeThink();

	bool	m_bOpen;
	bool	m_bActive;
	bool	m_airBorneShotSpent;	// Track if first airborne shot has been fired
	int	m_nChangeState;			//For delayed state change of elements
	float	m_flCheckSuppressTime;	//Amount of time to suppress the checking for targets
	bool	m_flLastDenySoundPlayed;	//Debounce for deny sound
	int	m_nAttack2Debounce;

	CNetworkVar( bool, m_bIsCurrentlyUpgrading );
	CNetworkVar( float, m_flTimeForceView );

	float	m_flWaitForDeploy;		// Time to wait for deploy anim
	float	m_flShrinkRate;			// Standard shrink rate for energy recovery
	float	m_flElementDebounce;
	float	m_flElementPosition;
	float	m_flElementDestination;

	CHandle<CBeam>	m_hBeams[NUM_BEAMS];
	CHandle<CSprite>	m_hGlowSprites[NUM_SPRITES];
	CHandle<CSprite>	m_hEndSprites[2];
	float			m_flEndSpritesOverride[2];
	CHandle<CSprite>	m_hCenterSprite;
	CHandle<CSprite>	m_hBlastSprite;

	CSoundPatch		*m_sndMotor;// Whirring sound for the gun

	//CGrabControllerConcussion m_grabController;

	int			m_EffectState;// Current state of the effects on the gun

	bool			m_bPhysconcussionState;

	// A list of the objects thrown or punted recently, and the time done so.
	CUtlVector< thrown_objects_conc_t > m_ThrownEntities;

	float			m_flTimeNextObjectPurge;

	//TE120--
	// For GC overheat mechanic
	float	m_flNextCoolDown;	// Time to wait before cooling down
	float	m_flOverHeatWait;	// Additional time to wait for refire
	float	m_flEnergyRequired;	// Energy needed for next shot
	float	m_flLastRecoveryTime;	// Randomization of cool down
	float	m_flRecoveryRateScale;	// For exponential recovery
	float	m_flRecoveryRate;	// Standard recovery rate
	//TE120--

protected:
	// Because the physconcussion is a leaf class, we can use
	// static variables to store this information, and save some memory.
	// Should the physconcussion end up having inheritors, their activate may
	// stomp these numbers, in which case you should make these ordinary members
	// again.
	//
	// The physconcussion also caches some pose parameters in SetupGlobalModelData().
	static int m_poseActive;
	static bool m_sbStaticPoseParamsLoaded;

private:
	int m_nNumShotsFired;
};

//-----------------------------------------------------------------------------
// Do we have the super-phys gun?
//-----------------------------------------------------------------------------

// force the physcannon to drop an object (if carried)
//void PhysConcussionForceDrop( CBaseCombatWeapon *pActiveWeapon, CBaseEntity *pOnlyIfHoldingThis );
void PhysConcussionBeginUpgrade( CBaseAnimating *pAnim );

//bool PlayerPickupControllerIsHoldingEntity( CBaseEntity *pPickupController, CBaseEntity *pHeldEntity );
//float PlayerPickupGetHeldObjectMass( CBaseEntity *pPickupControllerEntity, IPhysicsObject *pHeldObject );
//float PhysConcussionGetHeldObjectMass( CBaseCombatWeapon *pActiveWeapon, IPhysicsObject *pHeldObject );

//CBaseEntity *PhysConcussionGetHeldEntity( CBaseCombatWeapon *pActiveWeapon );
//CBaseEntity *GetPlayerHeldEntity( CBasePlayer *pPlayer );

bool PhysConcussionAccountableForObject( CBaseCombatWeapon *pPhysConcussion, CBaseEntity *pObject );

#endif // WEAPON_PHYSCONCUSSION_H