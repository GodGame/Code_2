#include "stdafx.h"
#include "MyInline.h"
#include "SceneLobby.h"
#include "SceneInGame.h"
#include "GameFramework.h"
#include "SoundManager.h"
CSceneLobby::CSceneLobby() : CScene()
{
}

CSceneLobby::~CSceneLobby()
{
	SOUND_MGR.StopSoundBG(1);
}

void CSceneLobby::InitializeRecv()
{
	if (false == CLIENT.Setting(FRAMEWORK.m_hWnd, SERVER_PORT))
	{
		cout << "Server�� ������ ���� �ʾҽ��ϴ�." << endl;
	}
	//	CLIENT.SetPlayerShader(m_pPlayerShader);
	//	CLIENT.ReadPacket();
	// �÷��̾� ������ȣ ����, �� ��ġ ���� ���� �ް��� �����Ŵ
	CLIENT.SetAsyncSelect(); // Async Select�� �����Ѵ�.
}

void CSceneLobby::BuildMeshes(ID3D11Device * pd3dDevice)
{
}

void CSceneLobby::BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo)
{
	m_nThread = m_nShaders = 0;

	CLobbyScreenShader * pTitle = new CLobbyScreenShader(); //new CUIShader();
	pTitle->CreateShader(pd3dDevice);
	pTitle->BuildObjects(pd3dDevice, SceneInfo->pd3dBackRTV, this);
	m_pUIShader = pTitle;
	SOUND_MGR.PlaySoundBG(1);
	InitializeRecv();
	CreateShaderVariables(pd3dDevice);
}

void CSceneLobby::ReleaseObjects()
{
	CScene::ReleaseObjects();
}

void CSceneLobby::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
}

void CSceneLobby::ReleaseShaderVariables()
{
	if (m_pLights) delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CSceneLobby::GetGameMessage(CScene * byObj, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_SCENE_CHANGE:
		//FRAMEWORK.ChangeGameScene(new CSceneInGame(1));
		return;
	case eMessage::MSG_MOUSE_DOWN:
	case eMessage::MSG_MOUSE_DOWN_OVER:
	case eMessage::MSG_MOUSE_UP:
	case eMessage::MSG_MOUSE_UP_OVER:
		if (m_pUIShader) m_pUIShader->GetGameMessage(nullptr, eMSG, extra);
		return;
	}
}

void CSceneLobby::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
}

bool CSceneLobby::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_SPACE:
			//FRAMEWORK.ChangeGameScene(new CSceneInGame());
			break;
		case VK_INSERT:
		{
			cout << "���ȳ�?" << endl;
			if (true == CLIENT.GetReadyState())
			{
				CLIENT.SetReadyState(false);
				PACKET_MGR.SendReadyPacket(CLIENT.GetClientID(), false);
			}
			else if (false == CLIENT.GetReadyState())
			{
				CLIENT.SetReadyState(true);
				PACKET_MGR.SendReadyPacket(CLIENT.GetClientID(), true);
			}
			break;
		}
		case '1':
		{
			cout << "1�� �������� ����" << endl;
			PACKET_MGR.SendLobbyStatePacket(1);
			break;
		}
		case  '2':
		{
			cout << "2�� �������� ����" << endl;
			PACKET_MGR.SendLobbyStatePacket(2);
			break;
		}
		break;
		}

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

bool CSceneLobby::ProcessInput(HWND hWnd, float fTime, POINT & pt)
{
	static UCHAR pKeyBuffer[256];

	SendMouseOverMessage(hWnd, pt);

	if (GetKeyboardState(pKeyBuffer))
	{
		/*	if (pKeyBuffer['1'] & 0xF0)
			{
				FRAMEWORK.ChangeGameScene(new CSceneInGame(1));
				return true;
			}
			if (pKeyBuffer['2'] & 0xF0)
			{
				FRAMEWORK.ChangeGameScene(new CSceneInGame(2));
				return true;
			}*/
		if (false == toggle)
		{
			if (pKeyBuffer[VK_SPACE] & 0xF0)
			{
				toggle = true;
				cs_packet_RequestStart request_Packet;
				request_Packet.size = sizeof(cs_packet_RequestStart);
				request_Packet.type = CS_SIGNAL;
				/*movePacket.direction = direction;
				movePacket.Position = pPlayer->GetPosition();
				movePacket.LookVector = pPlayer->GetLookVector();
				movePacket.animation = pPlayer->GetAnimationState();*/

				PACKET_MGR.Send(CLIENT.GetClientSocket(), reinterpret_cast<CHAR*>(&request_Packet), request_Packet.size);
				cout << boolalpha << toggle << endl;
				//	FRAMEWORK.ChangeGameScene(new CSceneInGame());
				return true;
			}
		}
	}

	return false;
}

//void CSceneLobby::AnimateObjects(float fTimeElapsed)
//{
//	EVENTMgr.Update(fTimeElapsed);
//}

void CSceneLobby::Render(ID3D11DeviceContext * pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
	if (m_ppShaders)
	{
		for (int i = 0; i < m_nShaders; ++i)
			m_ppShaders[i]->Render(pd3dDeviceContext, 0, nullptr);
	}
}

//void CSceneLobby::UIRender(ID3D11DeviceContext * pd3dDeviceContext)
//{
//	pTitle->Render(pd3dDeviceContext, 0, nullptr);
//}
