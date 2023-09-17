//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#include "cbase.h"
#include "c_prop_gravity_ball.h"
#include "materialsystem/imaterial.h"
#include "model_types.h"
#include "c_physicsprop.h"
#include "c_te_effect_dispatch.h"
#include "fx_quad.h"
#include "fx.h"
#include "clienteffectprecachesystem.h"
#include "view.h"
#include "view_scene.h"
#include "beamdraw.h"
#include "ragdollexplosionenumerator.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Precache our effects
CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectGravityBall)
CLIENTEFFECT_MATERIAL("effects/ar2_altfire2")
CLIENTEFFECT_MATERIAL("effects/ar2_altfire2b")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle1b_nocull")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle2b_nocull")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle2c")
CLIENTEFFECT_MATERIAL("effects/ar2_altfire2")
CLIENTEFFECT_MATERIAL("effects/ar2_altfire2b")
CLIENTEFFECT_REGISTER_END()

IMPLEMENT_CLIENTCLASS_DT(C_PropGravityBall, DT_PropGravityBall, CPropGravityBall)
END_RECV_TABLE();

//-----------------------------------------------------------------------------
// Purpose: Cache the material handles
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_PropGravityBall::InitMaterials( void )
{
	// Motion blur
	if ( m_pBlurMaterial == NULL )
	{
		m_pBlurMaterial = materials->FindMaterial( "effects/ar2_altfire2b", NULL, false );

		if ( m_pBlurMaterial == NULL )
			return false;
	}

	// Main body of the ball
	if ( m_pBodyMaterial == NULL )
	{
		m_pBodyMaterial = materials->FindMaterial( "effects/ar2_altfire2", NULL, false );

		if ( m_pBodyMaterial == NULL )
			return false;
	}

	// Flicker material
	if ( m_pFlickerMaterial == NULL )
	{
		m_pFlickerMaterial = materials->FindMaterial( "effects/combinemuzzle2c", NULL, false );

		if ( m_pFlickerMaterial == NULL )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  : &data -
//-----------------------------------------------------------------------------
void GravityBallImpactCallback( const CEffectData &data )
{
	// Quick flash
	FX_AddQuad( data.m_vOrigin,
				data.m_vNormal,
				data.m_flRadius * 10.0f,
				0,
				0.75f,
				1.0f,
				0.0f,
				0.4f,
				random->RandomInt( 0, 360 ),
				0,
				Vector( 1.0f, 1.0f, 1.0f ),
				0.25f,
				"effects/combinemuzzle1b_nocull",
				(FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA) );

	// Lingering burn
	FX_AddQuad( data.m_vOrigin,
				data.m_vNormal,
				data.m_flRadius * 2.0f,
				data.m_flRadius * 4.0f,
				0.75f,
				1.0f,
				0.0f,
				0.4f,
				random->RandomInt( 0, 360 ),
				0,
				Vector( 1.0f, 1.0f, 1.0f ),
				0.5f,
				"effects/combinemuzzle2b_nocull",
				(FXQUAD_BIAS_SCALE|FXQUAD_BIAS_ALPHA) );

	// Throw sparks
	// FX_ElectricSpark( data.m_vOrigin, 2, 1, &data.m_vNormal );
}

DECLARE_CLIENT_EFFECT( "gball_bounce", GravityBallImpactCallback );