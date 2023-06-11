//=================== Half-Life 2: Short Stories Mod 2007 =====================//
//
// Purpose:	The Pit Monster - Xen Movement Limiter
//
//=============================================================================//


#include "cbase.h"
#include "basehlcombatweapon.h"
#include "basecombatcharacter.h"
#include "Sprite.h"
#include "explode.h"

#include "physics_prop_ragdoll.h"
#include "physics.h"
#include "vphysics_interface.h"
#include "physobj.h"

#include "tier0/memdbgon.h"

#define XENPLANT_GLOW_SPRITE	"sprites/glow1.vmt"

enum XenPlant_animations_enum
{
	Anim_idle01 = 0,
	Anim_hide01,
	Anim_unhide01,
	Anim_pain01,
	NUM_XENPLANT_ANIMATIONS,
};

char *XenPlantAnimations[NUM_XENPLANT_ANIMATIONS] =
{
	"idle01",
	"hide01",
	"unhide01",
	"pain01",
};


class CXenPlant : public CBaseCombatCharacter//, virtual public CBaseAnimating OR BaseCombatCharacter
{
	DECLARE_CLASS(CXenPlant, CBaseCombatCharacter);

public:
	CXenPlant();
	~CXenPlant();

	Class_T Classify(void) { return CLASS_BEE; }

	void	Spawn(void);
	void	Precache(void);

	bool	CreateVPhysics(void);
	void	CreateBoneFollowers();
	void	Touch(CBaseEntity *pOther);
	void	OnRestore(void);
	void	SetParent(CBaseEntity *pNewParent, int iAttachment);
	void	BoneFollowerHierarchyChanged();


	void	TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr);

	void	PlantThink(void);

	bool	PlayAnimation(int animation);

	//void	InputClose( inputdata_t &inputdata );
	void	InputEnableHide(inputdata_t &inputdata);
	void	InputDisableHide(inputdata_t &inputdata);

	void	UpdateOnRemove();

	void	Hide(float flTime);

private:

	//bool	m_bClosed;
	bool				m_bCanHide;
	int					m_iHideDistance;

	CSprite				*m_pBulbGlow;
	//CSprite			*m_pBulb2Glow;

	COutputEvent		m_OnHide;
	COutputEvent		m_OnUnhide;

	void				CreateBulbGlows();
	void				ShowBulbGlows(bool bHide);

	int					m_iAnimation;

	float				m_flShouldPainHideTime;
	float				m_flHideCheckTime;

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

protected:
	CBoneFollowerManager	m_BoneFollowerManager;
};

BEGIN_DATADESC(CXenPlant)
//	DEFINE_KEYFIELD( m_bClosed,				FIELD_BOOLEAN, "closed" ),
DEFINE_KEYFIELD(m_bCanHide, FIELD_BOOLEAN, "canhide"),

DEFINE_FIELD(m_iAnimation, FIELD_INTEGER),
DEFINE_KEYFIELD(m_iHideDistance, FIELD_INTEGER, "hidedistance"),
DEFINE_FIELD(m_flShouldPainHideTime, FIELD_TIME),
DEFINE_FIELD(m_flHideCheckTime, FIELD_TIME),

// Function Pointers
DEFINE_FUNCTION(PlantThink),

//	DEFINE_INPUTFUNC( FIELD_VOID, "Close", InputClose ),
DEFINE_INPUTFUNC(FIELD_VOID, "EnableHide", InputEnableHide),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableHide", InputDisableHide),

DEFINE_OUTPUT(m_OnHide, "OnHide"),
DEFINE_OUTPUT(m_OnUnhide, "OnUnhide"),

DEFINE_EMBEDDED(m_BoneFollowerManager),

END_DATADESC()
LINK_ENTITY_TO_CLASS(prop_xenplant, CXenPlant);

IMPLEMENT_SERVERCLASS_ST(CXenPlant, DT_XenPlant)
END_SEND_TABLE()


CXenPlant::CXenPlant()
{
	//m_bClosed=false;
	m_bCanHide = true;
	m_iHideDistance = 128;
}

CXenPlant::~CXenPlant()
{
}

//-----------------------------------------------------------------------------
// On remove!
//-----------------------------------------------------------------------------
void CXenPlant::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	m_BoneFollowerManager.DestroyBoneFollowers();

	if (m_pBulbGlow)
	{
		UTIL_Remove(m_pBulbGlow);
		m_pBulbGlow = NULL;
	}
	/*if ( m_pBulb2Glow )
	{
	UTIL_Remove( m_pBulb2Glow );
	m_pBulb2Glow = NULL;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*void CXenPlant::InputClose( inputdata_t &inputdata )
{
m_bClosed = true;
}*/

void CXenPlant::InputEnableHide(inputdata_t &inputdata)
{
	m_bCanHide = true;
}

void CXenPlant::InputDisableHide(inputdata_t &inputdate)
{
	m_bCanHide = false;
}

void CXenPlant::Precache(void)
{
	PrecacheModel(STRING(GetModelName()));
	PrecacheModel(XENPLANT_GLOW_SPRITE);
}

bool CXenPlant::CreateVPhysics(void)
{
	// Create the object in the physics system
	//VPhysicsInitNormal( SOLID_VPHYSICS, 0, false );
	CreateBoneFollowers();

	return true;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CXenPlant::Touch(CBaseEntity *pOther)
{
	BaseClass::Touch(pOther);

	if (pOther->VPhysicsGetObject())
	{
		Vector vecVelocity;
		AngularImpulse angImpulse;

		pOther->VPhysicsGetObject()->GetVelocity(&vecVelocity, &angImpulse);

		//DevMsg("hit entity speed %f\n", vecVelocity.Length());

		if (vecVelocity.Length() > 80)
		{
			Hide(random->RandomFloat(1, 2));
		}
	}
}

void CXenPlant::CreateBoneFollowers()
{
	// already created bone followers?  Don't do so again.
	if (m_BoneFollowerManager.GetNumBoneFollowers())
		return;

	KeyValues *modelKeyValues = new KeyValues("");
	if (modelKeyValues->LoadFromBuffer(modelinfo->GetModelName(GetModel()), modelinfo->GetModelKeyValueText(GetModel())))
	{
		// Do we have a bone follower section?
		KeyValues *pkvBoneFollowers = modelKeyValues->FindKey("bone_followers");
		if (pkvBoneFollowers)
		{
			// Loop through the list and create the bone followers
			KeyValues *pBone = pkvBoneFollowers->GetFirstSubKey();
			while (pBone)
			{
				// Add it to the list
				const char *pBoneName = pBone->GetString();
				m_BoneFollowerManager.AddBoneFollower(this, pBoneName);

				pBone = pBone->GetNextKey();
			}
		}

		modelKeyValues->deleteThis();
	}

	// if we got here, we don't have a bone follower section, but if we have a ragdoll
	// go ahead and create default bone followers for it
	if (m_BoneFollowerManager.GetNumBoneFollowers() == 0)
	{
		vcollide_t *pCollide = modelinfo->GetVCollide(GetModelIndex());
		if (pCollide && pCollide->solidCount > 1)
		{
			CreateBoneFollowersFromRagdoll(this, &m_BoneFollowerManager, pCollide);
		}
	}
}

void CXenPlant::Spawn(void)
{
	Precache();

	BaseClass::Spawn();

	SetSolid(SOLID_VPHYSICS);
	//AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetModel(STRING(GetModelName()));

	SetMoveType(MOVETYPE_NOCLIP);
	//SetCollisionGroup( COLLISION_GROUP_INTERACTIVE );;

	m_takedamage = DAMAGE_EVENTS_ONLY;
	m_bloodColor = BLOOD_COLOR_YELLOW;

	m_iAnimation = Anim_unhide01;

	int nSequence = LookupSequence(XenPlantAnimations[m_iAnimation]);

	SetCycle(0);
	m_flAnimTime = gpGlobals->curtime;
	SetAnimatedEveryTick(true);
	ResetSequence(nSequence);
	ResetClientsideFrame();

	m_flShouldPainHideTime = 0;

	m_flHideCheckTime = gpGlobals->curtime + random->RandomFloat(0.0f, 1.0f);

	CreateVPhysics();

	BoneFollowerHierarchyChanged();

	ShowBulbGlows(false);

	SetThink(&CXenPlant::PlantThink);

	SetNextThink(gpGlobals->curtime);
}

void CXenPlant::ShowBulbGlows(bool bHide)
{
	CreateBulbGlows();

	if (m_pBulbGlow)
	{
		if (bHide)
		{	//We Show them_pBulb1Glow
			m_pBulbGlow->SetRenderColorA(1);
			m_pBulbGlow->SetTransparency(kRenderTransAdd, 228, 232, 113, 128, kRenderFxNoDissipation);

			EntityMessageBegin(this, true);
			WRITE_BYTE(1);
			MessageEnd();
		}
		else
		{	//We Hide the LightGlow
			m_pBulbGlow->SetRenderMode(kRenderTransColor);
			m_pBulbGlow->SetRenderColorA(0);

			EntityMessageBegin(this, true);
			WRITE_BYTE(0);
			MessageEnd();
		}
	}

	/*if (m_pBulb2Glow)
	{
	if (bHide)
	{	//We Show them_pBulb1Glow
	m_pBulb2Glow->SetRenderColorA(1);
	m_pBulb2Glow->SetTransparency( kRenderTransAdd, 255, 255, 255, 128, kRenderFxNoDissipation );
	m_pBulb2Glow->SetColor( 255, 255, 255 );
	} else
	{	//We Hide the LightGlow
	m_pBulb2Glow->SetRenderMode(kRenderTransColor );
	m_pBulb2Glow->SetRenderColorA(0);
	}
	}*/
}

void CXenPlant::CreateBulbGlows()
{
	if (m_pBulbGlow == NULL)
	{
		m_pBulbGlow = CSprite::SpriteCreate(XENPLANT_GLOW_SPRITE, GetLocalOrigin(), false);
		m_pBulbGlow->SetAttachment(this, LookupAttachment("Bulb"));


		m_pBulbGlow->SetTransparency(kRenderTransAdd, 228, 232, 113, 128, kRenderFxNoDissipation);
		//m_pBulbGlow->SetColor( 255, 0, 255 );

		m_pBulbGlow->SetBrightness(128, 0.1f);
		m_pBulbGlow->SetScale(0.50f, 0.2f);
		m_pBulbGlow->SetAsTemporary();
	}
}

bool CXenPlant::PlayAnimation(int animation)
{
	if (animation < 0 || animation >= NUM_XENPLANT_ANIMATIONS)
	{
		DevWarning("Xen Plant %s, invalid animation index\n", GetDebugName());
		return false;
	}

	if (animation == m_iAnimation)
	{
		return false;
	}

	if ((m_iAnimation != Anim_idle01) && !IsSequenceFinished()) //&& m_iAnimation != Anim_blocked_idle01
	{
		//DevMsg( XenPlantAnimations[m_iAnimation] );
		return false;
	}

	//DevMsg( XenPlantAnimations[animation] );

	m_iAnimation = animation;

	ResetSequence(LookupSequence(XenPlantAnimations[animation]));
	return true;
}

void CXenPlant::TraceAttack(const CTakeDamageInfo &inputInfo, const Vector &vecDir, trace_t *ptr)
{
	Hide(random->RandomFloat(2, 3));

	BaseClass::TraceAttack(inputInfo, vecDir, ptr);
}

void CXenPlant::Hide(float flTime)
{
	float flShouldPainHideTime = gpGlobals->curtime + flTime;
	if (flShouldPainHideTime > (m_flShouldPainHideTime - 0.9f))
	{
		m_flShouldPainHideTime = flShouldPainHideTime;
	}

	m_flHideCheckTime = 0;

	if (m_iAnimation == Anim_idle01)
	{
		if (PlayAnimation(Anim_hide01))
		{
			ShowBulbGlows(false);
			m_OnHide.FireOutput(this, this);

		}
	}
	else if (m_iAnimation == Anim_hide01)
	{
		PlayAnimation(Anim_pain01);
		//TERO: we mark it as hide animation so that we can repeat it
		m_iAnimation = Anim_hide01;
	}
}

void CXenPlant::PlantThink(void)
{
	if (m_iAnimation == Anim_unhide01 && IsSequenceFinished())
	{
		ShowBulbGlows(true);
		PlayAnimation(Anim_idle01);
	}
	else
	{
		CBaseEntity *pEntity = NULL;

		Vector origin = GetAbsOrigin();
		float radius = (float)m_iHideDistance;

		// Find anything within our radius
		if (m_flHideCheckTime < gpGlobals->curtime)
		{
			bool bShouldHide = (m_flShouldPainHideTime > gpGlobals->curtime);

			m_flHideCheckTime = gpGlobals->curtime + 0.4f;

			while (m_bCanHide && !bShouldHide && (pEntity = gEntList.FindEntityInSphere(pEntity, origin, radius)) != NULL)
			{
				if ((pEntity->IsPlayer() || pEntity->IsNPC()) &&
					pEntity->Classify() != CLASS_NONE &&
					pEntity->Classify() != CLASS_BEE &&
					pEntity->Classify() != CLASS_FLARE &&
					pEntity->Classify() != CLASS_CONSCRIPT)
				{
					bShouldHide = true;
				}
			}

			if (bShouldHide)
			{
				if (m_iAnimation == Anim_idle01)
				{
					if (PlayAnimation(Anim_hide01))
					{
						ShowBulbGlows(false);
						m_OnHide.FireOutput(this, this);

					}
				}
			}
			else
			{
				if (m_iAnimation == Anim_hide01 || m_iAnimation == Anim_pain01)
				{
					if (PlayAnimation(Anim_unhide01))
					{
						m_OnUnhide.FireOutput(this, this);
					}
				}
			}
		}

		/*CBasePlayer *pPlayer = UTIL_GetLocalPlayer();

		if (pPlayer)
		{
		float flDist = UTIL_DistApprox2D( GetAbsOrigin(), pPlayer->GetAbsOrigin() );

		if (flDist < m_iHideDistance || bShouldHide )
		{
		if (m_iAnimation == Anim_idle01)
		{
		if (PlayAnimation( Anim_hide01 ))
		{
		ShowBulbGlows(false);
		m_OnHide.FireOutput( this, this );

		}
		}

		} else
		{
		if (m_iAnimation == Anim_hide01 || m_iAnimation == Anim_pain01 )
		{
		if (PlayAnimation( Anim_unhide01 ))
		{
		m_OnUnhide.FireOutput( this, this );
		}
		}

		}
		}*/


	}

	SetSimulationTime(gpGlobals->curtime);

	StudioFrameAdvance();

	m_BoneFollowerManager.UpdateBoneFollowers(this);

	// Think as soon as possible
	SetNextThink(gpGlobals->curtime); //TÄHÄN JOKU PITEMPI AIKA
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CXenPlant::OnRestore(void)
{
	BaseClass::OnRestore();

	BoneFollowerHierarchyChanged();

	CreateBulbGlows();

	if (m_iAnimation == Anim_idle01)
	{
		ShowBulbGlows(true);
	}
	else
	{
		ShowBulbGlows(false);
	}
}

void CXenPlant::SetParent(CBaseEntity *pNewParent, int iAttachment)
{
	BaseClass::SetParent(pNewParent, iAttachment);
	BoneFollowerHierarchyChanged();
}

// Call this when creating bone followers or changing hierarchy to make sure the bone followers get updated when hierarchy changes
void CXenPlant::BoneFollowerHierarchyChanged()
{
	// If we have bone followers and we're parented to something, we need to constantly update our bone followers
	if (m_BoneFollowerManager.GetNumBoneFollowers() && GetParent())
	{
		WatchPositionChanges(this, this);
	}
}