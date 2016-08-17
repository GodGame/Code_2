#pragma once
#ifndef __TXMGR
#define __TXMGR

#include "MgrType.h"

typedef BYTE SETSHADER;
#define SET_SHADER_NONE 0
#define SET_SHADER_VS	(1 << 0)
#define SET_SHADER_HS	(1 << 1)
#define SET_SHADER_DS	(1 << 2)
#define SET_SHADER_GS	(1 << 3)
#define SET_SHADER_PS	(1 << 4)
#define SET_SHADER_CS	(1 << 5)

class CTexture
{
public:
	CTexture(int nTextures = 1, int nSamplers = 1, int nTextureStartSlot = 0, int nSamplerStartSlot = 0, SETSHADER nSetInfo = SET_SHADER_PS);
	virtual ~CTexture();

private:
	SETSHADER	m_uTextureSet;
	int m_nReferences;

public:
	void AddRef()  { m_nReferences++; }
#ifdef _DEBUG
	int Release() 
	{ 
		if (--m_nReferences < 1) 
		{
			delete this;
			return 0;
		}
		return m_nReferences;
	}
#else
	void Release() { if (--m_nReferences < 1) delete this; }
#endif
private:
	//�ؽ��� ���ҽ��� �����̴�.
	int m_nTextures;
	ID3D11ShaderResourceView **m_ppd3dsrvTextures;
	//�ؽ��� ���ҽ��� ������ ���� �����̴�.
	int m_nTextureStartSlot;
	//���÷� ���� ��ü�� �����̴�.
	int m_nSamplers;
	ID3D11SamplerState **m_ppd3dSamplerStates;
	//���÷� ���� ��ü�� ������ ���� �����̴�.
	int m_nSamplerStartSlot;

public:
	void ChangeSetShader(SETSHADER uInfo)        { m_uTextureSet = uInfo; }
	void AddSetShader(SETSHADER uAdd)            { m_uTextureSet |= (uAdd); }

	void SetTexture(int nIndex, ID3D11ShaderResourceView *pd3dsrvTexture);
	void SetSampler(int nIndex, ID3D11SamplerState *pd3dSamplerState);

	//�ؽ��� ���ҽ��� ���÷� ���� ��ü�� ���� ���̴� ������ �����Ѵ�.
	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);
	//�ؽ��� ���ҽ��� ���� ���̴� ������ �����Ѵ�.
	void UpdateTextureShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);
	//���÷� ���� ��ü�� ���� ���̴� ������ �����Ѵ�.
	void UpdateSamplerShaderVariable(ID3D11DeviceContext *pd3dDeviceContext);

public:
	ID3D11ShaderResourceView * GetSRV(int index) const {return m_ppd3dsrvTextures[index];}
	ID3D11SamplerState * GetSampler(int index)   const { return m_ppd3dSamplerStates[index]; }
	ID3D11ShaderResourceView ** GetSRVList(void) const { return m_ppd3dsrvTextures; }
	ID3D11SamplerState ** GetSamplerList(void)   const { return m_ppd3dSamplerStates; }

	bool IsSampler()                             { return m_nSamplers > 0; }
	bool IsSRV()                                 { return m_nTextures > 0; }

	static ID3D11ShaderResourceView * CreateTexture2DArraySRV(ID3D11Device *pd3dDevice, const wchar_t *ppstrFilePaths, const wchar_t * ppstrFormat, UINT nTextures);
};

typedef CMgr<CTexture> TEXTURE_MGR;
class CTextureMgr : public TEXTURE_MGR
{
private:
	CTextureMgr();
	virtual ~CTextureMgr();
	CTextureMgr& operator=(const CTextureMgr&);

public:
	static CTextureMgr& GetInstance();
	bool InsertShaderResourceView(ID3D11ShaderResourceView * pSRV, string name, UINT uSlotNum, SETSHADER nSetInfo = SET_SHADER_PS);
	bool InsertSamplerState(ID3D11SamplerState * pSamplerState, string name, UINT uSlotNum, SETSHADER nSetInfo = SET_SHADER_PS);

	ID3D11ShaderResourceView * GetShaderResourceView(string name)  { return m_mpList[name]->GetSRV(0);}
	ID3D11SamplerState * GetSamplerState(string name)              { return m_mpList[name]->GetSampler(0);}

public:
	//virtual void ReleaseObjects();
	virtual void BuildResources(ID3D11Device *pd3dDevice);
	void BuildSamplers(ID3D11Device *pd3dDevice);
	void BuildTextures(ID3D11Device *pd3dDevice);

	void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, string name);

	static CTexture * MakeFbxcjhTextures(ID3D11Device * pd3dDevice, wstring & wstrBaseDir, vector<wstring>& vcFileNameArrays, int nStartTxSlot = 0);
};

#define TXMgr CTextureMgr::GetInstance()
///////////////////////////////////////////////////////////////////////////////////////////////////////

//������ ������ ��Ÿ���� ����ü�̴�.
struct MATERIAL
{
	XMFLOAT4 m_xcAmbient;
	XMFLOAT4 m_xcDiffuse;
	XMFLOAT4 m_xcSpecular; //(r,g,b,a=power)
	XMFLOAT4 m_xcEmissive;
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

private:
	SETSHADER	m_uMaterialSet;
	int m_nReferences;

public:
	void AddRef() { m_nReferences++; }
#ifdef _DEBUG
	int Release()
	{
		if (--m_nReferences <= 0)
		{
			delete this;
			return 0;
		}
		return m_nReferences;
	}
#else
	void Release() { if (--m_nReferences <= 0) delete this; }
#endif
	MATERIAL m_Material;
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef CMgr<CMaterial> MATERIAL_MGR;
class CMaterialMgr : public MATERIAL_MGR
{
private:
	CMaterialMgr();
	virtual ~CMaterialMgr();

public:
	static CMaterialMgr& GetInstance();

	virtual void BuildResources(ID3D11Device *pd3dDevice);

};
#define MaterialMgr CMaterialMgr::GetInstance()
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMesh;
typedef CMgr<CMesh> MESH_MGR;
class CGameObject;
typedef CMgr<CGameObject> OBJ_MGR;
#ifdef _USE_MESH_MGR
class CMesh;
class CMeshMgr : public CMgr<CMesh>
{
private:
	CMeshMgr();
	virtual ~CMeshMgr();

public:
	static CMeshMgr& GetInstance();

	virtual void BuildResources(ID3D11Device * pd3dDevice);
};
#define MESHMgr CMeshMgr::GetInstance()
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct CB_TX_ANIMATION_INFO
{
	float  m_fGameTime;
	float  m_fFramePerTime;
	XMFLOAT2 m_xmf2Size;
	//float  m_fFrameWidthPercent;
	//float  m_fFrameHeightPercent;

	XMFLOAT3 m_xmf3Pos;
	float    m_nColorNum;

	XMFLOAT3 m_xmf3LookVector;
	float    m_bMove;
};

struct CB_PARTICLE
{
	XMFLOAT3 m_vParticleEmitPos;
	float m_fGameTime;
	XMFLOAT3 m_vParticleVelocity;
	float m_fLifeTime;
	XMFLOAT3 m_vAccel;
	float m_fTimeStep;
	float m_fNewTime;
	float m_fMaxSize;
	UINT  m_nColorNum;
	UINT  m_bEnable;
};

class CViewManager
{
	CMgr<ID3D11ShaderResourceView>    m_mgrSrv;
	CMgr<ID3D11RenderTargetView>	  m_mgrRtv;
	CMgr<ID3D11DepthStencilView>	  m_mgrDsv;
	CMgr<ID3D11UnorderedAccessView>   m_mgrUav;
	CMgr<ID3D11Buffer>				  m_mgrBuffer;
	CMgr<ID3D11Texture2D>			  m_mgrTex2D;

	CViewManager() {}
	~CViewManager() {}
	CViewManager& operator=(const CViewManager&);
public:
	static CViewManager& GetInstance();

	void InsertSRV(ID3D11ShaderResourceView  * pSRV, string  name) { m_mgrSrv.InsertObject(pSRV, name); }
	void InsertRTV(ID3D11RenderTargetView    * pRTV, string  name) { m_mgrRtv.InsertObject(pRTV, name); }
	void InsertDSV(ID3D11DepthStencilView    * pDSV, string  name) { m_mgrDsv.InsertObject(pDSV, name); }
	void InsertUAV(ID3D11UnorderedAccessView * pUAV, string  name) { m_mgrUav.InsertObject(pUAV, name); }
	void InsertTex2D(ID3D11Texture2D * pTex2D, string name)		   { m_mgrTex2D.InsertObject(pTex2D, name); }
	void InsertBuffer(ID3D11Buffer * pBuffer, string name)	       { m_mgrBuffer.InsertObject(pBuffer, name); }

	ID3D11ShaderResourceView  * GetSRV(string  name)               { return m_mgrSrv.GetObjects(name); }
	ID3D11RenderTargetView    * GetRTV(string  name)               { return m_mgrRtv.GetObjects(name); }
	ID3D11DepthStencilView	  * GetDSV(string  name)               { return m_mgrDsv.GetObjects(name); }
	ID3D11UnorderedAccessView * GetUAV(string  name)               { return m_mgrUav.GetObjects(name); }
	ID3D11Texture2D * GetTex2D(string name)		                   { return m_mgrTex2D.GetObjects(name); }
	ID3D11Buffer    * GetBuffer(string name)                       { return m_mgrBuffer.GetObjects(name); }

	void EraseSRV(string  name)   { m_mgrSrv.EraseObjects(name); }
	void EraseRTV(string  name)   { m_mgrRtv.EraseObjects(name); }
	void EraseDSV(string  name)   { m_mgrDsv.EraseObjects(name); }
	void EraseUAV(string  name)   { m_mgrUav.EraseObjects(name); }
	void EraseTex2D(string name)  { m_mgrTex2D.EraseObjects(name); }
	void EraseBuffer(string name) { m_mgrBuffer.EraseObjects(name); }

public:
	static ID3D11ShaderResourceView * CreateRandomTexture1DSRV(ID3D11Device * pd3dDevice);

	void ReleaseResources();
	void BuildResources(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);
	void BuildViews(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);
	//void BuildBuffers(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);

	void CreatePostProcessViews(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);
	void CreateViewsInGame(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);

	void BuildConstantBuffers(ID3D11Device * pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext);
};
#define ViewMgr CViewManager::GetInstance()

void MapConstantBuffer(ID3D11DeviceContext * pd3dDeviceContext, void * data, size_t size, ID3D11Buffer * pBuffer);
void MapMatrixConstantBuffer(ID3D11DeviceContext * pd3dDeviceContext, XMFLOAT4X4 & matrix, ID3D11Buffer * pBuffer);

class CFileLoadManager
{
	CFileLoadManager(){}
	~CFileLoadManager(){}
	CFileLoadManager& operator=(const CFileLoadManager&);
public:
	static CFileLoadManager& GetInstance();

	void LoadSceneResources(ID3D11Device * pd3dDevice, void * resourceMgr, const char * fileName);
	void LoadSceneAnimationObjects(ID3D11Device * pd3dDevice, void * resourceMgr, const char * fileName);
};
#define FILE_LOAD_Mgr CFileLoadManager::GetInstance()

#endif