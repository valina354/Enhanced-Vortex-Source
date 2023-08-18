#include "cbase.h"
#include "functionproxy.h"
#include "c_baseanimating.h"


class CLightedMouthProxy : public CResultProxy
{
public:
	virtual void OnBind(void *);
};

void CLightedMouthProxy::OnBind(void *pArg)
{
	IClientRenderable* pRend = (IClientRenderable*)pArg;
	C_BaseEntity* pEnt = BindArgToEntity(pRend);

	if (pEnt && pEnt->GetBaseAnimating())
	{
		float value = pEnt->GetBaseAnimating()->GetMouth()->mouthopen / 64.0f;

		if (value > 1.0)
			value = 1.0;

		value = (1.0 - value);

		SetFloatResult(value);
	}
	else
	{
		SetFloatResult(0);
	}
}

EXPOSE_INTERFACE(CLightedMouthProxy, IMaterialProxy, "LightedMouth" IMATERIAL_PROXY_INTERFACE_VERSION);