//========= Copyright, Dorian Gorski, All rights reserved. ============//
//
// Purpose: Manage Striders in finale combat, one stays near player the other patrols
// at nearby defined patrol location.
//
//=================================================================================//

#ifndef AI_STRIDERFINALE_H
#define AI_STRIDERFINALE_H
#if defined( _WIN32 )
#pragma once
#endif

#include "utlvector.h"
#include "ai_utils.h"
#include "npc_strider.h"

//-----------------------------------------------------------------------------
//
// CAI_StriderFinale
//
//-----------------------------------------------------------------------------

const float AISF_THINK_INTERVAL = 4.0;
const float AISF_UPDATE_DIST = 512.0;

class CAI_StriderFinale : public CBaseEntity
{
	DECLARE_CLASS( CAI_StriderFinale, CBaseEntity );

public:
	bool		m_bActive;
	string_t		m_iszStrider1;
	string_t		m_iszStrider2;
	string_t		m_iszStrider3;

	void Spawn();
	virtual void InputActivate( inputdata_t &inputdata );

	virtual void InputDeactivate( inputdata_t &inputdata )
	{
		if ( m_bActive )
		{
			m_bActive = false;
			SetThink(NULL);
		}
	}

	void InputSetPatrolPath( inputdata_t &inputdata )
	{
		m_pPatrolPath = gEntList.FindEntityByName( NULL, inputdata.value.String(), NULL, inputdata.pActivator, inputdata.pCaller );

		StriderPatrol();
	}

	void InputSetGuardPath( inputdata_t &inputdata )
	{
		m_pGuardPath = gEntList.FindEntityByName( NULL, inputdata.value.String(), NULL, inputdata.pActivator, inputdata.pCaller );

		StriderGuard();
	}

	void UpdateOnRemove()
	{
		if ( m_bActive )
		{
			m_bActive = false;
		}

		BaseClass::UpdateOnRemove();
	}

	void MovementThink();
	void FillRoles();

private:
	CNPC_Strider	*m_npcStrider1;
	CNPC_Strider	*m_npcStrider2;
	CNPC_Strider	*m_npcStrider3;
	CBaseEntity	*m_pPatrolPath;
	CBaseEntity	*m_pGuardPath;
	Vector		m_vLastPos;

	void StriderFollowPlayer();
	void StriderPatrol();
	void StriderGuard();

	DECLARE_DATADESC();
};

#endif // AI_STRIDERFINALE_H