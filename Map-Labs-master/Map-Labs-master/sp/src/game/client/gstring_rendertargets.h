#ifndef GSTRING_RENDERTARGETS_H
#define GSTRING_RENDERTARGETS_H

#include "baseclientrendertargets.h"

class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CGstringRenderTargets : public CBaseClientRenderTargets
{
	DECLARE_CLASS_GAMEROOT(CGstringRenderTargets, CBaseClientRenderTargets);

public:
	virtual void InitClientRenderTargets(IMaterialSystem *pMaterialSystem, IMaterialSystemHardwareConfig *pHardwareConfig);
	virtual void ShutdownClientRenderTargets();

	ITexture *GetHoloGUITexture();

private:
	CTextureReference m_HoloGUITexture;
};

extern CGstringRenderTargets *g_pGstringRenderTargets;

#endif