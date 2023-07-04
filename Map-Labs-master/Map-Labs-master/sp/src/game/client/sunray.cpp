#include "cbase.h"
#include "c_sunray_effect.h"
#include "c_sun.h"
#include "view.h"
#include "tier0/memdbgon.h"

ConVar cl_sunray_enabled("cl_sunray_enabled", "1", FCVAR_ARCHIVE, "Enable/disable sunray effect");
ConVar cl_sunray_brightness("cl_sunray_brightness", "1.0", FCVAR_ARCHIVE, "Sunray brightness");
ConVar cl_sunray_density("cl_sunray_density", "50", FCVAR_ARCHIVE, "Sunray density");

static C_SunrayEffect g_SunrayEffect;

void SunRayEffect_PreRender()
{
	if (!cl_sunray_enabled.GetBool())
		return;

	CViewSetup view;
	view.Init(ViewEntity());
	g_SunrayEffect.PreRender();
}

void SunRayEffect_Shutdown()
{
	g_SunrayEffect.Shutdown();
}

void SunRayEffect_Update(float frametime)
{
	if (!cl_sunray_enabled.GetBool())
		return;

	g_SunrayEffect.Update(frametime);
}

void SunRayEffect_Init()
{
	if (!cl_sunray_enabled.GetBool())
		return;

	g_SunrayEffect.Init();
}

static CViewRender g_ViewRender;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CViewRender, IViewRender, VIEWRENDER_INTERFACE_VERSION, g_ViewRender);

void C_SunrayEffect::PreRender()
{
	if (!m_pSunEntity)
		return;

	Vector vSunRayColor = m_pSunEntity->m_clrOverlay.ToVector();
	vSunRayColor *= cl_sunray_brightness.GetFloat();

	float flDensity = cl_sunray_density.GetFloat();
	float flAspectRatio = static_cast<float>(view.width) / view.height;

	Vector vRayOrigin;
	Vector vRayDirection;

	m_pSunEntity->GetAttachment(m_iSunAttachmentIndex, vRayOrigin, vRayDirection);

	float flRayLength = (view.origin - vRayOrigin).Length();

	Vector vOrthoX, vOrthoY;
	CrossProduct(vRayDirection, Vector(0, 0, 1), vOrthoX);
	VectorNormalize(vOrthoX);
	CrossProduct(vRayDirection, vOrthoX, vOrthoY);
	VectorNormalize(vOrthoY);

	float flWidth = tan(0.5f * DEG2RAD(view.fov)) * flRayLength * 2.0f;
	float flHeight = flWidth / flAspectRatio;

	flWidth *= 0.5f;
	flHeight *= 0.5f;

	Vector vSunRayStart = vRayOrigin + vOrthoX * -flWidth + vOrthoY * -flHeight;
	Vector vSunRayEnd = vRayOrigin + vOrthoX * flWidth + vOrthoY * flHeight;

	CMatRenderContextPtr pRenderContext(materials);
	IMesh* pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, m_iSunAttachmentIndex);
	CMeshBuilder meshBuilder;
	meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 1);

	meshBuilder.Position3fv(vSunRayStart.Base());
	meshBuilder.Color4ubv(reinterpret_cast<unsigned char*>(&vSunRayColor));
	meshBuilder.TexCoord2f(0, 0.5f, 0.5f);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3fv(vSunRayEnd.Base());
	meshBuilder.Color4ubv(reinterpret_cast<unsigned char*>(&vSunRayColor));
	meshBuilder.TexCoord2f(0, 0.5f, 0.5f);
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();

	pRenderContext->Flush();
}

void C_SunrayEffect::Update(float frametime)
{
	C_BaseEntity* pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pLocalPlayer)
		return;

	m_pSunEntity = dynamic_cast<C_Sun*>(pLocalPlayer->GetEntityByName("sun"));
}

void C_SunrayEffect::Shutdown()
{
	if (!m_pSunEntity)
		return;

	m_pSunEntity->RemoveClientOverride("sunray");
}

bool C_SunrayEffect::Init()
{
	m_iSunAttachmentIndex = LookupAttachment("attach_sunglow");
	if (m_iSunAttachmentIndex == -1)
		return false;

	return true;
}
