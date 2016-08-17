#include "stdafx.h"
#include "MyInline.h"
#include "Scene.h"
#include "GameFramework.h"

CScene::CScene()
{
	m_ppShaders              = nullptr;
	m_pSceneShader           = nullptr;
	m_pPlayerShader          = nullptr;
	m_pUIShader              = nullptr;

	m_nShaders               = 0;
	m_pCamera                = nullptr;
	//m_pSelectedObject      = nullptr;
	m_pd3dcbLights           = nullptr;

	m_pLights                = nullptr;
	m_nMRT                   = 1;
	m_nThread                = 0;

	EVENTMgr.Initialize();
	//m_nRenderThreads       = 0;

	//m_pRenderingThreadInfo = nullptr;
}

CScene::~CScene()
{
	//for (int i = 0; i < m_nRenderThreads; ++i)
	//{
	//	m_pRenderingThreadInfo[i].m_pd3dDeferredContext->Release();
	//	::CloseHandle(m_pRenderingThreadInfo[i].m_hRenderingBeginEvent);
	//	::CloseHandle(m_pRenderingThreadInfo[i].m_hRenderingEndEvent);
	//}

	//if (m_pRenderingThreadInfo) delete[] m_pRenderingThreadInfo;
	//if (m_hRenderingEndEvents) delete[] m_hRenderingEndEvents;
}

void CScene::ChangeGamePlayerID(int id)
{
	if (nullptr == m_pPlayerShader) return;

	m_pPlayerShader->SetPlayerID(FRAMEWORK.GetDevice(), id);
	
	m_pCamera = (m_pPlayerShader->GetPlayer()->GetCamera());

	SYSTEMMgr.SetPlayerNum(id);
	//FRAMEWORK.SetCamera(m_pCamera);
	//FRAMEWORK.SetPlayer(this, m_pPlayerShader->GetPlayer());
}

void CScene::ReleaseObjects()
{
	ReleaseShaderVariables();

	for (int j = 0; j < m_nShaders; j++)
	{
		if (m_ppShaders[j]) m_ppShaders[j]->ReleaseObjects();
		if (m_ppShaders[j]) delete m_ppShaders[j];
	}
	if (m_ppShaders) delete[] m_ppShaders;

	CShader * pShaderArray[] = { m_pPlayerShader, m_pSceneShader, m_pUIShader };
	for (int i = 0; i < 3; ++i)
	{
		if (pShaderArray[i])
		{
			pShaderArray[i]->ReleaseObjects();
			delete pShaderArray[i];
		}
	}

	m_SceneResoucres.mgrMaterial.ReleaseObjects();
	m_SceneResoucres.mgrMesh.ReleaseObjects();
	m_SceneResoucres.mgrTexture.ReleaseObjects();
}

bool CScene::PacketProcess(LPARAM lParam)
{
	CLIENT.ReadPacket();
	return true;
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	static POINT pt;
	GetCursorPos(&pt);
	//ScreenToClient(hWnd, &pt);
	//ClientToScreen(hWnd, &pt);

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		GetGameMessage(this, eMessage::MSG_MOUSE_DOWN, &pt);
		if (m_pUIShader) m_pUIShader->MouseDown(hWnd, pt);
		return false;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		GetGameMessage(this, eMessage::MSG_MOUSE_UP, &pt);
		if (m_pUIShader) m_pUIShader->MouseDown(hWnd, pt);
		//		m_pSelectedObject = PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam));
	case WM_MOUSEMOVE:
		SendMouseOverMessage(hWnd, pt);
		return false;
	}
	return true;
}

void CScene::ReleaseShaderVariables()
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CScene::GetGameMessage(CScene * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_MOUSE_DOWN_OVER:
		return;
	case eMessage::MSG_MOUSE_UP_OVER:
		return;
	default:
		return;
	}
}

void CScene::SendGameMessage(CScene * toObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	default:
		return;
	}
}

void CScene::MessageObjToObj(CScene * byObj, CScene * toObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	default:
		return;
	}
}

void CScene::BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext)
{
}

void CScene::PreProcessing(ID3D11DeviceContext * pd3dDeviceContext)
{
}

void CScene::UpdateLights(ID3D11DeviceContext *pd3dDeviceContext)
{
	if (m_pLights && m_pd3dcbLights) UpdateShaderVariable(pd3dDeviceContext, m_pLights);
}

void CScene::SendMouseOverMessage(HWND hwnd, POINT & pt)
{
	if (GetCapture() == hwnd)
		m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_MOUSE_DOWN_OVER, &pt);
	else
		m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_MOUSE_UP_OVER, &pt);
}

void CScene::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	LIGHTS *pcbLight = (LIGHTS *)d3dMappedResource.pData;
	memcpy(pcbLight, pLights, sizeof(LIGHTS));
	pd3dDeviceContext->Unmap(m_pd3dcbLights, 0);
	pd3dDeviceContext->PSSetConstantBuffers(CB_PS_SLOT_LIGHT, 1, &m_pd3dcbLights);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::ProcessInput(HWND hWnd, float fTime, POINT & pt)
{
	return false;
}

void CScene::BuildMeshes(ID3D11Device * pd3dDevice)
{
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nShaders; i++)
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);

	if (m_pPlayerShader) 
		m_pPlayerShader->AnimateObjects(fTimeElapsed);

	EVENTMgr.Update(fTimeElapsed);
}

void CScene::Render(ID3D11DeviceContext*pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
	for (int i = 0; i < m_nShaders; ++i)
		m_ppShaders[i]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
}

void CScene::UIRender(ID3D11DeviceContext * pd3dDeviceContext)
{
	if (m_pUIShader) m_pUIShader->Render(pd3dDeviceContext, DRAW_AND_ACTIVE, nullptr);
}