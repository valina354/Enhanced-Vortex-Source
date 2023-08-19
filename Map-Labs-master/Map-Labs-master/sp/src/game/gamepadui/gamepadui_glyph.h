#ifndef GAMEPADUI_GLYPH_H
#define GAMEPADUI_GLYPH_H
#ifdef _WIN32
#pragma once
#endif

#include "gamepadui_interface.h"
#include "gamepadui_util.h"

#ifdef HL2_RETAIL // Steam input and Steam Controller are not supported in SDK2013 (Madi)
#include "steam/hl2/isteaminput.h"
#include "imageutils.h"
#endif // HL2_RETAIL

#include "inputsystem/iinputsystem.h"
#include "bitmap/bitmap.h"

class GamepadUIGlyph
{
public:
    GamepadUIGlyph()
    {
#ifdef HL2_RETAIL
        m_nOriginTextures[0] = -1;
        m_nOriginTextures[1] = -1;
#elif MAPBASE
        m_wchGlyph = L'\0';
#endif // HL2_RETAIL
    }

    ~GamepadUIGlyph()
    {
        Cleanup();
    }

    bool SetupGlyph( int nSize, const char *pszAction, bool bBaseLight = false )
    {
#ifdef HL2_RETAIL // Steam input and Steam Controller are not supported in SDK2013 (Madi)
        uint64 nSteamInputHandles[STEAM_INPUT_MAX_COUNT];
        if ( !GamepadUI::GetInstance().GetSteamAPIContext() || !GamepadUI::GetInstance().GetSteamAPIContext()->SteamInput() )
            return false;

        GamepadUI::GetInstance().GetSteamAPIContext()->SteamInput()->GetConnectedControllers( nSteamInputHandles );

        uint64 nController = g_pInputSystem->GetActiveSteamInputHandle();
        if ( !nController )
            nController = nSteamInputHandles[0];

        if ( !nController )
            return false;

        InputActionSetHandle_t hActionSet = GamepadUI::GetInstance().GetSteamAPIContext()->SteamInput()->GetCurrentActionSet( nController );
        InputDigitalActionHandle_t hDigitalAction = GamepadUI::GetInstance().GetSteamAPIContext()->SteamInput()->GetDigitalActionHandle( pszAction );
        if ( !hDigitalAction )
        {
            Cleanup();
            return false;
        }

        EInputActionOrigin eOrigins[STEAM_INPUT_MAX_ORIGINS] = {};
        int nOriginCount = GamepadUI::GetInstance().GetSteamAPIContext()->SteamInput()->GetDigitalActionOrigins( nController, hActionSet, hDigitalAction, eOrigins );
        EInputActionOrigin eOrigin = eOrigins[0];
        if ( !nOriginCount || eOrigin == k_EInputActionOrigin_None )
        {
            Cleanup();
            return false;
        }

        if ( m_eActionOrigin == eOrigin )
            return IsValid();

        Cleanup();

        m_eActionOrigin = eOrigin;

        if ( !IsPowerOfTwo( nSize ) )
            nSize = NextPowerOfTwo( nSize );

        int nGlyphSize = 256;
        ESteamInputGlyphSize eGlyphSize = k_ESteamInputGlyphSize_Large;
        if (nSize <= 32)
        {
            eGlyphSize = k_ESteamInputGlyphSize_Small;
            nGlyphSize = 32;
        }
        else if (nSize <= 128)
        {
            eGlyphSize = k_ESteamInputGlyphSize_Medium;
            nGlyphSize = 128;
        }
        else
        {
            eGlyphSize = k_ESteamInputGlyphSize_Large;
            nGlyphSize = 256;
        }

        ESteamInputGlyphStyle kStyles[2] =
        {
            bBaseLight ? ESteamInputGlyphStyle_Light : ESteamInputGlyphStyle_Knockout,
            ESteamInputGlyphStyle_Dark,
        };

        for ( int i = 0; i < 2; i++ )
        {
            const char* pszGlyph = GamepadUI::GetInstance().GetSteamAPIContext()->SteamInput()->GetGlyphPNGForActionOrigin( eOrigin, eGlyphSize, kStyles[i] );
            if (!pszGlyph)
            {
                Cleanup();
                return false;
            }

            Bitmap_t bitmap;
            ConversionErrorType error = ImgUtl_LoadBitmap( pszGlyph, bitmap );
            if ( error != CE_SUCCESS )
            {
                Cleanup();
                return false;
            }

            ImgUtl_ResizeBitmap( bitmap, nSize, nSize, &bitmap );

            m_nOriginTextures[i] = vgui::surface()->CreateNewTextureID(true);
            if ( m_nOriginTextures[i] <= 0 )
            {
                Cleanup();
                return false;
            }
            g_pMatSystemSurface->DrawSetTextureRGBAEx2( m_nOriginTextures[i], bitmap.GetBits(), bitmap.Width(), bitmap.Height(), ImageFormat::IMAGE_FORMAT_RGBA8888, true, false );
        }
#elif MAPBASE
        wchar_t* pwscGlyph = nullptr;
        if (V_strcmp(pszAction, "menu_lb") == 0)
        {
            pwscGlyph = g_pVGuiLocalize->Find("#GameUI_Icons_L_SHOULDER");
        }
        else if (V_strcmp(pszAction, "menu_rb") == 0)
        {
            pwscGlyph = g_pVGuiLocalize->Find("#GameUI_Icons_R_SHOULDER");
        }
        else if (V_strcmp(pszAction, "menu_cancel") == 0)
        {
            pwscGlyph = g_pVGuiLocalize->Find("#GameUI_Icons_B_BUTTON");
        }
        else if (V_strcmp(pszAction, "menu_select") == 0)
        {
            pwscGlyph = g_pVGuiLocalize->Find("#GameUI_Icons_A_BUTTON");
        }
        else if (V_strcmp(pszAction, "menu_y") == 0)
        {
            pwscGlyph = g_pVGuiLocalize->Find("#GameUI_Icons_Y_BUTTON");
        }
        else if (V_strcmp(pszAction, "menu_x") == 0)
        {
            pwscGlyph = g_pVGuiLocalize->Find("#GameUI_Icons_X_BUTTON");
        }
        
        if (pwscGlyph)
        {
            m_wchGlyph = pwscGlyph[0];
        }
        else
        {
            return false;
        }
#else
        return false;
#endif // HL2_RETAIL

        return true;
    }

    void PaintGlyph( int nX, int nY, int nSize, int nBaseAlpha )
    {
#ifdef HL2_RETAIL
        int nPressedAlpha = 255 - nBaseAlpha;

        if (nBaseAlpha)
        {
            vgui::surface()->DrawSetColor(Color(255, 255, 255, nBaseAlpha));
            vgui::surface()->DrawSetTexture(m_nOriginTextures[0]);
            vgui::surface()->DrawTexturedRect(nX, nY, nX + nSize, nY + nSize);
        }

        if (nPressedAlpha)
        {
            vgui::surface()->DrawSetColor(Color(255, 255, 255, nPressedAlpha));
            vgui::surface()->DrawSetTexture(m_nOriginTextures[1]);
            vgui::surface()->DrawTexturedRect(nX, nY, nX + nSize, nY + nSize);
        }

        vgui::surface()->DrawSetTexture(0);
#elif MAPBASE
        vgui::surface()->DrawSetTextColor(255, 255, 255, 255);
        vgui::surface()->DrawSetTextFont(GamepadUI::GetInstance().GetButtonGlyphFont());
        vgui::surface()->DrawSetTextPos(nX, nY);
        //const float flScale = (float)nSize / 32.f;
        //vgui::surface()->DrawSetTextScale(flScale, flScale);
        vgui::surface()->DrawUnicodeChar(m_wchGlyph);
#endif // HL2_RETAIL
    }

    bool IsValid()
    {
#ifdef HL2_RETAIL
        return m_nOriginTextures[0] > 0 && m_nOriginTextures[1] > 0;
#elif MAPBASE
        return m_wchGlyph != L'\0';
#else
        return false;
#endif // HL2_RETAIL

    }

    void Cleanup()
    {
#ifdef HL2_RETAIL
        for ( int i = 0; i < 2; i++ )
        {
            if ( m_nOriginTextures[ i ] > 0 )
                vgui::surface()->DestroyTextureID( m_nOriginTextures[ i ] );
            m_nOriginTextures[ i ] = -1;
        }

        m_eActionOrigin = k_EInputActionOrigin_None;
#endif // HL2_RETAIL
    }

private:

#ifdef HL2_RETAIL
    EInputActionOrigin m_eActionOrigin = k_EInputActionOrigin_None;
    int m_nOriginTextures[2];
#elif MAPBASE
    wchar_t m_wchGlyph;
#endif // HL2_RETAIL
};

#endif
