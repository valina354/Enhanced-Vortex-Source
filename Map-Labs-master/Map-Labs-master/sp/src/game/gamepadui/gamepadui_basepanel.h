#ifndef GAMEPADUI_BASEPANEL_H
#define GAMEPADUI_BASEPANEL_H
#ifdef _WIN32
#pragma once
#endif

#include "gamepadui_interface.h"

class GamepadUIMainMenu;
class GamepadUIFrame;

class GamepadUIBasePanel : public vgui::Panel
{
    DECLARE_CLASS_SIMPLE( GamepadUIBasePanel, vgui::Panel );
public:
    GamepadUIBasePanel( vgui::VPANEL parent );

    void ApplySchemeSettings( vgui::IScheme* pScheme ) OVERRIDE;

    GamepadUIMainMenu *GetMainMenuPanel() const;

    GamepadUIFrame *GetCurrentFrame() const;
    void SetCurrentFrame( GamepadUIFrame *pFrame );

    void OnMenuStateChanged();

    void ActivateBackgroundEffects();
    bool IsBackgroundMusicPlaying();
    bool StartBackgroundMusic( float flVolume );
    void ReleaseBackgroundMusic();

private:
    void HideGameMenuLogos();

private:
    GamepadUIMainMenu *m_pMainMenu = NULL;

    GamepadUIFrame *m_pCurrentFrame = NULL;

    int m_nBackgroundMusicGUID;
    bool m_bBackgroundMusicEnabled;

};

#endif // GAMEPADUI_BASEPANEL_H
