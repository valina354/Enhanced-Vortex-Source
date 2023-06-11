//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	Alien Controllers from HL1 now in updated form
//
//=============================================================================//

#include "cbase.h"
#include "basehlcombatweapon.h"

#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "in_buttons.h"
#include "globalstate.h"

#include "engine/IEngineSound.h"
#include "items.h"
#include "soundent.h"

#include "npcevent.h"
#include "IEffects.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Singleton interfaces
//-----------------------------------------------------------------------------
extern IMaterialSystemHardwareConfig *g_pMaterialSystemHardwareConfig;

class CHLSS_CameraOutput : public CPointEntity
{
	DECLARE_CLASS(CHLSS_CameraOutput, CPointEntity);

public:
	CHLSS_CameraOutput *m_pNext;

	CHLSS_CameraOutput();
	~CHLSS_CameraOutput();

	void InputEnable(inputdata_t &data);
	void InputDisable(inputdata_t &data);
	void InputHideCamera(inputdata_t &data);

	void CheckTakePhoto(CBaseEntity *pTarget);

	COutputEvent		m_OnPhotoTaken;
	bool				m_bEnabled;
	string_t			m_iszTarget;
	int					m_iTargetType;

	static CHLSS_CameraOutput *GetCameraOutputList();

	DECLARE_DATADESC();
};


class CWeapon_Camera : public CBaseHLCombatWeapon
{
public:
	DECLARE_CLASS(CWeapon_Camera, CBaseHLCombatWeapon);
	DECLARE_SERVERCLASS();

	CWeapon_Camera()
	{
		m_flDrawScreenTime = 0;
		m_bCameraLowered = false;
		m_flNextPhoto = 0;
	}

	void			Precache(void);

	void			ItemPostFrame(void);
	void			PrimaryAttack(void);
	void			Drop(const Vector &vecVelocity);

	void			Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);

	//bool			Deploy(void);

	void			WeaponIdle(void);

	//void			WeaponSwitch( void );

	CNetworkVar(float, m_flDrawScreenTime);
	float m_flNextPhoto;
	bool m_bCameraLowered;

	DECLARE_ACTTABLE();
	DECLARE_DATADESC();
};

acttable_t	CWeapon_Camera::m_acttable[] =
{
	{ ACT_RANGE_ATTACK1, ACT_RANGE_ATTACK_SLAM, true },
};

IMPLEMENT_ACTTABLE(CWeapon_Camera);

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CWeapon_Camera)
DEFINE_FIELD(m_flNextPhoto, FIELD_TIME),
DEFINE_FIELD(m_flDrawScreenTime, FIELD_TIME),
DEFINE_FIELD(m_bCameraLowered, FIELD_BOOLEAN),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CWeapon_Camera, DT_Weapon_Camera)
SendPropFloat(SENDINFO(m_flDrawScreenTime), SPROP_CHANGES_OFTEN),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_camera, CWeapon_Camera);
PRECACHE_WEAPON_REGISTER(weapon_camera);

void CWeapon_Camera::Precache(void)
{
	PrecacheScriptSound("NPC_CScanner.TakePhoto");

	BaseClass::Precache();
}

void CWeapon_Camera::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	switch (pEvent->event)
	{
	case EVENT_WEAPON_THROW:
		if (pOwner)
		{
			Vector vecForward;
			pOwner->GetVectors(&vecForward, NULL, NULL);
			Vector vecOrigin = pOwner->EyePosition();

			trace_t	tr;
			UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

			if (tr.m_pEnt)
			{

				for (CHLSS_CameraOutput *pCamera = CHLSS_CameraOutput::GetCameraOutputList(); pCamera != NULL; pCamera = pCamera->m_pNext)
				{
					if (pCamera == NULL)
						continue;

					pCamera->CheckTakePhoto(tr.m_pEnt);
				}
			}

			m_flDrawScreenTime = gpGlobals->curtime + 1.0f;

			color32 white = { 255, 255, 255, 195 };

			if ((g_pMaterialSystemHardwareConfig != NULL) && (g_pMaterialSystemHardwareConfig->GetHDRType() != HDR_TYPE_NONE))
			{
				white.a = (byte)((float)white.a * 0.9f);
			}

			float flFadeTime = 0.3f;
			UTIL_ScreenFade(pOwner, white, flFadeTime, 0.3, FFADE_IN);

			EmitSound("NPC_CScanner.TakePhoto");
		}

		break;
	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWeapon_Camera::ItemPostFrame(void)
{
	BaseClass::ItemPostFrame();

	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner)
	{
		if (!m_bCameraLowered && pOwner->m_nButtons & IN_ATTACK)
		{
			PrimaryAttack();

			return;
		}
	}

	WeaponIdle();
}

void CWeapon_Camera::WeaponIdle()
{
	if (m_bCameraLowered)
	{
		if (GetActivity() != ACT_VM_IDLE_LOWERED)
		{
			SendWeaponAnim(ACT_VM_IDLE_LOWERED);
		}
		else if (HasWeaponIdleTimeElapsed())
		{
			// Keep idling low
			SendWeaponAnim(ACT_VM_IDLE_LOWERED);
		}
	}
	else
	{
		if (HasWeaponIdleTimeElapsed())
		{
			// Keep idling low
			SendWeaponAnim(ACT_VM_IDLE);
		}
	}
}

void CWeapon_Camera::Drop(const Vector &vecVelocity)
{
	BaseClass::Drop(vecVelocity);

	//UTIL_Remove( this );
}

void CWeapon_Camera::PrimaryAttack()
{
	if (m_flNextPhoto > gpGlobals->curtime)
		return;


	m_flNextPhoto = gpGlobals->curtime + 1.0f;

	m_flDrawScreenTime = 0;
	SendWeaponAnim(ACT_VM_THROW);
}



// global pointer to Larson for fast lookups
CEntityClassList<CHLSS_CameraOutput> g_CameraOutputList;
template <> CHLSS_CameraOutput *CEntityClassList<CHLSS_CameraOutput>::m_pClassList = NULL;

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CHLSS_CameraOutput)
DEFINE_KEYFIELD(m_bEnabled, FIELD_BOOLEAN, "enabled"),
DEFINE_KEYFIELD(m_iszTarget, FIELD_STRING, "photo_target"),
DEFINE_KEYFIELD(m_iTargetType, FIELD_INTEGER, "photo_target_type"),

DEFINE_INPUTFUNC(FIELD_VOID, "Enable", InputEnable),
DEFINE_INPUTFUNC(FIELD_VOID, "Disable", InputDisable),
DEFINE_INPUTFUNC(FIELD_VOID, "HideCamera", InputHideCamera),
DEFINE_OUTPUT(m_OnPhotoTaken, "OnPhotoTaken"),
END_DATADESC()


LINK_ENTITY_TO_CLASS(hlss_camera_output, CHLSS_CameraOutput);

//=========================================================
// Returns a pointer to Eloise's entity
//=========================================================
CHLSS_CameraOutput *CHLSS_CameraOutput::GetCameraOutputList(void)
{
	return g_CameraOutputList.m_pClassList;
}

CHLSS_CameraOutput::CHLSS_CameraOutput(void)
{
	m_bEnabled = false;
	g_CameraOutputList.Insert(this);
}

CHLSS_CameraOutput::~CHLSS_CameraOutput()
{
	g_CameraOutputList.Remove(this);
}

void CHLSS_CameraOutput::InputEnable(inputdata_t &data)
{
	m_bEnabled = true;
}

void CHLSS_CameraOutput::InputDisable(inputdata_t &data)
{
	m_bEnabled = false;
}

void CHLSS_CameraOutput::InputHideCamera(inputdata_t &data)
{
	CBasePlayer *pPlayer = UTIL_PlayerByIndex(1);

	if (!pPlayer)
		return;

	//if we have the slot occupied dont pick up the weapon
	for (int i = 0; i < pPlayer->WeaponCount(); i++)
	{
		CBaseCombatWeapon *pSearch = pPlayer->GetWeapon(i);

		if (!pSearch)
			continue;

		CWeapon_Camera *pCamera = dynamic_cast<CWeapon_Camera*>((CBaseCombatWeapon*)pSearch);

		if (pCamera)
		{
			pCamera->m_bCameraLowered = true;
		}
	}
}

void CHLSS_CameraOutput::CheckTakePhoto(CBaseEntity *pEntity)
{
	if (!m_bEnabled)
		return;


	if (m_iTargetType == 1)
	{
		if (FClassnameIs(pEntity, STRING(m_iszTarget)))
		{
			m_OnPhotoTaken.FireOutput(this, this);
		}
	}
	else
	{
		string_t iszTargetEnt = MAKE_STRING(pEntity->GetDebugName());

		if (IDENT_STRINGS(iszTargetEnt, m_iszTarget))
		{
			m_OnPhotoTaken.FireOutput(this, this);
		}
	}
}