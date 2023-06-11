//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	Takes away induvidual ammo from the player
//			by Au-heppa
//
//=============================================================================//


#include "cbase.h"
#include "hl2_gamerules.h"
#include "hl2_player.h"
#include "ammodef.h"
#include "hl2_shareddefs.h"
#include "globalstate.h"
#include "game.h"


#include "game.h"
#include "engine/IEngineSound.h"
#include "KeyValues.h"
#include "ai_basenpc.h"
#include "AI_Criteria.h"
#include "isaverestore.h"
#include "sceneentity.h"

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

class CHLSSWeaponStripper : public CLogicalEntity
{
public:
	DECLARE_CLASS(CHLSSWeaponStripper, CLogicalEntity);
	DECLARE_DATADESC();

private:

	void TakeAmmo(inputdata_t &inputdata);
	void TakeAllAmmo(inputdata_t &inputdata);

	COutputEvent		m_AmmoTaken;
	COutputEvent		m_NoAmmo;
};

LINK_ENTITY_TO_CLASS(hlss_weaponstripper, CHLSSWeaponStripper);

BEGIN_DATADESC(CHLSSWeaponStripper)

DEFINE_INPUTFUNC(FIELD_STRING, "TakeAmmo", TakeAmmo),
DEFINE_INPUTFUNC(FIELD_STRING, "TakeAllAmmo", TakeAllAmmo),
DEFINE_OUTPUT(m_AmmoTaken, "AmmoRemoved"),
DEFINE_OUTPUT(m_NoAmmo, "NoAmmo"),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Takes one ammo of the type
//-----------------------------------------------------------------------------
void CHLSSWeaponStripper::TakeAmmo(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if (pPlayer)
	{
		string_t iszAmmoType = inputdata.value.StringID();
		if (iszAmmoType == NULL_STRING)
		{
			DevMsg("NULL string for ammo type\n");
			return;
		}

		int iAmmoType = GetAmmoDef()->Index(iszAmmoType.ToCStr());
		if (iAmmoType == -1)
		{
			DevMsg("Undefined ammotype\n");
			return;
		}

		if (pPlayer->GetAmmoCount(iAmmoType) > 0)
		{
			pPlayer->RemoveAmmo(1, iAmmoType);
			m_AmmoTaken.FireOutput(pPlayer, this, 0);
		}
		else
		{
			//There's no ammo
			m_NoAmmo.FireOutput(pPlayer, this, 0);
		}

	}
}

//-----------------------------------------------------------------------------
// Purpose: Takes all ammo of the type
//-----------------------------------------------------------------------------
void CHLSSWeaponStripper::TakeAllAmmo(inputdata_t &inputdata)
{
	CBasePlayer *pPlayer = AI_GetSinglePlayer();
	if (pPlayer)
	{
		string_t iszAmmoType = inputdata.value.StringID();
		if (iszAmmoType == NULL_STRING)
		{
			DevMsg("NULL string for ammo type\n");
			return;
		}

		int iAmmoType = GetAmmoDef()->Index(iszAmmoType.ToCStr());
		if (iAmmoType == -1)
		{
			DevMsg("Undefined ammotype\n");
			return;
		}

		if (pPlayer->GetAmmoCount(iAmmoType) > 0)
		{
			//There is ammo
			pPlayer->RemoveAmmo(GetAmmoDef()->MaxCarry(iAmmoType), iAmmoType);
			m_AmmoTaken.FireOutput(pPlayer, this, 0);
		}
		else
		{
			//There's no ammo
			m_NoAmmo.FireOutput(pPlayer, this, 0);
		}

	}
}


class CSetOwnerEntity : public CLogicalEntity
{
public:
	DECLARE_CLASS(CHLSSWeaponStripper, CLogicalEntity);
	DECLARE_DATADESC();

private:

	void TakeAmmo(inputdata_t &inputdata);
	void TakeAllAmmo(inputdata_t &inputdata);

	COutputEvent		m_AmmoTaken;
};