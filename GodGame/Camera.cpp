#include "stdafx.h"
#include "MyInline.h"
#include "Player.h"
#include "Camera.h"

CCamera::CCamera(CCamera *pCamera)
{
	//Chae::XMFloat4x4Identity(&m_xmf44ViewProjection);
	if (pCamera)
	{
		//ī�޶� �̹� ������ ���� ī�޶��� ������ ���ο� ī�޶� �����Ѵ�.
		m_xv3Position         = pCamera->GetPosition();
		m_xv3Right            = pCamera->GetRightVector();
		m_xv3Look             = pCamera->GetLookVector();
		m_xv3Up               = pCamera->GetUpVector();
		m_fPitch              = pCamera->GetPitch();
		m_fRoll               = pCamera->GetRoll();
		m_fYaw                = pCamera->GetYaw();

		m_xmf44View           = pCamera->GetViewMatrix();
		m_xmf44Projection     = pCamera->GetProjectionMatrix();
		m_xmf44ViewProjection = pCamera->GetViewProjectionMatrix();
		m_d3dViewport         = pCamera->GetViewport();
		m_xv3LookAtWorld      = pCamera->GetLookAtPosition();
		m_xv3Offset           = pCamera->GetOffset();
		m_fTimeLag            = pCamera->GetTimeLag();
		m_pPlayer             = pCamera->GetPlayer();
		m_pd3dcbCamera        = pCamera->GetCameraConstantBuffer();
		m_pd3dcbCameraPos     = pCamera->GetCameraConstantBuffer();
		if (m_pd3dcbCamera) m_pd3dcbCamera->AddRef();
	}
	else
	{
		//ī�޶� ������ �⺻ ������ �����Ѵ�.
		m_xv3Position         = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xv3Right            = XMFLOAT3(1.0f, 0.0f, 0.0f);
		m_xv3Up               = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xv3Look             = XMFLOAT3(0.0f, 0.0f, 1.0f);
		m_fPitch              = 0.0f;
		m_fRoll               = 0.0f;
		m_fYaw                = 0.0f;
		m_fTimeLag            = 0.0f;
		m_xv3LookAtWorld      = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xv3Offset           = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_nMode               = 0x00;
		m_pPlayer             = nullptr;
		Chae::XMFloat4x4Identity(&m_xmf44View);
		Chae::XMFloat4x4Identity(&m_xmf44Projection);
		m_pd3dcbCamera        = nullptr;
		m_pd3dcbCameraPos     = nullptr;
	}
}

CCamera::~CCamera()
{
	if (m_pd3dcbCamera) m_pd3dcbCamera->Release();
	if (m_pd3dcbCameraPos) m_pd3dcbCameraPos->Release();
}

void CCamera::SetViewport(ID3D11DeviceContext *pd3dDeviceContext, DWORD xTopLeft, DWORD yTopLeft, DWORD nWidth, DWORD nHeight, float fMinZ, float fMaxZ)
{
	m_d3dViewport.TopLeftX = float(xTopLeft);
	m_d3dViewport.TopLeftY = float(yTopLeft);
	m_d3dViewport.Width    = float(nWidth);
	m_d3dViewport.Height   = float(nHeight);
	m_d3dViewport.MinDepth = fMinZ;
	m_d3dViewport.MaxDepth = fMaxZ;
	pd3dDeviceContext->RSSetViewports(1, &m_d3dViewport);
}

void CCamera::SetViewport(ID3D11DeviceContext * pd3dDeviceContext, DWORD nWidth, DWORD nHeight)
{
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX      = 0.0f;//-nWidth;
	viewport.TopLeftY	   = 0.0f;//-nHeight;
	viewport.Width         = float(nWidth);
	viewport.Height        = float(nHeight);
	viewport.MinDepth      = 0.0f;
	viewport.MaxDepth      = 1.0f;
	pd3dDeviceContext->RSSetViewports(1, &viewport);
}

void CCamera::Update(XMFLOAT3& xv3LookAt, float fTimeElapsed)
{
}

void CCamera::GenerateViewMatrix()
{
	XMMATRIX xmtxView = XMMatrixLookAtLH(XMLoadFloat3(&m_xv3Position), XMLoadFloat3(&m_pPlayer->GetPosition()), XMLoadFloat3(&m_xv3Up));
	XMStoreFloat4x4(&m_xmf44View, xmtxView);
	MakeViewProjectionMatrix();
}

void CCamera::GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle)
{
	XMMATRIX xmtxFov = XMMatrixPerspectiveFovLH(XMConvertToRadians(fFOVAngle), fAspectRatio, fNearPlaneDistance, fFarPlaneDistance);
	XMStoreFloat4x4(&m_xmf44Projection, xmtxFov);
	MakeViewProjectionMatrix();
}

void CCamera::MakeViewProjectionMatrix()
{
	Chae::XMFloat4x4Mul(&m_xmf44ViewProjection, &m_xmf44View, &m_xmf44Projection);
}

void CCamera::SetPlayer(CPlayer *pPlayer)
{
	m_pPlayer = pPlayer;

	XMFLOAT3 xmfPlayerPos = pPlayer->GetPosition();

	xmfPlayerPos.x += m_xv3Offset.x;
	xmfPlayerPos.y += m_xv3Offset.y;
	xmfPlayerPos.z += m_xv3Offset.z;

	m_xv3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xv3Right    = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xv3Up       = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xv3Look     = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_fPitch      = 0.0f;
	m_fRoll       = 0.0f;
	m_fYaw        = 0.0f;

	SetPosition(xmfPlayerPos);
	SetLookAt(m_pPlayer->GetPosition());
	RegenerateViewMatrix();
}

void CCamera::SetOffset(XMFLOAT3 xv3Offset) {
	m_xv3Offset = xv3Offset;
	Chae::XMFloat3Add(&m_xv3Position, &m_xv3Position, &xv3Offset);
}

void CCamera::Move(XMFLOAT3& xv3Shift)
{
	Chae::XMFloat3Add(&m_xv3Position, &m_xv3Position, &xv3Shift);
	//m_xv3Position = m_xv3Position, xv3Shift;
}

void CCamera::RegenerateViewMatrix()
{
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvRight;
	XMVECTOR xmvPostion = XMLoadFloat3(&m_xv3Position);

	//ī�޶��� z-�� ���͸� ����ȭ�Ѵ�.
	xmvLook = XMVector3Normalize(xmvLook); //Chae::XMFloat3Normalize(&m_xv3Look);
	//ī�޶��� z-��� y-�࿡ ������ ���͸� x-������ �����Ѵ�.
	xmvRight = XMVector3Cross(xmvUp, xmvLook); //xv3ec3Cross(&m_xv3Right, &m_xv3Up, &m_xv3Look);
	//ī�޶��� x-�� ���͸� ����ȭ�Ѵ�.
	XMVector3Normalize(xmvRight);
	//ī�޶��� z-��� x-�࿡ ������ ���͸� y-������ �����Ѵ�.
	xmvUp = XMVector3Cross(xmvLook, xmvRight);	//xv3ec3Cross(&m_xv3Up, &m_xv3Look, &m_xv3Right);
	//ī�޶��� y-�� ���͸� ����ȭ�Ѵ�.
	XMVector3Normalize(xmvUp);	//xv3ec3Normalize(&m_xv3Up, &m_xv3Up);

	m_xmf44View._11 = m_xv3Right.x;
	m_xmf44View._12 = m_xv3Up.x;
	m_xmf44View._13 = m_xv3Look.x;
	m_xmf44View._21 = m_xv3Right.y;
	m_xmf44View._22 = m_xv3Up.y;
	m_xmf44View._23 = m_xv3Look.y;
	m_xmf44View._31 = m_xv3Right.z;
	m_xmf44View._32 = m_xv3Up.z;
	m_xmf44View._33 = m_xv3Look.z;

	XMStoreFloat(&m_xmf44View._41, -XMVector3Dot(xmvPostion, xmvRight));
	XMStoreFloat(&m_xmf44View._42, -XMVector3Dot(xmvPostion, xmvUp)); 
	XMStoreFloat(&m_xmf44View._43, -XMVector3Dot(xmvPostion, xmvLook)); 
	//ī�޶��� ��ġ�� ������ �ٲ��(ī�޶� ��ȯ ����� �ٲ��) ����ü ����� �ٽ� ����Ѵ�.
	CalculateFrustumPlanes();

	XMStoreFloat3(&m_xv3Look, xmvLook);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Right, xmvRight);
}

void CCamera::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage          = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth      = sizeof(VS_CB_VIEWPROJECTION);
	bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbCamera);

	//bd.ByteWidth = sizeof(VS_CB_CAMERAPOS);
	//pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbCameraPos);
}

void CCamera::UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 & xmf44ViewProj, XMFLOAT3 & xmfCameraPos)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	/*��� ������ �޸� �ּҸ� �����ͼ� ī�޶� ��ȯ ��İ� ���� ��ȯ ����� �����Ѵ�. ���̴����� ����� ��� ���� �ٲ�� �Ϳ� �����϶�.*/
	pd3dDeviceContext->Map(m_pd3dcbCamera, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	VS_CB_VIEWPROJECTION *pcbViewProjection = (VS_CB_VIEWPROJECTION *)d3dMappedResource.pData;
	pcbViewProjection->m_xf3CameraPos = XMFLOAT4(xmfCameraPos.x, xmfCameraPos.y, xmfCameraPos.z, 1);//XMFLOAT4(xmPos.x, xmPos.y, xmPos.z, 1.0f);
	Chae::XMFloat4x4Transpose(&pcbViewProjection->m_xmf44View, &xmf44ViewProj);	//XMFLOAT4X4Transpose(&pcbViewProjection->m_xmf44View, &m_xmf44View);
	pd3dDeviceContext->Unmap(m_pd3dcbCamera, 0);
	
	//��� ���۸� ����(VS_SLOT_CAMERA)�� �����Ѵ�.
	pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
	pd3dDeviceContext->GSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
	//pd3dDeviceContext->HSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
	//pd3dDeviceContext->DSSetConstantBuffers(CB_SLOT_VIEWPROJECTION, 1, &m_pd3dcbCamera);
}

void CCamera::UpdateCameraPositionCBBuffer(ID3D11DeviceContext *pd3dDeviceContext)
{
	//D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
}
CSpaceShipCamera::CSpaceShipCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = SPACESHIP_CAMERA;
}

void CSpaceShipCamera::Rotate(float x, float y, float z)
{
	if (x == 0.0f && y == 0.0f && z == 0.0f) return;

	XMMATRIX mtxRotate;
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMVECTOR xmvAxisVector;

	XMVECTOR xmvCameraPos = XMLoadFloat3(&m_xv3Position);
	XMVECTOR xmvPlayerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	if (m_pPlayer && (x != 0.0f))
	{
		xmvAxisVector = XMLoadFloat3(&m_pPlayer->GetRightVector());

		//�÷��̾��� ���� x-�࿡ ���� x ������ ȸ�� ����� ����Ѵ�.
		mtxRotate = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(x));	//XMFLOAT4X4RotationAxis(&mtxRotate, &m_pPlayer->GetRightVector(), (float)D3DXToRadian(x));
		//ī�޶��� ���� x-��, y-��, z-���� ȸ���Ѵ�.
		xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
		/*ī�޶��� ��ġ ���Ϳ��� �÷��̾��� ��ġ ���͸� ����. ����� �÷��̾� ��ġ�� �������� �� ī�޶��� ��ġ �����̴�.*/
		xmvCameraPos -= xmvPlayerPos; //m_xv3Position -= m_pPlayer->GetPosition();
		//�÷��̾��� ��ġ�� �߽����� ī�޶��� ��ġ ����(�÷��̾ �������� ��)�� ȸ���Ѵ�.
		xmvCameraPos = XMVector3TransformCoord(xmvCameraPos, mtxRotate);	//xv3ec3TransformCoord(&m_xv3Position, &m_xv3Position, &mtxRotate);
		//ȸ����Ų ī�޶��� ��ġ ���Ϳ� �÷��̾��� ��ġ�� ���Ѵ�.
		xmvCameraPos += xmvPlayerPos;//m_xv3Position += m_pPlayer->GetPosition();
	}
	if (m_pPlayer && (y != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetUpVector());
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(y));

		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
		xmvCameraPos    -= xmvPlayerPos;
		xmvCameraPos     = XMVector3TransformCoord(xmvCameraPos, mtxRotate);
		xmvCameraPos    += xmvPlayerPos;
	}
	if (m_pPlayer && (z != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetLookVector());
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(z));

		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
		xmvCameraPos    -= xmvPlayerPos;
		xmvCameraPos     = XMVector3TransformCoord(xmvCameraPos, mtxRotate);
		xmvCameraPos    += xmvPlayerPos;
	}
	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);

	XMStoreFloat3(&m_xv3Position, xmvCameraPos);
}

CFirstPersonCamera::CFirstPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = FIRST_PERSON_CAMERA;
	if (pCamera)
	{
		/*1��Ī ī�޶�� �����ϱ� ������ ī�޶� �����̽�-�� ī�޶��̸� ī�޶��� Up ���͸� ������ǥ�� y-���� �ǵ��� �Ѵ�. �̰��� �����̽�-�� ī�޶��� ���� y-�� ���Ͱ� � �����̵��� 1��Ī ī�޶�(��κ� ����� ���)�� ���� y-�� ���Ͱ� ������ǥ�� y-���� �ǵ��� ��, �ȹٷ� ���ִ� ���·� �����Ѵٴ� �ǹ��̴�. �׸��� ���� x-�� ���Ϳ� ���� z-�� ������ y-��ǥ�� 0.0f�� �ǵ��� �Ѵ�. �̰��� <�׸� 8>�� ���� ���� x-�� ���Ϳ� ���� z-�� ���͸� xz-���(����)���� �����ϴ� ���� �ǹ��Ѵ�. ��, 1��Ī ī�޶��� ���� x-�� ���Ϳ� ���� z-�� ���ʹ� xz-��鿡 �����ϴ�.*/
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xv3Up           = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xv3Right.y      = 0.0f;
			m_xv3Look.y       = 0.0f;

			XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
			XMVECTOR xmvLook  = XMLoadFloat3(&m_xv3Look);

			xmvRight          = XMVector3Normalize(xmvRight);
			xmvLook           = XMVector3Normalize(xmvLook);

			XMStoreFloat3(&m_xv3Right, xmvRight);
			XMStoreFloat3(&m_xv3Look, xmvLook);
		}
	}
}

void CFirstPersonCamera::Rotate(float x, float y, float z)
{
	if (x == 0.0f && y == 0.0f && z == 0.0f) return;

	XMMATRIX mtxRotate;
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook = XMLoadFloat3(&m_xv3Look);
	XMVECTOR xmvAxisVector;

	XMVECTOR xmvCameraPos = XMLoadFloat3(&m_xv3Position);
	XMVECTOR xmvPlayerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	if (x != 0.0f)
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetRightVector());
		//ī�޶��� ���� x-���� �������� ȸ���ϴ� ����� �����Ѵ�. ���� �����̴� �����̴�.
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(x));
		//ī�޶��� ���� x-��, y-��, z-���� ȸ���Ѵ�.
		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
	}
	if (m_pPlayer && (y != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetUpVector());
		//�÷��̾��� ���� y-���� �������� ȸ���ϴ� ����� �����Ѵ�.
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(y));
		//ī�޶��� ���� x-��, y-��, z-���� ȸ���Ѵ�.
		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
	}
	if (m_pPlayer && (z != 0.0f))
	{
		xmvAxisVector    = XMLoadFloat3(&m_pPlayer->GetLookVector());
		//�÷��̾��� ���� z-���� �������� ȸ���ϴ� ����� �����Ѵ�.
		mtxRotate        = XMMatrixRotationAxis(xmvAxisVector, XMConvertToRadians(z));
		//ī�޶��� ��ġ ���͸� �÷��̾� ��ǥ��� ǥ���Ѵ�(������ ����).
		xmvCameraPos    -= xmvPlayerPos;
		//������ ���� ���͸� ȸ���Ѵ�.
		xmvCameraPos     = XMVector3TransformCoord(xmvCameraPos, mtxRotate);
		//ȸ���� ī�޶��� ��ġ�� ���� ��ǥ��� ǥ���Ѵ�.
		xmvCameraPos    += xmvPlayerPos;
		//ī�޶��� ���� x-��, y-��, z-���� ȸ���Ѵ�.
		xmvRight         = XMVector3TransformNormal(xmvRight, mtxRotate);
		xmvUp            = XMVector3TransformNormal(xmvUp, mtxRotate);
		xmvLook          = XMVector3TransformNormal(xmvLook, mtxRotate);
	}
	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);

	XMStoreFloat3(&m_xv3Position, xmvCameraPos);
}

void CCamera::CalculateFrustumPlanes()
{
	/*ī�޶� ��ȯ ��İ� ���� ���� ��ȯ ����� ���� ����� ����Ͽ� ����ü ������ ���Ѵ�. �� ���� ��ǥ�迡�� ����ü �ø��� �Ѵ�.*/
	XMFLOAT4X4 mtxViewProject;
	Chae::XMFloat4x4Mul(&mtxViewProject, &m_xmf44View, &m_xmf44Projection);

	//����ü�� ���� ���
	m_xmpFrustumPlanes[0].x = -(mtxViewProject._14 + mtxViewProject._11);
	m_xmpFrustumPlanes[0].y = -(mtxViewProject._24 + mtxViewProject._21);
	m_xmpFrustumPlanes[0].z = -(mtxViewProject._34 + mtxViewProject._31);
	m_xmpFrustumPlanes[0].w = -(mtxViewProject._44 + mtxViewProject._41);

	//����ü�� ������ ���
	m_xmpFrustumPlanes[1].x = -(mtxViewProject._14 - mtxViewProject._11);
	m_xmpFrustumPlanes[1].y = -(mtxViewProject._24 - mtxViewProject._21);
	m_xmpFrustumPlanes[1].z = -(mtxViewProject._34 - mtxViewProject._31);
	m_xmpFrustumPlanes[1].w = -(mtxViewProject._44 - mtxViewProject._41);

	//����ü�� ���� ���
	m_xmpFrustumPlanes[2].x = -(mtxViewProject._14 - mtxViewProject._12);
	m_xmpFrustumPlanes[2].y = -(mtxViewProject._24 - mtxViewProject._22);
	m_xmpFrustumPlanes[2].z = -(mtxViewProject._34 - mtxViewProject._32);
	m_xmpFrustumPlanes[2].w = -(mtxViewProject._44 - mtxViewProject._42);

	//����ü�� �Ʒ��� ���
	m_xmpFrustumPlanes[3].x = -(mtxViewProject._14 + mtxViewProject._12);
	m_xmpFrustumPlanes[3].y = -(mtxViewProject._24 + mtxViewProject._22);
	m_xmpFrustumPlanes[3].z = -(mtxViewProject._34 + mtxViewProject._32);
	m_xmpFrustumPlanes[3].w = -(mtxViewProject._44 + mtxViewProject._42);

	//����ü�� �����
	m_xmpFrustumPlanes[4].x = -(mtxViewProject._13);
	m_xmpFrustumPlanes[4].y = -(mtxViewProject._23);
	m_xmpFrustumPlanes[4].z = -(mtxViewProject._33);
	m_xmpFrustumPlanes[4].w = -(mtxViewProject._43);

	//����ü�� �����
	m_xmpFrustumPlanes[5].x = -(mtxViewProject._14 - mtxViewProject._13);
	m_xmpFrustumPlanes[5].y = -(mtxViewProject._24 - mtxViewProject._23);
	m_xmpFrustumPlanes[5].z = -(mtxViewProject._34 - mtxViewProject._33);
	m_xmpFrustumPlanes[5].w = -(mtxViewProject._44 - mtxViewProject._43);

	/*����ü�� �� ����� ���� ���� (a, b. c)�� ũ��� a, b, c, d�� ������. ��, ���� ���͸� ����ȭ�ϰ� �������� �������� �Ÿ��� ����Ѵ�.*/
	XMVECTOR xmPlane;
	for (int i = 0; i < 6; i++)
	{
		xmPlane = XMPlaneNormalize(XMLoadFloat4(&m_xmpFrustumPlanes[i]));
		XMStoreFloat4(&m_xmpFrustumPlanes[i], xmPlane);
		//D3DXPlaneNormalize(&m_xmpFrustumPlanes[i], &m_xmpFrustumPlanes[i]);
	}
}

bool CCamera::IsInFrustum(XMFLOAT3& xv3Minimum, XMFLOAT3& xv3Maximum)
{
	XMFLOAT3 xv3NearPoint, xv3FarPoint, xv3Normal;
	for (int i = 0; i < 6; i++)
	{
		/*����ü�� �� ��鿡 ���Ͽ� �ٿ�� �ڽ��� �������� ����Ѵ�. �������� x, y, z ��ǥ�� ���� ������ �� ��Ұ� �����̸� �ٿ�� �ڽ��� �ִ����� ��ǥ�� �ǰ� �׷��� ������ �ٿ�� �ڽ��� �ּ����� ��ǥ�� �ȴ�.*/
		xv3Normal = XMFLOAT3(m_xmpFrustumPlanes[i].x, m_xmpFrustumPlanes[i].y, m_xmpFrustumPlanes[i].z);
		if (xv3Normal.x >= 0.0f) {
			if (xv3Normal.y >= 0.0f) {
				if (xv3Normal.z >= 0.0f) {
					//���� ������ x, y, z ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� �ٿ�� �ڽ��� �ּ����̴�.
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					/*���� ������ x, y ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� x, y ��ǥ�� �ٿ�� �ڽ��� �ּ����� x, y ��ǥ�̰� ���� ������ z ��ǥ�� ����̹Ƿ� �������� z ��ǥ�� �ٿ�� �ڽ��� �ִ����� z ��ǥ�̴�.*/
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
			else {
				if (xv3Normal.z >= 0.0f) {
					/*���� ������ x, z ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� x, z ��ǥ�� �ٿ�� �ڽ��� �ּ����� x, z ��ǥ�̰� ���� ������ y ��ǥ�� ����̹Ƿ� �������� y ��ǥ�� �ٿ�� �ڽ��� �ִ����� y ��ǥ�̴�.*/
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					/*���� ������ y, z ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� y, z ��ǥ�� �ٿ�� �ڽ��� �ִ����� y, z ��ǥ�̰� ���� ������ x ��ǥ�� ����̹Ƿ� �������� x ��ǥ�� �ٿ�� �ڽ��� �ּ����� x ��ǥ�̴�.*/
					xv3NearPoint.x = xv3Minimum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
		}
		else {
			if (xv3Normal.y >= 0.0f) {
				if (xv3Normal.z >= 0.0f) {
					/*���� ������ y, z ��ǥ�� ��ȣ�� ��� ����̹Ƿ� �������� y, z ��ǥ�� �ٿ�� �ڽ��� �ּ����� y, z ��ǥ�̰� ���� ������ x ��ǥ�� �����̹Ƿ� �������� x ��ǥ�� �ٿ�� �ڽ��� �ִ����� x ��ǥ�̴�.*/
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					/*���� ������ x, z ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� x, z ��ǥ�� �ٿ�� �ڽ��� �ִ����� x, z ��ǥ�̰� ���� ������ y ��ǥ�� ����̹Ƿ� �������� y ��ǥ�� �ٿ�� �ڽ��� �ּ����� y ��ǥ�̴�.*/
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Minimum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
			else {
				if (xv3Normal.z >= 0.0f) {
					/*���� ������ x, y ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� x, y ��ǥ�� �ٿ�� �ڽ��� �ִ����� x, y ��ǥ�̰� ���� ������ z ��ǥ�� ����̹Ƿ� �������� z ��ǥ�� �ٿ�� �ڽ��� �ּ����� z ��ǥ�̴�.*/
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Minimum.z;
				}
				else {
					//���� ������ x, y, z ��ǥ�� ��ȣ�� ��� �����̹Ƿ� �������� �ٿ�� �ڽ��� �ִ����̴�.
					xv3NearPoint.x = xv3Maximum.x; xv3NearPoint.y = xv3Maximum.y; xv3NearPoint.z = xv3Maximum.z;
				}
			}
		}
		if ((Chae::XMFloat3Dot(&xv3Normal, &xv3NearPoint) + m_xmpFrustumPlanes[i].w) > 0.0f) return(false);
	}
	return(true);
}

bool CCamera::IsInFrustum(AABB *pAABB)
{
	return(IsInFrustum(pAABB->m_xv3Minimum, pAABB->m_xv3Maximum));
}

CThirdPersonCamera::CThirdPersonCamera(CCamera *pCamera) : CCamera(pCamera)
{
	m_nMode = THIRD_PERSON_CAMERA;
	if (pCamera)
	{
		/*3��Ī ī�޶�� �����ϱ� ������ ī�޶� �����̽�-�� ī�޶��̸� ī�޶��� Up ���͸� ������ǥ�� y-���� �ǵ��� �Ѵ�. �̰��� �����̽�-�� ī�޶��� ���� y-�� ���Ͱ� � �����̵��� 3��Ī ī�޶�(��κ� ����� ���)�� ���� y-�� ���Ͱ� ������ǥ�� y-���� �ǵ��� ��, �ȹٷ� ���ִ� ���·� �����Ѵٴ� �ǹ��̴�. �׸��� ���� x-�� ���Ϳ� ���� z-�� ������ y-��ǥ�� 0.0f�� �ǵ��� �Ѵ�. �̰��� ���� x-�� ���Ϳ� ���� z-�� ���͸� xz-���(����)���� �����ϴ� ���� �ǹ��Ѵ�. ��, 3��Ī ī�޶��� ���� x-�� ���Ϳ� ���� z-�� ���ʹ� xz-��鿡 �����ϴ�.*/
		if (pCamera->GetMode() == SPACESHIP_CAMERA)
		{
			m_xv3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
			m_xv3Right.y = 0.0f;
			m_xv3Look.y = 0.0f;
			Chae::XMFloat3Normalize(&m_xv3Right);
			Chae::XMFloat3Normalize(&m_xv3Look);
		}
	}
}

void CThirdPersonCamera::Update(XMFLOAT3& xv3LookAt, float fTimeElapsed)
{
	/*�÷��̾��� ȸ���� ���� 3��Ī ī�޶� ȸ���ؾ� �Ѵ�.*/
	XMFLOAT4X4 mtxRotate;
	Chae::XMFloat4x4Identity(&mtxRotate);
	XMFLOAT3 xv3Right  = m_pPlayer->GetRightVector();
	XMFLOAT3 xv3Up     = m_pPlayer->GetUpVector();
	XMFLOAT3 xv3Look   = m_pPlayer->GetLookVector();
	XMFLOAT3 xv3Player = m_pPlayer->GetPosition();
	//m_xv3Position.y = max(m_xv3Position.y, xv3Player.y + m_xv3Offset.y);

	//�÷��̾��� ���� x-��, y-��, z-�� ���ͷκ��� ȸ�� ����� �����Ѵ�.
	mtxRotate._11 = xv3Right.x; mtxRotate._21 = xv3Up.x; mtxRotate._31 = xv3Look.x;
	mtxRotate._12 = xv3Right.y; mtxRotate._22 = xv3Up.y; mtxRotate._32 = xv3Look.y;
	mtxRotate._13 = xv3Right.z; mtxRotate._23 = xv3Up.z; mtxRotate._33 = xv3Look.z;

	XMMATRIX xmtxRotate = XMLoadFloat4x4(&mtxRotate);
	XMVECTOR xmv3Offset = XMLoadFloat3(&m_xv3Offset);
	XMVECTOR xmvCameraPos = XMLoadFloat3(&m_xv3Position);
	XMVECTOR xmvPlayerPos = XMLoadFloat3(&m_pPlayer->GetPosition());

	xmv3Offset = XMVector3TransformCoord(xmv3Offset, xmtxRotate); // xv3ec3TransformCoord(&xv3Offset, &m_xv3Offset, &mtxRotate);
	//ȸ���� ī�޶��� ��ġ�� �÷��̾��� ��ġ�� ȸ���� ī�޶� ������ ���͸� ���� ���̴�.
	XMVECTOR xv3Position = (xmvPlayerPos + xmv3Offset);
	//������ ī�޶��� ��ġ���� ȸ���� ī�޶��� ��ġ������ �����̴�.
	XMVECTOR xv3Direction = xv3Position - xmvCameraPos;

	float fLength;
	XMStoreFloat(&fLength, XMVector3Length(xv3Direction));
	xv3Direction = XMVector3Normalize(xv3Direction);
	/*3��Ī ī�޶��� ����(Lag)�� �÷��̾ ȸ���ϴ��� ī�޶� ���ÿ� ���� ȸ������ �ʰ� �ణ�� ������ �ΰ� ȸ���ϴ� ȿ���� �����ϱ� ���� ���̴�. m_fTimeLag�� 1���� ũ�� fTimeLagScale�� �۾����� ���� ȸ���� ���� �Ͼ ���̴�.*/

	float fTimeLagScale = (m_fTimeLag) ? fTimeElapsed * (1.0f / m_fTimeLag) : 1.0f;
	float fDistance = fLength * fTimeLagScale;
	if (fDistance > fLength) fDistance = fLength;
	if (fLength < 0.01f) fDistance = fLength;
	if (fDistance > 0)
	{
		xmvCameraPos += xv3Direction * fDistance;
		SetLookAt(xv3LookAt);
	}
	XMStoreFloat3(&m_xv3Position, xmvCameraPos);
}

void CThirdPersonCamera::SetLookAt(XMFLOAT3& xv3LookAt)
{
	XMMATRIX mtxLookAt = XMMatrixLookAtLH(XMLoadFloat3(&m_xv3Position), XMLoadFloat3(&xv3LookAt), XMLoadFloat3(&m_pPlayer->GetUpVector()));
	XMStoreFloat4x4(&m_xmf44View, mtxLookAt);
	//XMFLOAT4X4 mtxLookAt;
	//XMFLOAT4X4LookAtLH(&mtxLookAt, &m_xv3Position, &xv3LookAt, &m_pPlayer->GetUpVector());

	m_xv3Right = XMFLOAT3(m_xmf44View._11, m_xmf44View._21, m_xmf44View._31);
	m_xv3Up    = XMFLOAT3(m_xmf44View._12, m_xmf44View._22, m_xmf44View._32);
	m_xv3Look  = XMFLOAT3(m_xmf44View._13, m_xmf44View._23, m_xmf44View._33);

	MakeViewProjectionMatrix();
}