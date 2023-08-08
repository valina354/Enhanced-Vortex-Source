//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"

#include "SDK_screenspaceeffect_vs20.inc"
#include "vignetting_ps20.inc"
#include "vignetting_ps20b.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER( Vignetting, "Help for Vignetting" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )
		SHADER_PARAM( VIGNETDARKNESS, SHADER_PARAM_TYPE_FLOAT, "1.25", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if( params[FBTEXTURE]->IsDefined() )
		{
			LoadTexture( FBTEXTURE );
		}
	}

	// ----------------------------------------------------------------------------
	// We want this shader to operate on the frame buffer itself. Therefore,
	// we need to set this to true.
	// ----------------------------------------------------------------------------
	bool NeedsFullFrameBufferTexture(IMaterialVar **params, bool bCheckSpecificToThisFrame /* = true */) const
	{
		return true;
	}

	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			Assert( 0 );
			return "Wireframe";
		}
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			// Pre-cache shaders
			DECLARE_STATIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 );
			SET_STATIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( vignetting_ps20b );
				SET_STATIC_PIXEL_SHADER( vignetting_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( vignetting_ps20 );
				SET_STATIC_PIXEL_SHADER( vignetting_ps20 );
			}
		}

		DYNAMIC_STATE
		{
			float fVignetDarkness[4];
			fVignetDarkness[0] = params[VIGNETDARKNESS]->GetFloatValue();
			fVignetDarkness[1] = fVignetDarkness[2] = fVignetDarkness[3] = fVignetDarkness[0];
			pShaderAPI->SetPixelShaderConstant( 0, fVignetDarkness );

			BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );
			DECLARE_DYNAMIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( sdk_screenspaceeffect_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( vignetting_ps20b );
				SET_DYNAMIC_PIXEL_SHADER( vignetting_ps20b );
			}
			else
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( vignetting_ps20 );
				SET_DYNAMIC_PIXEL_SHADER( vignetting_ps20 );
			}
		}
		Draw();
	}
END_SHADER