
#include "cbase.h"
#include "gstring_rendertargets.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"

void CGstringRenderTargets::InitClientRenderTargets(IMaterialSystem *pMaterialSystem, IMaterialSystemHardwareConfig *pHardwareConfig)
{
	// 'RT_SIZE_LITERAL' - Fucking. Lol.
	m_HoloGUITexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_HoloGUI",
		1024, 1024, RT_SIZE_LITERAL,
		IMAGE_FORMAT_RGB888,
		MATERIAL_RT_DEPTH_NONE,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR));

	BaseClass::InitClientRenderTargets(pMaterialSystem, pHardwareConfig);
}

void CGstringRenderTargets::ShutdownClientRenderTargets()
{
	m_HoloGUITexture.Shutdown();

	BaseClass::ShutdownClientRenderTargets();
}

ITexture *CGstringRenderTargets::GetHoloGUITexture()
{
	return m_HoloGUITexture;
}

static CGstringRenderTargets g_GstringRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGstringRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_GstringRenderTargets);
CGstringRenderTargets *g_pGstringRenderTargets = &g_GstringRenderTargets;