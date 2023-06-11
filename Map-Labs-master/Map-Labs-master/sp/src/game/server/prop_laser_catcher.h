#pragma once

#include "cbase.h"
#include "props.h"
#include "Sprite.h"

class CFuncLaserDetector : public CBaseEntity {
public:
	DECLARE_CLASS(CFuncLaserDetector, CBaseEntity);

	~CFuncLaserDetector();

	virtual void Precache();
	virtual void Spawn();

	virtual void AddEmitter(CBaseEntity* emitter);
	virtual void RemoveEmitter(CBaseEntity* emitter);

	virtual void CreateSounds();
	void DestroySounds();

	bool IsActivated() const;

	static CFuncLaserDetector* Create(const Vector& origin, const QAngle& angles, const Vector& mins, const Vector& maxs, CBaseEntity* owner);

	void DebugThink();
private:
	COutputEvent m_OnPowered;
	COutputEvent m_OnUnpowered;

	EHANDLE m_pProp;

	const char* m_szIdleAnimation;
	const char* m_szActiveAnimation;
	const char* m_szPropEntity;

	bool m_bAnimateOnActive;

	bool m_bActivated;

	CSoundPatch* m_pActiveSound;

	CUtlVector<CBaseEntity*> m_LaserList;
	DECLARE_DATADESC();
};

class CPropLaserCatcher : public CDynamicProp {
public:
	DECLARE_CLASS(CPropLaserCatcher, CDynamicProp);

	CPropLaserCatcher();

	virtual void Precache();
	virtual void Spawn();

	void FirePowerOnOutput();
	void FirePowerOffOutput();
private:
	CFuncLaserDetector* m_pLaserDetector;
	CSprite* m_pActivatedSprite;

	const char* m_szIdleAnimation;
	const char* m_szActiveAnimation;

	COutputEvent m_OnPowered;
	COutputEvent m_OnUnpowered;
	DECLARE_DATADESC();
};