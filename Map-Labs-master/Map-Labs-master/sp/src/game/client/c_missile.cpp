//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"

#include "particles_simple.h"
#include "particles_attractor.h"

#include "dlight.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar missile_dlight("hlss_missile_dlight", "1");

class C_Missile : public C_BaseCombatCharacter
{
	DECLARE_CLASS(C_Missile, C_BaseCombatCharacter);
	DECLARE_CLIENTCLASS();

public:

	C_Missile();

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	OnRestore(void);
	virtual void	UpdateOnRemove(void);

	virtual void	ClientThink(void);

	void CreateRocketTrail();

	bool m_bRocketTrail;

private:

	CNewParticleEffect				*m_hRocketTrail;
	//dlight_t						*m_pDLight;
	dlight_t						*m_pELight;
};

IMPLEMENT_CLIENTCLASS_DT(C_Missile, DT_Missile, CMissile)
RecvPropBool(RECVINFO(m_bRocketTrail)),
END_RECV_TABLE()

C_Missile::C_Missile()
{
	m_bRocketTrail = false;
	m_hRocketTrail = NULL;
	//m_pDLight = NULL;
	m_pELight = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_Missile::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);

	//UpdateParticleEffects();

	CreateRocketTrail();

	if (missile_dlight.GetBool())
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_Missile::OnRestore(void)
{
	BaseClass::OnRestore();

	CreateRocketTrail();

	if (missile_dlight.GetBool())
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

void C_Missile::ClientThink()
{
	BaseClass::ClientThink();

	if (missile_dlight.GetBool())
	{
		Vector effect_origin;
		GetAttachment("0", effect_origin);

		/*if (!m_pDLight)
		{
		m_pDLight = effects->CL_AllocDlight( entindex() );
		m_pDLight->die		= FLT_MAX;
		}

		if (m_pDLight)
		{
		m_pDLight->origin	= GetAbsOrigin();
		m_pDLight->radius	= random->RandomFloat( 60.0f, 80.0f );
		m_pDLight->minlight = 10.0f;
		m_pDLight->color.r = 64;
		m_pDLight->color.g = 64;
		m_pDLight->color.b = 64;
		}*/

		if (!m_pELight)
		{
			m_pELight = effects->CL_AllocDlight(entindex());
			m_pELight->die = FLT_MAX;
		}

		if (m_pELight)
		{
			m_pELight->origin = GetAbsOrigin();
			m_pELight->radius = random->RandomFloat(80.0f, 100.0f);
			m_pELight->minlight = (10.0f / 255.0f);
			m_pELight->color.r = 128;
			m_pELight->color.g = 128;
			m_pELight->color.b = 128;
		}
	}
	else
	{
		/*if (m_pDLight)
		{
		m_pDLight->die = gpGlobals->curtime;
		}*/
		if (m_pELight)
		{
			m_pELight->die = gpGlobals->curtime;
		}
	}
}

void C_Missile::CreateRocketTrail(void)
{
	if (m_bRocketTrail)
	{
		if (!m_hRocketTrail)
		{

			// Place a beam between the two points //m_pEnt->
			CNewParticleEffect *pRocketTrail = ParticleProp()->Create("he_missile", PATTACH_POINT_FOLLOW, 1); //"0"
			if (pRocketTrail)
			{
				m_hRocketTrail = pRocketTrail;
			}
		}
	}
}

void C_Missile::UpdateOnRemove(void)
{
	if (m_hRocketTrail)
	{
		m_hRocketTrail->StopEmission();
		m_hRocketTrail = NULL;
	}

	/*if (m_pDLight)
	{
	m_pDLight->die = gpGlobals->curtime;
	}*/

	if (m_pELight)
	{
		m_pELight->die = gpGlobals->curtime;
	}

	BaseClass::UpdateOnRemove();
}