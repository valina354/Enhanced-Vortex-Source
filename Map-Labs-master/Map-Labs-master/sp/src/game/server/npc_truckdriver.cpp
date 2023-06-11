//=================== Half-Life 2: Short Stories Mod 2008 =====================//
//
// Purpose:	Truck Driver for HE
//			by Au-heppa
//
//=============================================================================//

#include "cbase.h"
#include "ai_network.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_node.h"
#include "ai_task.h"
#include "ai_senses.h"
#include "ai_navigator.h"
#include "ai_route.h"
#include "entitylist.h"
#include "soundenvelope.h"
#include "gamerules.h"
#include "ndebugoverlay.h"
#include "soundflags.h"
#include "trains.h"
#include "globalstate.h"
#include "vehicle_base.h"
#include "npc_vehicledriver.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TRUCKDRIVER_MODEL "models/HLSS_Characters/TruckDriver.mdl" //

#define	DRIVER_DEBUG_PATH				1
#define	DRIVER_DEBUG_PATH_SPLINE		2

extern ConVar	sk_citizen_health;
extern ConVar	g_debug_vehicledriver;

ConVar	truck_driver_min_follow_distance("hlss_truck_driver_min_follow_distance", "384");
ConVar	truck_driver_max_follow_distance("hlss_truck_driver_max_follow_distance", "2048");

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_TruckDriver : public CNPC_VehicleDriver
{
	DECLARE_CLASS(CNPC_TruckDriver, CNPC_VehicleDriver);
	DECLARE_DATADESC();
public:
	//DECLARE_DATADESC();
	//DEFINE_CUSTOM_AI;

	CNPC_TruckDriver(void);

	virtual void	Spawn(void);
	virtual void	Precache(void);

	//virtual void	Activate( void );

	//virtual bool	OverridePathMove( float flInterval );

	virtual void	PrescheduleThink();

	int				BloodColor(void) { return BLOOD_COLOR_RED; }

	Class_T Classify()
	{
		if (GlobalEntity_GetState("gordon_precriminal") == GLOBAL_ON)
			return CLASS_CITIZEN_PASSIVE;

		if (GlobalEntity_GetState("citizens_passive") == GLOBAL_ON)
			return CLASS_CITIZEN_PASSIVE;

		return CLASS_CITIZEN_REBEL;
	}

	void			UpdateHead();

	// Driving
	virtual void	DriveVehicle(void);

	void 			InputEnableFollow(inputdata_t &inputdata);
	void 			InputDisableFollow(inputdata_t &inputdata);
	void			InputFollowPlayer(inputdata_t &inputdata);

private:

	bool	m_bFollowPlayerSpeed;

	float	m_flFollowPlayerSpeedScale;
	//float	m_flFollowPlayerFrameTime;
};

BEGIN_DATADESC(CNPC_TruckDriver)

DEFINE_KEYFIELD(m_bFollowPlayerSpeed, FIELD_BOOLEAN, "followplayerspeed"),
DEFINE_KEYFIELD(m_flFollowPlayerSpeedScale, FIELD_FLOAT, "followplayerspeedscale"),
//DEFINE_FIELD(		m_flFollowPlayerFrameTime, FIELD_TIME),

DEFINE_INPUTFUNC(FIELD_VOID, "EnablePlayerFollow", InputEnableFollow),
DEFINE_INPUTFUNC(FIELD_VOID, "DisablePlayerFollow", InputDisableFollow),
DEFINE_INPUTFUNC(FIELD_VOID, "FollowPlayer", InputFollowPlayer),

END_DATADESC()

LINK_ENTITY_TO_CLASS(npc_truckdriver, CNPC_TruckDriver);


CNPC_TruckDriver::CNPC_TruckDriver(void)
{
	m_flFollowPlayerSpeedScale = 0.75;
	m_bFollowPlayerSpeed = false;
}


//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CNPC_TruckDriver::Spawn(void)
{
	Precache();

	CAI_BaseNPC::Spawn();

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD);
	//CapabilitiesAdd( bits_CAP_MOVE_SHOOT );

	SetModel(TRUCKDRIVER_MODEL);
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();
	m_iMaxHealth = m_iHealth = sk_citizen_health.GetFloat();
	m_flFieldOfView = VIEW_FIELD_FULL;

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_NONE);

	m_lifeState = LIFE_ALIVE;
	//SetCycle( 0 );
	//ResetSequenceInfo();

	AddFlag(FL_NPC);

	m_flMaxSpeed = 0;
	m_flGoalSpeed = m_flInitialSpeed;

	m_vecDesiredVelocity = vec3_origin;
	m_vecPrevPoint = vec3_origin;
	m_vecPrevPrevPoint = vec3_origin;
	m_vecPostPoint = vec3_origin;
	m_vecPostPostPoint = vec3_origin;
	m_vecDesiredPosition = vec3_origin;
	m_flSteering = 45;
	m_flDistanceAlongSpline = 0.2;
	m_pCurrentWaypoint = m_pNextWaypoint = NULL;

	GetNavigator()->SetPathcornerPathfinding(false);

	//m_flFollowPlayerFrameTime = 0;

	NPCInit();

	m_takedamage = DAMAGE_YES;
}

#define TRUCKDRIVER_FOLLOW_PLAYER_FRAME 0.5f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_TruckDriver::InputFollowPlayer(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

	ClearWaypoints();

	// Drive to the point
	if (pPlayer->GetVehicle() && pPlayer->GetVehicle()->GetVehicleEnt())
	{
		//AI_NavGoal_t goal( GOALTYPE_ENEMY, pPlayer->GetVehicle()->GetVehicleEnt()->GetLocalOrigin(), ACT_WALK, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST,  pPlayer->GetVehicle()->GetVehicleEnt() );
		//GetNavigator()->SetGoal( goal );

		SetGoalEnt(pPlayer->GetVehicle()->GetVehicleEnt());
	}
	else
	{
		//AI_NavGoal_t goal( GOALTYPE_ENEMY, pPlayer->GetLocalOrigin(), ACT_WALK, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST,  pPlayer );
		//GetNavigator()->SetGoal( goal );

		SetGoalEnt(pPlayer);
	}

	//m_flFollowPlayerFrameTime = gpGlobals->curtime + TRUCKDRIVER_FOLLOW_PLAYER_FRAME;


	if (m_NPCState == NPC_STATE_IDLE)
	{
		SetState(NPC_STATE_ALERT);
	}
	SetCondition(COND_PROVOKED);

	// Force him to start forward
	InputStartForward(inputdata);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_TruckDriver::Precache(void)
{
	PrecacheModel(TRUCKDRIVER_MODEL);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CNPC_TruckDriver::Activate( void )
{
CAI_BaseNPC::Activate();

// Restore doesn't need to do this
if ( m_hVehicleEntity )
return;

// Make sure we've got a vehicle
if ( m_iszVehicleName == NULL_STRING )
{
Warning( "npc_truckdriver %s has no vehicle to drive.\n", STRING(GetEntityName()) );
UTIL_Remove( this );
return;
}

m_hVehicleEntity = (gEntList.FindEntityByName( NULL, STRING(m_iszVehicleName) ));
if ( !m_hVehicleEntity )
{
Warning( "npc_truckdriver %s couldn't find his vehicle named %s.\n", STRING(GetEntityName()), STRING(m_iszVehicleName) );
UTIL_Remove( this );
return;
}

m_pVehicleInterface = m_hVehicleEntity->GetServerVehicle();
Assert( m_pVehicleInterface );
if ( !m_pVehicleInterface->NPC_CanDrive() )
{
Warning( "npc_truckdriver %s doesn't know how to drive vehicle %s.\n", STRING(GetEntityName()), STRING(m_hVehicleEntity->GetEntityName()) );
UTIL_Remove( this );
return;
}

// We've found our vehicle. Move to it and start following it.
Vector vecOrigin = m_hVehicleEntity->GetLocalOrigin();
QAngle vecAngles = m_hVehicleEntity->GetAbsAngles() ; //+ QAngle(0,-90,0);

SetParent( m_hVehicleEntity);

SetAbsOrigin( vecOrigin );
SetAbsAngles( vecAngles );
m_pVehicleInterface->NPC_SetDriver( this );

RecalculateSpeeds();
}*/

void CNPC_TruckDriver::InputEnableFollow(inputdata_t &inputdata)
{
	m_bFollowPlayerSpeed = true;
}

void CNPC_TruckDriver::InputDisableFollow(inputdata_t &inputdata)
{
	m_bFollowPlayerSpeed = false;
}

/*bool CNPC_TruckDriver::OverridePathMove( float flInterval )
{
if (m_flFollowPlayerFrameTime!=0 && m_flFollowPlayerFrameTime < gpGlobals->curtime)
{
ClearWaypoints();

CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

if (pPlayer->GetVehicle() && pPlayer->GetVehicle()->GetVehicleEnt() )
{
AI_NavGoal_t goal( GOALTYPE_ENEMY, pPlayer->GetVehicle()->GetVehicleEnt()->GetLocalOrigin(), ACT_WALK, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST,  pPlayer->GetVehicle()->GetVehicleEnt() );
GetNavigator()->SetGoal( goal );

//SetGoalEnt( pPlayer->GetVehicle()->GetVehicleEnt() );
}
else
{
AI_NavGoal_t goal( GOALTYPE_ENEMY, pPlayer->GetLocalOrigin(), ACT_WALK, AIN_DEF_TOLERANCE, AIN_YAW_TO_DEST,  pPlayer );
GetNavigator()->SetGoal( goal );

//SetGoalEnt( pPlayer );
}

RecalculateSpeeds();

m_flFollowPlayerFrameTime = gpGlobals->curtime + TRUCKDRIVER_FOLLOW_PLAYER_FRAME;
}

return BaseClass::OverridePathMove( flInterval );
}*/

//-----------------------------------------------------------------------------
// Purpose: This takes the current place the NPC's trying to get to, figures out
//			what keys to press to get the vehicle to go there, and then sends
//			them to the vehicle.
//-----------------------------------------------------------------------------
void CNPC_TruckDriver::DriveVehicle(void)
{
	AngularImpulse angVel;
	Vector vecVelocity;
	IPhysicsObject *pVehiclePhysics = m_hVehicleEntity->VPhysicsGetObject();
	if (!pVehiclePhysics)
		return;
	pVehiclePhysics->GetVelocity(&vecVelocity, &angVel);
	float flSpeed = VectorNormalize(vecVelocity);

	// If we have no target position to drive to, brake to a halt
	if (!m_flMaxSpeed || m_vecDesiredPosition == vec3_origin)
	{
		if (flSpeed > 1)
		{
			m_pVehicleInterface->NPC_Brake();
		}
		return;
	}

	if (g_debug_vehicledriver.GetInt() & DRIVER_DEBUG_PATH)
	{
		NDebugOverlay::Box(m_vecDesiredPosition, -Vector(20, 20, 20), Vector(20, 20, 20), 0, 255, 0, true, 0.1);
		NDebugOverlay::Line(GetAbsOrigin(), GetAbsOrigin() + m_vecDesiredVelocity, 0, 255, 0, true, 0.1);
	}

	//TERO: Later add here so that we will go the same speed as the players vehicle
	m_flGoalSpeed = VectorNormalize(m_vecDesiredVelocity);

	if (m_bFollowPlayerSpeed)
	{
		//TERO: Lets check if we are close enough to player
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

		if (pPlayer)
		{
			Vector vecDist = GetAbsOrigin() - pPlayer->GetAbsOrigin();
			float flDist = VectorNormalize(vecDist);

			if (flDist < truck_driver_max_follow_distance.GetFloat() && flDist > truck_driver_min_follow_distance.GetFloat())
			{
				//Vector forward;
				//GetVectors( NULL, &forward, NULL ); //we take forward from the "right" because the models are fucked up

				float flDot = DotProduct(m_vecDesiredVelocity, vecDist);

				//	TERO: Lets see if the player is behind us
				if (flDot > 0.8)
				{
					if (pPlayer->GetVehicle() &&
						pPlayer->GetVehicle()->GetVehicleEnt() &&
						pPlayer->GetVehicle()->GetVehicleEnt()->VPhysicsGetObject())
					{
						pPlayer->GetVehicle()->GetVehicleEnt()->VPhysicsGetObject()->GetVelocity(&vecDist, NULL);
					}
					else
					{
						vecDist = pPlayer->GetAbsVelocity();
					}

					if (vecDist.Length() < m_flGoalSpeed)
						m_flGoalSpeed = vecDist.Length() * m_flFollowPlayerSpeedScale; // * 1.2;
					//else
					//	m_flGoalSpeed = (vecDist.Length()/2) + (m_flGoalSpeed/2); 

					//DevMsg("Player speed: %f\n", m_flGoalSpeed);
				}
			}

		}

	}//end if m_bFollowPlayerSpeed


	// Is our target in front or behind us?
	Vector vecForward, vecRight;
	m_hVehicleEntity->GetVectors(&vecForward, &vecRight, NULL);
	float flDot = DotProduct(vecForward, m_vecDesiredVelocity);
	bool bBehind = (flDot < 0);
	float flVelDot = DotProduct(vecVelocity, m_vecDesiredVelocity);
	bool bGoingWrongWay = (flVelDot < 0);

	// Figure out whether we should accelerate / decelerate
	if (bGoingWrongWay || (flSpeed < m_flGoalSpeed))
	{
		// If it's behind us, go backwards not forwards
		if (bBehind)
		{
			m_pVehicleInterface->NPC_ThrottleReverse();
		}
		else
		{
			m_pVehicleInterface->NPC_ThrottleForward();
		}
	}
	else
	{
		// Brake if we're go significantly too fast
		if ((flSpeed - 200) > m_flGoalSpeed)
		{
			m_pVehicleInterface->NPC_Brake();
		}
		else
		{
			m_pVehicleInterface->NPC_ThrottleCenter();
		}
	}

	// Do we need to turn?
	float flDotRight = DotProduct(vecRight, m_vecDesiredVelocity);
	if (bBehind)
	{
		// If we're driving backwards, flip our turning
		flDotRight *= -1;
	}
	// Map it to the vehicle's steering
	flDotRight *= (m_flSteering / 90);

	float steering = GetPoseParameter("vehicle_steer");
	SetPoseParameter("vehicle_steer", UTIL_Approach(flDotRight, steering, 50));

	if (flDotRight < 0)
	{
		// Turn left
		m_pVehicleInterface->NPC_TurnLeft(-flDotRight);
	}
	else if (flDotRight > 0)
	{
		// Turn right
		m_pVehicleInterface->NPC_TurnRight(flDotRight);
	}
	else
	{
		m_pVehicleInterface->NPC_TurnCenter();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_TruckDriver::PrescheduleThink(void)
{
	UpdateHead();

	BaseClass::PrescheduleThink();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_TruckDriver::UpdateHead(void)
{
	float yaw = GetPoseParameter("head_yaw");
	float pitch = GetPoseParameter("head_pitch");

	// If we should be watching our enemy, turn our head
	if ((GetEnemy() != NULL))
	{
		Vector	enemyDir = GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter();
		VectorNormalize(enemyDir);

		float angle = GetAbsOrigin().z + 90;
		float angleDiff = VecToYaw(enemyDir);
		angleDiff = UTIL_AngleDiff(angleDiff, angle + yaw);

		SetPoseParameter("head_yaw", UTIL_Approach(yaw + angleDiff, yaw, 50));

		angle = UTIL_VecToPitch(BodyDirection3D());
		angleDiff = UTIL_VecToPitch(enemyDir);
		angleDiff = UTIL_AngleDiff(angleDiff, angle + pitch);

		SetPoseParameter("head_pitch", UTIL_Approach(pitch + angleDiff, pitch, 50));
	}
	else
	{
		// Otherwise turn the head back to its normal position
		SetPoseParameter("head_yaw", UTIL_Approach(0, yaw, 10));
		SetPoseParameter("head_pitch", UTIL_Approach(0, pitch, 10));
	}
}