
//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "particles_simple.h"
#include "citadel_effects_shared.h"
#include "particles_attractor.h"
#include "iefx.h"
#include "dlight.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_te_effect_dispatch.h"
#include "fx_quad.h"

#include "c_ai_basenpc.h"

// For material proxy
#include "ProxyEntity.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define NUM_INTERIOR_PARTICLES	8

#define DLIGHT_RADIUS (150.0f)
#define DLIGHT_MINLIGHT (40.0f/255.0f)

class C_XenPlant : public C_BaseCombatCharacter
{
	DECLARE_CLASS(C_XenPlant, C_BaseCombatCharacter);
	DECLARE_CLIENTCLASS();

public:
	C_XenPlant::C_XenPlant()
	{
		m_pDLight = NULL;
	}

	virtual void	UpdateOnRemove(void);
	virtual void	ReceiveMessage(int classID, bf_read &msg);

private:

	dlight_t						*m_pDLight;

};

IMPLEMENT_CLIENTCLASS_DT(C_XenPlant, DT_XenPlant, CXenPlant)
END_RECV_TABLE()


//-----------------------------------------------------------------------------
// Purpose: Receive messages from the server
//-----------------------------------------------------------------------------
void C_XenPlant::ReceiveMessage(int classID, bf_read &msg)
{
	// Is the message for a sub-class?
	if (classID != GetClientClass()->m_ClassID)
	{
		BaseClass::ReceiveMessage(classID, msg);
		return;
	}


	int messageType = msg.ReadByte();

	if (messageType == 1)
	{
		if (m_pDLight == NULL)
		{
			Vector effect_origin;
			QAngle effect_angles;

			GetAttachment("Bulb", effect_origin, effect_angles);

			m_pDLight = effects->CL_AllocDlight(entindex());
			m_pDLight->origin = effect_origin;

			m_pDLight->color.r = 228;
			m_pDLight->color.g = 232;
			m_pDLight->color.b = 113;

			m_pDLight->minlight = DLIGHT_MINLIGHT;
			m_pDLight->die = FLT_MAX;

			m_pDLight->radius = random->RandomFloat(245.0f, 256.0f);
		}
	}
	else
	{
		if (m_pDLight != NULL)
		{
			m_pDLight->die = gpGlobals->curtime;
			m_pDLight = NULL;
		}
	}
}



void C_XenPlant::UpdateOnRemove(void)
{
	if (m_pDLight != NULL)
	{
		m_pDLight->die = gpGlobals->curtime;
	}

	BaseClass::UpdateOnRemove();
}