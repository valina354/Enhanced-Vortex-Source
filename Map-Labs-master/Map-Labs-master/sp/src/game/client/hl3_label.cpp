#include "cbase.h"
#include "cdll_int.h"
#include "engine/IVDebugOverlay.h"
#include "engine/IVModelInfo.h"

class CHL3Label : public C_BaseEntity
{
public:
	DECLARE_CLASS(CHL3Label, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	CHL3Label()
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}

	void ClientThink()
	{
		BaseClass::ClientThink();
	}

	virtual int DrawModel(int flags)
	{
		// Don't draw the model if the text is disabled
		if (!hl3_show_text.GetBool())
			return 0;

		return BaseClass::DrawModel(flags);
	}

	void OnDataChanged(DataUpdateType_t type)
	{
		BaseClass::OnDataChanged(type);

		// Set the entity's model to an invisible model
		SetModel("models/invisible.mdl");
	}

	void FireGameEvent(IGameEvent* event)
	{
		const char* eventName = event->GetName();

		if (Q_strcmp(eventName, "hl3_label_text_change") == 0)
		{
			const char* newText = event->GetString("text");
			hl3_text.SetValue(newText);
		}
	}

private:
	CNetworkVar(bool, m_bEnabled);
};

IMPLEMENT_CLIENTCLASS_DT(CHL3Label, DT_HL3Label, CHL3Label)
RecvPropBool(RECVINFO(m_bEnabled)),
END_RECV_TABLE()

ConVar hl3_show_text("hl3_show_text", "1", FCVAR_ARCHIVE, "Toggle HL3 text display on/off.");
ConVar hl3_text("hl3_text", "HL3 Pre-alpha 0.01", FCVAR_ARCHIVE, "Set the HL3 text.");

void InitializeHL3Label()
{
	ListenForGameEvent("hl3_label_text_change");

	CHL3Label* pHL3Label = new CHL3Label();
	pHL3Label->SetSolid(SOLID_NONE);
	pHL3Label->AddEffects(EF_NODRAW);
	pHL3Label->ChangeTeam(TEAM_UNASSIGNED);

	ClientEntityList().AddNonNetworkableEntity(pHL3Label);
}

void HL3TextCallback(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		DevMsg("Usage: hl3_text <new_text>\n");
		return;
	}

	const char* newText = args.Arg(1);

	IGameEvent* event = gameeventmanager->CreateEvent("hl3_label_text_change");
	if (event)
	{
		event->SetString("text", newText);
		gameeventmanager->FireEventClientSide(event);
	}

	DevMsg("HL3 text set to: %s\n", newText);
}

ConCommand hl3_text_cmd("hl3_text", HL3TextCallback, "Change the HL3 text.", FCVAR_CLIENTDLL);

class CHL3LabelInit
{
public:
	CHL3LabelInit()
	{
		InitializeHL3Label();
	}
};

static CHL3LabelInit g_HL3LabelInit;
