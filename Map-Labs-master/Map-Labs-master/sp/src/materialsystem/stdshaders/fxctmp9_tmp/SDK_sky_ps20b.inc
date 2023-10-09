#include "shaderlib/cshader.h"
class sdk_sky_ps20b_Static_Index
{
private:
	int m_nCONVERT_TO_SRGB;
#ifdef _DEBUG
	bool m_bCONVERT_TO_SRGB;
#endif
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCONVERT_TO_SRGB = i;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = true;
#endif
	}
	void SetCONVERT_TO_SRGB( bool i )
	{
		m_nCONVERT_TO_SRGB = i ? 1 : 0;
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = true;
#endif
	}
private:
	int m_nUSES_TEXTURE_BLENDS;
#ifdef _DEBUG
	bool m_bUSES_TEXTURE_BLENDS;
#endif
public:
	void SetUSES_TEXTURE_BLENDS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUSES_TEXTURE_BLENDS = i;
#ifdef _DEBUG
		m_bUSES_TEXTURE_BLENDS = true;
#endif
	}
	void SetUSES_TEXTURE_BLENDS( bool i )
	{
		m_nUSES_TEXTURE_BLENDS = i ? 1 : 0;
#ifdef _DEBUG
		m_bUSES_TEXTURE_BLENDS = true;
#endif
	}
private:
	int m_nHAS_BLEND1;
#ifdef _DEBUG
	bool m_bHAS_BLEND1;
#endif
public:
	void SetHAS_BLEND1( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHAS_BLEND1 = i;
#ifdef _DEBUG
		m_bHAS_BLEND1 = true;
#endif
	}
	void SetHAS_BLEND1( bool i )
	{
		m_nHAS_BLEND1 = i ? 1 : 0;
#ifdef _DEBUG
		m_bHAS_BLEND1 = true;
#endif
	}
private:
	int m_nHAS_BLEND2;
#ifdef _DEBUG
	bool m_bHAS_BLEND2;
#endif
public:
	void SetHAS_BLEND2( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHAS_BLEND2 = i;
#ifdef _DEBUG
		m_bHAS_BLEND2 = true;
#endif
	}
	void SetHAS_BLEND2( bool i )
	{
		m_nHAS_BLEND2 = i ? 1 : 0;
#ifdef _DEBUG
		m_bHAS_BLEND2 = true;
#endif
	}
private:
	int m_nHAS_BLEND3;
#ifdef _DEBUG
	bool m_bHAS_BLEND3;
#endif
public:
	void SetHAS_BLEND3( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHAS_BLEND3 = i;
#ifdef _DEBUG
		m_bHAS_BLEND3 = true;
#endif
	}
	void SetHAS_BLEND3( bool i )
	{
		m_nHAS_BLEND3 = i ? 1 : 0;
#ifdef _DEBUG
		m_bHAS_BLEND3 = true;
#endif
	}
private:
	int m_nHAS_BLEND4;
#ifdef _DEBUG
	bool m_bHAS_BLEND4;
#endif
public:
	void SetHAS_BLEND4( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHAS_BLEND4 = i;
#ifdef _DEBUG
		m_bHAS_BLEND4 = true;
#endif
	}
	void SetHAS_BLEND4( bool i )
	{
		m_nHAS_BLEND4 = i ? 1 : 0;
#ifdef _DEBUG
		m_bHAS_BLEND4 = true;
#endif
	}
public:
	sdk_sky_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = true;
#endif // _DEBUG
		m_nCONVERT_TO_SRGB =  g_pHardwareConfig->NeedsShaderSRGBConversion();
#ifdef _DEBUG
		m_bUSES_TEXTURE_BLENDS = false;
#endif // _DEBUG
		m_nUSES_TEXTURE_BLENDS = 0;
#ifdef _DEBUG
		m_bHAS_BLEND1 = false;
#endif // _DEBUG
		m_nHAS_BLEND1 = 0;
#ifdef _DEBUG
		m_bHAS_BLEND2 = false;
#endif // _DEBUG
		m_nHAS_BLEND2 = 0;
#ifdef _DEBUG
		m_bHAS_BLEND3 = false;
#endif // _DEBUG
		m_nHAS_BLEND3 = 0;
#ifdef _DEBUG
		m_bHAS_BLEND4 = false;
#endif // _DEBUG
		m_nHAS_BLEND4 = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bCONVERT_TO_SRGB && m_bUSES_TEXTURE_BLENDS && m_bHAS_BLEND1 && m_bHAS_BLEND2 && m_bHAS_BLEND3 && m_bHAS_BLEND4;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 2 * m_nCONVERT_TO_SRGB ) + ( 4 * m_nUSES_TEXTURE_BLENDS ) + ( 8 * m_nHAS_BLEND1 ) + ( 16 * m_nHAS_BLEND2 ) + ( 32 * m_nHAS_BLEND3 ) + ( 64 * m_nHAS_BLEND4 ) + 0;
	}
};
#define shaderStaticTest_sdk_sky_ps20b psh_forgot_to_set_static_USES_TEXTURE_BLENDS + psh_forgot_to_set_static_HAS_BLEND1 + psh_forgot_to_set_static_HAS_BLEND2 + psh_forgot_to_set_static_HAS_BLEND3 + psh_forgot_to_set_static_HAS_BLEND4 + 0
class sdk_sky_ps20b_Dynamic_Index
{
private:
	int m_nWRITE_DEPTH_TO_DESTALPHA;
#ifdef _DEBUG
	bool m_bWRITE_DEPTH_TO_DESTALPHA;
#endif
public:
	void SetWRITE_DEPTH_TO_DESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITE_DEPTH_TO_DESTALPHA = i;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
	void SetWRITE_DEPTH_TO_DESTALPHA( bool i )
	{
		m_nWRITE_DEPTH_TO_DESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = true;
#endif
	}
public:
	sdk_sky_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bWRITE_DEPTH_TO_DESTALPHA;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWRITE_DEPTH_TO_DESTALPHA ) + 0;
	}
};
#define shaderDynamicTest_sdk_sky_ps20b psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + 0
