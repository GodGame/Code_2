#include "stdafx.h"
#include "MyInline.h"
#include "SceneTitle.h"
#include "SceneInGame.h"
#include "SceneLobby.h"
#include "GameFramework.h"
#include "SoundManager.h"
//CTitleScreenShader * pTitle = nullptr;

CSceneTitle::CSceneTitle() : CScene()
{
}

CSceneTitle::~CSceneTitle()
{
	SOUND_MGR.StopSoundBG(0);
}

void CSceneTitle::BuildMeshes(ID3D11Device * pd3dDevice)
{
}

void CSceneTitle::BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo)
{
	m_nThread = m_nShaders = 0;
	//m_ppShaders = new CShader*[m_nShaders];

	CTitleScreenShader * pTitle = new CTitleScreenShader(); //new CUIShader();
	pTitle->CreateShader(pd3dDevice);
	pTitle->BuildObjects(pd3dDevice, SceneInfo->pd3dBackRTV, this);
	m_pUIShader = pTitle;
	//m_ppShaders[0] = pTitle;
	toggle = false;
	SOUND_MGR.PlaySoundBG(0);
	CreateShaderVariables(pd3dDevice);
}

void CSceneTitle::ReleaseObjects()
{
	CScene::ReleaseObjects();
}

void CSceneTitle::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
}

void CSceneTitle::ReleaseShaderVariables()
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CSceneTitle::GetGameMessage(CScene * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_SCENE_CHANGE:
		FRAMEWORK.ChangeGameScene(new CSceneLobby());
		return;
	case eMessage::MSG_MOUSE_DOWN:
	case eMessage::MSG_MOUSE_DOWN_OVER:
	case eMessage::MSG_MOUSE_UP:
	case eMessage::MSG_MOUSE_UP_OVER:
		if(m_pUIShader) m_pUIShader->GetGameMessage(nullptr, eMSG, extra);
		return;
	}
}

void CSceneTitle::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
}

bool CSceneTitle::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			//			FRAMEWORK.ChangeGameScene(new CSceneInGame());
			break;
		}
		break;
	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_SPACE:
			toggle = false;
			cout << boolalpha << toggle << endl;
			//FRAMEWORK.ChangeGameScene(new CSceneInGame());
			break;
		}

		break;
	}
	}
	return(false);
}

bool CSceneTitle::ProcessInput(HWND hWnd, float fTime, POINT & pt)
{
	static UCHAR pKeyBuffer[256];
	
	SendMouseOverMessage(hWnd, pt);
	
	if (GetKeyboardState(pKeyBuffer))
	{
		if (false == toggle)
		{
			if (pKeyBuffer[VK_SPACE] & 0xF0)
			{
				toggle = true;
				cout << boolalpha << toggle << endl;
				FRAMEWORK.ChangeGameScene(new CSceneLobby());
				return true;
			}
		}
	}

	return false;
}

void CSceneTitle::Render(ID3D11DeviceContext * pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; ++i)
			m_ppShaders[i]->Render(pd3dDeviceContext, 0, nullptr);
	}
}


#ifdef PICKING
CGameObject *CScene::PickObjectPointedByCursor(int xClient, int yClient)
{
	if (!m_pCamera) return(nullptr);

	XMFLOAT4X4 xmtxView = m_pCamera->GetViewMatrix();
	XMFLOAT4X4 xmtxProjection = m_pCamera->GetProjectionMatrix();
	D3D11_VIEWPORT d3dViewport = m_pCamera->GetViewport();

	XMFLOAT3 xv3PickPosition;
	/*ȭ�� ��ǥ���� �� (xClient, yClient)�� ȭ�� ��ǥ ��ȯ�� ����ȯ�� ���� ��ȯ�� ����ȯ�� �Ѵ�. �� ����� ī�޶� ��ǥ���� ���̴�. ���� ����� ī�޶󿡼� z-������ �Ÿ��� 1�̹Ƿ� z-��ǥ�� 1�� �����Ѵ�.*/
	xv3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmtxProjection._11;
	xv3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmtxProjection._22;
	xv3PickPosition.z = 1.0f;

	int nIntersected = 0;
	float fNearHitDistance = FLT_MAX;
	MESHINTERSECTINFO d3dxIntersectInfo;
	CGameObject *pIntersectedObject = nullptr, *pNearestObject = nullptr;
	//���� ��� ���̴� ��ü�� ���Ͽ� ��ŷ�� ó���Ͽ� ī�޶�� ���� ����� ��ŷ�� ��ü�� ã�´�.
	for (int i = 0; i < m_nShaders; i++)
	{
		pIntersectedObject = m_ppShaders[i]->PickObjectByRayIntersection(&xv3PickPosition, &xmtxView, &d3dxIntersectInfo);
		if (pIntersectedObject && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
		{
			fNearHitDistance = d3dxIntersectInfo.m_fDistance;
			pNearestObject = pIntersectedObject;
		}
	}
	return(pNearestObject);
}
#endif