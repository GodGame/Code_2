#include "stdafx.h"
#include "MyInline.h"
#include "Object.h"
#include "Shader.h"
//////////////////////////////////////////////////////////////////////////

CEntity::CEntity()
{
	m_uSize       = 0;
	m_bVisible    = true;
	m_bActive     = true;
	m_bUseCollide = false;
	m_bObstacle   = true;
	m_bDetailCollide = false;

	ZeroMemory(&m_bcMeshBoundingCube, sizeof(m_bcMeshBoundingCube));
}

void CEntity::GetGameMessage(CEntity * byEntity, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bVisible = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bVisible = false;
		return;
	case eMessage::MSG_COLLIDE:
		return;
	case eMessage::MSG_COLLIDED:
		return;
	}
}

void CEntity::SendGameMessage(CEntity * toEntity, eMessage eMSG, void * extra)
{
	switch (eMSG)
	{
	case eMessage::MSG_NORMAL:
		return;
		// �ݴ�� �޼��� �����ϵ��� ����
	case eMessage::MSG_COLLIDE:
		toEntity->GetGameMessage(this, MSG_COLLIDED);
		return;
	case eMessage::MSG_COLLIDED:
		toEntity->GetGameMessage(this, MSG_COLLIDE);
		return;
	}
}

void CEntity::MessageEntToEnt(CEntity * byEntity, CEntity * toEntity, eMessage eMSG, void * extra)
{
	byEntity->SendGameMessage(toEntity, eMSG);
	toEntity->GetGameMessage(byEntity, eMSG);
}

#pragma region GameObject
CGameObject::CGameObject(int nMeshes)
{
	Chae::XMFloat4x4Identity(&m_xmf44World);

	m_bUseInheritAutoRender = true;

	m_nMeshes  = nMeshes;
	m_ppMeshes = nullptr;

	if (m_nMeshes > 0)
		m_ppMeshes = new CMesh*[m_nMeshes];

	for (int i = 0; i < m_nMeshes; i++)
		m_ppMeshes[i] = nullptr;

	m_bcMeshBoundingCube = AABB();

	m_nReferences = 0;
	m_pMaterial   = nullptr;
	m_pTexture    = nullptr;

	m_pChild      = nullptr;
	m_pSibling    = nullptr;
	//m_pParent     = nullptr;
}

CGameObject::~CGameObject()
{
	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Release();
			m_ppMeshes[i] = nullptr;
		}
		delete[] m_ppMeshes;
	}
	if (m_pMaterial) m_pMaterial->Release();
	if (m_pTexture) m_pTexture->Release();

	// child, sibling�� ReleaseRelationShip�� �̿��� �ܺο��� �����ش�.
}

void CGameObject::_SetMaterialAndTexture(ID3D11DeviceContext * pd3dDeviceContext)
{
	if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
	if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
}

void CGameObject::AddRef() { m_nReferences++; }

void CGameObject::Release()
{
	//if (m_nReferences > 0) m_nReferences--;
	if (--m_nReferences < 1)
	{
		ReleaseRelationShip();

		delete this;
	}
}

void CGameObject::ReleaseRelationShip()
{
	// ��Ʈ�� �ƴ϶��, �ڽ��� ��� ����� ������ �θ𿡰� �Ѱ��ְ� �����Ѵ�.
	if (m_pChild)   m_pChild->Release();
	//if (m_pParent)	SuccessSibling();
	if (m_pSibling) m_pSibling->Release();
}

void CGameObject::SetTexture(CTexture* const pTexture, bool beforeRelease)
{
	if (beforeRelease && m_pTexture) m_pTexture->Release();
	m_pTexture = pTexture;
	if (m_pTexture) m_pTexture->AddRef();
}

void CGameObject::UpdateSubResources(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	XMFLOAT4X4 xmf44Result;

	if (false == m_bUseInheritAutoRender) 
		CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	else if (pmtxParentWorld)
	{
		Chae::XMFloat4x4Mul(&xmf44Result, &m_xmf44World, pmtxParentWorld);

		if (m_pChild)   m_pChild->Render(pd3dDeviceContext, uRenderState, pCamera, &xmf44Result);
		if (m_pSibling) m_pSibling->Render(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);

		CShader::UpdateShaderVariable(pd3dDeviceContext, xmf44Result);
	}
	else
	{
		if (m_pChild)   m_pChild->Render(pd3dDeviceContext, uRenderState, pCamera, &m_xmf44World);
		if (m_pSibling) m_pSibling->Render(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);

		CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	}
}

void CGameObject::UpdateBoundingBox()
{
	m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());
}

void CGameObject::SetMesh(CMesh* const pMesh, int nIndex)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}

	if (pMesh)
	{
		AABB bcBoundingCube = pMesh->GetBoundingCube();
		m_bcMeshBoundingCube.Union(&bcBoundingCube);
		//m_bcMeshBoundingCube.Update(m_xmf44World, nullptr);

		XMVECTOR xmvMax = XMLoadFloat3(&m_bcMeshBoundingCube.m_xv3Maximum);
		XMVECTOR xmvMin = XMLoadFloat3(&m_bcMeshBoundingCube.m_xv3Minimum);

		xmvMax = xmvMax - xmvMin;
		xmvMax = XMVector3Length(xmvMax);

		float fSize;
		XMStoreFloat(&fSize, xmvMax);
		m_uSize = (UINT)(fSize * 0.25f);
	}
	else
	{
		m_bcMeshBoundingCube.m_xv3Maximum = { FLT_MIN, FLT_MIN, FLT_MIN };
		m_bcMeshBoundingCube.m_xv3Minimum = { FLT_MAX, FLT_MAX, FLT_MAX };
	}
}

void CGameObject::Animate(float fTimeElapsed)
{
	if (m_pChild) m_pChild->Animate(fTimeElapsed);
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed);
}

bool CGameObject::IsVisible(CCamera * pCamera)
{
	OnPrepareRender();

	if (pCamera)
	{
		AABB bcBoundingCube = m_bcMeshBoundingCube;
		bcBoundingCube.Update(m_xmf44World);
		if (pCamera) m_bVisible = pCamera->IsInFrustum(&bcBoundingCube);
	}
	return(m_bVisible);
}

void CGameObject::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	if (false == m_bActive) return;
	if (m_bVisible || uRenderState & RS_SHADOWMAP || (pmtxParentWorld && m_bUseInheritAutoRender))
	{
		CGameObject::UpdateSubResources(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
		CGameObject::_SetMaterialAndTexture(pd3dDeviceContext);

		for (int i = 0; i < m_nMeshes; i++)
		{
#ifdef _QUAD_TREE
			m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
			if (false == (uRenderState & DRAW_AND_ACTIVE))
				m_bVisible = false;
#else
			bool bIsVisible = true;
			if (pCamera)
			{
				AABB bcBoundingCube = m_ppMeshes[i]->GetBoundingCube();
				bcBoundingCube.Update(m_xmf44World);
				bIsVisible = pCamera->IsInFrustum(&bcBoundingCube);
			}
			if (bIsVisible)
				m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
#endif
		}
	}
}
 
void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf44World._41 = x;
	m_xmf44World._42 = y;
	m_xmf44World._43 = z;
}
void CGameObject::SetPosition(const XMFLOAT3& xv3Position)
{
//	memcpy(&m_xmf44World._41, &xv3Position, sizeof(XMFLOAT3));
	m_xmf44World._41 = xv3Position.x;
	m_xmf44World._42 = xv3Position.y;
	m_xmf44World._43 = xv3Position.z;
}
void CGameObject::SetPosition(const XMVECTOR * xv3Position)
{
	XMFLOAT3 xmf3Pos;
	XMStoreFloat3(&xmf3Pos, *xv3Position);
	SetPosition(xmf3Pos);
}
#ifdef PICKING
void CGameObject::GenerateRayForPicking(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxWorld, XMFLOAT4X4 *pxmtxView, XMFLOAT3 *pxv3PickRayPosition, XMFLOAT3 *pxv3PickRayDirection)
{
	//pxv3PickPosition: ī�޶� ��ǥ���� ��(ȭ�� ��ǥ�迡�� ���콺�� Ŭ���� ���� ����ȯ�� ��)
	//pxmtxWorld: ���� ��ȯ ���, pxmtxView: ī�޶� ��ȯ ���
	//pxv3PickRayPosition: ��ŷ ������ ������, pxv3PickRayDirection: ��ŷ ���� ����
	/*��ü�� ���� ��ȯ ����� �־����� ��ü�� ���� ��ȯ ��İ� ī�޶� ��ȯ ����� ���ϰ� ������� ���Ѵ�. �̰��� ī�޶� ��ȯ ����� ����İ� ��ü�� ���� ��ȯ ����� ������� ���� ����. ��ü�� ���� ��ȯ ����� �־����� ������ ī�޶� ��ȯ ����� ������� ���Ѵ�. ��ü�� ���� ��ȯ ����� �־����� �� ��ǥ���� ��ŷ ������ ���ϰ� �׷��� ������ ���� ��ǥ���� ��ŷ ������ ���Ѵ�.*/
	XMFLOAT4X4 xmtxInverse;
	XMFLOAT4X4 xmtxWorldView = *pxmtxView;
	if (pxmtxWorld) XMFLOAT4X4Multiply(&xmtxWorldView, pxmtxWorld, pxmtxView);
	XMFLOAT4X4Inverse(&xmtxInverse, nullptr, &xmtxWorldView);
	XMFLOAT3 xv3CameraOrigin(0.0f, 0.0f, 0.0f);
	/*ī�޶� ��ǥ���� ���� (0, 0, 0)�� ������ ���� ����ķ� ��ȯ�Ѵ�. ��ȯ�� ����� ī�޶� ��ǥ���� ������ �����Ǵ� �� ��ǥ���� �� �Ǵ� ���� ��ǥ���� ���̴�.*/
	xv3ec3TransformCoord(pxv3PickRayPosition, &xv3CameraOrigin, &xmtxInverse);
	/*ī�޶� ��ǥ���� ���� ������ ���� ����ķ� ��ȯ�Ѵ�. ��ȯ�� ����� ���콺�� Ŭ���� ���� �����Ǵ� �� ��ǥ���� �� �Ǵ� ���� ��ǥ���� ���̴�.*/
	xv3ec3TransformCoord(pxv3PickRayDirection, pxv3PickPosition, &xmtxInverse);
	//��ŷ ������ ���� ���͸� ���Ѵ�.
	*pxv3PickRayDirection = *pxv3PickRayDirection - *pxv3PickRayPosition;
}

int CGameObject::PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	//pxv3PickPosition: ī�޶� ��ǥ���� ��(ȭ�� ��ǥ�迡�� ���콺�� Ŭ���� ���� ����ȯ�� ��)
	//pxmtxView: ī�޶� ��ȯ ���
	XMFLOAT3 xv3PickRayPosition, xv3PickRayDirection;
	int nIntersected = 0;
	//Ȱ��ȭ�� ��ü�� ���Ͽ� �޽��� ������ ��ŷ ������ ���ϰ� ��ü�� �޽��� �浹 �˻縦 �Ѵ�.
	if (m_bVisible && m_ppMeshes)
	{
		//��ü�� �� ��ǥ���� ��ŷ ������ ���Ѵ�.
		GenerateRayForPicking(pxv3PickPosition, &m_xmf44World, pxmtxView, &xv3PickRayPosition, &xv3PickRayDirection);
		/*�� ��ǥ���� ��ŷ ������ �޽��� �浹�� �˻��Ѵ�. ��ŷ ������ �޽��� �ﰢ������ ���� �� �浹�� �� �ִ�. �˻��� ����� �浹�� Ƚ���̴�.*/
		for (int i = 0; i < m_nMeshes; i++)
		{
			nIntersected = m_ppMeshes[i]->CheckRayIntersection(&xv3PickRayPosition, &xv3PickRayDirection, pd3dxIntersectInfo);
			if (nIntersected > 0) break;
		}
	}
	return(nIntersected);
}
#endif

void CGameObject::SetMaterial(CMaterial *pMaterial)
{
	if (m_pMaterial) m_pMaterial->Release();
	m_pMaterial = pMaterial;
	if (m_pMaterial) m_pMaterial->AddRef();
}

void CGameObject::ChangeChild(CGameObject * pObject)
{
	if (m_pChild)
	{
		CGameObject * pSiblingOfChild = m_pChild->GetSiblingObject();
		
		if (pSiblingOfChild)
			pObject->SetSibling(pSiblingOfChild);

		m_pChild->Release();
	}
	m_pChild = pObject;
	m_pChild->AddRef();
}

void CGameObject::SetChild(CGameObject * pObject)
{
	CGameObject * pBeforeChild = nullptr;
	pBeforeChild = m_pChild;

	m_pChild = pObject;
	if (m_pChild)	m_pChild->AddRef();

	if (pBeforeChild) 
	{
		if (m_pChild) m_pChild->SetSibling(pBeforeChild);
		pBeforeChild->Release();
	}
}

void CGameObject::SetSibling(CGameObject * pObject)
{
	CGameObject * pBeforeSibling = nullptr;
	pBeforeSibling = m_pSibling;

	if (pBeforeSibling)
	{
		pBeforeSibling->SetSibling(m_pSibling);
	}
	else
	{
		m_pSibling = pObject;
		m_pSibling->AddRef();
	}
}

XMFLOAT3 CGameObject::GetLookAt() const 
{
	//���� ��ü�� ���� z-�� ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 xv3LookAt(m_xmf44World._31, m_xmf44World._32, m_xmf44World._33);
	//Chae::XMFloat3Normalize(&xv3LookAt);
	return(xv3LookAt);
}
XMFLOAT3 CGameObject::GetUp() const
{
	//���� ��ü�� ���� y-�� ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 xv3Up(m_xmf44World._21, m_xmf44World._22, m_xmf44World._23);
	//Chae::XMFloat3Normalize(&xv3Up);
	return(xv3Up);
}
XMFLOAT3 CGameObject::GetRight() const
{
	//���� ��ü�� ���� x-�� ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 xv3Right(m_xmf44World._11, m_xmf44World._12, m_xmf44World._13);
	//Chae::XMFloat3Normalize(&xv3Right);
	return(xv3Right);
}

void CGameObject::MoveStrafe(float fDistance)
{
	//���� ��ü�� ���� x-�� �������� �̵��Ѵ�.
	XMVECTOR xmv3Position = XMLoadFloat3(&GetPosition());
	XMVECTOR xv3Right = XMLoadFloat3(&GetRight());
	xmv3Position += fDistance * xv3Right;

	CGameObject::SetPosition(&xmv3Position);
}
void CGameObject::MoveUp(float fDistance)
{
	//���� ��ü�� ���� y-�� �������� �̵��Ѵ�.
	XMVECTOR xv3Position = XMLoadFloat3(&GetPosition());
	XMVECTOR xv3Up = XMLoadFloat3(&GetUp());
	xv3Position += fDistance * xv3Up;
	CGameObject::SetPosition(&xv3Position);
}
void CGameObject::MoveForward(float fDistance)
{
	//���� ��ü�� ���� z-�� �������� �̵��Ѵ�.
	XMVECTOR xv3Position = XMLoadFloat3(&GetPosition());
	XMVECTOR xv3LookAt = XMLoadFloat3(&GetLookAt());
	xv3Position += fDistance * xv3LookAt;
	CGameObject::SetPosition(&xv3Position);
}
void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	//���� ��ü�� �־��� ������ ȸ���Ѵ�.
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf44World);
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	//XMFLOAT4X4RotationYawPitchRoll(&mtxRotate, (float)D3DXToRadian(fYaw), (float)D3DXToRadian(fPitch), (float)D3DXToRadian(fRoll));
	mtxWorld = XMMatrixMultiply(mtxRotate, mtxWorld);
	XMStoreFloat4x4(&m_xmf44World, mtxWorld);
}
void CGameObject::Rotate(XMFLOAT3 *pxv3Axis, float fAngle)
{
	//���� ��ü�� �־��� ȸ������ �߽����� ȸ���Ѵ�.
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf44World);
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxv3Axis), XMConvertToRadians(fAngle));

	mtxWorld = XMMatrixMultiply(mtxRotate, mtxWorld);
	XMStoreFloat4x4(&m_xmf44World, mtxWorld);
}

XMFLOAT3 CGameObject::GetPosition() const
{
	return(XMFLOAT3(m_xmf44World._41, m_xmf44World._42, m_xmf44World._43));
}

#pragma endregion


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CStaticObject::CStaticObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xmf3ExternalPower = { 0, 0, 0 };
	m_fFriction = 0.f;
}

CStaticObject::~CStaticObject()
{
}

CDynamicObject::CDynamicObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xv3Velocity = { 0, 0, 0 };
}


CAnimatedObject::CAnimatedObject(int nMeshes) : CDynamicObject(nMeshes)
{
	m_vcfAnimationCycleTime.resize(nMeshes, 1.0f);
	m_vcfFramePerTime.resize(nMeshes, 1.0f);

	m_wdAnimateState = 0;
}

CAnimatedObject::~CAnimatedObject()
{
}

void CAnimatedObject::ChangeAnimationState(WORD wd, bool bReserveIdle, WORD * pNextStateArray, int nNum)
{
	if (m_wdAnimateState != wd)
	{
		m_wdAnimateState = wd;
		static_cast<CAnimatedMesh*>(m_ppMeshes[m_wdAnimateState])->ResetIndex();

		m_bReserveBackIdle = bReserveIdle;

		if (pNextStateArray)
		{
			m_vcNextAnimState.clear();

			for (int i = 0; i < nNum; ++i)
				m_vcNextAnimState.push_back(*(pNextStateArray + nNum - (i + 1)));
		}

	}
}

void CAnimatedObject::SetAnimationCycleTime(WORD wdAnimNum, float fCycleTime)
{
	m_vcfAnimationCycleTime[wdAnimNum] = fCycleTime;

	if (m_ppMeshes[wdAnimNum])
	{
		m_vcfFramePerTime[wdAnimNum] =
			static_cast<ANI_MESH*>(m_ppMeshes[wdAnimNum])->SetOneCycleTime(m_vcfAnimationCycleTime[wdAnimNum]);
	}
}

void CAnimatedObject::UpdateFramePerTime()
{
	for (int i = 0; i < m_nMeshes; ++i)
	{
		if (m_ppMeshes[i])
		{
			m_vcfFramePerTime[i] =
				static_cast<ANI_MESH*>(m_ppMeshes[i])->SetOneCycleTime(m_vcfAnimationCycleTime[i]);
		}
	}
}

void CAnimatedObject::SetMesh(CMesh * pMesh, int nIndex)
{
	CLoadAnimatedMeshByADFile * pAnimMesh = dynamic_cast<CLoadAnimatedMeshByADFile*>(pMesh);
	CMesh * pTempMesh = pMesh;
	
	if (pAnimMesh) 
	{
		CAnimatedMesh * pCopyMesh = new CAnimatedMesh(*pAnimMesh);
		pTempMesh = pCopyMesh;
	}

	CGameObject::SetMesh(pTempMesh, nIndex);
	
	m_vcfFramePerTime[nIndex] =
		static_cast<ANI_MESH*>(m_ppMeshes[nIndex])->SetOneCycleTime(m_vcfAnimationCycleTime[nIndex]);
}

void CAnimatedObject::UpdateBoundingBox()
{
	m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());// m_wdAnimateState]->GetBoundingCube());
}

void CAnimatedObject::Animate(float fTimeElapsed)
{
	ANI_MESH * pMesh = static_cast<ANI_MESH*>(m_ppMeshes[m_wdAnimateState]);
	pMesh->SetFramePerTime(m_vcfFramePerTime[m_wdAnimateState]);
	pMesh->Animate(fTimeElapsed);
	
	if (pMesh->IsEndAnimation())
	{
		if (!m_vcNextAnimState.empty())
		{
			ChangeAnimationState(*(m_vcNextAnimState.end() - 1), m_bReserveBackIdle, nullptr, 0);
			m_vcNextAnimState.pop_back();
		}
		else if (m_bReserveBackIdle)
		{
			ChangeAnimationState(eANI_IDLE, false, nullptr, 0);
			m_bReserveBackIdle = false;
		}
	}
}

void CAnimatedObject::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
	if (false == m_bActive) return;

	if (m_ppMeshes && m_bVisible)
	{
		CGameObject::UpdateSubResources(pd3dDeviceContext, uRenderState, pCamera, pmtxParentWorld);
		//��ü�� ����(�������)�� ���̴� ������ ����(����)�Ѵ�.
		if (m_pMaterial) CIlluminatedShader::UpdateShaderVariable(pd3dDeviceContext, &m_pMaterial->m_Material);
		//��ü�� �ؽ��ĸ� ���̴� ������ ����(����)�Ѵ�.
		if (m_pTexture) m_pTexture->UpdateShaderVariable(pd3dDeviceContext);
#ifdef _QUAD_TREE
		m_ppMeshes[m_wdAnimateState]->Render(pd3dDeviceContext, uRenderState);
		if (!(uRenderState & DRAW_AND_ACTIVE))
			m_bVisible = false;
#else
		bool bIsVisible = true;
		if (pCamera)
		{
			AABB bcBoundingCube = m_ppMeshes[i]->GetBoundingCube();
			bcBoundingCube.Update(m_xmf44World);
			bIsVisible = pCamera->IsInFrustum(&bcBoundingCube);
		}
		if (bIsVisible)
			m_ppMeshes[i]->Render(pd3dDeviceContext, uRenderState);
#endif
	}
}


////////////////////////////////////////////////////////////////////////////////////////
CUIObject::CUIObject(int nMeshes, UIInfo info) : CGameObject(nMeshes) 
{
	if (info.m_msgUI != MSG_UI_NONE)
	{
		SetCollide(true);
	}
	
	m_info = info;
}

bool CUIObject::CollisionCheck(XMFLOAT3 & pos)
{
	return CollisionCheck(POINT{ (LONG)pos.x, (LONG)pos.y });
}

bool CUIObject::CollisionCheck(POINT & pt)
{
	if (!m_bUseCollide)		  return false;

	RECT & rect = m_info.m_rect;

	if (pt.x > rect.right)  return false;
	if (pt.x < rect.left)   return false;
	if (pt.y > rect.top)    return false;
	if (pt.y < rect.bottom) return false;

	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CRotatingObject::CRotatingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_fRotationSpeed = 15.0f;
}
CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	CGameObject::Rotate(&m_xv3RotationAxis, m_fRotationSpeed * fTimeElapsed);
}

CRevolvingObject::CRevolvingObject(int nMeshes) : CGameObject(nMeshes)
{
	m_xv3RevolutionAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed)
{
	CGameObject::Animate(fTimeElapsed);
	//CGameObject::Rotate(&m_xv3RevolutionAxis, XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	//������ ��Ÿ���� ���� ȸ�� ����� �����ʿ� ���Ѵ�.
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xv3RevolutionAxis), (float)XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	XMMATRIX mtxWorld = XMLoadFloat4x4(&m_xmf44World);
	mtxWorld = mtxWorld * mtxRotate;

	XMStoreFloat4x4(&m_xmf44World, mtxWorld);
}

CHeightMapTerrain::CHeightMapTerrain(ID3D11Device *pd3dDevice, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale) : CGameObject(0)
{
	MAPMgr.Build(pFileName, nWidth, nLength, nBlockWidth, nBlockLength, xv3Scale);

	/*���� ��ü�� ���� �޽����� �迭�� ���� ���̴�. nBlockWidth, nBlockLength�� ���� �޽� �ϳ��� ����, ���� ũ���̴�. cxQuadsPerBlock, czQuadsPerBlock�� ���� �޽��� ���� ����� ���� ���� �簢���� �����̴�.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
#ifdef TS_TERRAIN
	CTerrainPartMesh *pHeightMapGridMesh = nullptr;
	for (float zStart = 0; zStart < m_xv3Scale.z; zStart++)
	{
		for (float xStart = 0; xStart < m_xv3Scale.x; xStart++)
		{
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� ���� ��ġ�̴�.
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� �����Ͽ� ���� �޽��� �����Ѵ�.
			pHeightMapGridMesh = new CTerrainPartMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, m_pHeightMap);
			SetMesh(pHeightMapGridMesh, xStart + (zStart*m_xv3Scale.x));
		}
	}
#else
	//�������� ���� ����, ���� �������� ���� �޽��� �� ���� �ִ� ���� ��Ÿ����.
	int cxBlocks = (nWidth - 1) / cxQuadsPerBlock;
	int czBlocks = (nLength - 1) / czQuadsPerBlock;
	//���� ��ü�� ǥ���ϱ� ���� ���� �޽��� �����̴�.
	m_nMeshes = cxBlocks * czBlocks;
	//���� ��ü�� ǥ���ϱ� ���� ���� �޽��� ���� ������ �迭�� �����Ѵ�.
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = nullptr;

	CHeightMap * heightMap = MAPMgr.GetHeightMap();
	CHeightMapGridMesh *pHeightMapGridMesh = nullptr;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� ���� ��ġ�̴�.
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� �����Ͽ� ���� �޽��� �����Ѵ�.
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, heightMap);
			SetMesh(pHeightMapGridMesh, x + (z*cxBlocks));
		}
	}
#endif
	MAPMgr.SetMaxHeight(m_bcMeshBoundingCube.m_xv3Maximum.y);
	MAPMgr.SetMinHeight(m_bcMeshBoundingCube.m_xv3Minimum.y);

	Chae::XMFloat4x4Identity(&m_xmf44World);
}

CHeightMapTerrain::~CHeightMapTerrain()
{
//	if (m_pHeightMap) delete m_pHeightMap;
}

CWaterTerrain::CWaterTerrain(ID3D11Device * pd3dDevice, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xv3Scale)
{
	m_fDepth = 0.0f;
	/*���� ��ü�� ���� �޽����� �迭�� ���� ���̴�. nBlockWidth, nBlockLength�� ���� �޽� �ϳ��� ����, ���� ũ���̴�. cxQuadsPerBlock, czQuadsPerBlock�� ���� �޽��� ���� ����� ���� ���� �簢���� �����̴�.*/
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;
	
	int cxBlocks = (nWidth - 1) / cxQuadsPerBlock;
	int czBlocks = (nLength - 1) / czQuadsPerBlock;
	//���� ��ü�� ǥ���ϱ� ���� ���� �޽��� �����̴�.
	m_nMeshes = cxBlocks * czBlocks;
	//���� ��ü�� ǥ���ϱ� ���� ���� �޽��� ���� ������ �迭�� �����Ѵ�.
	m_ppMeshes = new CMesh*[m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)m_ppMeshes[i] = nullptr;

	CHeightMap * heightMap = MAPMgr.GetHeightMap();
	CWaterGridMesh *pHeightMapGridMesh = nullptr;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� ���� ��ġ�̴�.
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			//������ �Ϻκ��� ��Ÿ���� ���� �޽��� �����Ͽ� ���� �޽��� �����Ѵ�.
			pHeightMapGridMesh = new CWaterGridMesh(pd3dDevice, xStart, zStart, nBlockWidth, nBlockLength, xv3Scale, heightMap);
			SetMesh(pHeightMapGridMesh, x + (z*cxBlocks));
		}
	}
	Chae::XMFloat4x4Identity(&m_xmf44World);
}

CWaterTerrain::~CWaterTerrain()
{
}


#define SKYBOX_CUBE
CSkyBox::CSkyBox(ID3D11Device *pd3dDevice, UINT uImageNum) : CGameObject(1)
{
	CSkyBoxMesh *pSkyBoxMesh = new CSkyBoxMesh(pd3dDevice, uImageNum, 20.0f, 20.0f, 20.0f);
	SetMesh(pSkyBoxMesh, 0);
}

CSkyBox::~CSkyBox()
{
}

void CSkyBox::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera, XMFLOAT4X4 * pmtxParentWorld)
{
#ifdef SKYBOX_CUBE
	XMFLOAT3 xv3CameraPos = pCamera->GetPosition();
	SetPosition(xv3CameraPos.x, xv3CameraPos.y, xv3CameraPos.z);
	CShader::UpdateShaderVariable(pd3dDeviceContext, m_xmf44World);
	//m_pTexture->UpdateShaderVariable(pd3dDeviceContext);

	if (m_ppMeshes && m_ppMeshes[0])
		m_ppMeshes[0]->Render(pd3dDeviceContext, uRenderState);

#else
	//��ī�� �ڽ� ��ü�� ��ġ�� ī�޶��� ��ġ�� �����Ѵ�.
	XMFLOAT3 xv3CameraPos = pCamera->GetPosition();
	SetPosition(xv3CameraPos.x, xv3CameraPos.y, xv3CameraPos.z);
	CShader::UpdateShaderVariable(pd3dDeviceContext, &m_xmf44World);

	//��ī�� �ڽ� �޽�(6���� �簢��)�� �������Ѵ�.
	if (m_ppMeshes && m_ppMeshes[0]) m_ppMeshes[0]->Render(pd3dDeviceContext);
#endif
}

CBillboardObject::CBillboardObject(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size) : CGameObject(1)
{
	m_xv4InstanceData = XMFLOAT4(pos.x, pos.y, pos.z, (float)fID);
	m_xv2Size = xmf2Size;
	SetSize((m_xv2Size.x + m_xv2Size.y) * 0.5);
	SetPosition(pos);
}

void CBillboardObject::UpdateInstanceData()
{
	const XMFLOAT3 pos = GetPosition();
	memcpy(&m_xv4InstanceData, &pos, sizeof(XMFLOAT3));
}

bool CBillboardObject::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bVisible;

	if (pCamera)
	{
		m_bcMeshBoundingCube.Update(m_xmf44World, &m_ppMeshes[0]->GetBoundingCube());
		bIsVisible = pCamera->IsInFrustum(&m_bcMeshBoundingCube);
	}

	XMFLOAT3 xmfPos = GetPosition();
	memcpy(&m_xv4InstanceData, &xmfPos, sizeof(XMFLOAT3));
	return(bIsVisible);
}

//////////////////////////////////////
CAbsorbMarble::CAbsorbMarble() : CBillboardObject()
{
	m_bObstacle = false;
}

CAbsorbMarble::CAbsorbMarble(XMFLOAT3 pos, UINT fID, XMFLOAT2 xmf2Size) : CBillboardObject(pos, fID, xmf2Size)
{
	//Initialize();
}

CAbsorbMarble::~CAbsorbMarble()
{
}

void CAbsorbMarble::Initialize()
{
	m_bObstacle = false;
	SetCollide(true);

	m_bAbsorb          = false;
	m_fAbsorbTime      = 0.0f;
	m_fSpeed           = 0.0f;
	m_pTargetObject    = nullptr;

	SetSize(miAbsorbSize);

	m_xvRandomVelocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
}

void CAbsorbMarble::Revive()
{
	CMapManager * pTerrain = nullptr;
	XMFLOAT3 xmf3Pos = MAPMgr.GetRandPos();
	xmf3Pos.y += 10.f;

	Initialize();
	SetPosition(xmf3Pos);

	SetCollide(true);
}

void CAbsorbMarble::SetTarget(CGameObject * pGameObject)
{
	if (false == m_bAbsorb)
	{
		SetSize(10);
		SetCollide(false);

		m_pTargetObject = pGameObject;
		m_bAbsorb = true;

		XMVECTOR xmvTarget = XMLoadFloat3(&m_pTargetObject->GetPosition());
		XMVECTOR xvPos = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvFromTarget = xvPos - xmvTarget;
		xmvFromTarget = XMVector3Normalize(xmvFromTarget);
		XMStoreFloat3(&m_xvRandomVelocity, xmvFromTarget);

		m_xvRandomVelocity.y = abs(m_xvRandomVelocity.y);
		m_fSpeed = rand() % 10 + 10.0f;
	}
}

void CAbsorbMarble::Animate(float fTimeElapsed)
{
	if (m_bAbsorb)
	{
		m_fAbsorbTime		+= fTimeElapsed;
		XMFLOAT3 xmfPos      = m_pTargetObject->GetPosition();
		xmfPos.y += 10;
		XMVECTOR xmvTarget   = XMLoadFloat3(&xmfPos);
		XMVECTOR xvPos       = XMLoadFloat3(&CBillboardObject::GetPosition());
		XMVECTOR xmvToTarget = xmvTarget - xvPos;
		xmvToTarget          = XMVector3Normalize(xmvToTarget) * 2.0f;

		XMVECTOR xvSpeed     = XMVectorReplicate(m_fSpeed);
		XMVECTOR xvRandom    = XMLoadFloat3(&m_xvRandomVelocity);

		xvPos = (0.5f * xmvToTarget * m_fAbsorbTime * m_fAbsorbTime) + (xvRandom * m_fAbsorbTime) + xvPos;
		XMStoreFloat3(&xmfPos, xvPos);
		SetPosition(xmfPos);
		CBillboardObject::UpdateInstanceData();

		XMVECTOR lengthSq = XMVector3LengthSq(xmvTarget - xvPos);
		float flengthSq;
		XMStoreFloat(&flengthSq, lengthSq);

		if (flengthSq < 60.0f && m_fAbsorbTime > 0.3f)
		{
			GetGameMessage(this, eMessage::MSG_COLLIDE);
		}
	}
}

void CAbsorbMarble::GetGameMessage(CEntity * byObj, eMessage eMSG, void * extra)
{
	CInGamePlayer * pPlayer = nullptr;

	switch (eMSG)
	{
	case eMessage::MSG_CULL_IN:
		m_bVisible = true;
		return;
	case eMessage::MSG_CULL_OUT:
		m_bVisible = false;
		return;
	case eMessage::MSG_COLLIDE:
		m_pTargetObject->GetGameMessage(this, eMessage::MSG_PLAYER_SOUL, &GetInstanceData());
		Revive();
		//EVENTMgr.InsertDelayMessage(1.0f, eMessage::MSG_OBJECT_RENEW, CGameEventMgr::MSG_TYPE_OBJECT, this);
		return;
	case eMessage::MSG_COLLIDED:
		if (pPlayer = dynamic_cast<CInGamePlayer*>(byObj))
			SetTarget(pPlayer);
		return;

	//case eMessage::MSG_QUAD_DELETE:

	//	return;
	case eMessage::MSG_OBJECT_RENEW:
		CBillboardObject::UpdateInstanceData();
		SetCollide(true);
		QUADMgr.InsertStaticEntity(this);
		return;
	case eMessage::MSG_NORMAL:
		return;
	}
}

bool CAbsorbMarble::IsVisible(CCamera *pCamera)
{
	bool bIsVisible = m_bVisible;

	if (pCamera)
		bIsVisible = CBillboardObject::IsVisible(pCamera);
	else if (bIsVisible || m_bAbsorb)
		bIsVisible = m_bVisible = true;

	return(bIsVisible);
}

