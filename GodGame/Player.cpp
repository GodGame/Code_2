#include "stdafx.h"
#include "MyInline.h"
#include "Weapon.h"
#include "Player.h"
#include "StatePlayer.h"
#include "Protocol.h"
CPlayer::CPlayer(int nMeshes) : CCharacter(nMeshes)
{
	mDirection = 0x0;
	m_iPlayerNum			= -1;
	m_pCamera               = nullptr;

	m_fPitch                = 0.0f;
	m_fRoll                 = 0.0f;
	m_fYaw                  = 0.0f;

	m_pCameraUpdatedContext = nullptr;

	m_pScene = nullptr;
}

CPlayer::~CPlayer()
{
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
}

void CPlayer::UpdateShaderVariables(ID3D11DeviceContext *pd3dDeviceContext)
{
	//�÷��̾��� ���� ī�޶��� UpdateShaderVariables() ��� �Լ��� ȣ���Ѵ�.
	if (m_pCamera) m_pCamera->UpdateShaderVariables(pd3dDeviceContext, m_pCamera->GetViewProjectionMatrix(), m_pCamera->GetPosition());
	//printf("Player : %0.2f %0.2f %0.2f \n", m_xmf44World._41, m_xmf44World._42, m_xmf44World._43);
	//cout << "player" << endl;
	//cout << "bb max : " << m_bcMeshBoundingCube.m_xv3Maximum.x << ", " << m_bcMeshBoundingCube.m_xv3Maximum.y << ", " << m_bcMeshBoundingCube.m_xv3Maximum.z << endl;
	//cout << "bb min : " << m_bcMeshBoundingCube.m_xv3Minimum.x << ", " << m_bcMeshBoundingCube.m_xv3Minimum.y << ", " << m_bcMeshBoundingCube.m_xv3Minimum.z << endl;
}

void CPlayer::InitPosition(XMFLOAT3 xv3Position)
{
	m_xmf3InitPos = xv3Position;
	m_xv3Position = m_xmf3InitPos;
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (false == m_Status.IsCanMove()) return;

	if (dwDirection)
	{
		WORD wdNextState = 0;
		XMVECTOR xv3Shift = XMVectorReplicate(0);
		//ȭ��ǥ Ű ���桯�� ������ ���� x-�� �������� �̵��Ѵ�. ���硯�� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_LEFT)
		{
			xv3Shift -= XMLoadFloat3(&m_xv3Right) * fDistance;// *0.8f;
			wdNextState = eANI_WALK_LEFT;
		}
		if (dwDirection & DIR_RIGHT)
		{
			xv3Shift += XMLoadFloat3(&m_xv3Right) * fDistance;// *0.8f;
			wdNextState = eANI_WALK_RIGHT;
		}
		//ȭ��ǥ Ű ���衯�� ������ ���� z-�� �������� �̵�(����)�Ѵ�. ���顯�� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_BACKWARD)
		{
			xv3Shift -= XMLoadFloat3(&m_xv3Look) * fDistance;// *0.8f;
			wdNextState = eANI_WALK_BACK;
		}
		if (dwDirection & DIR_FORWARD)
		{
			xv3Shift += XMLoadFloat3(&m_xv3Look) * fDistance;// *1.2f;
			wdNextState = eANI_RUN_FORWARD;
		}
		static XMFLOAT3 xmf3Shift;
		XMStoreFloat3(&xmf3Shift, xv3Shift);

		if (wdNextState != m_wdAnimateState)
		{
			m_wdAnimateState = wdNextState;
			CAnimatedMesh* pAnimatedMesh = static_cast<CAnimatedMesh*>(m_ppMeshes[m_wdAnimateState]);

			pAnimatedMesh->ResetIndex();
		}
		//�÷��̾ ���� ��ġ ���Ϳ��� xv3Shift ���� ��ŭ �̵��Ѵ�.
		Move(xmf3Shift, bUpdateVelocity);
	}
	else
	{
		if (m_wdAnimateState != eANI_IDLE)
		{
			m_wdAnimateState = eANI_IDLE;
			CAnimatedMesh* pAnimatedMesh = static_cast<CAnimatedMesh*>(m_ppMeshes[m_wdAnimateState]);
			pAnimatedMesh->ResetIndex();
		}
	}
}
void CPlayer::Move(XMFLOAT3& xv3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity�� ���̸� �÷��̾ �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�.
	if (bUpdateVelocity)
	{
		m_xv3Velocity.x += xv3Shift.x;
		m_xv3Velocity.y += xv3Shift.y;
		m_xv3Velocity.z += xv3Shift.z;
	}
	else
	{
		//�÷��̾ ���� ��ġ ���Ϳ��� xv3Shift ���� ��ŭ �̵��Ѵ�.
		XMFLOAT3 xv3Position;
		xv3Position.x = m_xv3Position.x + xv3Shift.x;
		xv3Position.y = m_xv3Position.y + xv3Shift.y;
		xv3Position.z = m_xv3Position.z + xv3Shift.z;

		m_xv3Position = xv3Position;
		//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� xv3Shift ���� ��ŭ �̵��Ѵ�.
		if (m_pCamera) m_pCamera->Move(xv3Shift);
	}
}

//�÷��̾ ���� x-��, y-��, z-���� �߽����� ȸ���Ѵ�.
void CPlayer::Rotate(float x, float y, float z)
{
	XMVECTOR xmvRight = XMLoadFloat3(&m_xv3Right);
	XMVECTOR xmvUp    = XMLoadFloat3(&m_xv3Up);
	XMVECTOR xmvLook  = XMLoadFloat3(&m_xv3Look);

	XMMATRIX mtxRotate;
	DWORD nCurrentCameraMode = m_pCamera ? m_pCamera->GetMode() : THIRD_PERSON_CAMERA;

	//1��Ī ī�޶� �Ǵ� 3��Ī ī�޶��� ��� �÷��̾��� ȸ���� �ణ�� ������ ������.
	if ((nCurrentCameraMode == FIRST_PERSON_CAMERA) || (nCurrentCameraMode == THIRD_PERSON_CAMERA))
	{
		/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x-���� �߽����� ȸ���ϴ� ������ -89.0~+89.0�� ���̷� �����Ѵ�. x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ���� Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		//���� y-���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����.
		if (y != 0.0f)
		{
			m_fYaw = m_fYaw + y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵȴ�. z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������ ������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
		if (z != 0.0f)
		{
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		//ī�޶� x, y, z ��ŭ ȸ���Ѵ�. �÷��̾ ȸ���ϸ� ī�޶� ȸ���ϰ� �ȴ�.
		if (m_pCamera) m_pCamera->Rotate(x, y, z);

		/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��. �÷��̾��� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��. �⺻������ Up ���͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
		if (y != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvUp, (float)XMConvertToRadians(y));
			xmvLook = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvRight = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
	}
	else if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		/*�����̽�-�� ī�޶󿡼� �÷��̾��� ȸ���� ȸ�� ������ ������ ����. �׸��� ��� ���� �߽����� ȸ���� �� �� �ִ�.*/
		if (m_pCamera) m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvRight, (float)XMConvertToRadians(x));
			xmvLook   = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvUp     = XMVector3TransformNormal(xmvUp, mtxRotate);
		}
		if (y != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvUp, (float)XMConvertToRadians(y));
			xmvLook   = XMVector3TransformNormal(xmvLook, mtxRotate);
			xmvRight  = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
		if (z != 0.0f)
		{
			mtxRotate = XMMatrixRotationAxis(xmvLook, (float)XMConvertToRadians(z));
			xmvUp     = XMVector3TransformNormal(xmvUp, mtxRotate);
			xmvRight  = XMVector3TransformNormal(xmvRight, mtxRotate);
		}
	}

	/*ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ� z-��(LookAt ����)�� �������� �Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.*/
	xmvLook  = XMVector3Normalize(xmvLook);
	xmvRight = XMVector3Cross(xmvUp, xmvLook);
	xmvRight = XMVector3Normalize(xmvRight);
	xmvUp    = XMVector3Cross(xmvLook, xmvRight);
	xmvUp    = XMVector3Normalize(xmvUp);

	XMStoreFloat3(&m_xv3Right, xmvRight);
	XMStoreFloat3(&m_xv3Up, xmvUp);
	XMStoreFloat3(&m_xv3Look, xmvLook);
}

void CPlayer::Rotate(XMFLOAT3 & xmf3RotAxis, float fAngle)
{
	CCharacter::Rotate(xmf3RotAxis, fAngle);
}

void CPlayer::Update(float fTimeElapsed)
{
	if (mDirection) Move(mDirection, 50.f * fTimeElapsed, false);
//	else
//	{
//		if(GetAnimationState() != eANI_IDLE)
//			ChangeAnimationState(eANI_IDLE, true, nullptr, 0);
//	}
	CCharacter::Update(fTimeElapsed);
}

CCamera *CPlayer::OnChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	CCamera *pNewCamera = nullptr;
	//���ο� ī�޶��� ��忡 ���� ī�޶� ���� �����Ѵ�.
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		pNewCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		pNewCamera->SetTimeLag(0.25f);
		pNewCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		pNewCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 1.0f));
		break;
	}
	/*���� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰� ���ο� ī�޶� 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� Up ���͸� ������ǥ���� y-�� ���� ����(0, 1, 0)�� �ǵ��� �Ѵ�. ��, �ȹٷ� ������ �Ѵ�. �׸��� �����̽�-�� ī�޶��� ��� �÷��̾��� �̵����� ������ ����. Ư��, y-�� ������ �������� �����Ӵ�. �׷��Ƿ� �÷��̾��� ��ġ�� ����(��ġ ������ y-��ǥ�� 0���� ũ��)�� �� �� �ִ�. �̶� ���ο� ī�޶� 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� ��ġ�� ������ �Ǿ�� �Ѵ�. �׷��Ƿ� �÷��̾��� Right ���Ϳ� Look ������ y ���� 0���� �����. ���� �÷��̾��� Right ���Ϳ� Look ���ʹ� �������Ͱ� �ƴϹǷ� ����ȭ�Ѵ�.*/
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xv3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
		m_xv3Right.y = 0.0f;
		m_xv3Look.y = 0.0f;
		Chae::XMFloat3Normalize(&m_xv3Right, &m_xv3Right);
		Chae::XMFloat3Normalize(&m_xv3Look, &m_xv3Look);
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look ���Ϳ� ������ǥ���� z-��(0, 0, 1)�� �̷�� ����(����=cos)�� ����Ͽ� �÷��̾��� y-���� ȸ�� ���� m_fYaw�� �����Ѵ�.*/
		m_fYaw = (float)XMConvertToDegrees(acosf(m_xv3Look.z));
		if (m_xv3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		/*���ο� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰� ���� ī�޶� ��尡 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷��̾��� ���� ���� ���� ī�޶��� ���� ��� ���� �����.*/
		m_xv3Right = m_pCamera->GetRightVector();
		m_xv3Up    = m_pCamera->GetUpVector();
		m_xv3Look  = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		//���� ī�޶� ������ ���ο� ī�޶� ���� ���̴� ������ �����Ѵ�.
		if (!m_pCamera) pNewCamera->CreateShaderVariables(pd3dDevice);
		pNewCamera->SetMode(nNewCameraMode);
		//���� ī�޶� ����ϴ� �÷��̾� ��ü�� �����Ѵ�.
		pNewCamera->SetPlayer(this);
	}

	if (m_pCamera) delete m_pCamera;

	return(pNewCamera);
}

void CPlayer::OnPlayerUpdated(float fTimeElapsed)
{
}

void CPlayer::OnCameraUpdated(float fTimeElapsed)
{
}

void CPlayer::ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed)
{
}

void CPlayer::CollisionProcess(float fTimeElapsed)
{
	// HACK : ���߿� ���� ó���� ��. �ϴ��� ��Ʈ���ϴ� �÷��̾ �浹üũ �ϵ��� �Ѵ�.
	vector<CEntity*> vcArray = QUADMgr.CollisionCheckList(this);
	auto it = find(vcArray.begin(), vcArray.end(), m_pChild);
	if (it != vcArray.end())
	{
		dynamic_cast<CStaff*>(*it)->InheritByPlayer(XMFLOAT3(10, 0, 0));
	}

	QUADMgr.ContainedErase();
}

void CPlayer::OnPrepareRender()
{
	m_xmf44World._11 = m_xv3Right.x;
	m_xmf44World._12 = m_xv3Right.y;
	m_xmf44World._13 = m_xv3Right.z;
	m_xmf44World._21 = m_xv3Up.x;
	m_xmf44World._22 = m_xv3Up.y;
	m_xmf44World._23 = m_xv3Up.z;
	m_xmf44World._31 = m_xv3Look.x;
	m_xmf44World._32 = m_xv3Look.y;
	m_xmf44World._33 = m_xv3Look.z;
	m_xmf44World._41 = m_xv3Position.x;
	m_xmf44World._42 = m_xv3Position.y;
	m_xmf44World._43 = m_xv3Position.z;
}

void CPlayer::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	//if (pCamera->GetPlayer() == this SetVisible(true);
	//const UINT uRender = (uRenderState | DRAW_AND_ACTIVE);
	CAnimatedObject::Render(pd3dDeviceContext, uRenderState, pCamera);
	//SetVisible(false);
}

void CPlayer::Animate(float fTimeElapsed)
{
	Update(fTimeElapsed);

	CGameObject::Animate(fTimeElapsed); // child, sibling
	CAnimatedObject::Animate(fTimeElapsed); 

	CollisionProcess(fTimeElapsed);

	OnCameraUpdated(fTimeElapsed);
}

///////////////////

CTerrainPlayer::CTerrainPlayer(int nMeshes) : CPlayer(nMeshes)
{
	m_bVibrationEffect = false;
}

CTerrainPlayer::~CTerrainPlayer()
{
}

void CTerrainPlayer::ChangeCamera(ID3D11Device *pd3dDevice, DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		//1��Ī ī�޶��� �� �÷��̾ y-�� �������� �߷��� �ۿ��Ѵ�.
		m_pCamera = OnChangeCamera(pd3dDevice, FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		//�����̽� �� ī�޶��� �� �÷��̾ �߷��� �ۿ����� �ʴ´�.
		SetGravity(0.0f);
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(pd3dDevice, SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		//3��Ī ī�޶��� �� �÷��̾ y-�� �������� �߷��� �ۿ��Ѵ�.
		m_pCamera = OnChangeCamera(pd3dDevice, THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.55f);
		m_pCamera->SetOffset(XMFLOAT3(0, 20.0f, -50.0)); //XMFLOAT3(0.0f, 30.0f, -30.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 1000.0f, ASPECT_RATIO, 60.0f);
		break;
	default:
		break;
	}
	Update(fTimeElapsed);
}

void CTerrainPlayer::OnPlayerUpdated(float fTimeElapsed)
{
	CCharacter::OnContextUpdated(fTimeElapsed);
}

void CTerrainPlayer::OnCameraUpdated(float fTimeElapsed)
{
	if (nullptr == m_pCamera) return;
	
	m_pCamera->Update(m_xv3Position, fTimeElapsed);

	CMapManager *pTerrain		= (CMapManager*)m_pCameraUpdatedContext;
	XMFLOAT3 xv3Scale           = pTerrain->GetScale();
	CCamera *pCamera            = GetCamera();
	XMFLOAT3 xv3CameraPosition  = pCamera->GetPosition();
	int z                       = (int)(xv3CameraPosition.z / xv3Scale.z);
	bool bReverseQuad = !(z % 2);//((z % 2) != 1);
	/*���� �ʿ��� ī�޶��� ���� ��ġ (x, z)�� ����(y ��)�� ���Ѵ�. �� ���� ī�޶��� ��ġ�� �ش��ϴ� ������ ���� ���� ������ ī�޶� ���ӿ� �ְ� �ȴ�.
	�̷��� �Ǹ� <�׸� 4>�� ���ʰ� ���� ������ �׷����� �ʴ� ��찡 �߻��Ѵ�(ī�޶� ���� �ȿ� �����Ƿ� ���ε� ������ �ٲ��).
	�̷��� ��찡 �߻����� �ʵ��� ī�޶��� ��ġ�� �ּҰ��� (������ ���� + 5)�� �����Ѵ�.
	ī�޶��� ��ġ�� �ּҰ��� ������ ��� ��ġ���� ī�޶� ���� �Ʒ��� ��ġ���� �ʵ��� �����Ѵ�.*/
	float fHeight = pTerrain->GetHeight(xv3CameraPosition.x, xv3CameraPosition.z, bReverseQuad) + 6;

	if (xv3CameraPosition.y < fHeight)
	{
		xv3CameraPosition.y = fHeight;// +6;
		pCamera->SetPosition(xv3CameraPosition);
	}

	XMFLOAT3 look = GetPosition();
	if (m_bVibrationEffect)
	{
		look.x += rand() % 6 - 3;
		look.y += rand() % 6 - 3;
		look.z += rand() % 6 - 3;
	}
	else
		look.y += 10.0f;
	pCamera->SetLookAt(look);

	m_pCamera->RegenerateViewMatrix();
}

CInGamePlayer::CInGamePlayer(int m_nMeshes) : CTerrainPlayer(m_nMeshes)
{
	ZeroMemory(&m_nElemental, sizeof(m_nElemental));

	m_pStateMachine = nullptr;

	m_bDominating = false;
	m_bMagicCancled = false;

	m_fDominateCoolTime = 0.f;
}

CInGamePlayer::~CInGamePlayer()
{
	if (m_pStateMachine) delete m_pStateMachine;
}

void CInGamePlayer::BuildObject(CMesh ** ppMeshList, int nMeshes, CTexture * pTexture, CMaterial * pMaterial)
{
	for (int i = 0; i < eANI_TOTAL_NUM; ++i)
	{
		CInGamePlayer::SetMesh(ppMeshList[i], i);
	}
	CInGamePlayer::InitializeAnimCycleTime();

	CInGamePlayer::SetMaterial(pMaterial);
	CInGamePlayer::SetTexture(pTexture);

	m_pStateMachine = new CStateMachine<CInGamePlayer>(this);
	m_pStateMachine->SetCurrentState(&CPlayerIdleState::GetInstance());
	CMapManager * pTerrain = &MAPMgr;
	//�÷��̾��� ��ġ�� ����� �� ������ ������ ���� �÷��̾��� ��ġ�� ������ �� �ֵ��� �����Ѵ�.
	CInGamePlayer::SetPlayerUpdatedContext(pTerrain);
	//ī�޶��� ��ġ�� ����� �� ������ ������ ���� ī�޶��� ��ġ�� ������ �� �ֵ��� �����Ѵ�.
	CInGamePlayer::SetCameraUpdatedContext(pTerrain);
	/*������ xz-����� ����� �÷��̾ ��ġ�ϵ��� �Ѵ�. �÷��̾��� y-��ǥ�� ������ ���� ���� ũ�� �߷��� �ۿ��ϵ��� �÷��̾ �����Ͽ����Ƿ� �÷��̾�� ���������� �ϰ��ϰ� �ȴ�.*/
	CInGamePlayer::InitPosition(XMFLOAT3(pTerrain->GetWidth()*0.5f + (rand() % 40 - 20), pTerrain->GetPeakHeight() + 10.0f, 300 + (rand () % 40 - 20)));

	SetFriction(250.0f);
	SetGravity(-20.0f);
	SetMaxVelocityXZ(300.0f);
	SetMaxVelocityY(400.0f);

	Reset();
}

void CInGamePlayer::GetGameMessage(CEntity * byObj, eMessage eMSG, void * extra)
{
	static XMFLOAT4 xmfInfo;
	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bVisible = true;
		return;
	case eMessage::MSG_PLAYER_SOUL:
		xmfInfo = *(XMFLOAT4*)extra;
		AddEnergy(xmfInfo.w);
		EVENTMgr.InsertDelayMessage(0.0f, MSG_PARTICLE_ON, CGameEventMgr::MSG_TYPE_SCENE, m_pScene, nullptr, extra);
		EVENTMgr.InsertDelayMessage(0.01f, eMessage::MSG_OBJECT_RENEW, CGameEventMgr::MSG_TYPE_ENTITY, byObj);
		return;
	case eMessage::MSG_COLLIDE:
		return;
	case eMessage::MSG_COLLIDED:
	{
		CEffect * pEffect = dynamic_cast<CEffect*>(byObj);
		if (pEffect) Damaged(pEffect);
		return;
	}
	case eMessage::MSG_EFFECT_VIBRATE_ON:
		m_bVibrationEffect = true;
		return;
	case eMessage::MSG_EFFECT_VIBRATE_OFF:
		m_bVibrationEffect = false;
		return;
	//case eMessage::MSG_NORMAL:
	//	return;
	case eMessage::MSG_COLLIDE_LOCATION:
		//toObj->GetGameMessage(this, MSG_COLLIDE);
		return;
	//case eMessage::MSG_MAGIC_SHOT:
	//	return;
	case eMessage::MSG_PLAYER_DOMIATE_END:
		StopDominate();
		return;
	}
}

void CInGamePlayer::SendGameMessage(CEntity * toObj, eMessage eMSG, void * extra)
{
	CGameObject * pObj = nullptr;

	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
		// �ݴ�� �޼��� �����ϵ��� ����
	case eMessage::MSG_COLLIDE:
		ForcedByObj(toObj);
		toObj->GetGameMessage(this, MSG_COLLIDED);
		return;
	case eMessage::MSG_COLLIDED:
		toObj->GetGameMessage(this, MSG_COLLIDE);
		return;
	}
}

void CInGamePlayer::InitializeAnimCycleTime()
{
	SetAnimationCycleTime(eANI_IDLE,            mfIdleAnim);
	SetAnimationCycleTime(eANI_RUN_FORWARD,     mfRunForwardAnim);

	SetAnimationCycleTime(eANI_1H_CAST,         mf1HCastAnimTime);
	SetAnimationCycleTime(eANI_1H_MAGIC_ATTACK, mf1HMagicAttackAnimTime);
	SetAnimationCycleTime(eANI_1H_MAGIC_AREA,   mf1HMagicAreaAnimTime);

	SetAnimationCycleTime(eANI_BLOCK_START,		mfBlockStartAnimTime);
	SetAnimationCycleTime(eANI_BLOCK_IDLE,		mfBlockIdleAnimTime);
	SetAnimationCycleTime(eANI_BLOCK_END,		mfBlockEndAnimTime);

	SetAnimationCycleTime(eANI_DAMAGED_FRONT_01, mfDamagedAnimTime01);
	SetAnimationCycleTime(eANI_DAMAGED_FRONT_02, mfDamagedAnimTime02);
	SetAnimationCycleTime(eANI_DEATH_FRONT,      mfDeathAnimTime);
	SetAnimationCycleTime(eANI_JUMP,			 mfJumpTime);
}

void CInGamePlayer::Update(float fTimeElapsed)
{
	m_fDominateCoolTime -= fTimeElapsed;
	m_fMagicCoolTime    -= fTimeElapsed;

	m_pStateMachine->Update(fTimeElapsed);
	CPlayer::Update(fTimeElapsed);
}

void CInGamePlayer::CollisionProcess(float fTimeElapsed)
{
	// HACK : ���߿� ���� ó���� ��. �ϴ��� ��Ʈ���ϴ� �÷��̾ �浹üũ �ϵ��� �Ѵ�.
	vector<CEntity*> vcArray = QUADMgr.CollisionCheckList(this);
	auto it = find(vcArray.begin(), vcArray.end(), m_pChild);
	if (it != vcArray.end())
	{
		dynamic_cast<CStaff*>(*it)->InheritByPlayer(XMFLOAT3(10, 0, 0));
	}

	QUADMgr.ContainedErase();
}

void CInGamePlayer::ForcedByObj(CEntity * pObj)
{
	if (pObj->IsObstacle())
	{
#if 0
		if (pObj->IsDetailCollide())
		{
			pObj->UpdateBoundingBox();
			if (AABB::CollisionAABB(m_bcMeshBoundingCube, pObj->m_bcMeshBoundingCube))
			{
				//m_xv3BeforePos.y = MAPMgr.GetHeight(m_xv3BeforePos);
				XMFLOAT3 nowPos = GetPosition();
				SetPosition(m_xv3BeforePos);
				m_xv3BeforePos = nowPos;
				//cout << "Before!!" << GetPosition() << "\n->>"<< pObj->m_bcMeshBoundingCube << endl;
			}
		}
		else
#endif
		{
			XMVECTOR xmvOtherPos = XMLoadFloat3(&pObj->GetPosition());
			XMVECTOR toThisVector = XMLoadFloat3(&GetPosition()) - xmvOtherPos;
			toThisVector = XMVector3Normalize(toThisVector);

			XMVECTOR xmvSize = XMVectorReplicate(GetSize() + pObj->GetSize() + 1);
			xmvOtherPos = xmvOtherPos + (toThisVector * xmvSize);

			XMFLOAT3 xmf3NewPos;
			XMStoreFloat3(&xmf3NewPos, xmvOtherPos);
			SetPosition(xmf3NewPos);
			//cout << "Forced, Size : " << pObj->GetSize() << endl;
			//if (dynamic_cast<CBillboardObject*>(pObj)) { cout << "Billboard �浹! : " << pObj->GetSize() << endl;; }
		}
	}
}

void CInGamePlayer::RenewPacket(sc_packet_pos & packet)
{
	SetDirection(packet.dwDirection);
	SetPosition(packet.Position);

	m_xv3Look = packet.LookVector;
	Chae::XMFloat3Cross(&m_xv3Right, &m_xv3Up, &m_xv3Look);
	Chae::XMFloat3Normalize(&m_xv3Right, &m_xv3Right);
}
void CInGamePlayer::RenewPacket(sc_packet_rotate & packet)
{
	m_xv3Look = packet.LookVector;
	Chae::XMFloat3Cross(&m_xv3Right, &m_xv3Up, &m_xv3Look);
	Chae::XMFloat3Normalize(&m_xv3Right, &m_xv3Right);
}

void CInGamePlayer::Attack(CCharacter * pToChar, short stDamage)
{
}

void CInGamePlayer::AttackSuccess(CCharacter * pToChar, short stDamage)
{
	pToChar->Damaged(this, stDamage);
}

void CInGamePlayer::Damaged(CEffect * pEffect)
{
	Damaged(static_cast<CCharacter*>(pEffect->GetMaster()), pEffect->GetDamage());
	//pEffect->Get
	// �ٲ� �κ�
	/*cs_packet_damage my_packet;
	my_packet.id = CLIENT.GetClientID();
	my_packet.size = sizeof(cs_packet_state);
	my_packet.type = CS_CHAR_DAMAGE;
	my_packet.damage = pEffect->GetDamage();
	CLIENT.SendPacket(reinterpret_cast<unsigned  char*>(&my_packet));*/
}
void CInGamePlayer::Damaged(CCharacter * pByChar, short stDamage)
{
	if (GetAnimationState() == eANI_1H_MAGIC_ATTACK || GetAnimationState() == eANI_1H_CAST) m_bMagicCancled = true;
	if (false == m_Status.IsAlive()) return;

	m_Status.Damaged(stDamage);
	m_Status.SetUnbeatable(true);

	if (stDamage > 9)
	{
		m_pDamagedEntity = pByChar;
		m_pStateMachine->ChangeState(&CPlayerKnockbackState::GetInstance());
	}
	else
	{
		m_pStateMachine->ChangeState(&CPlayerDamagedState::GetInstance());
	}

	if (m_Status.GetHP() < 1)
	{
		Death(pByChar);
	}
}

void CInGamePlayer::Revive()
{
	Reset();
	m_pStateMachine->ChangeState(&CPlayerIdleState::GetInstance());

	CCamera * pCamera = GetCamera();
	XMFLOAT3 xmfVector = pCamera->GetRightVector();
	m_xmf44World._11 = xmfVector.x;
	m_xmf44World._12 = xmfVector.y;
	m_xmf44World._13 = xmfVector.z;

	xmfVector = pCamera->GetUpVector();
	m_xmf44World._21 = xmfVector.x;
	m_xmf44World._22 = xmfVector.y;
	m_xmf44World._23 = xmfVector.z;

	xmfVector = pCamera->GetLookVector();
	m_xmf44World._31 = xmfVector.x;
	m_xmf44World._32 = xmfVector.y;
	m_xmf44World._33 = xmfVector.z;
}

void CInGamePlayer::Reset()
{
	SetPosition(m_xmf3InitPos);
	ThrowItem();
	m_Status.ResetStatus();
	m_Status.SetHP(mMAX_HEALTH);
	GetAniMesh()->ResetIndex();
}

void CInGamePlayer::PlayerKeyEventOn(WORD key, void * extra)
{
	//static WORD wdNext[] = { eANI_1H_MAGIC_ATTACK };

	switch (key)
	{
	case 'E':
		ThrowItem();
		return;

	case 'N':
		return;

	case 'M':
		ChangeAnimationState(eANI_1H_MAGIC_AREA, true, nullptr, 0);
		EVENTMgr.InsertDelayMessage(1.0f, eMessage::MSG_MAGIC_AREA, CGameEventMgr::MSG_TYPE_SCENE, extra, nullptr, this);
		return;

	case 'G':
		StartDominate();
		return;
	}
}

void CInGamePlayer::PlayerKeyEventOff(WORD key, void * extra)
{
	switch (key)
	{
	case 'W':
	case 'S' :
	case 'A' :
	case 'D' :
	{
		if (m_pStateMachine->CanChangeState() && m_Status.IsCanMove())
		{
			ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
		}
		return;
	}
	case 'G':
		StopDominate();
		return;

	case '0':
		Revive();
		return;
	case 'J':
	{
		cout << "PLUS" << endl;
		PACKET_MGR.SendServerCheat(0);
		return;
	}
	case 'K':
	{
		cout << "MINUS" << endl;
		PACKET_MGR.SendServerCheat(1);
		return;
	}
	case VK_DELETE:
	{
		PACKET_MGR.SendServerCheat(2);
		return;
	}
	case VK_HOME:
	{
		PACKET_MGR.SendServerCheat(3);
		return;
	}
	case VK_END:
	{
		PACKET_MGR.SendServerCheat(4);
		return;
	}
	case '5':
		m_bVibrationEffect = true;
		EVENTMgr.InsertDelayMessage(0.5f, eMessage::MSG_EFFECT_VIBRATE_OFF, CGameEventMgr::MSG_TYPE_ENTITY, this);
		return;

	case '6':
		EVENTMgr.InsertDelayMessage(0.0f, eMessage::MSG_EFFECT_GLARE_ON, CGameEventMgr::MSG_TYPE_SCENE, m_pScene);
		EVENTMgr.InsertDelayMessage(1.2f, eMessage::MSG_EFFECT_GLARE_OFF, CGameEventMgr::MSG_TYPE_SCENE, m_pScene);
		return;
	case '7':
		m_Status.GetBuffMgr().OnPlusDamage();
		return;
	case '8':
		m_Status.GetBuffMgr().DownPlusDamage();
		return;
	case '9':
		for (int i = 0; i < ELEMENT_NUM; ++i)
			AddEnergy(i, 3);
		return;
	}
}

void CInGamePlayer::AddEnergy(UINT index, UINT num)
{
	m_nElemental.m_nSum += num;

	BYTE nNewNum = (m_nElemental.m_nEnergies[index] += num);
	if (m_nElemental.m_nMinNum < nNewNum)
	{
		m_nElemental.m_nMinNum = nNewNum;
	}

#ifdef _DEBUG
	//cout << m_nElemental << endl;
	//cout << "�� ������ : " << (UINT)m_nElemental.m_nSum << "\t";
	//cout << index << "�� ������ : " <<  (UINT)m_nElemental.m_nEnergies[index] << endl;
#endif

}

int CInGamePlayer::UseMagic()
{
	if (m_fMagicCoolTime > 0.f) return -1;

	if (m_pChild)
	{
		CStaff * pMyItem = static_cast<CStaff*>(m_pChild);
		const int element = pMyItem->GetElement();
		const int lv = pMyItem->GetLevel() + 1;
		
		return UseEnergy(element, 3 * lv, false);
	}
	return UseAllEnergy(1);
}

UINT CInGamePlayer::UseEnergy(UINT index, BYTE energyNum, bool bForced)
{
	if (bForced)
	{
		BYTE num = min(m_nElemental.m_nEnergies[index], energyNum);
		m_nElemental.m_nEnergies[index] -= num;
		m_nElemental.m_nSum -= num;
		return num;
	}
	else if (m_nElemental.m_nEnergies[index] >= energyNum)
	{
		m_nElemental.m_nEnergies[index] -= energyNum;
		m_nElemental.m_nSum -= energyNum;
		return energyNum;
	}
	return 0;
}

UINT CInGamePlayer::UseEnergy(UINT energyNum, bool bForced)
{
	if (false == bForced || m_nElemental.m_nSum < energyNum)
		return false;
	
	BYTE num = energyNum;

	for (int i = 0; i < ELEMENT_NUM; ++i)
	{
		if (0 == (num -= UseEnergy(i, num, bForced)))
			break;
	}
	//cout << m_nElemental << endl;
	return num;
}

UINT CInGamePlayer::UseAllEnergy(UINT energyNum, bool bForced)
{
	if (false == bForced && m_nElemental.m_nMinNum < energyNum)
		return false;

	for (int i = 0; i < ELEMENT_NUM; ++i)
	{	
		UseEnergy(i, energyNum, bForced);
	}
	return true;
}

void CInGamePlayer::Kill(CCharacter * pChar)
{
	auto info = SYSTEMMgr.GetPlayerInfo();
	info[m_iPlayerNum].m_nKillCount++;
	info[m_iPlayerNum].m_iPlayerPoint += 10;
}

void CInGamePlayer::Death(CCharacter * pChar)
{
	auto info = SYSTEMMgr.GetPlayerInfo();
	info[m_iPlayerNum].m_nDeathCount++;
	m_pStateMachine->ChangeState(&CPlayerDeathState::GetInstance());
	PACKET_MGR.SendPlayerInfo(this);
	auto enemy = dynamic_cast<CInGamePlayer*>(pChar);
	if (enemy) 
		enemy->Kill(nullptr);
}

void CInGamePlayer::Jump()
{
	if (false == GetFSM()->isInState(CPlayerJumpState::GetInstance()))
		GetFSM()->ChangeState(&CPlayerJumpState::GetInstance());
}

void CInGamePlayer::MagicShot()
{
	m_fMagicCoolTime = 1.8f;
	m_Status.SetCanMove(false);
	ChangeAnimationState(eANI_1H_CAST, true, &mwd1HMagicShot[1], 1);
	EVENTMgr.InsertDelayMessage(0.25f, eMessage::MSG_MAGIC_CAST, CGameEventMgr::MSG_TYPE_SCENE, m_pScene, nullptr, this);
	EVENTMgr.InsertDelayMessage(mf1HCastAnimTime + 0.6f, eMessage::MSG_MAGIC_SHOT, CGameEventMgr::MSG_TYPE_SCENE, m_pScene, nullptr, this);
}

void CInGamePlayer::AcquireItem(CItem * pItem)
{
	CStaff * pStaff = nullptr;
	m_pItem = pItem;
	if (pStaff = dynamic_cast<CStaff*>(pItem))
	{
		SetChild(pStaff);

		if(SYSTEMMgr.GetPlayerNum() == m_iPlayerNum)
			EVENTMgr.InsertDelayMessage(0.02f, eMessage::MSG_ITEM_STAFF_CHANGE, CGameEventMgr::MSG_TYPE_SCENE,
				m_pScene, nullptr, m_pChild);
	}
}

void CInGamePlayer::ThrowItem()
{
	if (m_pChild)
	{
		CStaff * pMyItem = static_cast<CStaff*>(m_pChild);

		XMFLOAT3 ThrowVelocity;
		Chae::XMFloat3AddAndMulFloat(&ThrowVelocity, &GetCenterPosition(), 1.f, &GetLookVector(), 20.f);

		pMyItem->ResetMaster(ThrowVelocity);

		m_pChild->Release();
		m_pChild = nullptr;

		EVENTMgr.InsertDelayMessage(0.01f, eMessage::MSG_ITEM_STAFF_CHANGE, CGameEventMgr::MSG_TYPE_SCENE,
			m_pScene, nullptr, m_pChild);
	}
}

void CInGamePlayer::StartDominate()
{
	if (CanStartDominating() && SYSTEMMgr.CheckCanDominateRange(this))
	{
		m_pStateMachine->ChangeState(&CPlayerDominateState::GetInstance());
		m_bDominating = true;
	}
}

void CInGamePlayer::StopDominate()
{
	if (m_bDominating)
	{
		m_fDominateCoolTime = mfDominateCoolTime;
		m_bDominating = false;

		WORD idle[] = { eANI_IDLE };
		ChangeAnimationState(eANI_BLOCK_END, true, idle, 1);

		if (SYSTEMMgr.CheckCanDomianteSuccess(this))
		{
			SucceessDominate();
		}
	}
}

void CInGamePlayer::SucceessDominate()
{
	EVENTMgr.InsertDelayMessage(0.0f, eMessage::MSG_MAGIC_AREA, CGameEventMgr::MSG_TYPE_SCENE,
		m_pScene, nullptr, &SYSTEMMgr.GetPortalZonePos());
	EVENTMgr.InsertDelayMessage(0.8f, eMessage::MSG_EFFECT_GLARE_ON, CGameEventMgr::MSG_TYPE_SCENE, m_pScene);
	EVENTMgr.InsertDelayMessage(2.2f, eMessage::MSG_EFFECT_GLARE_OFF, CGameEventMgr::MSG_TYPE_SCENE, m_pScene);
}

void CInGamePlayer::CheckGameSystem(float fTimeElapsed)
{
}

EFFECT_ON_INFO CInGamePlayer::GetCastEffectOnInfo()
{
	float fColor = 0.f;
	//if (m_pChild)
	//{
	//	auto staff = static_cast<CStaff*>(m_pChild);
	//	fColor = staff->GetElement();
	//}

	XMMATRIX mtx = XMLoadFloat4x4(&m_xmf44World);
	mtx = XMMatrixTranslation(-4.f, 10.0f, 15.0f) * mtx;
	XMFLOAT4X4 xmf44Change;
	XMStoreFloat4x4(&xmf44Change, mtx);

	XMFLOAT3 up = XMFLOAT3(xmf44Change._21, xmf44Change._22, xmf44Change._23);

	static EFFECT_ON_INFO info;
	info.m_pObject      = this;
	info.fColor         = fColor;
	info.fDamage        = 0.f;
	info.eEffect        = EFFECT_CASTING;
	info.m_xmf3Pos      = move(XMFLOAT3(xmf44Change._41, xmf44Change._42, xmf44Change._43));
	//info.m_xmf3Velocity = up;
	//info.m_xmfAccelate  = up;
	//info.bUseUpdateVelocity = true;

	return info;
}

EFFECT_ON_INFO CInGamePlayer::Get1HAnimShotParticleOnInfo()
{
	static EFFECT_TYPE effectArr[] = { EFFECT_STARBALL, EFFECT_ICEBALL, EFFECT_FIREBALL, EFFECT_FIREBALL};

	XMMATRIX mtx = XMLoadFloat4x4(&m_xmf44World);
	mtx = XMMatrixTranslation(0, 9.0f, 15.0f) * mtx;
	XMFLOAT4X4 xmf44Change;
	XMStoreFloat4x4(&xmf44Change, mtx);

	float fColor = 0.f;
	float fDamage = 10.f;
	int nStaffLv = 0;

	if (m_pChild)
	{
		auto staff = static_cast<CStaff*>(m_pChild);
		fColor = staff->GetElement();
		nStaffLv = staff->GetLevel() + 1;
		fDamage *= nStaffLv;
	}
	fDamage += (m_Status.GetBuffMgr().IsPlusDamage() ? 20 : 0);

	static EFFECT_ON_INFO info;
	info.m_pObject      = this;
	info.fColor         = fColor;
	info.fDamage		= fDamage;
	info.eEffect        = effectArr[nStaffLv];
	info.m_xmf3Pos      = move(XMFLOAT3(xmf44Change._41, xmf44Change._42, xmf44Change._43));
	info.m_xmf3Velocity = move(XMFLOAT3(xmf44Change._31, xmf44Change._32, xmf44Change._33));
	info.m_xmfAccelate  = move(XMFLOAT3(-xmf44Change._31, -xmf44Change._32, -xmf44Change._33));
	info.bUseUpdateVelocity = true;

	return info;
}
