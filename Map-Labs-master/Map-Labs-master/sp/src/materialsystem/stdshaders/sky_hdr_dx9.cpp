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
#include "SDK_sky_hdr_compressed_ps20.inc"
#include "SDK_sky_hdr_compressed_ps20b.inc"
#include "SDK_sky_hdr_compressed_rgbs_ps20.inc"
#include "SDK_sky_hdr_compressed_rgbs_ps20b.inc"

#include "convar.h"

static ConVar mat_use_compressed_hdr_textures("mat_use_compressed_hdr_textures", "1");

DEFINE_FALLBACK_SHADER(SDK_Sky, SDK_Sky_HDR_DX9)

BEGIN_VS_SHADER(SDK_Sky_HDR_DX9, "Help for Sky_HDR_DX9 shader")
BEGIN_SHADER_PARAMS
SHADER_PARAM(HDRBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "base texture when running with HDR enabled")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "base texture (compressed) for hdr compression method A")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE0, SHADER_PARAM_TYPE_TEXTURE, "", "compressed base texture0 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE1, SHADER_PARAM_TYPE_TEXTURE, "", "compressed base texture1 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "compressed base texture2 for hdr compression method B")
SHADER_PARAM_OVERRIDE(COLOR, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier", SHADER_PARAM_NOT_EDITABLE)

#ifdef MAPBASE
SHADER_PARAM(USE_TEXTURE_BLENDS, SHADER_PARAM_TYPE_BOOL, "0", "")
SHADER_PARAM(BLEND1, SHADER_PARAM_TYPE_FLOAT, "1.0", "How much to blend $basetexture")

SHADER_PARAM(HDRBASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "", "A second HDR sky base texture")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE_2, SHADER_PARAM_TYPE_TEXTURE, "", "second base texture (compressed) for hdr compression method A")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE0_2, SHADER_PARAM_TYPE_TEXTURE, "", "second compressed base texture0 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE1_2, SHADER_PARAM_TYPE_TEXTURE, "", "second compressed base texture1 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE2_2, SHADER_PARAM_TYPE_TEXTURE, "", "second compressed base texture2 for hdr compression method B")
SHADER_PARAM(FRAME2, SHADER_PARAM_TYPE_INTEGER, "", "frame number for $basetexture2")
SHADER_PARAM(BLEND2, SHADER_PARAM_TYPE_FLOAT, "0.0", "How much to blend $basetexture2")

SHADER_PARAM(HDRBASETEXTURE3, SHADER_PARAM_TYPE_TEXTURE, "", "A third HDR sky base texture")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE_3, SHADER_PARAM_TYPE_TEXTURE, "", "third base texture (compressed) for hdr compression method A")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE0_3, SHADER_PARAM_TYPE_TEXTURE, "", "third compressed base texture0 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE1_3, SHADER_PARAM_TYPE_TEXTURE, "", "third compressed base texture1 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE2_3, SHADER_PARAM_TYPE_TEXTURE, "", "third compressed base texture2 for hdr compression method B")
SHADER_PARAM(FRAME3, SHADER_PARAM_TYPE_INTEGER, "", "frame number for $basetexture3")
SHADER_PARAM(BLEND3, SHADER_PARAM_TYPE_FLOAT, "0.0", "How much to blend $basetexture3")

SHADER_PARAM(HDRBASETEXTURE4, SHADER_PARAM_TYPE_TEXTURE, "", "A fourth HDR sky base texture")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE_4, SHADER_PARAM_TYPE_TEXTURE, "", "fourth base texture (compressed) for hdr compression method A")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE0_4, SHADER_PARAM_TYPE_TEXTURE, "", "fourth compressed base texture0 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE1_4, SHADER_PARAM_TYPE_TEXTURE, "", "fourth compressed base texture1 for hdr compression method B")
SHADER_PARAM(HDRCOMPRESSEDTEXTURE2_4, SHADER_PARAM_TYPE_TEXTURE, "", "fourth compressed base texture2 for hdr compression method B")
SHADER_PARAM(FRAME4, SHADER_PARAM_TYPE_INTEGER, "", "frame number for $basetexture4")
SHADER_PARAM(BLEND4, SHADER_PARAM_TYPE_FLOAT, "0.0", "How much to blend $basetexture4")
#endif
END_SHADER_PARAMS

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90 || g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
	{
		return "SDK_Sky_DX9";
	}
	return 0;
}

SHADER_INIT_PARAMS()
{
	SET_FLAGS(MATERIAL_VAR_NOFOG);
	SET_FLAGS(MATERIAL_VAR_IGNOREZ);

#ifdef MAPBASE
	InitFloatParam(BLEND1, params, 1.0f);
#endif
}

void LoadSkyTexture(IMaterialVar **params, int iHDRBaseTexture, int iHDRCompressedTexture, int iHDRCompressedTexture0, int iHDRCompressedTexture1, int iHDRCompressedTexture2)
{
	// First figure out if sampler zero wants to be sRGB
	int nSamplerZeroFlags = 0;
	if ((params[iHDRCompressedTexture]->IsDefined()) && mat_use_compressed_hdr_textures.GetBool())
	{
		nSamplerZeroFlags = 0;
	}
	else
	{
		if (params[iHDRCompressedTexture0]->IsDefined())
		{
			nSamplerZeroFlags = 0;
		}
		else
		{
			nSamplerZeroFlags = TEXTUREFLAGS_SRGB;

			if (params[iHDRBaseTexture]->IsDefined() && params[iHDRBaseTexture]->IsTexture())
			{
				ITexture *txtr = params[iHDRBaseTexture]->GetTextureValue();
				ImageFormat fmt = txtr->GetImageFormat();
				if ((fmt == IMAGE_FORMAT_RGBA16161616F) || (fmt == IMAGE_FORMAT_RGBA16161616))
				{
					nSamplerZeroFlags = 0;
				}
			}
		}
	}

	// Next, figure out which texture will be on sampler zero
	int nSampler0 = iHDRCompressedTexture;
	if (params[iHDRCompressedTexture]->IsDefined() && mat_use_compressed_hdr_textures.GetBool())
	{
		nSampler0 = iHDRCompressedTexture;
	}
	else
	{
		if (params[iHDRCompressedTexture0]->IsDefined())
		{
			nSampler0 = iHDRCompressedTexture0;
		}
		else
		{
			nSampler0 = iHDRBaseTexture;
		}
	}

	// Load the appropriate textures, making sure that the texture set on sampler 0 is sRGB if necessary
	if (params[iHDRCompressedTexture]->IsDefined() && (mat_use_compressed_hdr_textures.GetBool()))
	{
		LoadTexture(iHDRCompressedTexture, iHDRCompressedTexture == nSampler0 ? nSamplerZeroFlags : 0);
	}
	else
	{
		if (params[iHDRCompressedTexture0]->IsDefined())
		{
			LoadTexture(iHDRCompressedTexture0, iHDRCompressedTexture0 == nSampler0 ? nSamplerZeroFlags : 0);
			if (params[iHDRCompressedTexture1]->IsDefined())
			{
				LoadTexture(iHDRCompressedTexture1, iHDRCompressedTexture2 == nSampler0 ? nSamplerZeroFlags : 0);
			}
			if (params[iHDRCompressedTexture2]->IsDefined())
			{
				LoadTexture(iHDRCompressedTexture2, iHDRCompressedTexture2 == nSampler0 ? nSamplerZeroFlags : 0);
			}
		}
		else
		{
			if (params[iHDRBaseTexture]->IsDefined())
			{
				LoadTexture(iHDRBaseTexture, iHDRBaseTexture == nSampler0 ? nSamplerZeroFlags : 0);
			}
		}
	}
}

inline void CheckSkySGRB(IMaterialVar **params, IShaderShadow* pShaderShadow, ITexture *txtr, Sampler_t iSampler)
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

SHADER_INIT
{
	LoadSkyTexture(params, HDRBASETEXTURE, HDRCOMPRESSEDTEXTURE, HDRCOMPRESSEDTEXTURE0, HDRCOMPRESSEDTEXTURE1, HDRCOMPRESSEDTEXTURE2);

#ifdef MAPBASE
	if (params[USE_TEXTURE_BLENDS]->GetIntValue() > 0)
	{
		LoadSkyTexture(params, HDRBASETEXTURE2, HDRCOMPRESSEDTEXTURE_2, HDRCOMPRESSEDTEXTURE0_2, HDRCOMPRESSEDTEXTURE1_2, HDRCOMPRESSEDTEXTURE2_2);
		LoadSkyTexture(params, HDRBASETEXTURE3, HDRCOMPRESSEDTEXTURE_3, HDRCOMPRESSEDTEXTURE0_3, HDRCOMPRESSEDTEXTURE1_3, HDRCOMPRESSEDTEXTURE2_3);
		LoadSkyTexture(params, HDRBASETEXTURE4, HDRCOMPRESSEDTEXTURE_4, HDRCOMPRESSEDTEXTURE0_4, HDRCOMPRESSEDTEXTURE1_4, HDRCOMPRESSEDTEXTURE2_4);
	}
#endif
}

SHADER_DRAW
{
	SHADOW_STATE
	{
		SetInitialShadowState();

		//			pShaderShadow->EnableAlphaWrites( true );
		pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
		pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, NULL, 0);

#ifdef MAPBASE
		bool bUsesBlends = params[USE_TEXTURE_BLENDS]->GetIntValue() > 0;
		bool bShouldBlend[4] = { true, false, false, false };
		if (bUsesBlends)
		{
			//bShouldBlend[0] = (params[BLEND1]->GetFloatValue() > 0.0f);
			//bShouldBlend[1] = (params[BLEND2]->GetFloatValue() > 0.0f);
			//bShouldBlend[2] = (params[BLEND3]->GetFloatValue() > 0.0f);
			//bShouldBlend[3] = (params[BLEND4]->GetFloatValue() > 0.0f);
			bShouldBlend[1] = (params[HDRBASETEXTURE2]->IsDefined() || params[HDRCOMPRESSEDTEXTURE_2]->IsDefined() || params[HDRCOMPRESSEDTEXTURE0_2]->IsDefined());
			bShouldBlend[2] = (params[HDRBASETEXTURE3]->IsDefined() || params[HDRCOMPRESSEDTEXTURE_3]->IsDefined() || params[HDRCOMPRESSEDTEXTURE0_3]->IsDefined());
			bShouldBlend[3] = (params[HDRBASETEXTURE4]->IsDefined() || params[HDRCOMPRESSEDTEXTURE_4]->IsDefined() || params[HDRCOMPRESSEDTEXTURE0_4]->IsDefined());

			if (bShouldBlend[1])
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, false);
			}
			if (bShouldBlend[2])
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, false);
			}
			if (bShouldBlend[3])
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER3, false);
			}

			Msg("HDR sky uses blends: %f %f %f %f\n", params[BLEND1]->GetFloatValue(), params[BLEND2]->GetFloatValue(), params[BLEND3]->GetFloatValue(), params[BLEND4]->GetFloatValue());
		}
		else
		{
			Msg("HDR sky does not use blends\n");
		}
#endif

		DECLARE_STATIC_VERTEX_SHADER(sdk_sky_vs20);
		SET_STATIC_VERTEX_SHADER(sdk_sky_vs20);

		if ((params[HDRCOMPRESSEDTEXTURE]->IsDefined()) &&
			mat_use_compressed_hdr_textures.GetBool())
		{
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, false);
			if (g_pHardwareConfig->SupportsPixelShaders_2_b())
			{
				DECLARE_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20b);
#ifdef MAPBASE
				SET_STATIC_PIXEL_SHADER_COMBO(USES_TEXTURE_BLENDS, bUsesBlends);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND1, bShouldBlend[0]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND2, bShouldBlend[1]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND3, bShouldBlend[2]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND4, bShouldBlend[3]);
#endif
				SET_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20b);
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20);
#ifdef MAPBASE
				SET_STATIC_PIXEL_SHADER_COMBO(USES_TEXTURE_BLENDS, bUsesBlends);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND1, bShouldBlend[0]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND2, bShouldBlend[1]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND3, bShouldBlend[2]);
				SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND4, bShouldBlend[3]);
#endif
				SET_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20);
			}
		}
		else
		{
			if (params[HDRCOMPRESSEDTEXTURE0]->IsDefined())
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
				pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);

				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, false);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, false);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, false);

#ifdef MAPBASE
				if (bUsesBlends)
				{
					if (bShouldBlend[1])
					{
						pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
						pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);
						pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);

						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER3, false);
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER4, false);
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER5, false);
					}
					if (bShouldBlend[2])
					{
						pShaderShadow->EnableTexture(SHADER_SAMPLER6, true);
						pShaderShadow->EnableTexture(SHADER_SAMPLER7, true);
						pShaderShadow->EnableTexture(SHADER_SAMPLER8, true);

						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER6, false);
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER7, false);
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER8, false);
					}
					if (bShouldBlend[3])
					{
						pShaderShadow->EnableTexture(SHADER_SAMPLER9, true);
						pShaderShadow->EnableTexture(SHADER_SAMPLER10, true);
						pShaderShadow->EnableTexture(SHADER_SAMPLER11, true);

						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER9, false);
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER10, false);
						pShaderShadow->EnableSRGBRead(SHADER_SAMPLER11, false);
					}
				}
#endif

				if (g_pHardwareConfig->SupportsPixelShaders_2_b())
				{
					DECLARE_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20b);
#ifdef MAPBASE
					SET_STATIC_PIXEL_SHADER_COMBO(USES_TEXTURE_BLENDS, bUsesBlends);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND1, bShouldBlend[0]);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND2, bShouldBlend[1]);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND3, bShouldBlend[2]);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND4, bShouldBlend[3]);
#endif
					SET_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20b);
				}
				else
				{
					DECLARE_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20);
#ifdef MAPBASE
					SET_STATIC_PIXEL_SHADER_COMBO(USES_TEXTURE_BLENDS, bUsesBlends);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND1, bShouldBlend[0]);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND2, bShouldBlend[1]);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND3, bShouldBlend[2]);
					SET_STATIC_PIXEL_SHADER_COMBO(HAS_BLEND4, bShouldBlend[3]);
#endif
					SET_STATIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20);
				}
			}
			else
			{
				CheckSkySGRB(params, pShaderShadow, params[HDRBASETEXTURE]->GetTextureValue(), SHADER_SAMPLER0);

#ifdef MAPBASE
				if (bUsesBlends)
				{
					if (bShouldBlend[1])
					{
						pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
						CheckSkySGRB(params, pShaderShadow, params[HDRBASETEXTURE2]->GetTextureValue(), SHADER_SAMPLER1);
					}
					if (bShouldBlend[2])
					{
						pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
						CheckSkySGRB(params, pShaderShadow, params[HDRBASETEXTURE3]->GetTextureValue(), SHADER_SAMPLER2);
					}
					if (bShouldBlend[3])
					{
						pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
						CheckSkySGRB(params, pShaderShadow, params[HDRBASETEXTURE4]->GetTextureValue(), SHADER_SAMPLER3);
					}

					Msg("Non-HDR sky uses blends: %f %f %f %f\n", params[BLEND1]->GetFloatValue(), params[BLEND2]->GetFloatValue(), params[BLEND3]->GetFloatValue(), params[BLEND4]->GetFloatValue());
				}
				else
				{
					Msg("Non-HDR sky does not use blends\n");
				}
#endif

				if (g_pHardwareConfig->SupportsPixelShaders_2_b())
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
			}
		}
		// we are writing linear values from this shader.
		pShaderShadow->EnableSRGBWrite(true);

		pShaderShadow->EnableAlphaWrites(true);
	}

		DYNAMIC_STATE
	{
		DECLARE_DYNAMIC_VERTEX_SHADER(sdk_sky_vs20);
		SET_DYNAMIC_VERTEX_SHADER(sdk_sky_vs20);

		// Texture coord transform
		SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, BASETEXTURETRANSFORM);

		float c0[4] = { 1, 1, 1, 1 };
		if (params[COLOR]->IsDefined())
		{
			memcpy(c0, params[COLOR]->GetVecValue(), 3 * sizeof(float));
		}
		if (
			params[HDRCOMPRESSEDTEXTURE]->IsDefined() &&
			mat_use_compressed_hdr_textures.GetBool()
			)
		{
			// set up data needs for pixel shader interpolation
			ITexture *txtr = params[HDRCOMPRESSEDTEXTURE]->GetTextureValue();
			float w = txtr->GetActualWidth();
			float h = txtr->GetActualHeight();
			float FUDGE = 0.01 / max(w, h);					// per ATI
			float c1[4] = { 0.5 / w - FUDGE, 0.5 / h - FUDGE, w, h };
			pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, c1);

#ifdef MAPBASE
			bool bUsesBlends = params[USE_TEXTURE_BLENDS]->GetIntValue() > 0;
			if (bUsesBlends)
			{
				float flBlends[4] = { params[BLEND1]->GetFloatValue(), params[BLEND2]->GetFloatValue(), params[BLEND3]->GetFloatValue(), params[BLEND4]->GetFloatValue() };
				pShaderAPI->SetPixelShaderConstant(1, flBlends, 1);

				if (flBlends[1] > 0.0f)
				{
					BindTexture(SHADER_SAMPLER1, HDRCOMPRESSEDTEXTURE_2, FRAME2);
				}
				if (flBlends[2] > 0.0f)
				{
					BindTexture(SHADER_SAMPLER2, HDRCOMPRESSEDTEXTURE_3, FRAME3);
				}
				if (flBlends[3] > 0.0f)
				{
					BindTexture(SHADER_SAMPLER3, HDRCOMPRESSEDTEXTURE_4, FRAME4);
				}

				Msg("In dynamic mode, HDR sky uses blends: %f %f %f %f\n", flBlends[0], flBlends[1], flBlends[2], flBlends[3]);
			}
			else
			{
				Msg("In dynamic mode, HDR sky doesn't use blends\n");
			}
#endif

			BindTexture(SHADER_SAMPLER0, HDRCOMPRESSEDTEXTURE, FRAME);
			c0[0] *= 8.0;
			c0[1] *= 8.0;
			c0[2] *= 8.0;
			if (g_pHardwareConfig->SupportsPixelShaders_2_b())
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20b);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha());
				SET_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20b);
			}
			else
			{
				DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20);
				SET_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_rgbs_ps20);
			}
		}
		else
		{
			float c1[4] = { 0, 0, 0, 0 };
			pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, c1);

			if (params[HDRCOMPRESSEDTEXTURE0]->IsDefined())
			{
				BindTexture(SHADER_SAMPLER0, HDRCOMPRESSEDTEXTURE0, FRAME);
				BindTexture(SHADER_SAMPLER1, HDRCOMPRESSEDTEXTURE1, FRAME);
				BindTexture(SHADER_SAMPLER2, HDRCOMPRESSEDTEXTURE2, FRAME);
#ifdef MAPBASE
				bool bUsesBlends = params[USE_TEXTURE_BLENDS]->GetIntValue() > 0;
				if (bUsesBlends)
				{
					float flBlends[4] = { params[BLEND1]->GetFloatValue(), params[BLEND2]->GetFloatValue(), params[BLEND3]->GetFloatValue(), params[BLEND4]->GetFloatValue() };
					pShaderAPI->SetPixelShaderConstant(1, flBlends, 1);

					if (flBlends[1] > 0.0f)
					{
						BindTexture(SHADER_SAMPLER3, HDRCOMPRESSEDTEXTURE0_2, FRAME);
						BindTexture(SHADER_SAMPLER4, HDRCOMPRESSEDTEXTURE1_2, FRAME);
						BindTexture(SHADER_SAMPLER5, HDRCOMPRESSEDTEXTURE2_2, FRAME);
					}
					if (flBlends[2] > 0.0f)
					{
						BindTexture(SHADER_SAMPLER6, HDRCOMPRESSEDTEXTURE0_3, FRAME);
						BindTexture(SHADER_SAMPLER7, HDRCOMPRESSEDTEXTURE1_3, FRAME);
						BindTexture(SHADER_SAMPLER8, HDRCOMPRESSEDTEXTURE2_3, FRAME);
					}
					if (flBlends[3] > 0.0f)
					{
						BindTexture(SHADER_SAMPLER9, HDRCOMPRESSEDTEXTURE0_4, FRAME);
						BindTexture(SHADER_SAMPLER10, HDRCOMPRESSEDTEXTURE1_4, FRAME);
						BindTexture(SHADER_SAMPLER11, HDRCOMPRESSEDTEXTURE2_4, FRAME);
					}
				}
#endif
				if (g_pHardwareConfig->SupportsPixelShaders_2_b())
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20b);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha());
					SET_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20b);
				}
				else
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20);
					SET_DYNAMIC_PIXEL_SHADER(sdk_sky_hdr_compressed_ps20);
				}

			}
			else
			{
				BindTexture(SHADER_SAMPLER0, HDRBASETEXTURE, FRAME);

#ifdef MAPBASE
				if (params[USE_TEXTURE_BLENDS]->GetIntValue())
				{
					float flBlends[4] = { params[BLEND1]->GetFloatValue(), params[BLEND2]->GetFloatValue(), params[BLEND3]->GetFloatValue(), params[BLEND4]->GetFloatValue() };
					pShaderAPI->SetPixelShaderConstant(1, flBlends, 1);

					float flMult = 0.0f;
					if (flBlends[0] > 0.0f)
					{
						BlendSkyRGBA(flMult, flBlends[0], params[HDRBASETEXTURE]->GetTextureValue());
					}

					if (flBlends[1] > 0.0f)
					{
						BindTexture(SHADER_SAMPLER1, HDRBASETEXTURE2, FRAME2);
						BlendSkyRGBA(flMult, flBlends[1], params[HDRBASETEXTURE2]->GetTextureValue());
					}

					if (flBlends[2] > 0.0f)
					{
						BindTexture(SHADER_SAMPLER2, HDRBASETEXTURE3, FRAME3);
						BlendSkyRGBA(flMult, flBlends[2], params[HDRBASETEXTURE3]->GetTextureValue());
					}

					if (flBlends[3] > 0.0f)
					{
						BindTexture(SHADER_SAMPLER3, HDRBASETEXTURE4, FRAME4);
						BlendSkyRGBA(flMult, flBlends[3], params[HDRBASETEXTURE4]->GetTextureValue());
					}

					c0[0] *= flMult;
					c0[1] *= flMult;
					c0[2] *= flMult;
				}
				else
#endif
				{
					ITexture *txtr = params[HDRBASETEXTURE]->GetTextureValue();
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
				if (g_pHardwareConfig->SupportsPixelShaders_2_b())
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20b);
					SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha());
					SET_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20b);
				}
				else
				{
					DECLARE_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20);
					SET_DYNAMIC_PIXEL_SHADER(sdk_sky_ps20);
				}
			}
		}
		pShaderAPI->SetPixelShaderConstant(0, c0, 1);
	}
	Draw();
}
END_SHADER