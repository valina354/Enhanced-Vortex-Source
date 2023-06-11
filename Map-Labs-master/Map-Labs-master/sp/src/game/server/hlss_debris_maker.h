#ifndef HLSS_DEBRIS_MAKER_H
#define HLSS_DEBRIS_MAKER_H
#ifdef _WIN32
#pragma once
#endif


#include "cbase.h"
#include "hl2_player.h"
#include "hl2_gamerules.h"
#include "ai_utils.h"
#include "props.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHLSS_Debris_Maker : public CPointEntity
{
	DECLARE_CLASS(CHLSS_Debris_Maker, CPointEntity);

public:
	CHLSS_Debris_Maker();

	virtual void Spawn(void);
	void DebrisMakerThink(void);

	static void Create(CPhysicsProp *pProp); //edict_t *pentOwner );

	bool ShouldMakeDebris(CBaseEntity *pEntity);

	EHANDLE			m_hDebrisTarget;
	bool			m_bNormalDoor;

	DECLARE_DATADESC();
};

#endif