#ifndef C17_SCREENSPACEEFFECTS_H
#define C17_SCREENSPACEEFFECTS_H
#ifdef _WIN32
#pragma once
#endif

#include "ScreenSpaceEffects.h"

class CSunShaftEffect : public IScreenSpaceEffect
{
public:
	CSunShaftEffect(void) { };

	virtual void Init(void);
	virtual void Shutdown(void);
	virtual void SetParameters(KeyValues *params) {};
	virtual void Enable(bool bEnable) { m_bEnabled = bEnable; }

	virtual bool IsEnabled() { return m_bEnabled; }
	virtual bool ShaftsRendering(void);

	virtual void Render(int x, int y, int w, int h);

private:
	bool				m_bEnabled;

	CMaterialReference	m_SunShaft_BlurX;
	CMaterialReference	m_SunShaft_BlurY;

	CMaterialReference	m_SunShaftBlendMat;
	CMaterialReference	m_SunShaftMask;
	CMaterialReference	m_SunShaftDebug;
};

#endif