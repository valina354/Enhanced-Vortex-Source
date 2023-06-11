//=================== Half-Life 2: Short Stories Mod 2009 =====================//
//
// Purpose:	Recharge behavior
//
//=============================================================================//

#include "cbase.h"
#include "ai_behavior_recharge.h"
#include "ai_navigator.h"
#include "ai_memory.h"
#include "ai_squad.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC(CRechargePoint)
DEFINE_FIELD(m_hLockedBy, FIELD_EHANDLE),
//	DEFINE_KEYFIELD( m_bActive,					FIELD_BOOLEAN,		"active" ),
DEFINE_KEYFIELD(m_flMaxDistance, FIELD_FLOAT, "max_distance"),
DEFINE_KEYFIELD(m_RechargeSequenceName, FIELD_STRING, "sequence"),
DEFINE_KEYFIELD(m_iszActor, FIELD_STRING, "actor"),
DEFINE_KEYFIELD(m_SearchType, FIELD_INTEGER, "SearchType"),

DEFINE_THINKFUNC(FindRechargeThink),

// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Deactivate", InputDeactivate),

// Outputs
DEFINE_OUTPUT(m_OnRecharge, "OnRecharge"),
END_DATADESC();

LINK_ENTITY_TO_CLASS(hlss_recharge_point, CRechargePoint);

void CRechargePoint::Spawn()
{
	BaseClass::Spawn();

	if (HasSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE))
	{
		if (!FindRechagerNPC())
		{
			SetThink(&CRechargePoint::FindRechargeThink);
			SetNextThink(gpGlobals->curtime + HLSS_RECHARGE_POINT_THINK_DELAY);
		}
	}
}

void CRechargePoint::InputActivate(inputdata_t &inputdata)
{
	if (!HasSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE))
	{
		AddSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE);

		if (!FindRechagerNPC())
		{
			SetThink(&CRechargePoint::FindRechargeThink);
			SetNextThink(gpGlobals->curtime + HLSS_RECHARGE_POINT_THINK_DELAY);
		}
	}
}

bool CRechargePoint::Lock(CAI_BaseNPC *pLocker)
{
	if (IsLocked())
	{
		// Already locked.
		return false;
	}

	m_hLockedBy.Set(pLocker);
	return true;
}


bool CRechargePoint::Unlock(CAI_BaseNPC *pUnlocker)
{
	if (IsLocked())
	{
		if (m_hLockedBy.Get() != pUnlocker)
		{
			// Refuse! Only the locker may unlock.
			return false;
		}
	}

	m_hLockedBy.Set(NULL);

	//We need a new guy
	if (HasSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE))
	{
		//TERO: I commented the FindRechargeNPC out so that if the NPC was picked up by some other recharge behavior the NPC will have some time to
		//		walk towards it before this tries to get it back
		//if (!FindRechagerNPC())
		//{
		SetThink(&CRechargePoint::FindRechargeThink);
		SetNextThink(gpGlobals->curtime + HLSS_RECHARGE_POINT_THINK_DELAY_CHANGE);
		//}
	}

	return true;
}

void CRechargePoint::FindRechargeThink()
{
#ifdef HLSS_DEBUG_RECHARGE_BEHAVIOR
	DevMsg("hlss_recharge_point: %S think!\n", GetDebugName());
#endif

	if (HasSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE) && !FindRechagerNPC())
	{
		SetNextThink(gpGlobals->curtime + HLSS_RECHARGE_POINT_THINK_DELAY);
	}
	else
	{
		SetThink(NULL);
	}
}

bool CRechargePoint::FindRechagerNPC()
{
	float flBestDist = m_flMaxDistance;
	CAI_BaseNPC *pBest = NULL;
	bool bBestAlreadyHasPoint = true;

	CBaseEntity *pEntity = NULL;

	switch (m_SearchType)
	{
	case ST_ENTNAME:
	{
		pEntity = gEntList.FindEntityByName(pEntity, m_iszActor);
		break;
	}

	case ST_CLASSNAME:
	{
		pEntity = gEntList.FindEntityByClassname(pEntity, STRING(m_iszActor));
		break;
	}
	}

	CAI_BaseNPC *pNPC = NULL;

#ifdef HLSS_DEBUG_RECHARGE_BEHAVIOR
	DevMsg("hlss_recharge_point %s looking for NPC\n", GetDebugName());
#endif

	while (pEntity)
	{
		pNPC = pEntity->MyNPCPointer();

		if (pNPC)
		{
			CAI_RechargeBehavior *pBehavior;

			if (pNPC->IsAlive() && pNPC->GetBehavior(&pBehavior))
			{
				if (pBehavior->CanRecharge() && (!pBehavior->HasRechargePoint() || !pBehavior->HasReachedRechargePoint()))
				{
					float flDist = (GetAbsOrigin() - pNPC->GetAbsOrigin()).Length();

#ifdef HLSS_DEBUG_RECHARGE_BEHAVIOR
					DevMsg("hlss_recharge_point: distance of %s is %f\n", pNPC->GetDebugName(), flDist);
#endif

					float flDistCurrent = flDist;

					if (bBestAlreadyHasPoint && pBehavior->HasRechargePoint() &&
						(pBehavior->m_flNextTimeChange < gpGlobals->curtime))
					{
						flDistCurrent = (pNPC->GetAbsOrigin() - pBehavior->m_hRechargePoint->GetAbsOrigin()).Length();

						//DevMsg("Old point has distance %f, new one %f\n", flDistCurrent, flDist);
					}

					//TERO: get the NPC closest to us
					if ((flDist < flBestDist && !pBehavior->HasRechargePoint()) || (flDistCurrent > flDist))
					{
						bBestAlreadyHasPoint = pBehavior->HasRechargePoint();

						pBest = pNPC;
						flBestDist = flDist;
					}
				}
			}
		}

		switch (m_SearchType)
		{
		case ST_ENTNAME:
		{
			pEntity = gEntList.FindEntityByName(pEntity, m_iszActor);
			break;
		}

		case ST_CLASSNAME:
		{
			pEntity = gEntList.FindEntityByClassname(pEntity, STRING(m_iszActor));
			break;
		}
		}
	}

	if (pBest)
	{
		CAI_RechargeBehavior *pBehavior;

		//not getting the behavior altough we already got it once shouldn't be possible
		if (!pBest->GetBehavior(&pBehavior))
			return false;

		//TERO: unlock the old one if we picked a new one over it
		if (pBehavior->HasRechargePoint())
		{
			pBehavior->m_flNextTimeChange = gpGlobals->curtime + 1.0f;
			pBehavior->UnlockRechargePoint();
		}
		else
		{
			pBehavior->m_flNextTimeChange = 0;
		}

		if (pBest->IsCurSchedule(pBehavior->SCHED_MOVE_TO_RECHARGE_POINT, false))
		{
			pBest->ClearSchedule("New recharge target!");
		}

		Lock(pBest);

		pBehavior->SetRechargePoint(this);

#ifdef HLSS_DEBUG_RECHARGE_BEHAVIOR
		DevMsg("hlss_recharge_point: %s selected npc %s\n", GetDebugName(), pBest->GetDebugName());

		NDebugOverlay::Box(pBest->GetAbsOrigin(), pBest->GetHullMins(), pBest->GetHullMaxs(), 0, 255, 0, 0, 5);
#endif

		return true;
	}

	return false;
}

void CRechargePoint::InputDeactivate(inputdata_t &inputdata)
{
	RemoveSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE);

	CAI_RechargeBehavior *pBehavior;

	//not getting the behavior altough we already got it once shouldn't be possible
	if (m_hLockedBy && m_hLockedBy->GetBehavior(&pBehavior))
	{
		pBehavior->m_hRechargePoint = NULL;
	}

	m_hLockedBy.Set(NULL);

	SetThink(NULL);
}

//******************************************************************************************************************************************
//
// THE ACTUAL BEHAVIOR
//
//******************************************************************************************************************************************

BEGIN_DATADESC(CAI_RechargeBehavior)
DEFINE_FIELD(m_hRechargePoint, FIELD_EHANDLE),
DEFINE_FIELD(m_flTimeDeferScheduleSelection, FIELD_TIME),
DEFINE_FIELD(m_flNextTimeChange, FIELD_TIME),
DEFINE_FIELD(m_bCanRecharge, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHasReachedRechargePoint, FIELD_BOOLEAN),
END_DATADESC();

CAI_RechargeBehavior::CAI_RechargeBehavior()
{
	m_bCanRecharge = false;
	m_hRechargePoint = NULL;
	m_bHasReachedRechargePoint = false;
	m_flNextTimeChange = 0;
	m_flTimeDeferScheduleSelection = 0;
}

void CAI_RechargeBehavior::SetRechargePoint(CRechargePoint *pRechargePoint)
{
	m_hRechargePoint = pRechargePoint;
	m_bHasReachedRechargePoint = false;

	SetCondition(COND_RECHARGE_CHANGED_POINT);
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CAI_RechargeBehavior::OnRestore()
{
	if (!m_hRechargePoint)
	{
		NotifyChangeBehaviorStatus();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CAI_RechargeBehavior::CanSelectSchedule()
{
	if (!GetOuter()->IsInterruptable())
		return false;

	if (GetOuter()->HasCondition(COND_RECEIVED_ORDERS))
		return false;

	// We're letting other AI run for a little while because the recharge AI failed recently.
	if (m_flTimeDeferScheduleSelection > gpGlobals->curtime)
		return false;

	// No schedule selection if no recharge is being conducted.
	if (!HasRechargePoint())
		return false;

#ifdef HLSS_DEBUG_RECHARGE_BEHAVIOR
	DevMsg("Can select recharge schedule\n");
#endif

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : scheduleType - 
// Output : int
//-----------------------------------------------------------------------------
int CAI_RechargeBehavior::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_ESTABLISH_LINE_OF_FIRE_FALLBACK:
		// This nasty schedule can allow the NPC to violate their position near
		// the recharge point. Translate it away to something stationary. (sjb)
		return SCHED_COMBAT_FACE;
		break;

	case SCHED_RANGE_ATTACK1:
		if (GetOuter()->GetShotRegulator()->IsInRestInterval())
		{
			if (GetOuter()->HasStrategySlotRange(SQUAD_SLOT_ATTACK1, SQUAD_SLOT_ATTACK2))
				GetOuter()->VacateStrategySlot();
			return SCHED_COMBAT_FACE; // @TODO (toml 07-02-03): Should do something more tactically sensible
		}
		break;

	case SCHED_MOVE_TO_WEAPON_RANGE:
	case SCHED_CHASE_ENEMY:
	{
		ClearCondition(COND_RECHARGE_CHANGED_POINT);
		return SCHED_MOVE_TO_RECHARGE_POINT;
	}
	break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_RechargeBehavior::OnStartSchedule(int scheduleType)
{
	if (scheduleType == SCHED_HIDE_AND_RELOAD) //!!!HACKHACK
	{
		// Dirty the recharge point flag so that we return to recharge point
		m_bHasReachedRechargePoint = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CAI_RechargeBehavior::ClearSchedule(const char *szReason)
{
	// Don't allow it if we're in a vehicle
	if (GetOuter()->IsInAVehicle())
		return;

	GetOuter()->ClearSchedule(szReason);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CAI_RechargeBehavior::SelectSchedule()
{
	Vector vecDiff = GetOuter()->GetAbsOrigin() - m_hRechargePoint->GetAbsOrigin();
	vecDiff.z = 0.0;

	m_bHasReachedRechargePoint = (vecDiff.LengthSqr() < Square(HLSS_RECHARGE_POINT_TOLERANCE));

	if (!HasReachedRechargePoint())
	{
		if (HasCondition(COND_PLAYER_PUSHING))
			return SCHED_MOVE_AWAY;

		if (HasCondition(COND_HEAR_DANGER))
			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
	}

	if (HasCondition(COND_CAN_MELEE_ATTACK1))
		return SCHED_MELEE_ATTACK1;

	// If you're empty, reload before trying to carry out any recharge functions.
	if (HasCondition(COND_NO_PRIMARY_AMMO))
		return SCHED_RELOAD;


	/*Vector vecDiff = GetOuter()->GetAbsOrigin() - m_hRechargePoint->GetAbsOrigin();
	vecDiff.z = 0.0;

	if( vecDiff.LengthSqr() > Square(HLSS_RECHARGE_POINT_TOLERANCE) )
	{
	// Someone moved me away. Get back to rally point.
	m_bHasReachedRechargePoint = false;
	return SCHED_MOVE_TO_RECHARGE_POINT;
	}

	return BaseClass::SelectSchedule();*/

	ClearCondition(COND_RECHARGE_CHANGED_POINT);
	m_bHasReachedRechargePoint = false;
	return SCHED_MOVE_TO_RECHARGE_POINT;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CAI_RechargeBehavior::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
		BaseClass::StartTask(pTask);
		break;

	case TASK_RECHARGE_DEFER_SCHEDULE_SELECTION:
	{
		if (m_hRechargePoint)
		{
			m_flTimeDeferScheduleSelection = gpGlobals->curtime + pTask->flTaskData;
		}
		TaskComplete();
	}
	break;

	case TASK_GET_PATH_TO_RECHARGE_POINT:
	{
		if (m_hRechargePoint)
		{
			AI_NavGoal_t goal(m_hRechargePoint->GetAbsOrigin());
			goal.pTarget = m_hRechargePoint;
			if (GetNavigator()->SetGoal(goal) == false)
			{
				// Try and get as close as possible otherwise
				AI_NavGoal_t nearGoal(GOALTYPE_LOCATION, m_hRechargePoint->GetAbsOrigin(), AIN_DEF_ACTIVITY, 0); //_NEAREST_NODE
				if (GetNavigator()->SetGoal(nearGoal, AIN_CLEAR_PREVIOUS_STATE))
				{
					//FIXME: HACK! The internal pathfinding is setting this without our consent, so override it!
					ClearCondition(COND_TASK_FAILED);
					GetNavigator()->SetArrivalDirection(m_hRechargePoint->GetAbsAngles());
					TaskComplete();
					return;
				}
			}
			GetNavigator()->SetArrivalDirection(m_hRechargePoint->GetAbsAngles());
		}
		else
		{
			TaskFail("No recharge point");
		}
	}
	break;

	case TASK_FACE_RECHARGE_POINT:
	{
		if (m_hRechargePoint)
		{
			GetMotor()->SetIdealYaw(m_hRechargePoint->GetAbsAngles().y);
			GetOuter()->SetTurnActivity();

			UpdateRechargePointDistance();

			if (!HasReachedRechargePoint())
			{
				TaskFail("No recharge point");
				break;
			}
		}
		else
		{
			TaskFail("No recharge point");
		}
	}
	break;

	case TASK_RECHARGE:
	{
		if (m_hRechargePoint)
		{
			int sequence = GetOuter()->LookupSequence(STRING(m_hRechargePoint->m_RechargeSequenceName));
			if (sequence != -1)
			{
				GetOuter()->ResetSequence(sequence);
				GetOuter()->SetIdealActivity(ACT_DO_NOT_DISTURB);
			}
			else
			{
				Recharge();
				TaskComplete();
			}

			UpdateRechargePointDistance();

			if (!HasReachedRechargePoint())
			{
				TaskFail("No recharge point");
				break;
			}
		}
		else
		{
			TaskFail("No recharge point");
		}
	}

	break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pTask - 
//-----------------------------------------------------------------------------
void CAI_RechargeBehavior::RunTask(const Task_t *pTask)
{
	/*if (m_hRechargePoint)
	{
	DevMsg("RunTask with %s\n", m_hRechargePoint->GetDebugName());
	}*/


	switch (pTask->iTask)
	{
	case TASK_FACE_RECHARGE_POINT:
	{
		if (m_hRechargePoint)
		{
			UpdateRechargePointDistance();

			if (!HasReachedRechargePoint())
			{
				TaskFail("No recharge point");
				break;
			}

			GetMotor()->UpdateYaw();

			if (GetOuter()->FacingIdeal())
			{
				TaskComplete();
			}

		}
		else
		{
			TaskFail("No recharge point");
		}
	}
	break;

	case TASK_RECHARGE:

		if (!m_hRechargePoint)
		{
			TaskFail("No recharge point");
		}
		else if (GetOuter()->IsSequenceFinished())
		{
			UpdateRechargePointDistance();

			if (!HasReachedRechargePoint())
			{
				TaskFail("No recharge point");
				break;
			}

			Recharge();
			TaskComplete();
		}

		break;

	case TASK_WAIT_FOR_MOVEMENT:
	{
		if (IsCurSchedule(SCHED_MOVE_TO_RECHARGE_POINT))
		{
			if (m_hRechargePoint)
			{
				UpdateRechargePointDistance();
			}
			else
			{
				TaskFail("No recharge point");
				break;
			}
		}

		BaseClass::RunTask(pTask);
	}
	break;

	default:
		BaseClass::RunTask(pTask);
		break;
	}
}

void CAI_RechargeBehavior::UpdateRechargePointDistance(void)
{
	Vector vecDiff = GetOuter()->GetAbsOrigin() - m_hRechargePoint->GetAbsOrigin();
	vecDiff.z = 0.0;

	if (vecDiff.LengthSqr() < Square(HLSS_RECHARGE_POINT_TOLERANCE))
	{
		// Someone moved me away. Get back to rally point.
		m_bHasReachedRechargePoint = true;
	}
	else
	{
		m_bHasReachedRechargePoint = false;
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CAI_RechargeBehavior::IsValidShootPosition(const Vector &vLocation, CAI_Node *pNode, CAI_Hint const *pHint)
{
	if (HasReachedRechargePoint())
		return false;

	return BaseClass::IsValidShootPosition(vLocation, pNode, pHint);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CAI_RechargeBehavior::GetMaxTacticalLateralMovement(void)
{
	if (HasReachedRechargePoint())
		return 0;

	return HLSS_RECHARGE_POINT_TOLERANCE - 0.1;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CAI_RechargeBehavior::UpdateOnRemove()
{
	// Ignore exclusivity. Our NPC just died.
	UnlockRechargePoint();
}

void CAI_RechargeBehavior::Recharge()
{
	m_hRechargePoint->m_OnRecharge.FireOutput(GetOuter(), GetOuter());
	m_hRechargePoint->RemoveSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_START_ACTIVE);

	if (!m_hRechargePoint->HasSpawnFlags(SF_HLSS_BEHAVIOR_RECHARGE_DONT_SPEAK))
	{
		GetOuter()->SpeakSentence(RECHARGE_SENTENCE_STARTING_RECHARGE);
	}

	m_flNextTimeChange = 0;

	UnlockRechargePoint();
}

void CAI_RechargeBehavior::UnlockRechargePoint()
{
	if (m_hRechargePoint.Get() != NULL)
	{
		m_hRechargePoint->Unlock(GetOuter());
	}

	m_hRechargePoint = NULL;
}

AI_BEGIN_CUSTOM_SCHEDULE_PROVIDER(CAI_RechargeBehavior)

DECLARE_TASK(TASK_GET_PATH_TO_RECHARGE_POINT)
DECLARE_TASK(TASK_FACE_RECHARGE_POINT)
DECLARE_TASK(TASK_RECHARGE)
DECLARE_TASK(TASK_RECHARGE_DEFER_SCHEDULE_SELECTION)

DECLARE_CONDITION(COND_RECHARGE_CHANGED_POINT)

//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_MOVE_TO_RECHARGE_POINT,

"	Tasks"
"		TASK_SET_FAIL_SCHEDULE					SCHEDULE:SCHED_RECHARGE_FAILED_TO_MOVE"
"		TASK_GET_PATH_TO_RECHARGE_POINT			0"
"		TASK_RUN_PATH							0"
"		TASK_WAIT_FOR_MOVEMENT					0"
"		TASK_STOP_MOVING						0"
"		TASK_FACE_RECHARGE_POINT				0"
"		TASK_RECHARGE							0"
"	"
"	Interrupts"
"		COND_RECHARGE_CHANGED_POINT"
//		"		COND_PROVOKED"
//		"		COND_NO_PRIMARY_AMMO"
//		"		COND_PLAYER_PUSHING"
)

//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
SCHED_RECHARGE_FAILED_TO_MOVE,

"	Tasks"
"		TASK_RECHARGE_DEFER_SCHEDULE_SELECTION	1"
"	"
"	Interrupts"
)

AI_END_CUSTOM_SCHEDULE_PROVIDER()