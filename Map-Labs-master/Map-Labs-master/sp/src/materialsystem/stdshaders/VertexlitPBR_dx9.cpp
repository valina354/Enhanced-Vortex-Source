//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
// Example shader that can be applied to models
//
//==================================================================================================

#include "BaseVSShader.h"
#include "convar.h"
#include "vertexlitpbr_dx9_helper.h"
//#include "lightpass_helper.h"
#include "emissive_scroll_blended_pass_helper.h"
#include "cloak_blended_pass_helper.h"
#include "flesh_interior_blended_pass_helper.h"

#ifdef GAME_SHADER_DLL
#include "weapon_sheen_pass_helper.h"
#endif

#ifdef STDSHADER
BEGIN_VS_SHADER(VertexLitPBR,
	"Help for LightmappedPBR")
#else
BEGIN_VS_SHADER(VertexLitPBR,
	"Help for LightmappedPBR")
#endif

BEGIN_SHADER_PARAMS
SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap")
SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map")

SHADER_PARAM(BRDF, SHADER_PARAM_TYPE_TEXTURE, "models/PBRTest/BRDF", "")
SHADER_PARAM(NOISE, SHADER_PARAM_TYPE_TEXTURE, "shaders/bluenoise", "")
SHADER_PARAM(ROUGHNESS, SHADER_PARAM_TYPE_TEXTURE, "", "")
SHADER_PARAM(METALLIC, SHADER_PARAM_TYPE_TEXTURE, "", "")
SHADER_PARAM(AO, SHADER_PARAM_TYPE_TEXTURE, "", "")
SHADER_PARAM(EMISSIVE, SHADER_PARAM_TYPE_TEXTURE, "", "")
SHADER_PARAM(MRAOTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.")
SHADER_PARAM(LIGHTMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "lightmap texture--will be bound by the engine")

SHADER_PARAM(USESMOOTHNESS, SHADER_PARAM_TYPE_BOOL, "0", "Invert roughness")
SHADER_PARAM(NORMALMAPALPHASMOOTHNESS, SHADER_PARAM_TYPE_BOOL, "0", "Use the alpha channel of bumpmap as inverted roughness")

// Emissive Scroll Pass
SHADER_PARAM(EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass")
SHADER_PARAM(EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
SHADER_PARAM(EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec")
SHADER_PARAM(EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength")
SHADER_PARAM(EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
SHADER_PARAM(EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")
SHADER_PARAM(EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map")
SHADER_PARAM(TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy")

// Cloak Pass
SHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass")
SHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
SHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint")
SHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "")

#ifdef GAME_SHADER_DLL
// Weapon Sheen Pass
SHADER_PARAM(SHEENPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables weapon sheen render in a second pass")
SHADER_PARAM(SHEENMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "sheenmap")
SHADER_PARAM(SHEENMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "sheenmap mask")
SHADER_PARAM(SHEENMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "")
SHADER_PARAM(SHEENMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "sheenmap tint")
SHADER_PARAM(SHEENMAPMASKSCALEX, SHADER_PARAM_TYPE_FLOAT, "1", "X Scale the size of the map mask to the size of the target")
SHADER_PARAM(SHEENMAPMASKSCALEY, SHADER_PARAM_TYPE_FLOAT, "1", "Y Scale the size of the map mask to the size of the target")
SHADER_PARAM(SHEENMAPMASKOFFSETX, SHADER_PARAM_TYPE_FLOAT, "0", "X Offset of the mask relative to model space coords of target")
SHADER_PARAM(SHEENMAPMASKOFFSETY, SHADER_PARAM_TYPE_FLOAT, "0", "Y Offset of the mask relative to model space coords of target")
SHADER_PARAM(SHEENMAPMASKDIRECTION, SHADER_PARAM_TYPE_INTEGER, "0", "The direction the sheen should move (length direction of weapon) XYZ, 0,1,2")
SHADER_PARAM(SHEENINDEX, SHADER_PARAM_TYPE_INTEGER, "0", "Index of the Effect Type (Color Additive, Override etc...)")
#endif

// Flesh Interior Pass
SHADER_PARAM(FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass")
SHADER_PARAM(FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture")
SHADER_PARAM(FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture")
SHADER_PARAM(FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture")
SHADER_PARAM(FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture")
SHADER_PARAM(FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture")
SHADER_PARAM(FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture")
SHADER_PARAM(FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border")
SHADER_PARAM(FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh")
SHADER_PARAM(FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
SHADER_PARAM(FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
SHADER_PARAM(FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
SHADER_PARAM(FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
SHADER_PARAM(FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color")
SHADER_PARAM(FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border")
SHADER_PARAM(FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)")
SHADER_PARAM(FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color")
SHADER_PARAM(FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity")
SHADER_PARAM(FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness")
SHADER_PARAM(FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed")
END_SHADER_PARAMS

void SetupVars(VertexLitPBR_DX9_Vars_t& info)
{
	info.m_nBaseTexture = BASETEXTURE;
	info.m_nBaseTextureFrame = FRAME;
	info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
	info.m_nAlphaTestReference = ALPHATESTREFERENCE;
	info.m_nRoughness = ROUGHNESS;
	info.m_nMetallic = METALLIC;
	info.m_nAO = AO;
	info.m_nEmissive = EMISSIVE;
	info.m_nEnvmap = ENVMAP;
	info.m_nBumpmap = BUMPMAP;
	info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
	info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
	info.m_nBRDF = BRDF;
	info.m_nUseSmoothness = USESMOOTHNESS;
	info.m_nLightmap = LIGHTMAP;
	info.m_nMRAOTexture = MRAOTEXTURE;
	info.m_nBumpAlphaSmoothness = NORMALMAPALPHASMOOTHNESS;
}

#if 0
void SetupVars(DrawLightPass_Vars_t& info)
{
	info.m_nBaseTexture = BASETEXTURE;
	info.m_nBaseTextureFrame = FRAME;
	info.m_nNoise = NOISE;
	info.m_nBumpmap = BUMPMAP;
	info.m_nRoughness = ROUGHNESS;
	info.m_nMetallic = METALLIC;
	info.m_nBumpmap2 = -1;
	info.m_nBumpFrame2 = -1;
	info.m_nBumpTransform2 = -1;
	info.m_nBaseTexture2 = -1;
	info.m_nBaseTexture2Frame = -1;
	info.m_nSeamlessMappingScale = -1;
	info.bModel = true;
	info.m_nUseSmoothness = USESMOOTHNESS;
}
#endif // 0

// Cloak Pass
void SetupVarsCloakBlendedPass(CloakBlendedPassVars_t& info)
{
	info.m_nCloakFactor = CLOAKFACTOR;
	info.m_nCloakColorTint = CLOAKCOLORTINT;
	info.m_nRefractAmount = REFRACTAMOUNT;

	// Delete these lines if not bump mapping!
	info.m_nBumpmap = BUMPMAP;
	info.m_nBumpFrame = -1;
	info.m_nBumpTransform = -1;
}

#ifdef GAME_SHADER_DLL
// Weapon Sheen Pass
void SetupVarsWeaponSheenPass(WeaponSheenPassVars_t& info)
{
	info.m_nSheenMap = SHEENMAP;
	info.m_nSheenMapMask = SHEENMAPMASK;
	info.m_nSheenMapMaskFrame = SHEENMAPMASKFRAME;
	info.m_nSheenMapTint = SHEENMAPTINT;
	info.m_nSheenMapMaskScaleX = SHEENMAPMASKSCALEX;
	info.m_nSheenMapMaskScaleY = SHEENMAPMASKSCALEY;
	info.m_nSheenMapMaskOffsetX = SHEENMAPMASKOFFSETX;
	info.m_nSheenMapMaskOffsetY = SHEENMAPMASKOFFSETY;
	info.m_nSheenMapMaskDirection = SHEENMAPMASKDIRECTION;
	info.m_nSheenIndex = SHEENINDEX;

	info.m_nBumpmap = BUMPMAP;
	info.m_nBumpFrame = BUMPFRAME;
	info.m_nBumpTransform = BUMPTRANSFORM;
}
#endif

bool NeedsPowerOfTwoFrameBufferTexture(IMaterialVar** params, bool bCheckSpecificToThisFrame) const
{
	if (params[CLOAKPASSENABLED]->GetIntValue()) // If material supports cloaking
	{
		if (bCheckSpecificToThisFrame == false) // For setting model flag at load time
			return true;
		else if ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)) // Per-frame check
			return true;
		// else, not cloaking this frame, so check flag2 in case the base material still needs it
	}

#ifdef GAME_SHADER_DLL
	if (params[SHEENPASSENABLED]->GetIntValue()) // If material supports weapon sheen
		return true;
#endif

	// Check flag2 if not drawing cloak pass
	return IS_FLAG2_SET(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
}

bool IsTranslucent(IMaterialVar** params) const
{
	if (params[CLOAKPASSENABLED]->GetIntValue()) // If material supports cloaking
	{
		if ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)) // Per-frame check
			return true;
		// else, not cloaking this frame, so check flag in case the base material still needs it
	}

	// Check flag if not drawing cloak pass
	return IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT);
}

// Emissive Scroll Pass
void SetupVarsEmissiveScrollBlendedPass(EmissiveScrollBlendedPassVars_t& info)
{
	info.m_nBlendStrength = EMISSIVEBLENDSTRENGTH;
	info.m_nBaseTexture = EMISSIVEBLENDBASETEXTURE;
	info.m_nFlowTexture = EMISSIVEBLENDFLOWTEXTURE;
	info.m_nEmissiveTexture = EMISSIVEBLENDTEXTURE;
	info.m_nEmissiveTint = EMISSIVEBLENDTINT;
	info.m_nEmissiveScrollVector = EMISSIVEBLENDSCROLLVECTOR;
	info.m_nTime = TIME;
}

// Flesh Interior Pass
void SetupVarsFleshInteriorBlendedPass(FleshInteriorBlendedPassVars_t& info)
{
	info.m_nFleshTexture = FLESHINTERIORTEXTURE;
	info.m_nFleshNoiseTexture = FLESHINTERIORNOISETEXTURE;
	info.m_nFleshBorderTexture1D = FLESHBORDERTEXTURE1D;
	info.m_nFleshNormalTexture = FLESHNORMALTEXTURE;
	info.m_nFleshSubsurfaceTexture = FLESHSUBSURFACETEXTURE;
	info.m_nFleshCubeTexture = FLESHCUBETEXTURE;

	info.m_nflBorderNoiseScale = FLESHBORDERNOISESCALE;
	info.m_nflDebugForceFleshOn = FLESHDEBUGFORCEFLESHON;
	info.m_nvEffectCenterRadius1 = FLESHEFFECTCENTERRADIUS1;
	info.m_nvEffectCenterRadius2 = FLESHEFFECTCENTERRADIUS2;
	info.m_nvEffectCenterRadius3 = FLESHEFFECTCENTERRADIUS3;
	info.m_nvEffectCenterRadius4 = FLESHEFFECTCENTERRADIUS4;

	info.m_ncSubsurfaceTint = FLESHSUBSURFACETINT;
	info.m_nflBorderWidth = FLESHBORDERWIDTH;
	info.m_nflBorderSoftness = FLESHBORDERSOFTNESS;
	info.m_ncBorderTint = FLESHBORDERTINT;
	info.m_nflGlobalOpacity = FLESHGLOBALOPACITY;
	info.m_nflGlossBrightness = FLESHGLOSSBRIGHTNESS;
	info.m_nflScrollSpeed = FLESHSCROLLSPEED;

	info.m_nTime = TIME;
}

SHADER_INIT_PARAMS()
{
	VertexLitPBR_DX9_Vars_t info;
	SetupVars(info);
	InitParamsVertexLitPBR_DX9(this, params, pMaterialName, info);

	// Cloak Pass
	if (!params[CLOAKPASSENABLED]->IsDefined())
	{
		params[CLOAKPASSENABLED]->SetIntValue(0);
	}
	else if (params[CLOAKPASSENABLED]->GetIntValue())
	{
		CloakBlendedPassVars_t info;
		SetupVarsCloakBlendedPass(info);
		InitParamsCloakBlendedPass(this, params, pMaterialName, info);
	}

#ifdef GAME_SHADER_DLL
	// Sheen Pass
	if (!params[SHEENPASSENABLED]->IsDefined())
	{
		params[SHEENPASSENABLED]->SetIntValue(0);
	}
	else if (params[SHEENPASSENABLED]->GetIntValue())
	{
		WeaponSheenPassVars_t info;
		SetupVarsWeaponSheenPass(info);
		InitParamsWeaponSheenPass(this, params, pMaterialName, info);
	}
#endif

	// Emissive Scroll Pass
	if (!params[EMISSIVEBLENDENABLED]->IsDefined())
	{
		params[EMISSIVEBLENDENABLED]->SetIntValue(0);
	}
	else if (params[EMISSIVEBLENDENABLED]->GetIntValue())
	{
		EmissiveScrollBlendedPassVars_t info;
		SetupVarsEmissiveScrollBlendedPass(info);
		InitParamsEmissiveScrollBlendedPass(this, params, pMaterialName, info);
	}

	// Flesh Interior Pass
	if (!params[FLESHINTERIORENABLED]->IsDefined())
	{
		params[FLESHINTERIORENABLED]->SetIntValue(0);
	}
	else if (params[FLESHINTERIORENABLED]->GetIntValue())
	{
		FleshInteriorBlendedPassVars_t info;
		SetupVarsFleshInteriorBlendedPass(info);
		InitParamsFleshInteriorBlendedPass(this, params, pMaterialName, info);
	}
}

SHADER_FALLBACK
{
	if (!g_pHardwareConfig->SupportsShaderModel_3_0())
		return "SDK_VertexLitGeneric";

	return 0;
}

SHADER_INIT
{
	VertexLitPBR_DX9_Vars_t info;
	SetupVars(info);
	InitVertexLitPBR_DX9(this, params, info);

	// Cloak Pass
	if (params[CLOAKPASSENABLED]->GetIntValue())
	{
		CloakBlendedPassVars_t info;
		SetupVarsCloakBlendedPass(info);
		InitCloakBlendedPass(this, params, info);
	}

#ifdef GAME_SHADER_DLL
	// TODO : Only do this if we're in range of the camera
	// Weapon Sheen
	if (params[SHEENPASSENABLED]->GetIntValue())
	{
		WeaponSheenPassVars_t info;
		SetupVarsWeaponSheenPass(info);
		InitWeaponSheenPass(this, params, info);
	}
#endif

	// Emissive Scroll Pass
	if (params[EMISSIVEBLENDENABLED]->GetIntValue())
	{
		EmissiveScrollBlendedPassVars_t info;
		SetupVarsEmissiveScrollBlendedPass(info);
		InitEmissiveScrollBlendedPass(this, params, info);
	}

	// Flesh Interior Pass
	if (params[FLESHINTERIORENABLED]->GetIntValue())
	{
		FleshInteriorBlendedPassVars_t info;
		SetupVarsFleshInteriorBlendedPass(info);
		InitFleshInteriorBlendedPass(this, params, info);
	}
}

SHADER_DRAW
{
	bool bDrawStandardPass = true;
	bool hasFlashlight = UsingFlashlight(params);
	if (params[CLOAKPASSENABLED]->GetIntValue() && (pShaderShadow == NULL)) // && not snapshotting
	{
		CloakBlendedPassVars_t info;
		SetupVarsCloakBlendedPass(info);
		if (CloakBlendedPassIsFullyOpaque(params, info))
		{
			bDrawStandardPass = false;
		}
	}

	if (bDrawStandardPass)
	{
		VertexLitPBR_DX9_Vars_t info;
		SetupVars(info);
		DrawVertexLitPBR_DX9(this, params, pShaderAPI, pShaderShadow, hasFlashlight, info, vertexCompression, pContextDataPtr);
	}
	else
	{
		// Skip this pass!
		Draw(false);
	}

#ifdef GAME_SHADER_DLL
	// Weapon sheen pass 
	// only if doing standard as well (don't do it if cloaked)
	if (params[SHEENPASSENABLED]->GetIntValue())
	{
		WeaponSheenPassVars_t info;
		SetupVarsWeaponSheenPass(info);
		if ((pShaderShadow != NULL) || (bDrawStandardPass && ShouldDrawMaterialSheen(params, info)))
		{
			DrawWeaponSheenPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
		}
		else
		{
			// Skip this pass!
			Draw(false);
		}
	}
#endif

	// Cloak Pass
	if (params[CLOAKPASSENABLED]->GetIntValue())
	{
		// If ( snapshotting ) or ( we need to draw this frame )
		if ((pShaderShadow != NULL) || ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)))
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			DrawCloakBlendedPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
		}
		else // We're not snapshotting and we don't need to draw this frame
		{
			// Skip this pass!
			Draw(false);
		}
	}

	// Emissive Scroll Pass
	if (params[EMISSIVEBLENDENABLED]->GetIntValue())
	{
		// If ( snapshotting ) or ( we need to draw this frame )
		if ((pShaderShadow != NULL) || (params[EMISSIVEBLENDSTRENGTH]->GetFloatValue() > 0.0f))
		{
			EmissiveScrollBlendedPassVars_t info;
			SetupVarsEmissiveScrollBlendedPass(info);
			DrawEmissiveScrollBlendedPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
		}
		else // We're not snapshotting and we don't need to draw this frame
		{
			// Skip this pass!
			Draw(false);
		}
	}

	// Flesh Interior Pass
	if (params[FLESHINTERIORENABLED]->GetIntValue())
	{
		// If ( snapshotting ) or ( we need to draw this frame )
		if ((pShaderShadow != NULL) || (true))
		{
			FleshInteriorBlendedPassVars_t info;
			SetupVarsFleshInteriorBlendedPass(info);
			DrawFleshInteriorBlendedPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
		}
		else // We're not snapshotting and we don't need to draw this frame
		{
			// Skip this pass!
			Draw(false);
		}
	}
}

END_SHADER


// Compatability
BEGIN_SHADER_FLAGS(PBR, "", SHADER_NOT_EDITABLE)
BEGIN_SHADER_PARAMS
END_SHADER_PARAMS
SHADER_FALLBACK {
	return IS_FLAG_SET(MATERIAL_VAR_MODEL) ? "VertexLitPBR" : "LightmappedPBR";
}
SHADER_INIT {}
SHADER_DRAW {}
END_SHADER