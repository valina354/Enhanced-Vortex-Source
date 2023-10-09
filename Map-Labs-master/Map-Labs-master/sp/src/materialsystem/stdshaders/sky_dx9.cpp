//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#include "BaseVSShader.h"
#include "SDK_sky_vs20.inc"
#include "SDK_sky_ps20.inc"
#include "SDK_sky_ps20b.inc"

#include "convar.h"

BEGIN_VS_SHADER(SDK_Sky_DX9, "Help for SDK_Sky_DX9 shader")

	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )

#ifdef MAPBASE
		SHADER_PARAM(USE_TEXTURE_BLENDS, SHADER_PARAM_TYPE_BOOL, "0", "")
		SHADER_PARAM(BLEND1, SHADER_PARAM_TYPE_FLOAT, "1.0", "How much to blend $basetexture")

		SHADER_PARAM(BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "A second sky base texture")
		SHADER_PARAM(FRAME2, SHADER_PARAM_TYPE_INTEGER, "", "frame number for $basetexture2")
		SHADER_PARAM(BLEND2, SHADER_PARAM_TYPE_FLOAT, "0.0", "How much to blend $basetexture2")

		SHADER_PARAM(BASETEXTURE3, SHADER_PARAM_TYPE_TEXTURE, "", "A third sky base texture")
		SHADER_PARAM(FRAME3, SHADER_PARAM_TYPE_INTEGER, "", "frame number for $basetexture3")
		SHADER_PARAM(BLEND3, SHADER_PARAM_TYPE_FLOAT, "0.0", "How much to blend $basetexture3")

		SHADER_PARAM(BASETEXTURE4, SHADER_PARAM_TYPE_TEXTURE, "", "A fourth sky base texture")
		SHADER_PARAM(FRAME4, SHADER_PARAM_TYPE_INTEGER, "", "frame number for $basetexture4")
		SHADER_PARAM(BLEND4, SHADER_PARAM_TYPE_FLOAT, "0.0", "How much to blend $basetexture4")
#endif
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "sky_dx6";
		}
		return 0;
	}

		inline void CheckSkySGRB(IShaderShadow* pShaderShadow, ITexture *txtr, Sampler_t iSampler)
	{
		ImageFormat fmt = txtr->GetImageFormat();
		if ((fmt == IMAGE_FORMAT_RGBA16161616F) || (fmt == IMAGE_FORMAT_RGBA16161616))
			pShaderShadow->EnableSRGBRead(iSampler, false);
		else
			pShaderShadow->EnableSRGBRead(iSampler, true);
	}

	inline void BlendSkyRGBA(float &flMult, float flBlend, ITexture *txtr)
	{
		ImageFormat fmt = txtr->GetImageFormat();
		if ((fmt == IMAGE_FORMAT_RGBA16161616) || ((fmt == IMAGE_FORMAT_RGBA16161616F) && (g_pHardwareConfig->GetHDRType() == HDR_TYPE_INTEGER)))
		{
			flMult = Lerp(flBlend, flMult, 16.0f);
		}
		else
		{
			flMult = Lerp(1.0f - flBlend, 0.0f, flMult);
		}
	}

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_NOFOG );
		SET_FLAGS( MATERIAL_VAR_IGNOREZ );
#ifdef MAPBASE
		InitFloatParam(BLEND1, params, 1.0f);
#endif
	}
	SHADER_INIT
	{
		if (params[BASETEXTURE]->IsDefined())
		{
			ImageFormat fmt = params[BASETEXTURE]->GetTextureValue()->GetImageFormat();
			LoadTexture( BASETEXTURE, (fmt==IMAGE_FORMAT_RGBA16161616F) || (fmt==IMAGE_FORMAT_RGBA16161616) ? 0 : TEXTUREFLAGS_SRGB );
		}

#ifdef MAPBASE
		if (params[BASETEXTURE2]->IsDefined())
		{
			ImageFormat fmt = params[BASETEXTURE2]->GetTextureValue()->GetImageFormat();
			LoadTexture(BASETEXTURE2, (fmt == IMAGE_FORMAT_RGBA16161616F) || (fmt == IMAGE_FORMAT_RGBA16161616) ? 0 : TEXTUREFLAGS_SRGB);
		}
		if (params[BASETEXTURE3]->IsDefined())
		{
			ImageFormat fmt = params[BASETEXTURE3]->GetTextureValue()->GetImageFormat();
			LoadTexture(BASETEXTURE3, (fmt == IMAGE_FORMAT_RGBA16161616F) || (fmt == IMAGE_FORMAT_RGBA16161616) ? 0 : TEXTUREFLAGS_SRGB);
		}
		if (params[BASETEXTURE4]->IsDefined())
		{
			ImageFormat fmt = params[BASETEXTURE4]->GetTextureValue()->GetImageFormat();
			LoadTexture(BASETEXTURE4, (fmt == IMAGE_FORMAT_RGBA16161616F) || (fmt == IMAGE_FORMAT_RGBA16161616) ? 0 : TEXTUREFLAGS_SRGB);
		}
#endif
	}
	SHADER_DRAW
	{
		SHADOW_STATE
		{
			SetInitialShadowState();

//			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			CheckSkySGRB(pShaderShadow, params[BASETEXTURE]->GetTextureValue(), SHADER_SAMPLER0);

#ifdef MAPBASE
			bool bUsesBlends = params[USE_TEXTURE_BLENDS]->GetIntValue() > 0;
			bool bShouldBlend[4] = { true, false, false, false };
			if (bUsesBlends)
			{
				//bShouldBlend[0] = (params[BLEND1]->GetFloatValue() > 0.0f);

				if (params[BASETEXTURE2]->IsDefined())
				{
					//bShouldBlend[1] = (params[BLEND2]->GetFloatValue() > 0.0f);
					bShouldBlend[1] = true;
					pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
					CheckSkySGRB(pShaderShadow, params[BASETEXTURE2]->GetTextureValue(), SHADER_SAMPLER1);
				}
				if (params[BASETEXTURE3]->IsDefined())
				{
					//bShouldBlend[2] = (params[BLEND3]->GetFloatValue() > 0.0f);
					bShouldBlend[2] = true;
					pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
					CheckSkySGRB(pShaderShadow, params[BASETEXTURE3]->GetTextureValue(), SHADER_SAMPLER2);
				}
				if (params[BASETEXTURE4]->IsDefined())
				{
					//bShouldBlend[3] = (params[BLEND4]->GetFloatValue() > 0.0f);
					bShouldBlend[3] = true;
					pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
					CheckSkySGRB(pShaderShadow, params[BASETEXTURE4]->GetTextureValue(), SHADER_SAMPLER3);
				}
			}
#endif

			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, NULL, 0 );

			DECLARE_STATIC_VERTEX_SHADER(sdk_sky_vs20);
			SET_STATIC_VERTEX_SHADER(sdk_sky_vs20);

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER(sdk_sky_ps20b);
#ifdef MAPBASE
				SET_STATIC_PIXEL_SHADER_COMBO(USES_TEXTURE_BLENDS, bUsesBlends);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND1, bShouldBlend[0]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND2, bShouldBlend[1]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND3, bShouldBlend[2]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND4, bShouldBlend[3]);
#endif
				SET_STATIC_PIXEL_SHADER(sdk_sky_ps20b);
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER(sdk_sky_ps20);
#ifdef MAPBASE
				SET_STATIC_PIXEL_SHADER_COMBO(USES_TEXTURE_BLENDS, bUsesBlends);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND1, bShouldBlend[0]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND2, bShouldBlend[1]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND3, bShouldBlend[2]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND4, bShouldBlend[3]);
#endif
				SET_STATIC_PIXEL_SHADER(sdk_sky_ps20);
			}
			// we are writing linear values from this shader.
			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->EnableAlphaWrites( true );
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, BASETEXTURE, FRAME );
			float c1[4]={0,0,0,0};
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, c1);

			float c0[4]={1,1,1,1};
			if (params[COLOR]->IsDefined())
			{
				memcpy(c0,params[COLOR]->GetVecValue(),3*sizeof(float));
			}
#ifdef MAPBASE
			if (params[USE_TEXTURE_BLENDS]->GetIntValue())
			{
				float flBlends[4] = { params[BLEND1]->GetFloatValue(), params[BLEND2]->GetFloatValue(), params[BLEND3]->GetFloatValue(), params[BLEND4]->GetFloatValue() };
				pShaderAPI->SetPixelShaderConstant(1, flBlends, 1);

				float flMult = 0.0f;
				if (flBlends[0] > 0.0f)
				{
					BlendSkyRGBA(flMult, flBlends[0], params[BASETEXTURE]->GetTextureValue());
				}

				if (flBlends[1] > 0.0f)
				{
					BindTexture(SHADER_SAMPLER1, BASETEXTURE2, FRAME2);
					BlendSkyRGBA(flMult, flBlends[1], params[BASETEXTURE2]->GetTextureValue());
				}

				if (flBlends[2] > 0.0f)
				{
					BindTexture(SHADER_SAMPLER2, BASETEXTURE3, FRAME3);
					BlendSkyRGBA(flMult, flBlends[2], params[BASETEXTURE3]->GetTextureValue());
				}

				if (flBlends[3] > 0.0f)
				{
					BindTexture(SHADER_SAMPLER3, BASETEXTURE4, FRAME4);
					BlendSkyRGBA(flMult, flBlends[3], params[BASETEXTURE4]->GetTextureValue());
				}

				c0[0] *= flMult;
				c0[1] *= flMult;
				c0[2] *= flMult;
			}
			else
#endif
			{
				ITexture *txtr = params[BASETEXTURE]->GetTextureValue();
				ImageFormat fmt = txtr->GetImageFormat();
				if (
					(fmt == IMAGE_FORMAT_RGBA16161616) ||
					((fmt == IMAGE_FORMAT_RGBA16161616F) &&
					(g_pHardwareConfig->GetHDRType() == HDR_TYPE_INTEGER))
					)
				{
					c0[0] *= 16.0;
					c0[1] *= 16.0;
					c0[2] *= 16.0;
				}
			}
			pShaderAPI->SetPixelShaderConstant(0,c0,1);
			DECLARE_DYNAMIC_VERTEX_SHADER(sdk_sky_vs20);
			SET_DYNAMIC_VERTEX_SHADER(sdk_sky_vs20);


			// Texture coord transform
			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, BASETEXTURETRANSFORM );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20b);
				SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha() );
				SET_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20b); 
			}
			else
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20);
				SET_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20);
			}
		}
		Draw( );
	}

END_SHADER

