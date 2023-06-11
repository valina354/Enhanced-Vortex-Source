#ifndef ITEM_JETPACK_H
#define ITEM_JETPACK_H

#ifdef _WIN32
#pragma once
#endif

#include "baseanimating.h"

class CItemJetpack : public CBaseAnimating
{
public:
	DECLARE_CLASS(CItemJetpack, CBaseAnimating);
	DECLARE_DATADESC();

	CItemJetpack();
	void Spawn() override;
	void Precache() override;
	bool KeyValue(const char* szKeyName, const char* szValue) override;
	void ActivateJetpack(CBasePlayer* pPlayer);
};

#endif  // ITEM_JETPACK_H
