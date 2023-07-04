#ifndef C_SUNRAY_EFFECT_H
#define C_SUNRAY_EFFECT_H

#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "view.h"

class C_SunrayEffect : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_SunrayEffect, C_BaseEntity);
	DECLARE_CLIENTCLASS_NOBASE();

	C_SunrayEffect();

	virtual void OnDataChanged(DataUpdateType_t updateType) {}
	virtual bool IsTransparent() { return true; }
	virtual RenderGroup_t GetRenderGroup() { return RENDER_GROUP_TRANSLUCENT_ENTITY; }

	bool Init();
	void Shutdown();
	void Update(float frametime);
	void PreRender();

private:
	C_Sun* m_pSunEntity;
	int m_iSunAttachmentIndex;
};

#endif // C_SUNRAY_EFFECT_H
