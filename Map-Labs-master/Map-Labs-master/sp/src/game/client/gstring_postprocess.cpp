
#include "cbase.h"
#include "view.h"
#include "view_scene.h"
#include "ivieweffects.h"
#include "shadereditor/shadereditorsystem.h"
#include "shadereditor/ivshadereditor.h"
#include "gstring/cenv_postprocessing.h"
#include "gstring/gstring_cvars.h"
#include "gstring/cgstring_globals.h"
#include "gstring/vgui/vutil.h"
#include "c_sun.h"

#include "gstring/c_gstring_player.h"

#include "materialsystem/imaterialvar.h"
#include "sourcevr/isourcevirtualreality.h"

#define DBG_SKYMASK_EDITOR_NAME "dbg_ppe_skymask"

#define GODRAYS_EDITOR_NAME "ppe_sunrays"
#define EXPLOSION_EDITOR_NAME "ppe_explosionlayer"
#define MOTIONBLUR_EDITOR_NAME "ppe_motionblur"
#define SCREENBLUR_EDITOR_NAME "ppe_gaussian_blur"
#define DREAMBLUR_EDITOR_NAME "ppe_dream"
#define BLOOMFLARE_EDITOR_NAME "ppe_bloomflare"
#define DESATURATION_EDITOR_NAME "ppe_desaturate"
#define FILMGRAIN_EDITOR_NAME "ppe_filmgrain"
#define VIGNETTE_EDITOR_NAME "ppe_vignette"
#define NIGHTVISION_EDITOR_NAME "ppe_nightvision"
#define HURTFX_EDITOR_NAME "ppe_hurtfx"
#define DOF_EDITOR_NAME "ppe_dof"
#define DOFCLEAR_EDITOR_NAME "ppe_dof_clear"

float GetSceneFadeScalar()
{
	byte color[4];
	bool blend;
	vieweffects->GetFadeParams(&color[0], &color[1], &color[2], &color[3], &blend);

	return 1.0f - (color[3] / 255.0f);
}

static IMaterialVar *GetPPEMaterialVar(const char *pszPPEName, const char *pszNode, const char *pszVar)
{
	if (!g_ShaderEditorSystem->IsReady())
	{
		Assert(0);
		return NULL;
	}

	const int iPPEIndex = shaderEdit->GetPPEIndex(pszPPEName);
	IMaterial *pMat = shaderEdit->GetPPEMaterial(iPPEIndex, pszNode);

	if (IsErrorMaterial(pMat))
	{
		Assert(0);
		return NULL;
	}

	IMaterialVar *pVarMutable = pMat->FindVar(pszVar, NULL, false);

	Assert(pVarMutable);

	return pVarMutable;
}

// No models drawn yet? - post process drawing doesn't work (breaks mblur)
void PerformScenePostProcessHack()
{
	if (!cvar_gstring_enable_postprocessing.GetBool())
		return;

	static CMaterialReference test_mat;

	if (!test_mat.IsValid())
		test_mat.Init(materials->FindMaterial(
		"dev/lumcompare",
		TEXTURE_GROUP_OTHER, true));

	CMatRenderContextPtr renderContext(materials);
	renderContext->DrawScreenSpaceRectangle(test_mat, 0, 0, 1, 1,
		0, 0, 0, 0,
		0, 0);
}

inline bool ShouldDrawCommon()
{
	if (!cvar_gstring_enable_postprocessing.GetBool())
		return false;

	return g_ShaderEditorSystem->IsReady() && engine->IsInGame() && !engine->IsPaused();
}

/**
* BARS AND GRAIN
*/
void DrawBarsAndGrain(int x, int y, int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	if (g_pPPCtrl != NULL && !g_pPPCtrl->IsBarsEnabled())
		return;

	const float flBendStrength = UseVR() ? 0.0f : cvar_gstring_bend_strength.GetFloat();
	const float flFilmgrainStrength = cvar_gstring_filmgrain_strength.GetFloat();
	if ( /*cvar_gstring_drawfilmgrain.GetBool()
		 &&*/ flFilmgrainStrength > 0.0f
		 || flBendStrength > 0.0f)
	{
		C_GstringPlayer *pPlayer = LocalGstringPlayer();
		float flNightvisionStrengthInv = 1.0f - (pPlayer ? pPlayer->GetNightvisionFraction() : 0.0f);

		static int iFilmgrainIndex = shaderEdit->GetPPEIndex(FILMGRAIN_EDITOR_NAME);

		if (iFilmgrainIndex >= 0)
		{
			DEFINE_SHADEREDITOR_MATERIALVAR(FILMGRAIN_EDITOR_NAME, "filmgrain", "$MUTABLE_01", pVar_Filmgrain_Strength);
			DEFINE_SHADEREDITOR_MATERIALVAR(FILMGRAIN_EDITOR_NAME, "filmgrain", "$MUTABLE_02", pVar_Bend_Strength);

			pVar_Filmgrain_Strength->SetFloatValue(flFilmgrainStrength * flNightvisionStrengthInv);
			pVar_Bend_Strength->SetFloatValue(flBendStrength * flNightvisionStrengthInv);

			shaderEdit->DrawPPEOnDemand(iFilmgrainIndex, x, y, w, h);
		}
	}

	const float flBarScale = UseVR() ? 0.0f : cvar_gstring_bars_scale.GetFloat();

	if ( /*cvar_gstring_drawbars.GetBool()
		 &&*/ flBarScale > 0.0f &&
		 (g_pGstringGlobals == NULL || !g_pGstringGlobals->IsSpaceMap()))
	{
		static CMaterialReference blackMaterial("effects/black", TEXTURE_GROUP_OTHER);

		const float flBarSize = h * flBarScale;

		UpdateScreenEffectTexture();

		ITexture *pTexture = GetFullFrameFrameBufferTexture(0);
		CMatRenderContextPtr renderContext(materials);

		renderContext->DrawScreenSpaceRectangle(blackMaterial, x, y, w, flBarSize,
			0, 0, w - 1, h - 1,
			pTexture->GetActualWidth(), pTexture->GetActualHeight());
		renderContext->DrawScreenSpaceRectangle(blackMaterial, x, y + h - flBarSize, w, h,
			0, 0, w - 1, h - 1,
			pTexture->GetActualWidth(), pTexture->GetActualHeight());
	}
}

void DrawVignette(int x, int y, int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	if (cvar_gstring_drawvignette.GetBool())
	{
		C_GstringPlayer *pPlayer = LocalGstringPlayer();
		float flNightvisionStrengthInv = 1.0f - (pPlayer ? pPlayer->GetNightvisionFraction() : 0.0f);

		static int iVignetteIndex = shaderEdit->GetPPEIndex(VIGNETTE_EDITOR_NAME);

		if (iVignetteIndex >= 0)
		{
			DEFINE_SHADEREDITOR_MATERIALVAR(VIGNETTE_EDITOR_NAME, "vignette", "$MUTABLE_01", pVar_Vignette_Strength);
			DEFINE_SHADEREDITOR_MATERIALVAR(VIGNETTE_EDITOR_NAME, "vignette", "$MUTABLE_02", pVar_Vignette_Ranges);

			pVar_Vignette_Strength->SetFloatValue(cvar_gstring_vignette_strength.GetFloat() * flNightvisionStrengthInv);
			pVar_Vignette_Ranges->SetVecValue(cvar_gstring_vignette_range_max.GetFloat(),
				cvar_gstring_vignette_range_min.GetFloat());

			shaderEdit->DrawPPEOnDemand(iVignetteIndex, x, y, w, h);
		}
	}
}

/**
* GODRAYS
*/
static float g_flGodRaysIntensity = 1.0f;

void SetGodraysColor(Vector col)
{
	if (shaderEdit == NULL)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(GODRAYS_EDITOR_NAME, "sunrays calc", "$MUTABLE_01", pVarMutable);
	Assert(pVarMutable);

	if (!pVarMutable)
		return;

	pVarMutable->SetVecValue(col.x, col.y, col.z, 0);
}

void SetGodraysIntensity(float i)
{
	g_flGodRaysIntensity = i;
}

bool ShouldDrawGodrays()
{
	if (!ShouldDrawCommon())
		return false;

	if (!cvar_gstring_drawgodrays.GetInt())
		return false;

	if (g_pPPCtrl != NULL &&
		!g_pPPCtrl->IsGodraysEnabled())
		return false;

	if (GetNumSuns() < 1)
		return false;

	return true;
}

void DrawGodrays(int x, int y, int w, int h)
{
	if (!ShouldDrawGodrays())
		return;

	static const int iGodrayIndex = shaderEdit->GetPPEIndex(GODRAYS_EDITOR_NAME);

	if (iGodrayIndex < 0)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(GODRAYS_EDITOR_NAME, "sunrays calc", "$MUTABLE_02", pVar_GodRays_Intensity);

	Assert(pVar_GodRays_Intensity);

	if (!pVar_GodRays_Intensity)
		return;

	pVar_GodRays_Intensity->SetVecValue(g_flGodRaysIntensity * GetSceneFadeScalar(), 0, 0, 0);

	shaderEdit->DrawPPEOnDemand(iGodrayIndex, x, y, w, h);
}


/**
* EXPLOSION BLUR
*/
struct ExpBlurHelper_t
{
	Vector pos;
	float creationtime;
	float lifeduration;

	inline float GetAmt()
	{
		return clamp((1.0f - (gpGlobals->curtime - creationtime) / lifeduration), 0, 1);
	};
	inline bool ShouldDie()
	{
		return gpGlobals->curtime < creationtime ||
			gpGlobals->curtime >(creationtime + lifeduration);
	};
};
static CUtlVector< ExpBlurHelper_t > g_hExplosionBlurQueue;

void QueueExplosionBlur(Vector origin, float lifetime)
{
	ExpBlurHelper_t e;
	e.creationtime = gpGlobals->curtime;
	e.lifeduration = lifetime;
	e.pos = origin;
	if (g_pGstringGlobals && g_pGstringGlobals->IsSpaceMap())
	{
		e.lifeduration *= 0.3f;
	}
	g_hExplosionBlurQueue.AddToTail(e);
}

void DrawExplosionBlur(int x, int y, int w, int h)
{
	if (g_hExplosionBlurQueue.Count() < 1)
		return;

	for (int i = 0; i < g_hExplosionBlurQueue.Count(); i++)
	{
		ExpBlurHelper_t &e = g_hExplosionBlurQueue[i];

		if (e.ShouldDie())
		{
			g_hExplosionBlurQueue.Remove(i);
			i--;
			continue;
		}
	}

	if (!ShouldDrawCommon())
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(EXPLOSION_EDITOR_NAME, "explosion mat", "$MUTABLE_01", pVar_Origin);
	DEFINE_SHADEREDITOR_MATERIALVAR(EXPLOSION_EDITOR_NAME, "explosion mat", "$MUTABLE_02", pVar_Strength);
	DEFINE_SHADEREDITOR_MATERIALVAR(EXPLOSION_EDITOR_NAME, "explosion mat", "$MUTABLE_03", pVar_LinearAmt);

	if (pVar_Origin == NULL ||
		pVar_Strength == NULL ||
		pVar_LinearAmt == NULL)
		return;

	static const int iExplosionIndex = shaderEdit->GetPPEIndex(EXPLOSION_EDITOR_NAME);

	if (iExplosionIndex < 0)
		return;

	if ( /*!cvar_gstring_drawexplosionblur.GetInt()
		 ||*/ cvar_gstring_explosionfx_strength.GetFloat() <= 0.0f)
		 return;

	float flStrength = cvar_gstring_explosionfx_strength.GetFloat();
	if (g_pGstringGlobals && g_pGstringGlobals->IsSpaceMap())
	{
		//flStrength *= 0.4f;
	}

	for (int i = 0; i < g_hExplosionBlurQueue.Count(); i++)
	{
		ExpBlurHelper_t &e = g_hExplosionBlurQueue[i];

		Vector screen;

		if (ScreenTransform(e.pos, screen))
			continue;

		screen.x *= 0.5f;
		screen.y *= -0.5f;
		screen.x += 0.5f;
		screen.y += 0.5f;

		Vector delta = e.pos - MainViewOrigin();
		Vector dir = delta;
		float dist = dir.NormalizeInPlace();

		float dot = DotProduct(MainViewForward(), dir);

		float strength = RemapValClamped(dot, 0, 0.3f, 0, 1.0f);
		strength *= RemapValClamped(dist, 256, 512, 1.0f, 0.0f);
		strength *= Bias(e.GetAmt(), 0.65f);
		strength *= flStrength;

		if (strength <= 0.0f)
			continue;

		pVar_Origin->SetVecValue(screen.x, screen.y, 0, 0);
		pVar_Strength->SetVecValue(strength, 0, 0, 0);
		pVar_LinearAmt->SetVecValue(1.0f - dot, 0, 0, 0);

		shaderEdit->DrawPPEOnDemand(iExplosionIndex, x, y, w, h);
	}
}



/**
* MOTION BLUR
*/
void DrawMotionBlur(int x, int y, int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	const C_GstringPlayer *pPlayer = LocalGstringPlayer();
	if (pPlayer && pPlayer->IsInSpacecraft())
		return;

	static const int iMotionBlur = shaderEdit->GetPPEIndex(MOTIONBLUR_EDITOR_NAME);

	if (iMotionBlur < 0)
		return;

	float motionblur_scale = cvar_gstring_motionblur_scale.GetFloat();

	if (!cvar_gstring_drawmotionblur.GetInt()
		|| motionblur_scale <= 0.0f)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(MOTIONBLUR_EDITOR_NAME, "motionblur", "$MUTABLE_01", pVar_LinearDirection);
	DEFINE_SHADEREDITOR_MATERIALVAR(MOTIONBLUR_EDITOR_NAME, "motionblur", "$MUTABLE_02", pVar_ForwardPosition);
	DEFINE_SHADEREDITOR_MATERIALVAR(MOTIONBLUR_EDITOR_NAME, "motionblur", "$MUTABLE_03", pVar_RotationCenter);
	DEFINE_SHADEREDITOR_MATERIALVAR(MOTIONBLUR_EDITOR_NAME, "motionblur", "$MUTABLE_04", pVar_ForwardBlend);
	DEFINE_SHADEREDITOR_MATERIALVAR(MOTIONBLUR_EDITOR_NAME, "motionblur", "$MUTABLE_05", pVar_RotationBlend);
	DEFINE_SHADEREDITOR_MATERIALVAR(MOTIONBLUR_EDITOR_NAME, "motionblur", "$MUTABLE_06", pVar_RotationScale);

	if (pVar_LinearDirection == NULL ||
		pVar_ForwardPosition == NULL ||
		pVar_RotationCenter == NULL ||
		pVar_ForwardBlend == NULL ||
		pVar_RotationBlend == NULL ||
		pVar_RotationScale == NULL)
	{
		Assert(0);
		return;
	}

	static QAngle ang_last = vec3_angle;
	const QAngle ang_cur = MainViewAngles();

	static Vector pos_last = vec3_origin;
	const Vector pos_cur = MainViewOrigin();

	Vector delta = pos_cur - pos_last;
	float distance = delta.Length();

	if (gpGlobals->frametime < (1.0f / 20.0f) &&
		gpGlobals->frametime > 0.0f)
	{
		float speed = distance / gpGlobals->frametime;
		float speed_interp_blend = RemapValClamped(speed, 160, 1000, 0, 1);
		float speed_interp_blur = Bias(speed_interp_blend, 0.65f);

		float amount_forward = 0.0f;
		float forward_linear_x = 0.0f;
		float forward_linear_y = 0.0f;

		Vector2D data_pos_speed(0, 0);
		float speed_dot = 0.0f;

		if (distance > 0)
		{
			Vector delta_normalized = delta / distance;

			float falling_percentage = RemapValClamped(DotProduct(Vector(0, 0, -1), delta_normalized), 0.707f, 1.0f, 0.0f, 1.0f) *
				RemapValClamped(speed, 200, 500, 0, 1);
			motionblur_scale += motionblur_scale * 4.0f * falling_percentage;

			speed_dot = DotProduct(delta_normalized, MainViewForward());
			amount_forward = RemapValClamped(speed_interp_blur, 0, 0.4f, 0, 1) * abs(speed_dot);

			const float side_amount = RemapValClamped(speed, 250, 500, 0, 1);

			forward_linear_x = DotProduct(delta_normalized, MainViewRight()) * (1.0f + falling_percentage) * side_amount;
			forward_linear_y = DotProduct(delta_normalized, MainViewUp()) * (1.0f + falling_percentage) * side_amount;

			Vector screen;
			if (ScreenTransform(MainViewOrigin() + delta_normalized * 100, screen) != 0)
				ScreenTransform(MainViewOrigin() - delta_normalized * 100, screen);
			data_pos_speed = screen.AsVector2D();
		}

		float linear_x = AngleDiff(ang_last.y, ang_cur.y);
		float linear_y = AngleDiff(ang_cur.x, ang_last.x);

		Vector2D linear_rotation(linear_x, linear_y);
		Vector2D linear_speed(forward_linear_x, forward_linear_y);

		speed_interp_blur = RemapValClamped(Bias(speed_interp_blur, 0.7f), 0.0f, 0.3f, 0, 1);
		Vector2D data_linear = Lerp(speed_interp_blur, linear_rotation * 0.05f, linear_speed * 0.35f) * motionblur_scale;
		amount_forward *= motionblur_scale;

		Vector screen_axis;
		if (ScreenTransform(MainViewOrigin() + Vector(0, 0, 100), screen_axis) != 0)
			ScreenTransform(MainViewOrigin() - Vector(0, 0, 100), screen_axis);
		Vector2D data_pos_axis = screen_axis.AsVector2D();

		float dot_z_axis = abs(DotProduct(Vector(0, 0, 1), MainViewForward()));

		float rotation_blend = (1.0f - speed_interp_blend) *
			RemapValClamped(dot_z_axis, 0.707f, 1.0f, 0.0f, 1.0f);
		float rotation_scale = abs(linear_x * 0.1f) * motionblur_scale;

		data_pos_speed *= Vector2D(1, -1);
		data_pos_axis *= Vector2D(1, -1);
		data_pos_speed += Vector2D(0.5f, 0.5f);
		data_pos_axis += Vector2D(0.5f, 0.5f);

		if (data_linear.LengthSqr() < 0.0001f)
			data_linear.Init();

		if (amount_forward <= 0.05f)
		{
			data_pos_speed.Init();
		}

		amount_forward = RemapVal(amount_forward, 0.05f, 1.0f, 0, 1);
		if (amount_forward < 0.0f)
			amount_forward = 0.0f;

		amount_forward *= RemapValClamped(abs(speed_dot), 0.5f, 1.0f, 0.0f, 1.0f);

		pVar_LinearDirection->SetVecValue(data_linear.x, data_linear.y, 0, 0);
		pVar_ForwardPosition->SetVecValue(data_pos_speed.x, data_pos_speed.y, 0, 0);
		pVar_RotationCenter->SetVecValue(data_pos_axis.x, data_pos_axis.y, 0, 0);
		pVar_ForwardBlend->SetVecValue(amount_forward, 0, 0, 0);
		pVar_RotationBlend->SetVecValue(rotation_blend, 0, 0, 0);
		pVar_RotationScale->SetVecValue(rotation_scale, 0, 0, 0);

		if (data_linear.LengthSqr() > 0.01f ||
			amount_forward > 0.01f ||
			rotation_scale > 0.01f)
		{
			shaderEdit->DrawPPEOnDemand(iMotionBlur, x, y, w, h);
		}
	}

	ang_last = ang_cur;
	pos_last = pos_cur;
}


/**
* SCREEN BLUR
*/
void DrawScreenGaussianBlur(int x, int y, int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	static const int iScreenBlur = shaderEdit->GetPPEIndex(SCREENBLUR_EDITOR_NAME);
	if (iScreenBlur < 0)
		return;

	if (!cvar_gstring_drawscreenblur.GetBool())
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(SCREENBLUR_EDITOR_NAME, "blendmaterial", "$MUTABLE_01", pVar_ScreenBlur_Strength);

	if (pVar_ScreenBlur_Strength == NULL)
	{
		Assert(0);
		return;
	}

	float intensity = g_pPPCtrl ? g_pPPCtrl->GetScreenBlurStrength() : 0;
	if (intensity <= 0.001f)
		return;

	pVar_ScreenBlur_Strength->SetFloatValue(clamp(intensity, 0, 1));
	CMatRenderContextPtr renderContext(materials);
	renderContext->OverrideDepthEnable(true, false);
	shaderEdit->DrawPPEOnDemand(iScreenBlur, x, y, w, h);
	renderContext->OverrideDepthEnable(false, false);
}


/**
* DREAM BLUR
*/
inline void UpdateDreamBuffer(int x, int y, int w, int h)
{
	static ITexture* pDreamBuffer = materials->FindTexture("_rt_dream_cache", TEXTURE_GROUP_OTHER, false);

	if (!IsErrorTexture(pDreamBuffer))
	{
		Rect_t rect;
		rect.x = x;
		rect.y = y;
		rect.width = w;
		rect.height = h;
		CMatRenderContextPtr renderContext(materials);
		renderContext->PushRenderTargetAndViewport();
		renderContext->CopyRenderTargetToTextureEx(pDreamBuffer, 0, &rect, &rect);
		renderContext->PopRenderTargetAndViewport();
	}
}

void DrawDreamBlur(int x, int y, int w, int h, StereoEye_t stereoEye)
{
	if (!ShouldDrawCommon())
		return;

	static const int iDreamBlur = shaderEdit->GetPPEIndex(DREAMBLUR_EDITOR_NAME);
	if (iDreamBlur < 0)
		return;

	static bool bDrewLastFrame = false;

	if (!cvar_gstring_drawdreamblur.GetBool())
	{
		bDrewLastFrame = false;
		return;
	}

	DEFINE_SHADEREDITOR_MATERIALVAR(DREAMBLUR_EDITOR_NAME, "dream", "$MUTABLE_01", pVar_DreamBlur_Strength);
	DEFINE_SHADEREDITOR_MATERIALVAR(DREAMBLUR_EDITOR_NAME, "dream", "$MUTABLE_02", pVar_DreamBlur_StereoConfig);

	if (pVar_DreamBlur_Strength == NULL || pVar_DreamBlur_StereoConfig == NULL)
	{
		Assert(0);
		return;
	}

	float intensity = g_pPPCtrl ? g_pPPCtrl->GetDreamStrength() : 0;
	if (intensity <= 0.001f)
	{
		bDrewLastFrame = false;
		return;
	}

	if (!bDrewLastFrame)
	{
		UpdateDreamBuffer(x, y, w, h);
	}

	bDrewLastFrame = bDrewLastFrame || stereoEye != STEREO_EYE_LEFT;

	intensity = clamp(intensity, 0, 1);

	float flIntensityCorrect = RemapValClamped(gpGlobals->frametime,
		(1.0f / 10.0f), (1.0f / 300.0f),
		0.7f, 1.15f);

	intensity *= flIntensityCorrect;

	pVar_DreamBlur_Strength->SetFloatValue(intensity);

	float stereoValues[2] = { 1.0f };
	if (stereoEye != STEREO_EYE_MONO)
	{
		stereoValues[0] = 0.5f;
		if (stereoEye == STEREO_EYE_RIGHT)
		{
			stereoValues[1] = 0.5f;
		}
	}
	pVar_DreamBlur_StereoConfig->SetVecValue(stereoValues, 2);

	shaderEdit->DrawPPEOnDemand(iDreamBlur, x, y, w, h);

	UpdateDreamBuffer(x, y, w, h);
}

/**
* BLOOM FLARE
*/
void DrawBloomFlare(int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	//int iBloomFlareMode = cvar_gstring_drawbloomflare.GetInt();
	//if ( iBloomFlareMode <= 0 )
	//	return;

	const float flStrength = cvar_gstring_bloomflare_strength.GetFloat();
	if (flStrength <= 0.0f)
		return;

	static const int iBloomFlare = shaderEdit->GetPPEIndex(BLOOMFLARE_EDITOR_NAME);
	if (iBloomFlare < 0)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(BLOOMFLARE_EDITOR_NAME, "bloomflare", "$MUTABLE_01", pVar_BloomFlare_Strength);
	DEFINE_SHADEREDITOR_MATERIALVAR(BLOOMFLARE_EDITOR_NAME, "bloomflare", "$MUTABLE_02", pVar_BloomFlare_Scale);

	//if ( g_pPPCtrl != NULL &&
	//	iBloomFlareMode == 1 && !g_pPPCtrl->IsBloomflareEnabled() )
	//	return;

	if (pVar_BloomFlare_Strength == NULL ||
		pVar_BloomFlare_Scale == NULL)
		return;

	pVar_BloomFlare_Strength->SetVecValue(cvar_gstring_bloomflare_strength.GetFloat() * GetSceneFadeScalar(), 0, 0, 0);

	float scale[2];
	scale[0] = (w / (float)h) / (16.0f / 9.0f);
	scale[0] = MIN(1.0f, scale[0]);
	scale[1] = (1.0f - scale[0]) * 0.5f;
	pVar_BloomFlare_Scale->SetVecValue(scale, 2);

	shaderEdit->DrawPPEOnDemand(iBloomFlare);
}

/**
* DESATURATION
*/
void DrawDesaturation(int x, int y, int w, int h)
{
	const float flDesaturationStrength = cvar_gstring_desaturation_strength.GetFloat();

	if (flDesaturationStrength <= 0.0f || !ShouldDrawCommon())
		return;

	static const int iDesaturationIndex = shaderEdit->GetPPEIndex(DESATURATION_EDITOR_NAME);

	if (iDesaturationIndex < 0)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(DESATURATION_EDITOR_NAME, "desaturate", "$MUTABLE_01", pVar_Desaturation_Strength);

	if (pVar_Desaturation_Strength == NULL)
	{
		Assert(0);
		return;
	}

	const float flSinCityMask = 1.0f - MAX(0.0f, flDesaturationStrength - 0.5f) * 2.0f;
	const float flDesatValues[] = { MIN(1.0f, flDesaturationStrength * 2.0f), flSinCityMask };
	pVar_Desaturation_Strength->SetVecValue(flDesatValues, 2);

	shaderEdit->DrawPPEOnDemand(iDesaturationIndex, x, y, w, h);
}

/**
* NIGHTVISION
*/
static float g_flNightvisionBlackFade = 0.0f;
static float g_flNightvisionAmount = 0.0f;
static float g_flNightvisionOverbright = 0.0f;

void SetNightvisionParams(float flBlackFade, float flNightvisionAmount, float flOverbright)
{
	g_flNightvisionBlackFade = flBlackFade;
	g_flNightvisionAmount = flNightvisionAmount;
	g_flNightvisionOverbright = flOverbright;
}

float GetNightvisionMinLighting()
{
	float flAmt = g_flNightvisionAmount - g_flNightvisionBlackFade;
	flAmt *= cvar_gstring_nightvision_minlighting.GetFloat();

	return max(0.0f, flAmt);
}

void DrawNightvision(int x, int y, int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	if (g_flNightvisionBlackFade <= 0.0f
		&& g_flNightvisionAmount <= 0.0f
		&& g_flNightvisionOverbright <= 0.0f)
		return;

	static const int iNightvisionIndex = shaderEdit->GetPPEIndex(NIGHTVISION_EDITOR_NAME);

	if (iNightvisionIndex < 0)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(NIGHTVISION_EDITOR_NAME, "nv calc", "$MUTABLE_01", pVar_Nightvision_Params);

	Assert(pVar_Nightvision_Params);

	if (!pVar_Nightvision_Params)
		return;

	pVar_Nightvision_Params->SetVecValue(1.0f - g_flNightvisionBlackFade,
		g_flNightvisionAmount,
		g_flNightvisionOverbright,
		0);

	shaderEdit->DrawPPEOnDemand(iNightvisionIndex, x, y, w, h);
}

/**
* HURTFX
*/

static int iHealthLast = -1;
static float flAnimationTime = 0.0f;
static float flDecayTime = 0.0f;
static float flHealthAnimationTime = 0.0f;
static Vector vecLastParams = vec3_origin;

void DrawHurtFX(int x, int y, int w, int h)
{
	if (!ShouldDrawCommon())
		return;

	const float flHurtFXEnable = cvar_gstring_drawhurtfx.GetBool() ? 1.0f : 0.0f;
	const float flChromaticAmount = cvar_gstring_chromatic_aberration.GetFloat();

	if (flHurtFXEnable <= 0.0f
		&& flChromaticAmount <= 0.0f)
		return;

	static const int iHurtFXIndex = shaderEdit->GetPPEIndex(HURTFX_EDITOR_NAME);

	if (iHurtFXIndex < 0)
		return;

	DEFINE_SHADEREDITOR_MATERIALVAR(HURTFX_EDITOR_NAME, "hurtfx", "$MUTABLE_01", pVar_HurtFX_Params);

	Assert(pVar_HurtFX_Params);

	if (!pVar_HurtFX_Params)
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return;

	const float flDecayDuration = 0.5f;
	const float flHealthAnimationDuration = 0.8f;
	//const float flChromaticHurtMax = ( flChromaticAmount > 0.0f ) ? 0.02f : 0.0f;

	int iHealthCurrent = pPlayer->GetHealth();
	iHealthCurrent = MAX(0, iHealthCurrent);

	float flChromatic = 0.0f; //RemapValClamped( iHealthCurrent, 0, 25, flChromaticHurtMax, flChromaticAmount );
	float flRedBlend = 0.0f; //RemapValClamped( iHealthCurrent, 0, 25, 2.5f, 0.0f );
	float flHealthBlend = 0.0f;

	if (iHealthLast != iHealthCurrent)
	{
		if (iHealthLast >= 0)
		{
			if (iHealthLast > iHealthCurrent)
			{
				float flIncrement = RemapValClamped(iHealthCurrent, 0, 90, 0.5f, 0.2f);

				flAnimationTime = gpGlobals->curtime + flIncrement;
			}
			else
			{
				flDecayTime = gpGlobals->curtime + flDecayDuration;

				flHealthAnimationTime = gpGlobals->curtime + RemapValClamped(iHealthLast, 0, 90,
					flHealthAnimationDuration, flHealthAnimationDuration * 0.5f);
			}
		}

		iHealthLast = iHealthCurrent;
	}

	float flAdd = flAnimationTime - gpGlobals->curtime;

	if (flAdd > 0.0f)
	{
		flChromatic += flAdd * 0.04f;
		flRedBlend += flAdd * 5.0f;
	}

	if (flHealthAnimationTime > gpGlobals->curtime)
	{
		const float flBlendTime = (flHealthAnimationTime - gpGlobals->curtime) / flHealthAnimationDuration;

		flHealthBlend = powf(flBlendTime, 0.7f) * 2.5f;
	}

	Vector params(flChromatic, flRedBlend, flHealthBlend);

	if (flDecayTime > gpGlobals->curtime)
	{
		const float flBlendTime = (flDecayTime - gpGlobals->curtime) / flDecayDuration;

		params = Lerp(flBlendTime, params, vecLastParams);
	}
	else
	{
		vecLastParams = params;
	}

	if (params.Length() < 0.0001f)
		return;

	pVar_HurtFX_Params->SetVecValue(params.x, params.y * flHurtFXEnable, params.z * flHurtFXEnable);

	shaderEdit->DrawPPEOnDemand(iHurtFXIndex, x, y, w, h);
}

/**
* DoF
*/

bool ShouldDrawDoF()
{
	if (!ShouldDrawCommon())
		return false;
	if (g_pPPCtrl != nullptr && !g_pPPCtrl->IsDoFEnabled() && !gstring_dof_override.GetBool())
		return false;
	return cvar_gstring_drawdof.GetBool();
}

void ClearDoF(int x, int y, int w, int h)
{
	DEFINE_SHADEREDITOR_MATERIALVAR(DOFCLEAR_EDITOR_NAME, "clear", "$MUTABLE_01", pVar_ClearValue);

	// pVar_ClearValue->SetFloatValue( 30000.0f / 192.0f );
	pVar_ClearValue->SetFloatValue(gstring_dof_max_distance.GetFloat() / 192.0f);

	static const int iDofIndex = shaderEdit->GetPPEIndex(DOFCLEAR_EDITOR_NAME);
	shaderEdit->DrawPPEOnDemand(iDofIndex, x, y, w, h);
}

void DrawDoF(int x, int y, int w, int h, StereoEye_t stereoEye)
{
	DEFINE_SHADEREDITOR_MATERIALVAR(DOF_EDITOR_NAME, "coc", "$MUTABLE_01", pVar_DistanceLengthAperature);
	DEFINE_SHADEREDITOR_MATERIALVAR(DOF_EDITOR_NAME, "blur0", "$MUTABLE_01", pVar_BlurRadius0);
	DEFINE_SHADEREDITOR_MATERIALVAR(DOF_EDITOR_NAME, "blur1", "$MUTABLE_01", pVar_BlurRadius1);
	DEFINE_SHADEREDITOR_MATERIALVAR(DOF_EDITOR_NAME, "blur2", "$MUTABLE_01", pVar_BlurRadius2);

	// Use cvar state by default
	float focalLength = gstring_dof_focallength.GetFloat();
	float focalDistance = gstring_dof_focaldistance.GetFloat();
	float aperture = gstring_dof_aperture.GetFloat();
	float radius = gstring_dof_radius.GetFloat();

	const float maxSSAODistance = 4000.0f;
	if (gstring_dof_autofocus.GetBool())
	{
		Vector vecForward;
		AngleVectors(MainViewAngles(), &vecForward);

		trace_t tr;
		UTIL_TraceLine(MainViewOrigin(), MainViewOrigin() + vecForward * MAX_TRACE_LENGTH, MASK_SOLID | MASK_VISIBLE_AND_NPCS,
			C_BasePlayer::GetLocalPlayer(), COLLISION_GROUP_NONE, &tr);

		focalDistance = (tr.endpos - tr.startpos).Length();
		if (tr.DidHit() && tr.surface.flags & SURF_SKY)
		{
			focalDistance = maxSSAODistance;
		}
	}

	// If there is a PPC and we are not overriding, use that instead
	if (!gstring_dof_override.GetBool() && g_pPPCtrl != nullptr)
	{
		radius = g_pPPCtrl->GetDoFRadius();
		focalLength = g_pPPCtrl->GetDoFFocalLength();
		const float autoFocusLerp = g_pPPCtrl->GetDoFAutoFocusInterp();
		focalDistance = Lerp(autoFocusLerp, g_pPPCtrl->GetDoFFocalDistance(), focalDistance);
	}

	focalDistance = MIN(focalDistance, gstring_dof_max_distance.GetFloat());

	// Normalize for SSAO depth
	focalDistance /= maxSSAODistance;

	static float smoothFocalDistance = focalDistance;
	if (stereoEye != STEREO_EYE_RIGHT)
	{
		//smoothFocalDistance = Approach( focalDistance, smoothFocalDistance, gpGlobals->frametime * gstring_dof_smooth_speed.GetFloat() );
		smoothFocalDistance += (focalDistance - smoothFocalDistance) * MIN(1.0f, gpGlobals->frametime * gstring_dof_smooth_speed.GetFloat());
	}

	pVar_DistanceLengthAperature->SetVecValue(smoothFocalDistance, focalLength, aperture);
	pVar_BlurRadius0->SetFloatValue(radius);
	pVar_BlurRadius1->SetFloatValue(radius);
	pVar_BlurRadius2->SetFloatValue(radius);

	static const int iDofIndex = shaderEdit->GetPPEIndex(DOF_EDITOR_NAME);
	shaderEdit->DrawPPEOnDemand(iDofIndex, x, y, w, h);
}

void ResetEffects()
{
	// hurtfx params
	iHealthLast = -1;
	flAnimationTime = 0.0f;
	flDecayTime = 0.0f;
	flHealthAnimationTime = 0.0f;
	vecLastParams = vec3_origin;

	// explosions
	g_hExplosionBlurQueue.RemoveAll();

	// godrays
	SetGodraysColor(Vector(1, 1, 1));
	SetGodraysIntensity(1.0f);
}