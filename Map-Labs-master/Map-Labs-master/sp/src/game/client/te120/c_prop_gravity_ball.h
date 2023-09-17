//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef CPROPGRAVITYBALL_H_
#define CPROPGRAVITYBALL_H_

#ifdef _WIN32
#pragma once
#endif

#include "c_prop_combine_ball.h"

class C_PropGravityBall : public C_PropCombineBall
{
	DECLARE_CLASS( C_PropGravityBall, C_PropCombineBall );
	DECLARE_CLIENTCLASS();

protected:
	virtual bool	InitMaterials( void );
};


#endif