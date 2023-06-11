//=================== Half-Life 2: Short Stories Mod 2008 =====================//
//
// Purpose:	Sets wanted vectors of Abs Angles and Abs Origin to another entity
//			by Au-heppa
//
//=============================================================================//

#include "cbase.h"
#include "entityinput.h"
#include "entityoutput.h"
#include "eventqueue.h"
#include "soundent.h"
#include "logicrelay.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CHLSSCopyAbs : public CLogicalEntity
{
public:
	DECLARE_CLASS(CHLSSCopyAbs, CLogicalEntity);
	DECLARE_DATADESC();

private:

	string_t		m_CopyTarget;
	string_t		m_CopySource;

	void SetAbs(inputdata_t &inputdata);
};

LINK_ENTITY_TO_CLASS(hlss_abs_copy, CHLSSCopyAbs);

BEGIN_DATADESC(CHLSSCopyAbs)
DEFINE_KEYFIELD(m_CopyTarget, FIELD_STRING, "copy_target"),
DEFINE_KEYFIELD(m_CopySource, FIELD_STRING, "copy_source"),
DEFINE_INPUTFUNC(FIELD_VOID, "SetAbs", SetAbs),
END_DATADESC()

const int SF_COPY_ORIGIN_X = 0x001;
const int SF_COPY_ORIGIN_Y = 0x002;
const int SF_COPY_ORIGIN_Z = 0x003;

const int SF_COPY_ANGLE_X = 0x004;
const int SF_COPY_ANGLE_Y = 0x005;
const int SF_COPY_ANGLE_Z = 0x006;

//-----------------------------------------------------------------------------
// Purpose: Sets the wanted angles and origin
//-----------------------------------------------------------------------------
void CHLSSCopyAbs::SetAbs(inputdata_t &inputdata)
{
	CBaseEntity *pTarget = dynamic_cast<CBaseEntity *>(gEntList.FindEntityByName(NULL, m_CopyTarget));
	CBaseEntity *pSource = dynamic_cast<CBaseEntity *>(gEntList.FindEntityByName(NULL, m_CopySource));

	if (pTarget)
	{
		if (!pSource)
			pSource = this;

		Vector vecTarget = pTarget->GetAbsOrigin();

		if (m_spawnflags & SF_COPY_ORIGIN_X)
			vecTarget.x = pSource->GetAbsOrigin().x;

		if (m_spawnflags & SF_COPY_ORIGIN_Y)
			vecTarget.y = pSource->GetAbsOrigin().y;

		if (m_spawnflags & SF_COPY_ORIGIN_Z)
			vecTarget.z = pSource->GetAbsOrigin().z;

		pTarget->SetAbsOrigin(vecTarget);

		QAngle angTarget = pTarget->GetAbsAngles();

		if (m_spawnflags & SF_COPY_ANGLE_X)
			angTarget.x = pSource->GetAbsAngles().x;

		if (m_spawnflags & SF_COPY_ANGLE_Y)
			angTarget.y = pSource->GetAbsAngles().y;

		if (m_spawnflags & SF_COPY_ANGLE_Z)
			angTarget.z = pSource->GetAbsAngles().z;

		pTarget->SetAbsAngles(angTarget);
	}
	else
		DevMsg("npc_abs_copy, target entity %s not found\n", m_CopyTarget);
}