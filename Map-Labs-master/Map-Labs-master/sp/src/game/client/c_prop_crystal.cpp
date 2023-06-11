//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "model_types.h"
#include "vcollide.h"
#include "vcollide_parse.h"
#include "solidsetdefaults.h"
#include "bone_setup.h"
#include "engine/ivmodelinfo.h"
#include "physics.h"
#include "view.h"
#include "clienteffectprecachesystem.h"
#include "c_physicsprop.h"
#include "tier0/vprof.h"
#include "ivrenderview.h"

#include "particles_simple.h"
#include "citadel_effects_shared.h"
#include "particles_attractor.h"
#include "dlight.h"
#include "iefx.h"

// For material proxy
#include "ProxyEntity.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"

#include "engine/IVDebugOverlay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define DLIGHT_RADIUS (150.0f)
#define DLIGHT_MINLIGHT (20.0f/255.0f)

class C_PropCrystal : public C_PhysicsProp
{
	typedef C_PhysicsProp BaseClass;
public:
	DECLARE_CLIENTCLASS();

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	UpdateOnRemove(void);
	virtual void	OnRestore(void);
	virtual void	ClientThink(void);
	void			CreateDLight(void);

	CNewParticleEffect				*m_hEffect;
	dlight_t						*m_pDLight;
};

IMPLEMENT_CLIENTCLASS_DT(C_PropCrystal, DT_PropCrystal, CPropCrystal)
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_PropCrystal::ClientThink(void)
{
	//
	// -- DLight 
	//

	CreateDLight();

	if (m_pDLight != NULL)
	{
		m_pDLight->origin = GetAbsOrigin();
		m_pDLight->radius = DLIGHT_RADIUS;
	}
}

void C_PropCrystal::CreateDLight(void)
{
	if (!m_pDLight)
	{
		m_pDLight = effects->CL_AllocDlight(index);
		m_pDLight->origin = GetAbsOrigin();
		m_pDLight->color.r = 255;
		m_pDLight->color.g = 128;
		m_pDLight->color.b = 0;
		m_pDLight->radius = 15;
		m_pDLight->minlight = DLIGHT_MINLIGHT;
		m_pDLight->die = FLT_MAX;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_PropCrystal::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (updateType == DATA_UPDATE_CREATED)
	{
		m_pDLight = NULL;
		CreateDLight();

		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}


void C_PropCrystal::UpdateOnRemove(void)
{
	/*if ( m_pDLight != NULL )
	{
	m_pDLight->die = gpGlobals->curtime;
	m_pDLight = NULL;
	}*/
	if (m_pDLight != NULL)
	{
		if (m_iHealth < 0)
		{
			m_pDLight->die = gpGlobals->curtime + 2.0f;
		}
		else
		{
			m_pDLight->die = gpGlobals->curtime;
		}

		m_pDLight = NULL;
	}

	BaseClass::UpdateOnRemove();
}

void C_PropCrystal::OnRestore(void)
{
	BaseClass::OnRestore();

	SetNextClientThink(CLIENT_THINK_ALWAYS);
}