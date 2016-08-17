#include "stdafx.h"
#include "MyInline.h"
#include "SceneInGame.h"
#include "SceneTitle.h"
#include "GameFramework.h"
#include "StatePlayer.h"
#include "SoundManager.h"
bool bIsKeyDown = false;
//bool bIsMouseDown = false;

CSceneInGame::CSceneInGame() : CScene()
{
	mSceneType = 0;
	m_nEffectShaderNum = 0;

	ZeroMemory(m_recvBuffer, sizeof(m_recvBuffer));
	//ZeroMemory(&m_fHeight, sizeof(m_fHeight));
}

CSceneInGame::CSceneInGame(int type)
{
	mSceneType = type;
	m_nEffectShaderNum = 0;

	ZeroMemory(m_recvBuffer, sizeof(m_recvBuffer));
	//ZeroMemory(&m_fHeight, sizeof(m_fHeight));
}

CSceneInGame::~CSceneInGame()
{
	SOUND_MGR.StopSoundBG(2);
}

void CSceneInGame::InitializeRecv()
{
	if (false == CLIENT.Setting(FRAMEWORK.m_hWnd, SERVER_PORT))
	{
		cout << "Server�� ������ ���� �ʾҽ��ϴ�." << endl;
	}
	CLIENT.SetPlayerShader(m_pPlayerShader);
	CLIENT.ReadPacket();
	// �÷��̾� ������ȣ ����, �� ��ġ ���� ���� �ް��� �����Ŵ
	CLIENT.SetAsyncSelect(); // Async Select�� �����Ѵ�.
}

void CSceneInGame::Reset()
{
	for (auto & shaders : m_vcResetShaders)
		shaders->Reset();
	SYSTEMMgr.ResetWaterHeight();
	//cout << "Reset" << endl;
//	EVENTMgr.InsertDelayMessage(SYSTEMMgr.mfLIMIT_ROUND_TIME,
//		eMessage::MSG_ROUND_END, CGameEventMgr::MSG_TYPE_SCENE, this, nullptr, m_pCamera->GetPlayer());
}

void CSceneInGame::BuildMeshes(ID3D11Device * pd3dDevice)
{
	SOUND_MGR.PlaySoundBG(2);
	wstring baseDir{ _T("../Assets/Image/Objects/") };
	vector<wstring> vcTxFileNames;

	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	CTexture * pTexture = nullptr;
	CMesh * pMesh = nullptr;
	char file[128];
	wchar_t result[128];

	if (mSceneType == 1)
		FILE_LOAD_Mgr.LoadSceneResources(pd3dDevice, &m_SceneResoucres, "../Assets/Data/Map00/Resources.txt");
	else if(mSceneType == 2)
		FILE_LOAD_Mgr.LoadSceneResources(pd3dDevice, &m_SceneResoucres, "../Assets/Data/Map01/Resources.txt");

	for (int i = 0; i < 6; ++i)
	{
		// ������ 1
		pTexture = new CTexture(2, 1, 0, 0);
		{
			wsprintf(result, _T("../Assets/Image/Objects/staff/Staff0%d_Diff.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			wsprintf(result, _T("../Assets/Image/Objects/staff/Staff0%d_Diff_NRM.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(1, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}
		sprintf(file, "../Assets/Image/Objects/staff/Staff0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 0.2f, vcTxFileNames);
		vcTxFileNames.clear();

		string ObjName = ITEMMgr.StaffNameArray[i][0];
		m_SceneResoucres.mgrTexture.InsertObject(pTexture, ObjName);
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, ObjName);
	}

	// ������2
	for (int i = 0; i < 7; ++i)
	{
		pTexture = new CTexture(3, 1, 0, 0);
		{
			wsprintf(result, _T("../Assets/Image/Objects/staff2/Staff2_0%d_Diff.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(0, pd3dsrvTexture);
			pd3dsrvTexture->Release();
			pTexture->SetTexture(1, nullptr);

			wsprintf(result, _T("../Assets/Image/Objects/staff2/Staff2_0%d_Spec.png"), i);
			ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, result, nullptr, nullptr, &pd3dsrvTexture, nullptr));
			pTexture->SetTexture(2, pd3dsrvTexture);
			pd3dsrvTexture->Release();

			pTexture->SetSampler(0, TXMgr.GetSamplerState("ss_linear_wrap"));
		}

		sprintf(file, "../Assets/Image/Objects/staff2/Staff2_0%d.fbxcjh", i);
		pMesh = new CLoadMeshByFbxcjh(pd3dDevice, file, 0.2f, vcTxFileNames);
		vcTxFileNames.clear();

		if (i != 6)
		{
			string ObjName = ITEMMgr.StaffNameArray[i][1];
			m_SceneResoucres.mgrTexture.InsertObject(pTexture, ObjName);
			m_SceneResoucres.mgrMesh.InsertObject(pMesh, ObjName);
		}
		else
		{
			string ObjName = ITEMMgr.StaffNameArray[0][2];
			m_SceneResoucres.mgrTexture.InsertObject(pTexture, ObjName);
			m_SceneResoucres.mgrMesh.InsertObject(pMesh, ObjName);
		}
	}
	
	FILE_LOAD_Mgr.LoadSceneAnimationObjects(pd3dDevice, &m_SceneResoucres, "../Assets/Data/Map01/AnimationList.txt");
	// �÷��̾� ĳ����bn
	{
		pMesh = new CLoadAnimatedMeshByADFile(pd3dDevice, "../Assets/Image/Objects/Skull/skeleton_0.ad", 3.5f, vcTxFileNames);
		pTexture = new CTexture(1, 0, 0, 0);
		ASSERT_S(D3DX11CreateShaderResourceViewFromFile(pd3dDevice, _T("../Assets/Image/Objects/Skull/Skull.png"), nullptr, nullptr, &pd3dsrvTexture, nullptr));
		pTexture->SetTexture(0, pd3dsrvTexture);
		pd3dsrvTexture->Release();
		vcTxFileNames.clear();

		m_SceneResoucres.mgrTexture.InsertObject(pTexture, "scene_skull");
		m_SceneResoucres.mgrMesh.InsertObject(pMesh, "scene_skull_0");
	}
}

void CSceneInGame::BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo)
{
	SYSTEMMgr.SetScene(this);
	m_SceneResoucres.sceneNum = mSceneType;

	m_nMRT     = NUM_MRT;
	//������ �����Ѵ�.
	CMaterial *pRedMaterial   = MaterialMgr.GetObjects("Red");
	CMaterial *pGreenMaterial = MaterialMgr.GetObjects("Green");
	CMaterial *pBlueMaterial  = MaterialMgr.GetObjects("Blue");
	CMaterial *pWhiteMaterial = MaterialMgr.GetObjects("White");

	int iCharacterShaderNum = -1;
	//�޽� ����
	BuildMeshes(pd3dDevice);
	{
		UINT index = 0;
		m_nShaders = NUM_SHADER;
		m_nThread = m_nShaders;
		m_ppShaders = new CShader*[m_nShaders];

		//��ī�̹ڽ��� �ͷ��� ���̴�
		CEnviromentShader * enviorment = new CEnviromentShader();
		enviorment->BuildObjects(pd3dDevice, m_SceneResoucres);
		m_vcStaticShadowShaders.push_back(enviorment);
		m_ppShaders[index++] = enviorment;

		iCharacterShaderNum = index;
		CCharacterShader *pCharShader = new CCharacterShader();
		pCharShader->CreateShader(pd3dDevice);
		pCharShader->BuildObjects(pd3dDevice, pWhiteMaterial, m_SceneResoucres);
		m_ppShaders[index++] = pCharShader;
		
		m_vcResetShaders.push_back(pCharShader);
		m_vcDynamicShadowShaders.push_back(pCharShader);

		CStaticModelingShader *pStaticShader = new CStaticModelingShader();
		pStaticShader->CreateShader(pd3dDevice);
		pStaticShader->BuildObjects(pd3dDevice, pWhiteMaterial, m_SceneResoucres);
		m_ppShaders[index++] = pStaticShader;
		
		m_vcStaticShadowShaders.push_back(pStaticShader);

		CStaticInstancingParentShader * pStInst = new CStaticInstancingParentShader();
		pStInst->CreateShader(pd3dDevice);
		pStInst->BuildObjects(pd3dDevice, nullptr, m_SceneResoucres);
		m_ppShaders[index++] = pStInst;
		
		m_vcStaticShadowShaders.push_back(pStInst);

		CPointInstanceShader *pPointShader = new CPointInstanceShader();
		pPointShader->CreateShader(pd3dDevice);
		pPointShader->BuildObjects(pd3dDevice, pWhiteMaterial);
		m_ppShaders[index++] = pPointShader;
		
		m_vcResetShaders.push_back(pPointShader);

		m_nEffectShaderNum = index;
		CEffectShader * pTxAni = new CEffectShader();
		pTxAni->CreateShader(pd3dDevice);
		pTxAni->BuildObjects(pd3dDevice);
		m_nEffectShaderNum = index;
		m_ppShaders[index++] = pTxAni;

		CWaterShader * pWaterShader = new CWaterShader();
		pWaterShader->CreateShader(pd3dDevice);
		pWaterShader->BuildObjects(pd3dDevice);
		m_ppShaders[index++] = pWaterShader;

		CSceneShader * pSceneShader = new CSceneShader();
		pSceneShader->CreateShader(pd3dDevice);
		pSceneShader->BuildObjects(pd3dDevice, SceneInfo->ppMRTSRVArray, 0, SceneInfo->pd3dBackRTV);
		pSceneShader->CreateConstantBuffer(pd3dDevice, pd3dDeviceContext);
		m_pSceneShader = pSceneShader;
	}
	{
		m_pPlayerShader = new CPlayerShader();
		m_pPlayerShader->CreateShader(pd3dDevice);
		m_pPlayerShader->BuildObjects(pd3dDevice, m_SceneResoucres, this);

		m_vcResetShaders.push_back(m_pPlayerShader);
		m_vcDynamicShadowShaders.push_back(m_pPlayerShader);

		SetCamera(m_pPlayerShader->GetPlayer()->GetCamera());
		CLIENT.SetPlayerShader(m_pPlayerShader);
		////////////// �÷��̾� ������ �� Build ������ ���� �ض�//////////////////////////////////////
//		InitializeRecv();
		cout << "Current Client ID : " << CLIENT.GetClientID() << endl;
		ChangeGamePlayerID(CLIENT.GetClientID());
		////////////// �÷��̾� ������ �� Build ������ ���� �ض�//////////////////////////////////////

		m_pCamera->SetViewport(pd3dDeviceContext, 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->GenerateViewMatrix();

		m_ppShaders[iCharacterShaderNum]->GetGameMessage(nullptr, eMessage::MSG_PASS_PLAYERPTR, m_pCamera->GetPlayer());
		CInGamePlayer* pPlayer = nullptr;
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(2));
		pPlayer->SetGravity(0);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(3));
		pPlayer->SetGravity(0);

	}
	{
		CInGameUIShader * pUIShader = new CInGameUIShader();
		pUIShader->CreateShader(pd3dDevice);
		pUIShader->BuildObjects(pd3dDevice, SceneInfo->pd3dBackRTV, this);
		m_pUIShader = pUIShader;
	}

	CreateShaderVariables(pd3dDevice);
	BuildStaticShadowMap(pd3dDeviceContext);
	PACKET_MGR.SendLoadEnd();
	SYSTEMMgr.GameReady();
	Reset();
	//SYSTEMMgr.GameStart();
}

void CSceneInGame::ReleaseObjects()
{
	CScene::ReleaseObjects();
	QUADMgr.ReleaseQuadTree();
	SYSTEMMgr.ReleaseScene();
}

void CSceneInGame::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	m_pLights = new LIGHTS;
	::ZeroMemory(m_pLights, sizeof(LIGHTS));
	//���� ���� ��ü�� ���ߴ� �ֺ������� �����Ѵ�.
	m_pLights->m_xcGlobalAmbient             = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);

	m_pLights->m_pLights[0].m_bEnable        = 1.0f;
	m_pLights->m_pLights[0].m_nType          = SPOT_LIGHT;
	m_pLights->m_pLights[0].m_fRange         = 100.0f;
	m_pLights->m_pLights[0].m_xcAmbient      = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[0].m_xcDiffuse      = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_pLights->m_pLights[0].m_xcSpecular     = XMFLOAT4(0.3f, 0.3f, 0.3f, 5.0f);
	m_pLights->m_pLights[0].m_xv3Position    = XMFLOAT3(500.0f, 300.0f, 500.0f);
	m_pLights->m_pLights[0].m_xv3Direction   = XMFLOAT3(0.0f, -1.f, 0.0f);
	m_pLights->m_pLights[0].m_xv3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights->m_pLights[0].m_fFalloff       = 8.0f;
	m_pLights->m_pLights[0].m_fPhi           = (float)cos(XMConvertToRadians(40.0f));
	m_pLights->m_pLights[0].m_fTheta         = (float)cos(XMConvertToRadians(20.0f));

	XMFLOAT3 pos = XMFLOAT3(1024, 0, 320);
	pos.y = MAPMgr.GetHeight(pos) + 10;

	m_pLights->m_pLights[1].m_bEnable        = 1.0f;//1.0f;
	m_pLights->m_pLights[1].m_nType          = POINT_LIGHT;
	m_pLights->m_pLights[1].m_fRange         = 20.0f;
	m_pLights->m_pLights[1].m_xcAmbient      = XMFLOAT4(0.5f, 0.0f, 0.0f, 1.0f);
	m_pLights->m_pLights[1].m_xcDiffuse      = XMFLOAT4(0.7f, 0.1f, 0.1f, 1.0f);
	m_pLights->m_pLights[1].m_xcSpecular     = XMFLOAT4(0.3f, 0.0f, 0.0f, 0.1f);
	m_pLights->m_pLights[1].m_xv3Position    = pos;
	m_pLights->m_pLights[1].m_xv3Direction   = XMFLOAT3(0.0f, -1.f, 0.0f);
	m_pLights->m_pLights[1].m_xv3Attenuation = XMFLOAT3(1.0f, 0.05f, 0.001f);

	pos = XMFLOAT3(1074, 0, 320);
	pos.y = MAPMgr.GetHeight(pos) + 10;

	m_pLights->m_pLights[2].m_bEnable        = 1.0f;//1.0f;
	m_pLights->m_pLights[2].m_nType          = POINT_LIGHT;
	m_pLights->m_pLights[2].m_fRange         = 20.0f;
	m_pLights->m_pLights[2].m_xcAmbient      = XMFLOAT4(0.0f, 0.0f, 0.4f, 1.0f);
	m_pLights->m_pLights[2].m_xcDiffuse      = XMFLOAT4(0.1f, 0.1f, 0.2f, 1.0f);
	m_pLights->m_pLights[2].m_xcSpecular     = XMFLOAT4(0.0f, 0.0f, 0.1f, 0.1f);
	m_pLights->m_pLights[2].m_xv3Position    = pos;
	m_pLights->m_pLights[2].m_xv3Direction   = XMFLOAT3(0.0f, -1.0f, 0.0f);
	m_pLights->m_pLights[2].m_xv3Attenuation = XMFLOAT3(1.0f, 0.05f, 0.001f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(d3dBufferDesc));
	d3dBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth      = sizeof(LIGHTS);
	d3dBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem        = m_pLights;
	if (!m_pd3dcbLights) pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dcbLights);
}

void CSceneInGame::ReleaseShaderVariables()
{
	if (m_pLights)		delete m_pLights;
	if (m_pd3dcbLights) m_pd3dcbLights->Release();
}

void CSceneInGame::BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext)
{
	// ���⼺ ������ ��ġ �ʿ� ����.
	//LIGHT * pLight = m_pScene->GetLight(2);
	CMapManager * pTerrain = &MAPMgr;
	float fHalf = MAPMgr.GetWidth() * 0.5f;//pTerrain->GetWidth() * 0.5;

	CShadowMgr * pSdwMgr = &ShadowMgr;
	pSdwMgr->BuildShadowMap(pd3dDeviceContext, XMFLOAT3(fHalf, 0.0f, fHalf), XMFLOAT3(fHalf + 80.f, 80.f, fHalf), fHalf);

	const UINT uRenderState = (RS_SHADOWMAP);
	pSdwMgr->SetStaticShadowMap(pd3dDeviceContext, m_pCamera);

	for(auto & shader : m_vcStaticShadowShaders)
		shader->Render(pd3dDeviceContext, uRenderState, m_pCamera);

	pSdwMgr->ResetStaticShadowMap(pd3dDeviceContext, m_pCamera);
	pSdwMgr->UpdateStaticShadowResource(pd3dDeviceContext);
}

void CSceneInGame::PreProcessing(ID3D11DeviceContext * pd3dDeviceContext)
{
	UINT uRenderState = (NOT_PSUPDATE | RS_SHADOWMAP | DRAW_AND_ACTIVE);

	//CHeightMapTerrain * pTerrain = GetTerrain();
	float fHalf = 100.0f;//pTerrain->GetWidth() * 0.3;
	XMFLOAT3 xmfTarget = m_pCamera->GetPlayer()->GetPosition();
	XMFLOAT3 xmfLight = xmfTarget;
	
	xmfLight.x += 20.0f;
	xmfLight.y += 20.0f;

	CShadowMgr * pSdwMgr = &ShadowMgr;
	pSdwMgr->BuildShadowMap(pd3dDeviceContext, xmfTarget, xmfLight, fHalf);
	pSdwMgr->SetDynamicShadowMap(pd3dDeviceContext, m_pCamera);

	m_pPlayerShader->Render(pd3dDeviceContext, uRenderState, m_pCamera);
	m_ppShaders[1]->Render(pd3dDeviceContext, uRenderState, m_pCamera);
	m_ppShaders[2]->Render(pd3dDeviceContext, uRenderState, m_pCamera);

	pSdwMgr->ResetDynamicShadowMap(pd3dDeviceContext, m_pCamera);
	pSdwMgr->UpdateDynamicShadowResource(pd3dDeviceContext);

	uRenderState = NULL;
}

void CSceneInGame::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbLights, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	LIGHTS *pcbLight = (LIGHTS *)d3dMappedResource.pData;
	memcpy(pcbLight, pLights, sizeof(LIGHTS));
	pd3dDeviceContext->Unmap(m_pd3dcbLights, 0);
	pd3dDeviceContext->PSSetConstantBuffers(CB_PS_SLOT_LIGHT, 1, &m_pd3dcbLights);
}

bool CSceneInGame::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer());

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'B' :
		case 'X':
		case 'C':
			((CEffectShader*)m_ppShaders[m_nEffectShaderNum])->ShaderKeyEventOn(m_pPlayerShader->GetPlayer(), wParam, nullptr);
			return(false);

		case 'P':

			//pPlayer->ChangeAnimationState(eANI_DEATH_FRONT, true, nullptr, 0);
			return false;

		//case 'Z':
		//	this->GetGameMessage(nullptr, eMessage::MSG_ROUND_END);
			//SYSTEMMgr.RoundEnd();
			return false;

		case 'G':
		case 'E':
		//case 'N':
		case 'M':
			pPlayer->PlayerKeyEventOn(wParam, this);
			return(false);

		case 'Z':
			pPlayer->Jump();
			PACKET_MGR.SendJumpPacket();
			return false;
		}
		return(false);

	case WM_KEYUP:
		switch (wParam)
		{
#if 0
		case VK_UP:
		case VK_DOWN:
		case VK_LEFT:
		case VK_RIGHT:
#endif

		case '0':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':

		case 'W' :
		case 'S' :
		case 'A' :
		case 'D' :
		case 'G':
		case 'J':
		case 'K':
		case VK_HOME:
		case VK_END:
		case VK_DELETE:
			pPlayer->PlayerKeyEventOff(wParam, this);
			return(false);
		}
	}
	return(false);
}

bool CSceneInGame::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (CScene::OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam)) return true;

	switch (nMessageID)
	{
	case WM_RBUTTONUP:
		CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer());
		if (pPlayer->GetFSM()->isInState(CPlayerJumpState::GetInstance())) 
			return false;

		int result = pPlayer->UseMagic();
		if (0 < result)
		{
			CPacketMgr::SendBehaviorPacket();
		}
		else if (0 == result)
		{
			m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_UI_DRAW_NEED_ELEMENT);
		}
		return true;
	}
	return false;
}

bool CSceneInGame::ProcessInput(HWND hWnd, float fFrameTime, POINT & pt)
{
	static UCHAR pKeyBuffer[256];
	//GetKeyboardState(pKeyBuffer);
	float cxDelta = 0.0f, cyDelta = 0.0f;
	/*���콺�� ĸ�������� ���콺�� �󸶸�ŭ �̵��Ͽ��� ���� ����Ѵ�. ���콺 ���� �Ǵ� ������ ��ư�� ������ ���� �޽���(WM_LBUTTONDOWN, WM_RBUTTONDOWN)�� ó���� �� ���콺�� ĸ���Ͽ���. �׷��Ƿ� ���콺�� ĸ�ĵ� ���� ���콺 ��ư�� ������ ���¸� �ǹ��Ѵ�. ���콺�� �¿� �Ǵ� ���Ϸ� �����̸� �÷��̾ x-�� �Ǵ� y-������ ȸ���Ѵ�.*/
	if (GetCapture() == hWnd)
	{
		//���� ���콺 Ŀ���� ��ġ�� �����´�.
		//GetCursorPos(&ptCursorPos);
		//���콺 ��ư�� ���� ä�� ���� ��ġ���� ���� ���콺 Ŀ���� ��ġ���� ������ ���� ���Ѵ�.
		cxDelta = (float)(pt.x - m_ptOldCursorPos.x) * 0.5f;
		cyDelta = (float)(pt.y - m_ptOldCursorPos.y) * 0.5f;

		//ClientToScreen(hWnd, &m_ptOldCursorPos);
		SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
		//m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_MOUSE_DOWN_OVER, nullptr);
	}
	//else
		//m_pUIShader->GetGameMessage(nullptr, eMessage::MSG_MOUSE_UP_OVER, nullptr);
	CPlayer * pPlayer = m_pCamera->GetPlayer();
	//cout << "Num : " << pPlayer->GetPlayerNum() << endl;
	{
		DWORD dwDirection = 0;
		/*Ű������ ���� ������ ��ȯ�Ѵ�. ȭ��ǥ Ű(���桯, ���硯, ���衯, ���顯)�� ������ �÷��̾ ������/����(���� x-��), ��/��(���� z-��)�� �̵��Ѵ�. ��Page Up���� ��Page Down�� Ű�� ������ �÷��̾ ��/�Ʒ�(���� y-��)�� �̵��Ѵ�.*/
		if (GetKeyboardState(pKeyBuffer))
		{
#if 0
			if (pKeyBuffer[VK_UP]    & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer[VK_DOWN]  & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer[VK_LEFT]  & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
			//if (pKeyBuffer['W'] & 0x0F) cout << "WF!!";
			//if (pKeyBuffer['W'] & 0xF0) cout << "FW!!";
#endif
			if (pKeyBuffer['W'] & 0xF0) dwDirection |= DIR_FORWARD;
			if (pKeyBuffer['S'] & 0xF0) dwDirection |= DIR_BACKWARD;
			if (pKeyBuffer['A'] & 0xF0) dwDirection |= DIR_LEFT;
			if (pKeyBuffer['D'] & 0xF0) dwDirection |= DIR_RIGHT;
		}
		//�÷��̾ �̵��ϰų�(dwDirection) ȸ���Ѵ�(cxDelta �Ǵ� cyDelta).
		const float fToTalDelta = cxDelta + cyDelta;
		if ( dwDirection || (cxDelta != 0.0f) || (cyDelta != 0.0f))
		{
			if (fToTalDelta)
			{
				if (!(pKeyBuffer[VK_RBUTTON] & 0xF0))
				{
					if (pPlayer->GetStatus().IsAlive())
					{
						pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
						PACKET_MGR.SendRotatePacket(pPlayer);
					}
					else 
						m_pCamera->Rotate(cyDelta, cxDelta, 0.0f);
				}
			}
			/*�÷��̾ dwDirection �������� �̵��Ѵ�(�����δ� �ӵ� ���͸� �����Ѵ�). �̵� �Ÿ��� �ð��� ����ϵ��� �Ѵ�. �÷��̾��� �̵� �ӷ��� (50/��)�� �����Ѵ�. ���� �÷��̾��� �̵� �ӷ��� �ִٸ� �� ���� ����Ѵ�.*/
		}
		//�÷��̾ ������ �̵��ϰ� ī�޶� �����Ѵ�. �߷°� �������� ������ �ӵ� ���Ϳ� �����Ѵ�.
		if (dwDirection != pPlayer->GetDirection())
		{
			PACKET_MGR.SendPositionPacket(pPlayer, dwDirection);
		}
		pPlayer->SetDirection(dwDirection);
	}
	if (pKeyBuffer[VK_SPACE] & 0xF0)
	{
		FRAMEWORK.ChangeGameScene(new CSceneTitle());

		ZeroMemory(pKeyBuffer, 256);
		SetKeyboardState(pKeyBuffer);
	}



	return false;
}

void CSceneInGame::LightUpdate(float fTimeElapsed)
{
	
	if (m_pLights && m_pd3dcbLights)
	{
		//���� ī�޶��� ��ġ ���͸� ������ ��Ÿ���� ��� ���ۿ� �����Ѵ�.
		XMFLOAT3 xv3CameraPosition = m_pCamera->GetPosition();
		m_pLights->m_xv4CameraPosition = XMFLOAT4(xv3CameraPosition.x, xv3CameraPosition.y, xv3CameraPosition.z, 1.0f);

		/*�� ��° ������ �÷��̾ ������ �ִ� ������(���� ����)�̴�. �׷��Ƿ� �÷��̾��� ��ġ�� ������ �ٲ�� ���� �÷��̾��� ��ġ�� z-�� ���� ���͸� ���� ������ ��ġ�� �������� �����Ѵ�.*/
		CPlayer *pPlayer = m_pCamera->GetPlayer();
		m_pLights->m_pLights[0].m_xv3Position = pPlayer->GetPosition();
		m_pLights->m_pLights[0].m_xv3Direction = pPlayer->GetLookVector();

		//XMFLOAT3 pos = m_pPlayerShader->GetPlayer(0)->GetPosition();
		//pos.y += 20.f;
		//m_pLights->m_pLights[2].m_xv3Position = pos;
		//pos = m_pPlayerShader->GetPlayer(1)->GetPosition();
		//pos.y += 20.f;
		//m_pLights->m_pLights[3].m_xv3Position = pos;
	}

}

void CSceneInGame::AnimateObjects(float fTimeElapsed)
{
	LightUpdate(fTimeElapsed);

	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->AnimateObjects(fTimeElapsed);
	}
	m_pPlayerShader->AnimateObjects(fTimeElapsed);
	m_pUIShader->AnimateObjects(fTimeElapsed);

#ifdef _QUAD_TREE
	QUADMgr.Update(m_pCamera);
#endif
	EVENTMgr.Update(fTimeElapsed);
	SYSTEMMgr.Update(fTimeElapsed);
}

void CSceneInGame::Render(ID3D11DeviceContext*pd3dDeviceContext, RENDER_INFO * pRenderInfo)
{
#ifdef _THREAD
	int index = pRenderInfo->ThreadID;

	if (index == m_nShaders - 1)
	{
		m_pPlayerShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
//#ifndef _DEBUG
	//if (index == m_nThread - 1)
	//{
	//	UpdateLights(pd3dDeviceContext);
	//	pd3dDeviceContext->OMSetRenderTargets(1, &(pRenderInfo->ppd3dMrtRTV[MRT_SCENE]), nullptr);
	//	ShadowMgr.UpdateStaticShadowResource(pd3dDeviceContext);
	//	m_pSceneShader->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	//	return;
	//}
//#endif
	m_ppShaders[index]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
#else
	for (int i = 0; i < m_nShaders; i++)
	{
		m_ppShaders[i]->Render(pd3dDeviceContext, *pRenderInfo->pRenderState, pRenderInfo->pCamera);
	}
#endif
	//m_ppShaders[m_nShaders - 1]->Render(pd3dDeviceContext, pCamera);
}

void CSceneInGame::UIRender(ID3D11DeviceContext * pd3dDeviceContext)
{
	m_pUIShader->Render(pd3dDeviceContext, DRAW_AND_ACTIVE, m_pCamera);
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
CHeightMapTerrain *CSceneInGame::GetTerrain()
{
	CTerrainShader *pTerrainShader = (CTerrainShader *)m_ppShaders[m_uHeightMapIndex];
	return(pTerrainShader->GetTerrain());
}
#endif

void CSceneInGame::GetGameMessage(CScene * byObj, eMessage eMSG, void * extra)
{
	//CInGamePlayer * pPlayer = static_cast<CInGamePlayer*>(extra);

	switch (eMSG)
	{
	case eMessage::MSG_ITEM_STAFF_CHANGE:
		static_cast<CInGameUIShader*>(m_pUIShader)->ChangeItemUI(static_cast<CStaff*>(extra));
		return;
	
	case eMessage::MSG_MAGIC_CAST:
	case eMessage::MSG_PARTICLE_ON:
	case eMessage::MSG_MAGIC_SHOT:
	case eMessage::MSG_MAGIC_AREA:
		m_ppShaders[m_nEffectShaderNum]->GetGameMessage(nullptr, eMSG, extra);
		return;
	
	case eMessage::MSG_MOUSE_DOWN:
		m_ptOldCursorPos = *(POINT*)extra;
	case eMessage::MSG_MOUSE_DOWN_OVER:
		m_pUIShader->GetGameMessage(nullptr, eMSG, extra);
		return;

	case eMessage::MSG_MOUSE_UP:
		m_pUIShader->GetGameMessage(nullptr, eMSG, extra);
	case eMessage::MSG_MOUSE_UP_OVER:
		return;

	case eMessage::MSG_EFFECT_RADIAL_ON :
	case eMessage::MSG_EFFECT_RADIAL_OFF:
	case eMessage::MSG_EFFECT_GLARE_ON  :
	case eMessage::MSG_EFFECT_GLARE_OFF :
		m_pSceneShader->GetGameMessage(nullptr, eMSG);
		return;

	case eMessage::MSG_ROUND_ENTER:
		SYSTEMMgr.RoundEnter();
		Reset();
		return;

	case eMessage::MSG_ROUND_START:
		SYSTEMMgr.RoundStart();
		m_pPlayerShader->RoundStart();
		EVENTMgr.InsertDelayMessage(10.0f, eMessage::MSG_ROUND_DEATH_MATCH, CGameEventMgr::MSG_TYPE_SCENE, this);
		return;

	case eMessage::MSG_ROUND_DEATH_MATCH:
		SYSTEMMgr.DeathMatchStart();
		return;

	case eMessage::MSG_ROUND_END:
		m_pPlayerShader->RoundEnd();
		SYSTEMMgr.RoundEnd();

		if (SYSTEMMgr.IsWinPlayer(static_cast<CInGamePlayer*>(m_pCamera->GetPlayer())))
			static_cast<CInGameUIShader*>(m_pUIShader)->UIReadyWinLogo(true);
		else
			static_cast<CInGameUIShader*>(m_pUIShader)->UIReadyLoseLogo(true);
		return;

	case eMessage::MSG_ROUND_CLEAR:
		static_cast<CInGameUIShader*>(m_pUIShader)->UIReadyLoseLogo(false);
		SYSTEMMgr.RoundClear();
		return;

	case eMessage::MSG_GAME_END:
		CLIENT.CloseConnect();
		FRAMEWORK.ChangeGameScene(new CSceneTitle());
		return;
	}
}

void CSceneInGame::SendGameMessage(CScene * toObj, eMessage eMSG, void * extra)
{
}

bool CSceneInGame::PacketProcess(LPARAM lParam)
{
	CLIENT.ReadPacket();
	//static const int SC_PUT_PLAYER = 1;
	static bool bFirstTime = true;
//	static int mId = -1;
	char * buffer = CLIENT.GetRecvBuffer();
	CInGamePlayer * pPlayer = nullptr;
	
	switch (buffer[1])
	{
	case SC_PUT_PLAYER:
	{	
		sc_packet_put_player * my_packet = reinterpret_cast<sc_packet_put_player *>(buffer);
		int id = my_packet->id;

		if (bFirstTime)
		{
			bFirstTime = false;
		}
		if (id != CLIENT.GetClientID())
		{
			//m_pPlayerShader->SetPlayerID(FRAMEWORK.GetDevice(), id);
			//	m_pPlayerShader->GetPlayer(id);
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(id));
			//pPlayer->SetPlayerNum(id);
			pPlayer->InitPosition(XMFLOAT3(my_packet->x, my_packet->y, my_packet->z));
			pPlayer->SetVisible(true);
		}
		//pPlayer->SetActive(true);
		break;
	}
	case SC_POS:
	{
		sc_packet_pos *my_packet = reinterpret_cast<sc_packet_pos *>(buffer);
		
		int other_id = my_packet->id;
		
		if (other_id == CLIENT.GetClientID())
		{
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(CLIENT.GetClientID()));
			if (pPlayer != nullptr)
			{
				pPlayer->SetPosition(my_packet->Position);
			}
		}

		if (other_id != CLIENT.GetClientID())
		{
			pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(other_id));
			if (pPlayer != NULL)
			{
				pPlayer->RenewPacket(*my_packet);
				if (my_packet->animationType == eANI_IDLE)
					pPlayer->ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
			}
		}

		break;
	}
	case SC_ROUND_TIME:
	{
		break;
	}
	case SC_GAME_STATE:
	{
		sc_packet_GameState* my_packet = reinterpret_cast<sc_packet_GameState*>(buffer);
		my_packet->gamestate;
		break;
	}
	case SC_PLAYER_INFO:
	{
		sc_packet_playerInfo* my_packet = reinterpret_cast<sc_packet_playerInfo*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		
		auto info = SYSTEMMgr.GetPlayerInfo();
		info[my_packet->id].m_nKillCount = my_packet->kill;
		info[my_packet->id].m_iPlayerPoint = my_packet->point;
		info[my_packet->id].m_nDeathCount = my_packet->death;
		//pPlayer->
		break;
	}
	case  SC_ROTATION:
	{

		sc_packet_rotate* my_packet = reinterpret_cast<sc_packet_rotate*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		pPlayer->RenewPacket(*my_packet);
		//pPlayer->Rotate(my_packet->cyDelta, my_packet->cxDelta, 0.0f);
		break;
	}
	case SC_DOMINATE:
	{
		sc_packet_dominate* my_packet = reinterpret_cast<sc_packet_dominate*>(buffer);
		SYSTEMMgr.DominatePortalGate(my_packet->id);


		break;
	}
	case SC_MAGIC_CASTING:
	{
		sc_packet_Behavior* my_packet = reinterpret_cast<sc_packet_Behavior*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		pPlayer->MagicShot();
		break;
	}
	case SC_ANI_IDLE:
	{
		sc_packet_Behavior* my_packet = reinterpret_cast<sc_packet_Behavior*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		break;
	}
	case SC_JUMP:
	{
		sc_packet_Jump* my_packet = reinterpret_cast<sc_packet_Jump*>(buffer);
		pPlayer = static_cast<CInGamePlayer*>(m_pPlayerShader->GetPlayer(my_packet->id));
		if(my_packet->id != CLIENT.GetClientID())
			pPlayer->Jump();
		break;
	}
	case SC_SERVER_CHEAT:
	{
		sc_packet_serverCherat* my_packet = reinterpret_cast<sc_packet_serverCherat*>(buffer);
		short temoMode = my_packet->mode;
		if (temoMode == 2)
			SYSTEMMgr.ResetWaterHeight();
		else if (temoMode == 3)
			SYSTEMMgr.IncreaseWaterHetght();
		else if (temoMode == 4)
			SYSTEMMgr.DecreaseWaterHetght();
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", buffer[1]);
		break;
	}

	return false;
}
