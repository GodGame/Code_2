#pragma once
#ifndef __INGAME__
#define __INGAME__

#include "Scene.h"

class CSceneInGame : public CScene
{
	//UINT m_nParticleShaderNum;
	CHAR m_nEffectShaderNum;

	POINT m_ptOldCursorPos;
	UINT  m_uHeightMapIndex;

	vector<CShader*> m_vcResetShaders;
	vector<CShader*> m_vcStaticShadowShaders;
	vector<CShader*> m_vcDynamicShadowShaders;
	char		m_recvBuffer[1024];
	//float m_fHeight[1000];
	int m_iMapWidth;
	int m_iMapHeight;
	int mSceneType;

public:
	CSceneInGame();
	CSceneInGame(int type);
	virtual ~CSceneInGame();
	
	void InitializeRecv();
	void Reset();
	void LightUpdate(float fTimeElapsed);

	virtual bool PacketProcess(LPARAM lParam);
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual bool ProcessInput(HWND hWnd, float fTime, POINT & pt);

	virtual void BuildMeshes(ID3D11Device * pd3dDevice);
	virtual void BuildObjects(ID3D11Device *pd3dDevice, ID3D11DeviceContext * pd3dDeviceContext, ShaderBuildInfo * SceneInfo);
	virtual void ReleaseObjects();

	virtual void AnimateObjects(float fTimeElapsed);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, RENDER_INFO * pRenderInfo);
	virtual void UIRender(ID3D11DeviceContext *pd3dDeviceContext);

	virtual void CreateShaderVariables(ID3D11Device *pd3dDevice);
	virtual void UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, LIGHTS *pLights);
	virtual void ReleaseShaderVariables();

	virtual void BuildStaticShadowMap(ID3D11DeviceContext * pd3dDeviceContext);
	virtual void PreProcessing(ID3D11DeviceContext * pd3dDeviceContext);

	virtual void GetGameMessage(CScene * byObj, eMessage eMSG, void * extra = nullptr);
	virtual void SendGameMessage(CScene * toObj, eMessage eMSG, void * extra = nullptr);
public:
#ifdef PICKING
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient);
#endif
	//CHeightMapTerrain *GetTerrain();
};

#endif