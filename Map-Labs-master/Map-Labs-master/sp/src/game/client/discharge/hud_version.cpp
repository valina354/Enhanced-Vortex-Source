//====== Copyright © 2015, Discharge Team, Some rights reserved. =======
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// battery.cpp
//
// implementation of CHudVersion class
//
#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"

#include "vgui_controls/AnimationController.h"
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Displays suit power (armor) on hud
//-----------------------------------------------------------------------------
class CHudVersion : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudVersion, CHudNumericDisplay );

public:
	CHudVersion( const char *pElementName );
	void Init( void );
	void Reset( void );
	void VidInit( void );
};

DECLARE_HUDELEMENT( CHudVersion );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudVersion::CHudVersion( const char *pElementName ) : BaseClass(NULL, "HudVersion"), CHudElement( pElementName )
{

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudVersion::Init( void )
{
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudVersion::Reset( void )
{				//Major Minor Release Build
	SetLabelText(L"Alpha 0.0.1.104");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudVersion::VidInit( void )
{
	Reset();
}
