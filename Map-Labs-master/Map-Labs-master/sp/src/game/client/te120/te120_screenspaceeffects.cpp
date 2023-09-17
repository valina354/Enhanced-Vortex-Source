#include "cbase.h"
#include "c_te_effect_dispatch.h"
#include "ScreenSpaceEffects.h"

class CTE120ConcEffect : public IScreenSpaceEffect
{
public:
	CTE120ConcEffect(void) :
		m_bEnabled(false) {}

	virtual void Init(void);
	virtual void Shutdown(void);
	virtual void SetParameters(KeyValues* params);
	virtual void Enable(bool bEnable) { m_bEnabled = bEnable; }
	virtual bool IsEnabled() { return m_bEnabled; }

	virtual void Render(int x, int y, int w, int h);

private:
	CMaterialReference m_EffectMaterial;
	bool		m_bEnabled;
};

ADD_SCREENSPACE_EFFECT(CTE120ConcEffect, te120_concussion);

void CTE120ConcEffect::Init(void)
{
}

void CTE120ConcEffect::Shutdown(void)
{
	m_EffectMaterial.Shutdown();
}

void CTE120ConcEffect::SetParameters(KeyValues* params)
{
}

void CTE120ConcEffect::Render(int x, int y, int w, int h)
{
	if (!IsEnabled())
		return;
}

void GravityBallFadeConcCallback(const CEffectData& data)
{
	g_pScreenSpaceEffects->EnableScreenSpaceEffect(&te120_concussion_effect);

	KeyValuesAD kv("params");
	kv->SetFloat("scale", data.m_flScale);
	te120_concussion_effect.SetParameters(kv);
}
DECLARE_CLIENT_EFFECT("CE_GravityBallFadeConcOn", GravityBallFadeConcCallback);