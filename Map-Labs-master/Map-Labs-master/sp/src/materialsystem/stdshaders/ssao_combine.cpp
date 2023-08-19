//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"

#include "screenspace_simple_vs30.inc"
#include "ssao_combine_ps30.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_VS_SHADER_FLAGS( SSAO_Combine, "Help for SSAO Combine", SHADER_NOT_EDITABLE )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( FRAMEBUFFER, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )
		SHADER_PARAM( SSAOBUFFER, SHADER_PARAM_TYPE_TEXTURE, "_rt_SSAOFB1", "" )
		SHADER_PARAM( LUMINOSITYTHRESHOLD, SHADER_PARAM_TYPE_FLOAT, "0.3", "" )
	END_SHADER_PARAMS

	SHADER_INIT
	{
		if( params[FRAMEBUFFER]->IsDefined() )
		{
			LoadTexture( FRAMEBUFFER );
		}
		if( params[SSAOBUFFER]->IsDefined() )
		{
			LoadTexture( SSAOBUFFER );
		}
	}
	
	SHADER_FALLBACK
	{
		// Requires DX9 + above
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 || !g_pHardwareConfig->SupportsShaderModel_3_0() )
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
			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );

			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER1, true );

			DECLARE_STATIC_VERTEX_SHADER(screenspace_simple_vs30);
			SET_STATIC_VERTEX_SHADER(screenspace_simple_vs30);

			DECLARE_STATIC_PIXEL_SHADER( ssao_combine_ps30 );
			SET_STATIC_PIXEL_SHADER( ssao_combine_ps30 );
		}

		DYNAMIC_STATE
		{
			float g_const0[4] = { params[LUMINOSITYTHRESHOLD]->GetFloatValue(), 0.0f, 0.0f, 0.0f };
			pShaderAPI->SetPixelShaderConstant( 0, g_const0 );

			BindTexture( SHADER_SAMPLER0, FRAMEBUFFER );
			BindTexture( SHADER_SAMPLER1, SSAOBUFFER );

			DECLARE_DYNAMIC_VERTEX_SHADER(screenspace_simple_vs30);
			SET_DYNAMIC_VERTEX_SHADER(screenspace_simple_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER( ssao_combine_ps30 );
			SET_DYNAMIC_PIXEL_SHADER( ssao_combine_ps30 );
		}
		Draw();
	}
END_SHADER