#include "stdafx.h"
#include "ParentShader.h"


CParentShader::CParentShader()
{
	m_nShaders = 0;
	m_ppShader = nullptr;
}

CParentShader::~CParentShader()
{
	if (m_ppShader)
	{
		for (int i = 0; i < m_nShaders; ++i)
			delete m_ppShader[i];
		delete[] m_ppShader;
	}
}

void CParentShader::CreateShader(ID3D11Device * pd3dDevice)
{
}

void CParentShader::ReleaseObjects()
{
	for (int i = 0; i < m_nShaders; ++i)
	{
		m_ppShader[i]->ReleaseObjects();
	}
}

void CParentShader::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; ++i)
	{
		m_ppShader[i]->AnimateObjects(fTimeElapsed);
	}
}

void CParentShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	for (int i = 0; i < m_nShaders; ++i)
	{
		m_ppShader[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEnviromentShader::CEnviromentShader() : CParentShader()
{
}

CEnviromentShader::~CEnviromentShader()
{
}

void CEnviromentShader::BuildObjects(ID3D11Device *pd3dDevice, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_nShaders = 2;
	m_ppShader = new CShader*[m_nShaders];

	int index = 0;
	m_ppShader[index] = new CSkyBoxShader();
	m_ppShader[index]->CreateShader(pd3dDevice);
	m_ppShader[index++]->BuildObjects(pd3dDevice, SceneMgr);

	m_ppShader[index] = new CTerrainShader();
	m_ppShader[index]->CreateShader(pd3dDevice);
	m_ppShader[index++]->BuildObjects(pd3dDevice, SceneMgr);
}

void CEnviromentShader::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState, CCamera * pCamera)
{
	if (uRenderState & RS_SHADOWMAP)
		m_ppShader[1]->Render(pd3dDeviceContext, uRenderState, pCamera);
	else
		for (int i = 0; i < m_nShaders; ++i)
		{
			m_ppShader[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
		}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CEffectShader::CEffectShader() : CParentShader()
{
}

CEffectShader::~CEffectShader()
{
}

void CEffectShader::ShaderKeyEventOn(CPlayer * pPlayer, WORD key, void * extra)
{
	CInGamePlayer * pInGamePlayer = static_cast<CInGamePlayer*>(pPlayer);

	switch (key)
	{
	//case 'B':
	//	((CParticleShader*)m_ppShader[mParticleEffectNum])->ParticleOn(4, pPlayer, &pPlayer->GetPosition(), &pPlayer->GetLookVector(), &pPlayer->GetLookVectorInverse());
	//	return;

	case 'X':
	{
		XMFLOAT3 pos = pPlayer->GetPosition();
		pos.y += 10.f;

		EFFECT_ON_INFO info;
		ZeroMemory(&info, sizeof(info));
		info.m_xmf3Pos = pos;
		info.m_pObject = pPlayer;
		info.m_xmfAccelate = info.m_xmf3Velocity = pPlayer->GetLookVector();
		info.eEffect = EFFECT_ICE_BOLT;
		info.bUseUpdateVelocity = true;
		((CTextureAniShader*)m_ppShader[mTxAniEffectNum])->EffectOn(info);
		return;
	}
	}
}

void CEffectShader::BuildObjects(ID3D11Device * pd3dDevice)
{
	m_nShaders = 2;
	m_ppShader = new CShader*[m_nShaders];

	CTextureAniShader * pTxAni = new CTextureAniShader();
	pTxAni->CreateShader(pd3dDevice);
	pTxAni->BuildObjects(pd3dDevice, nullptr);
	m_ppShader[0] = pTxAni;

	CParticleShader * pParticleShader = new CParticleShader();
	pParticleShader->CreateShader(pd3dDevice);
	pParticleShader->BuildObjects(pd3dDevice, nullptr);
	m_ppShader[1] = pParticleShader;
}

void CEffectShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
	CInGamePlayer * pPlayer = nullptr;

	switch (eMSG)
	{
	case eMessage::MSG_PARTICLE_ON:
	{
		static EFFECT_ON_INFO info;
		memcpy(&info.m_xmf3Pos, extra, sizeof(XMFLOAT3));// &xmf4Data, sizeof(XMFLOAT3));
		info.eEffect = EFFECT_TYPE::EFFECT_ABSORB;
		info.fColor = reinterpret_cast<XMFLOAT4*>(extra)->w; 
		static_cast<CParticleShader*>(m_ppShader[mParticleEffectNum])->ParticleOn(info);
		return;
	}
	case eMessage::MSG_MAGIC_CAST:
	{
		pPlayer = static_cast<CInGamePlayer*>(extra);
		((CTextureAniShader*)m_ppShader[mTxAniEffectNum])->EffectOn(pPlayer->GetCastEffectOnInfo());
		return;
	}
	case eMessage::MSG_MAGIC_SHOT:
		pPlayer = static_cast<CInGamePlayer*>(extra);
		if (false == pPlayer->IsCancled())
			static_cast<CParticleShader*>(m_ppShader[mParticleEffectNum])->ParticleOn(pPlayer->Get1HAnimShotParticleOnInfo());
		pPlayer->GetStatus().SetCanMove(true);
		pPlayer->SetCancled(false);
		return;

	case eMessage::MSG_MAGIC_AREA:
		((CTextureAniShader*)m_ppShader[mTxAniEffectNum])->EffectOn(0, pPlayer, static_cast<XMFLOAT3*>(extra), nullptr, nullptr, SYSTEMMgr.GetDominatePlayerNum());
		return;
	}
}

CStaticModelingShader::CStaticModelingShader()
{
}

CStaticModelingShader::~CStaticModelingShader()
{
}

void CStaticModelingShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_nShaders = 3;
	m_ppShader = new CShader*[m_nShaders];

	CItemShader *pItemShader = new CItemShader();
	pItemShader->CreateShader(pd3dDevice);
	pItemShader->BuildObjects(pd3dDevice, pMaterial, SceneMgr);
	m_ppShader[0] = pItemShader;

	CStaticShader *pStaticShader = new CStaticShader();
	pStaticShader->CreateShader(pd3dDevice);
	pStaticShader->BuildObjects(pd3dDevice, pMaterial, SceneMgr);
	m_ppShader[1] = pStaticShader;

	CBlackAlphaShader *pBlackImageShader = new CBlackAlphaShader();
	pBlackImageShader->CreateShader(pd3dDevice);
	pBlackImageShader->BuildObjects(pd3dDevice, pMaterial, SceneMgr);
	m_ppShader[2] = pBlackImageShader;
}

void CStaticInstancingParentShader::BuildObjects(ID3D11Device * pd3dDevice, CMaterial * pMaterial, BUILD_RESOURCES_MGR & SceneMgr)
{
	m_nShaders = 2;
	m_ppShader = new CShader*[m_nShaders];

	CBillboardShader * pTrees = new CBillboardShader();
	pTrees->CreateShader(pd3dDevice);
	pTrees->BuildObjects(pd3dDevice);
	m_ppShader[0] = pTrees;

	CStaticInstaningShader * pStatic = new CStaticInstaningShader();
	pStatic->CreateShader(pd3dDevice);
	pStatic->BuildObjects(pd3dDevice, SceneMgr);
	m_ppShader[1] = pStatic;
}

void CStaticInstancingParentShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	if (!(uRenderState & RS_SHADOWMAP))
	{
		for (int i = 0; i < m_nShaders; ++i)
			m_ppShader[i]->Render(pd3dDeviceContext, uRenderState, pCamera);
	}
	else
	{	
		static_cast<CBillboardShader*>(m_ppShader[0])->AllRender(pd3dDeviceContext, uRenderState, pCamera);
		static_cast<CStaticInstaningShader*>(m_ppShader[1])->AllRender(pd3dDeviceContext, uRenderState, pCamera);
	}
}