
#include "cbase.h"
#include "c_weapon__stubs.h"
#include "c_basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"

class C_Weapon_Camera : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_Weapon_Camera, C_BaseHLCombatWeapon);
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	//virtual void ClientThink( void );

	virtual bool IsWeaponCamera() { return ((m_flDrawScreenTime == 0) || (m_flDrawScreenTime < gpGlobals->curtime)); }

	virtual bool ShouldDrawCrosshair(void) { return false; }

private:

	float m_flDrawScreenTime;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_camera, C_Weapon_Camera);

IMPLEMENT_CLIENTCLASS_DT(C_Weapon_Camera, DT_Weapon_Camera, CWeapon_Camera)
RecvPropFloat(RECVINFO(m_flDrawScreenTime)),
END_RECV_TABLE()