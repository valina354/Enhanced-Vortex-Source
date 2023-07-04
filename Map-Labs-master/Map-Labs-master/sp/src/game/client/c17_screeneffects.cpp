#include "cbase.h"
#include "screenspaceeffects.h"
#include "rendertexture.h"
#include "model_types.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imaterialvar.h"
#include "cdll_client_int.h"
#include "materialsystem/itexture.h"
#include "keyvalues.h"
#include "ClientEffectPrecacheSystem.h"
#include "viewrender.h"
#include "view_scene.h"
#include "c_basehlplayer.h"
#include "tier0/vprof.h"
#include "view.h"
#include "hl2_gamerules.h"

#include "c17_screeneffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar fov_desired;
extern CViewRender g_DefaultViewRender;
extern bool g_bRenderingScreenshot;

ConVar r_post_sunshaft_blur("r_post_sunshaft_blur", "1", FCVAR_ARCHIVE);
ConVar r_post_sunshaft_blur_amount("r_post_sunshaft_blur_amount", "0.5", FCVAR_CHEAT);

extern ConVar r_post_sunshaft;
ConVar r_post_sunshaft_debug("r_post_sunshaft_debug", "0", FCVAR_CHEAT);

bool CSunShaftEffect::ShaftsRendering(void)
{
	return (r_post_sunshaft.GetBool() && engine->IsSkyboxVisibleFromPoint(CurrentViewOrigin()) && IsEnabled());
}

static void SetRenderTargetAndViewPort(ITexture *rt)
{
	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->SetRenderTarget(rt);
	pRenderContext->Viewport(0, 0, rt->GetActualWidth(), rt->GetActualHeight());
}


