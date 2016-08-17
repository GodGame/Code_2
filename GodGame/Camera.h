#pragma once

#include "Mesh.h"

#define FIRST_PERSON_CAMERA	0x01
#define SPACESHIP_CAMERA	0x02
#define THIRD_PERSON_CAMERA	0x03
#define ASPECT_RATIO 	(float(FRAME_BUFFER_WIDTH) / float(FRAME_BUFFER_HEIGHT))

struct VS_CB_VIEWPROJECTION
{
	XMFLOAT4X4  m_xmf44View;
	XMFLOAT4  m_xf3CameraPos;
};

struct VS_CB_CAMERAPOS
{
	XMFLOAT4  m_xf3CameraPos;
};

class CPlayer;
class AABB;
class CCamera
{
public:
	CCamera(CCamera * pCamera);
	virtual ~CCamera();

protected:
	//����ü�� 6�� ���(���� ��ǥ��)�� ��Ÿ����.
	XMFLOAT4 m_xmpFrustumPlanes[6];

	//ī�޶��� ��ġ(������ǥ��) �����̴�.
	XMFLOAT3 m_xv3Position;
	/* ī�޶��� ���� x - ��(Right), y - ��(Up), z - ��(Look)�� ��Ÿ���� �����̴�.*/
	XMFLOAT3 m_xv3Right;
	XMFLOAT3 m_xv3Up;
	XMFLOAT3 m_xv3Look;

	//ī�޶� x-��, z-��, y-������ �󸶸�ŭ ȸ���ߴ� ���� ��Ÿ���� �����̴�.
	float m_fPitch;
	float m_fRoll;
	float m_fYaw;

	//ī�޶��� ����(1��Ī ī�޶�, �����̽�-�� ī�޶�, 3��Ī ī�޶�)�� ��Ÿ����.
	DWORD m_nMode;

	//ī�޶� �ٶ󺸴� ��(������ǥ��)�� ��Ÿ���� �����̴�.
	XMFLOAT3 m_xv3LookAtWorld;
	//�÷��̾�� ī�޶��� �������� ��Ÿ���� �����̴�. �ַ� 3��Ī ī�޶󿡼� ���ȴ�.
	XMFLOAT3 m_xv3Offset;
	//�÷��̾ ȸ���� �� �󸶸�ŭ�� �ð��� ������Ų �� ī�޶� ȸ����ų ���ΰ��� ��Ÿ����.
	float m_fTimeLag;

	//////////////////////////////////////////////
	//XMFLOAT3 m_xv3Pos;
	//XMFLOAT3 m_xv3At;
	//XMFLOAT3 m_xv3Up;


	//ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ��Ÿ���� ��� ������ �����Ѵ�.
	XMFLOAT4X4 m_xmf44View;
	XMFLOAT4X4 m_xmf44Projection;
	// ī�޶� ��ȯ�� ���� ��ȯ�� �̸� �����س��´�.
	XMFLOAT4X4 m_xmf44ViewProjection;

	//��-��Ʈ�� ��Ÿ���� ��� ������ �����Ѵ�.
	D3D11_VIEWPORT m_d3dViewport;

	//ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ���� ��� ���� �������̽� �����͸� �����Ѵ�.
	ID3D11Buffer *m_pd3dcbCamera;
	ID3D11Buffer *m_pd3dcbCameraPos;

	//ī�޶� ����� �÷��̾� ��ü�� ���� �����͸� �����Ѵ�.
	CPlayer *m_pPlayer;

public:
	void SetPlayer(CPlayer *pPlayer);
	CPlayer *GetPlayer() {return(m_pPlayer);}
	//��-��Ʈ�� �����ϴ� ��� �Լ��� �����Ѵ�.
	void SetViewport(ID3D11DeviceContext *pd3dDeviceContext, DWORD xStart, DWORD yStart, DWORD nWidth, DWORD nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);
	static void SetViewport(ID3D11DeviceContext *pd3dDeviceContext, DWORD nWidth, DWORD nHeight);
	D3D11_VIEWPORT GetViewport() const { return(m_d3dViewport); }

	XMFLOAT4X4 GetViewMatrix() const { return(m_xmf44View); }
	XMFLOAT4X4 GetProjectionMatrix() const { return(m_xmf44Projection); }
	XMFLOAT4X4 GetViewProjectionMatrix() const { return(m_xmf44ViewProjection); }
	ID3D11Buffer *GetCameraConstantBuffer() const { return(m_pd3dcbCamera); }
	ID3D11Buffer *GetCameraPosConstantBuffer() const { return(m_pd3dcbCameraPos); }
	//ī�޶� ��ȯ����� �����Ѵ�.
	void GenerateViewMatrix();
	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);
	void MakeViewProjectionMatrix();
	//��� ���۸� �����ϰ� ������ �����ϴ� ��� �Լ��� �����Ѵ�.
	void CreateShaderVariables(ID3D11Device *pd3dDevice);
	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 & xmtxViewProj, XMFLOAT3 & xmfCameraPos);
//	void UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext);
	void UpdateCameraPositionCBBuffer(ID3D11DeviceContext *pd3dDeviceContext);
	
	/*ī�޶� ������ ȸ���� �ϰ� �Ǹ� ������ �Ǽ������� ����Ȯ�� ������ ī�޶��� ���� x-��(Right), y-��(Up), z-��(LookAt)�� ���� �������� ���� �� �ִ�. ī�޶��� ���� x-��(Right), y-��(Up), z-��(LookAt)�� ���� �����ϵ��� ������ش�.*/
	void RegenerateViewMatrix();

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() const { return (m_nMode); }

	void SetPosition(XMFLOAT3 xv3Position) { m_xv3Position = xv3Position; }
	XMFLOAT3& GetPosition() { return(m_xv3Position); }

	void SetLookAtPosition(XMFLOAT3 xv3LookAtWorld) { m_xv3LookAtWorld = xv3LookAtWorld; }
	XMFLOAT3& GetLookAtPosition() { return(m_xv3LookAtWorld); }

	XMFLOAT3& GetRightVector() { return(m_xv3Right); }
	XMFLOAT3& GetUpVector() { return(m_xv3Up); }
	XMFLOAT3& GetLookVector() { return(m_xv3Look); }

	float& GetPitch() { return(m_fPitch); }
	float& GetRoll() { return(m_fRoll); }
	float& GetYaw() { return(m_fYaw); }


	void SetOffset(XMFLOAT3 xv3Offset);
	XMFLOAT3& GetOffset() { return(m_xv3Offset); }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() const { return(m_fTimeLag); }

	//ī�޶� xv3Shift ��ŭ �̵��ϴ� �����Լ��̴�.
	virtual void Move(XMFLOAT3& xv3Shift);
	//ī�޶� x-��, y-��, z-������ ȸ���ϴ� �����Լ��̴�.
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) { }
	//ī�޶��� �̵�, ȸ���� ���� ī�޶��� ������ �����ϴ� �����Լ��̴�.
	virtual void Update(XMFLOAT3& xv3LookAt, float fTimeElapsed);
	/*3��Ī ī�޶󿡼� ī�޶� �ٶ󺸴� ������ �����ϴ� �����Լ��̴�. �Ϲ������� �÷��̾ �ٶ󺸵��� �����Ѵ�.*/
	virtual void SetLookAt(XMFLOAT3& vLookAt) { }


public:
	//����ü�� 6�� ����� ����Ѵ�.
	void CalculateFrustumPlanes();
	//�ٿ�� �ڽ��� ����ü�� ������ ���Եǰų� �Ϻζ� ���ԵǴ� ���� �˻��Ѵ�.
	bool IsInFrustum(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum);
	bool IsInFrustum(AABB *pAABB);
};

class CSpaceShipCamera : public CCamera
{
public:
	CSpaceShipCamera(CCamera *pCamera);
	virtual ~CSpaceShipCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};
class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(CCamera *pCamera);
	virtual ~CFirstPersonCamera() { }

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};
class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(CCamera *pCamera);
	virtual ~CThirdPersonCamera() { }

	virtual void Update(XMFLOAT3& xv3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};