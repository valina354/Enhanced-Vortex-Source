#include "shaderlib/cshader.h"
class lightmappedpbr_ps30_Static_Index
{
private:
	int m_nCONVERT_TO_SRGB;
#ifdef _DEBUG
	bool m_bCONVERT_TO_SRGB;
#endif
public:
	void SetCONVERT_TO_SRGB( int i )
	{
		Assert( i >= 0 && i <= 0 );
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
	int m_nFLASHLIGHT;
#ifdef _DEBUG
	bool m_bFLASHLIGHT;
#endif
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
	void SetFLASHLIGHT( bool i )
	{
		m_nFLASHLIGHT = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
private:
	int m_nCUBEMAP;
#ifdef _DEBUG
	bool m_bCUBEMAP;
#endif
public:
	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
	void SetCUBEMAP( bool i )
	{
		m_nCUBEMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
private:
	int m_nSMOOTHNESS;
#ifdef _DEBUG
	bool m_bSMOOTHNESS;
#endif
public:
	void SetSMOOTHNESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSMOOTHNESS = i;
#ifdef _DEBUG
		m_bSMOOTHNESS = true;
#endif
	}
	void SetSMOOTHNESS( bool i )
	{
		m_nSMOOTHNESS = i ? 1 : 0;
#ifdef _DEBUG
		m_bSMOOTHNESS = true;
#endif
	}
private:
	int m_nSEAMLESS;
#ifdef _DEBUG
	bool m_bSEAMLESS;
#endif
public:
	void SetSEAMLESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSEAMLESS = i;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif
	}
	void SetSEAMLESS( bool i )
	{
		m_nSEAMLESS = i ? 1 : 0;
#ifdef _DEBUG
		m_bSEAMLESS = true;
#endif
	}
private:
	int m_nBUMPMAP;
#ifdef _DEBUG
	bool m_bBUMPMAP;
#endif
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif
	}
	void SetBUMPMAP( bool i )
	{
		m_nBUMPMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif
	}
private:
	int m_nPARALLAXCORRECT;
#ifdef _DEBUG
	bool m_bPARALLAXCORRECT;
#endif
public:
	void SetPARALLAXCORRECT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPARALLAXCORRECT = i;
#ifdef _DEBUG
		m_bPARALLAXCORRECT = true;
#endif
	}
	void SetPARALLAXCORRECT( bool i )
	{
		m_nPARALLAXCORRECT = i ? 1 : 0;
#ifdef _DEBUG
		m_bPARALLAXCORRECT = true;
#endif
	}
private:
	int m_nMRAOTEX;
#ifdef _DEBUG
	bool m_bMRAOTEX;
#endif
public:
	void SetMRAOTEX( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nMRAOTEX = i;
#ifdef _DEBUG
		m_bMRAOTEX = true;
#endif
	}
	void SetMRAOTEX( bool i )
	{
		m_nMRAOTEX = i ? 1 : 0;
#ifdef _DEBUG
		m_bMRAOTEX = true;
#endif
	}
private:
	int m_nBUMPALPHASMOOTHNESS;
#ifdef _DEBUG
	bool m_bBUMPALPHASMOOTHNESS;
#endif
public:
	void SetBUMPALPHASMOOTHNESS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPALPHASMOOTHNESS = i;
#ifdef _DEBUG
		m_bBUMPALPHASMOOTHNESS = true;
#endif
	}
	void SetBUMPALPHASMOOTHNESS( bool i )
	{
		m_nBUMPALPHASMOOTHNESS = i ? 1 : 0;
#ifdef _DEBUG
		m_bBUMPALPHASMOOTHNESS = true;
#endif
	}
public:
	lightmappedpbr_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bCONVERT_TO_SRGB = false;
#endif // _DEBUG
		m_nCONVERT_TO_SRGB = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
#endif // _DEBUG
		m_nFLASHLIGHT = 0;
#ifdef _DEBUG
		m_bCUBEMAP = false;
#endif // _DEBUG
		m_nCUBEMAP = 0;
#ifdef _DEBUG
		m_bSMOOTHNESS = false;
#endif // _DEBUG
		m_nSMOOTHNESS = 0;
#ifdef _DEBUG
		m_bSEAMLESS = false;
#endif // _DEBUG
		m_nSEAMLESS = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif // _DEBUG
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bPARALLAXCORRECT = false;
#endif // _DEBUG
		m_nPARALLAXCORRECT = 0;
#ifdef _DEBUG
		m_bMRAOTEX = false;
#endif // _DEBUG
		m_nMRAOTEX = 0;
#ifdef _DEBUG
		m_bBUMPALPHASMOOTHNESS = false;
#endif // _DEBUG
		m_nBUMPALPHASMOOTHNESS = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bCONVERT_TO_SRGB && m_bFLASHLIGHT && m_bCUBEMAP && m_bSMOOTHNESS && m_bSEAMLESS && m_bBUMPMAP && m_bPARALLAXCORRECT && m_bMRAOTEX && m_bBUMPALPHASMOOTHNESS;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 48 * m_nCONVERT_TO_SRGB ) + ( 48 * m_nFLASHLIGHT ) + ( 96 * m_nCUBEMAP ) + ( 192 * m_nSMOOTHNESS ) + ( 384 * m_nSEAMLESS ) + ( 768 * m_nBUMPMAP ) + ( 1536 * m_nPARALLAXCORRECT ) + ( 3072 * m_nMRAOTEX ) + ( 6144 * m_nBUMPALPHASMOOTHNESS ) + 0;
	}
};
#define shaderStaticTest_lightmappedpbr_ps30 psh_forgot_to_set_static_CONVERT_TO_SRGB + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_SMOOTHNESS + psh_forgot_to_set_static_SEAMLESS + psh_forgot_to_set_static_BUMPMAP + psh_forgot_to_set_static_PARALLAXCORRECT + psh_forgot_to_set_static_MRAOTEX + psh_forgot_to_set_static_BUMPALPHASMOOTHNESS + 0
class lightmappedpbr_ps30_Dynamic_Index
{
private:
	int m_nWRITEWATERFOGTODESTALPHA;
#ifdef _DEBUG
	bool m_bWRITEWATERFOGTODESTALPHA;
#endif
public:
	void SetWRITEWATERFOGTODESTALPHA( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWRITEWATERFOGTODESTALPHA = i;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
	void SetWRITEWATERFOGTODESTALPHA( bool i )
	{
		m_nWRITEWATERFOGTODESTALPHA = i ? 1 : 0;
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = true;
#endif
	}
private:
	int m_nPIXELFOGTYPE;
#ifdef _DEBUG
	bool m_bPIXELFOGTYPE;
#endif
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
	void SetPIXELFOGTYPE( bool i )
	{
		m_nPIXELFOGTYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
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
private:
	int m_nFLASHLIGHTSHADOWS;
#ifdef _DEBUG
	bool m_bFLASHLIGHTSHADOWS;
#endif
public:
	void SetFLASHLIGHTSHADOWS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHTSHADOWS = i;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif
	}
	void SetFLASHLIGHTSHADOWS( bool i )
	{
		m_nFLASHLIGHTSHADOWS = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = true;
#endif
	}
private:
	int m_nCSM;
#ifdef _DEBUG
	bool m_bCSM;
#endif
public:
	void SetCSM( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCSM = i;
#ifdef _DEBUG
		m_bCSM = true;
#endif
	}
	void SetCSM( bool i )
	{
		m_nCSM = i ? 1 : 0;
#ifdef _DEBUG
		m_bCSM = true;
#endif
	}
private:
	int m_nCSM_PERF;
#ifdef _DEBUG
	bool m_bCSM_PERF;
#endif
public:
	void SetCSM_PERF( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nCSM_PERF = i;
#ifdef _DEBUG
		m_bCSM_PERF = true;
#endif
	}
	void SetCSM_PERF( bool i )
	{
		m_nCSM_PERF = i ? 1 : 0;
#ifdef _DEBUG
		m_bCSM_PERF = true;
#endif
	}
private:
	int m_nLIGHT_PREVIEW;
#ifdef _DEBUG
	bool m_bLIGHT_PREVIEW;
#endif
public:
	void SetLIGHT_PREVIEW( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nLIGHT_PREVIEW = i;
#ifdef _DEBUG
		m_bLIGHT_PREVIEW = true;
#endif
	}
	void SetLIGHT_PREVIEW( bool i )
	{
		m_nLIGHT_PREVIEW = i ? 1 : 0;
#ifdef _DEBUG
		m_bLIGHT_PREVIEW = true;
#endif
	}
public:
	lightmappedpbr_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bWRITEWATERFOGTODESTALPHA = false;
#endif // _DEBUG
		m_nWRITEWATERFOGTODESTALPHA = 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bWRITE_DEPTH_TO_DESTALPHA = false;
#endif // _DEBUG
		m_nWRITE_DEPTH_TO_DESTALPHA = 0;
#ifdef _DEBUG
		m_bFLASHLIGHTSHADOWS = false;
#endif // _DEBUG
		m_nFLASHLIGHTSHADOWS = 0;
#ifdef _DEBUG
		m_bCSM = false;
#endif // _DEBUG
		m_nCSM = 0;
#ifdef _DEBUG
		m_bCSM_PERF = false;
#endif // _DEBUG
		m_nCSM_PERF = 0;
#ifdef _DEBUG
		m_bLIGHT_PREVIEW = false;
#endif // _DEBUG
		m_nLIGHT_PREVIEW = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bWRITEWATERFOGTODESTALPHA && m_bPIXELFOGTYPE && m_bWRITE_DEPTH_TO_DESTALPHA && m_bFLASHLIGHTSHADOWS && m_bCSM && m_bCSM_PERF && m_bLIGHT_PREVIEW;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWRITEWATERFOGTODESTALPHA ) + ( 2 * m_nPIXELFOGTYPE ) + ( 4 * m_nWRITE_DEPTH_TO_DESTALPHA ) + ( 8 * m_nFLASHLIGHTSHADOWS ) + ( 16 * m_nCSM ) + ( 16 * m_nCSM_PERF ) + ( 16 * m_nLIGHT_PREVIEW ) + 0;
	}
};
#define shaderDynamicTest_lightmappedpbr_ps30 psh_forgot_to_set_dynamic_WRITEWATERFOGTODESTALPHA + psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_WRITE_DEPTH_TO_DESTALPHA + psh_forgot_to_set_dynamic_FLASHLIGHTSHADOWS + psh_forgot_to_set_dynamic_CSM + psh_forgot_to_set_dynamic_CSM_PERF + psh_forgot_to_set_dynamic_LIGHT_PREVIEW + 0
