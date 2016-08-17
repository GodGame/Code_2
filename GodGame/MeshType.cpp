#include "stdafx.h"
#include "MeshType.h"
#include "MyInline.h"
#include "CollisionMgr.h"

#include <wchar.h>

CMesh::CMesh(ID3D11Device *pd3dDevice)
{
	m_nBuffers = 0;
	m_pd3dPositionBuffer = nullptr;
	m_pd3dColorBuffer = nullptr;
	m_ppd3dVertexBuffers = nullptr;

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nSlot = 0;
	m_nStartVertex = 0;

	m_pd3dIndexBuffer = nullptr;
	m_nIndices = 0;
	m_nStartIndex = 0;
	m_nBaseVertex = 0;
	m_nIndexOffset = 0;
	m_dxgiIndexFormat = DXGI_FORMAT_R32_UINT;

	m_pd3dRasterizerState = nullptr;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(+FLT_MAX, +FLT_MAX, +FLT_MAX);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	m_nReferences = 0;

	m_pxv3Positions = nullptr;
	m_pnIndices = nullptr;

	m_pnVertexStrides = nullptr;
	m_pnVertexOffsets = nullptr;
}


CMesh::~CMesh()
{
	if (m_pd3dRasterizerState) m_pd3dRasterizerState->Release();
	if (m_pd3dPositionBuffer) m_pd3dPositionBuffer->Release();
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();

	if (m_ppd3dVertexBuffers)
	{
		//for (int i = 0; i < m_nBuffers; ++i)
			//if (m_ppd3dVertexBuffers[i])
			//	m_ppd3dVertexBuffers[i]->Release();
		delete[] m_ppd3dVertexBuffers;
	}
	if (m_pnVertexStrides) delete[] m_pnVertexStrides;
	if (m_pnVertexOffsets) delete[] m_pnVertexOffsets;

	if (m_pxv3Positions) delete[] m_pxv3Positions;
	if (m_pnIndices) delete[] m_pnIndices;
}

void CMesh::AssembleToVertexBuffer(int nBuffers, ID3D11Buffer **ppd3dBuffers, UINT *pnBufferStrides, UINT *pnBufferOffsets)
{
	//printf("1");
	ID3D11Buffer **ppd3dNewVertexBuffers = new ID3D11Buffer*[m_nBuffers + nBuffers];
	UINT *pnNewVertexStrides = new UINT[m_nBuffers + nBuffers];
	UINT *pnNewVertexOffsets = new UINT[m_nBuffers + nBuffers];

	if (m_nBuffers > 0)
	{
		for (int i = 0; i < m_nBuffers; i++)
		{
			ppd3dNewVertexBuffers[i] = m_ppd3dVertexBuffers[i];
			pnNewVertexStrides[i] = m_pnVertexStrides[i];
			pnNewVertexOffsets[i] = m_pnVertexOffsets[i];
		}
		if (m_ppd3dVertexBuffers) delete[] m_ppd3dVertexBuffers;
		if (m_pnVertexStrides) delete[] m_pnVertexStrides;
		if (m_pnVertexOffsets) delete[] m_pnVertexOffsets;
	}

	for (int i = 0; i < nBuffers; i++)
	{
		ppd3dNewVertexBuffers[m_nBuffers + i] = ppd3dBuffers[i];
		pnNewVertexStrides[m_nBuffers + i] = pnBufferStrides[i];
		pnNewVertexOffsets[m_nBuffers + i] = pnBufferOffsets[i];
	}

	m_nBuffers += nBuffers;
	m_ppd3dVertexBuffers = ppd3dNewVertexBuffers;
	m_pnVertexStrides = pnNewVertexStrides;
	m_pnVertexOffsets = pnNewVertexOffsets;
}

void CMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	//�޽��� ������ ���� ���� ���� ���۷� ǥ���ȴ�.
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	if (!(uRenderState & RS_SHADOWMAP)) pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexed(m_nIndices, m_nStartIndex, m_nBaseVertex);
	else
		pd3dDeviceContext->Draw(m_nVertices, m_nStartVertex);
}

void CMesh::RenderInstanced(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, int nInstances, int nStartInstance)
{
	//�ν��Ͻ��� ��� �Է� �����⿡ �޽��� ���� ���ۿ� �ν��Ͻ� ���� ���۰� ����ȴ�.
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	if (uRenderState == 0)
		pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	//��ü���� �ν��Ͻ����� �������Ѵ�. 
	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexedInstanced(m_nIndices, nInstances, m_nStartIndex, m_nBaseVertex, nStartInstance);
	else
		pd3dDeviceContext->DrawInstanced(m_nVertices, nInstances, m_nStartVertex, nStartInstance);
}

MeshBuffer CMesh::GetNormalMapVertexBuffer(ID3D11Device * pd3dDevice, NormalMapVertex * pData, int nSize, float fScale)
{
	MeshBuffer meshInfo;
	meshInfo.pd3dBuffer = nullptr;

	XMFLOAT3 min{ FLT_MAX, FLT_MAX, FLT_MAX }, max{ FLT_MIN, FLT_MIN, FLT_MIN };

	XMVECTOR xmvScale = XMVectorSet(fScale, fScale, fScale, 1);
	XMMATRIX xmtxRotate = XMMatrixIdentity();// XMMatrixRotationX(-90);

	meshInfo.nVertexes = nSize;
	for (unsigned int i = 0; i < nSize; ++i)
	{
		XMVECTOR xmvPos = XMVectorSet(pData[i].xmf3Pos.x, pData[i].xmf3Pos.y, pData[i].xmf3Pos.z, 1);
		xmvPos *= xmvScale;
		xmvPos = XMVector3TransformCoord(xmvPos, xmtxRotate);

		//XMStoreFloat3(&m_pxv3Positions[i], xmvPos);
		XMStoreFloat3(&pData[i].xmf3Pos, xmvPos);

		if (min.x > pData[i].xmf3Pos.x) min.x = pData[i].xmf3Pos.x;
		if (min.y > pData[i].xmf3Pos.y) min.y = pData[i].xmf3Pos.y;
		if (min.z > pData[i].xmf3Pos.z) min.z = pData[i].xmf3Pos.z;

		if (max.x < pData[i].xmf3Pos.x) max.x = pData[i].xmf3Pos.x;
		if (max.y < pData[i].xmf3Pos.y) max.y = pData[i].xmf3Pos.y;
		if (max.z < pData[i].xmf3Pos.z) max.z = pData[i].xmf3Pos.z;
		//		pChaeInfo[i].xmf3Normal = pChaeInfo[i].xmf3Normal;
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(NormalMapVertex) * nSize;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pData;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &meshInfo.pd3dBuffer);

	meshInfo.bb.m_xv3Maximum = max;
	meshInfo.bb.m_xv3Minimum = min;

	meshInfo.nStrides = sizeof(NormalMapVertex);
	meshInfo.nOffsets = 0;

	return meshInfo;
}

MeshBuffer CMesh::GetNormalMapVertexBufferAndCalculateTangent(ID3D11Device * pd3dDevice, NormalMapVertex * pData, int nSize, float fScale)
{
	MeshBuffer meshInfo;
	meshInfo.pd3dBuffer = nullptr;

	XMFLOAT3 min{ FLT_MAX, FLT_MAX, FLT_MAX }, max{ FLT_MIN, FLT_MIN, FLT_MIN };

	XMVECTOR xmvScale = XMVectorSet(fScale, fScale, fScale, 1);
	XMMATRIX xmtxRotate = XMMatrixIdentity();// XMMatrixRotationX(-90);

	UINT nVertices = meshInfo.nVertexes = nSize;

	float vecX, vecY, vecZ, tcU1, tcU2, tcV1, tcV2;
	XMVECTOR edge1, edge2;
	XMFLOAT3 tangent[3], normal[3];

	int CalIndex[] = { +1, +2, +1, -1, -2, -1 };//{ 1, 2, 1, 2, 1, 2 }; 
	UINT offsetLeft3 = 0, offset = 0;

	for (unsigned int i = 0; i < nVertices; ++i)
	{
		offset = (i % 3);
		if (offset == 0) 
		{
			for (UINT j = 0; j < 3; ++j)
			{
				offsetLeft3 = j * 2;
				tangent[j] = { 0, 0, 0 };

				NormalMapVertex * pNearVertexes[3] = { &pData[i],
					&pData[(i + CalIndex[offsetLeft3 + 1]) % nVertices],
					&pData[(i + CalIndex[offsetLeft3 + 2]) % nVertices] };

				//Get the vector describing one edge of our triangle (edge 0,2)
				vecX = pNearVertexes[0]->xmf3Pos.x - pNearVertexes[2]->xmf3Pos.x;
				vecY = pNearVertexes[0]->xmf3Pos.y - pNearVertexes[2]->xmf3Pos.y;
				vecZ = pNearVertexes[0]->xmf3Pos.z - pNearVertexes[2]->xmf3Pos.z;
				edge1 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our first edge

																//Get the vector describing another edge of our triangle (edge 2,1)
				vecX = pNearVertexes[2]->xmf3Pos.x - pNearVertexes[1]->xmf3Pos.x;
				vecY = pNearVertexes[2]->xmf3Pos.y - pNearVertexes[1]->xmf3Pos.y;
				vecZ = pNearVertexes[2]->xmf3Pos.z - pNearVertexes[1]->xmf3Pos.z;
				edge2 = XMVectorSet(vecX, vecY, vecZ, 0.0f);	//Create our second edge


				tcU1 = pNearVertexes[0]->xmf2Tex.x - pNearVertexes[2]->xmf2Tex.x;
				tcV1 = pNearVertexes[0]->xmf2Tex.y - pNearVertexes[2]->xmf2Tex.y;

				//Find second texture coordinate edge 2d vector
				tcU2 = pNearVertexes[2]->xmf2Tex.x - pNearVertexes[1]->xmf2Tex.x;
				tcV2 = pNearVertexes[2]->xmf2Tex.y - pNearVertexes[1]->xmf2Tex.y;

				//Find tangent using both tex coord edges and position edges
				double fVal = (double)(tcU1 * tcV2 - tcU2 * tcV1);
				if (fVal == 0.0) fVal += 0.00001;
				tangent[offset].x += (tcV1 * XMVectorGetX(edge1) - tcV2 * XMVectorGetX(edge2)) * 1 / fVal;
				tangent[offset].y += (tcV1 * XMVectorGetY(edge1) - tcV2 * XMVectorGetY(edge2)) * 1 / fVal;
				tangent[offset].z += (tcV1 * XMVectorGetZ(edge1) - tcV2 * XMVectorGetZ(edge2)) * 1 / fVal;
			}

			XMVECTOR tangentSum = XMLoadFloat3(&tangent[offset]);
			tangentSum += XMLoadFloat3(&tangent[offset + 1]);
			tangentSum += XMLoadFloat3(&tangent[offset + 2]);
			tangentSum = XMVector3Normalize(tangentSum);

			XMStoreFloat3(&pData[i].xmf3Tangent, tangentSum);
			XMStoreFloat3(&pData[i + 1].xmf3Tangent, tangentSum);
			XMStoreFloat3(&pData[i + 2].xmf3Tangent, tangentSum);
		}

		XMVECTOR xmvPos = XMVectorSet(pData[i].xmf3Pos.x, pData[i].xmf3Pos.y, pData[i].xmf3Pos.z, 1);
		xmvPos *= xmvScale;
		xmvPos = XMVector3TransformCoord(xmvPos, xmtxRotate);

		//XMStoreFloat3(&m_pxv3Positions[i], xmvPos);
		XMStoreFloat3(&pData[i].xmf3Pos, xmvPos);

		if (min.x > pData[i].xmf3Pos.x) min.x = pData[i].xmf3Pos.x;
		if (min.y > pData[i].xmf3Pos.y) min.y = pData[i].xmf3Pos.y;
		if (min.z > pData[i].xmf3Pos.z) min.z = pData[i].xmf3Pos.z;

		if (max.x < pData[i].xmf3Pos.x) max.x = pData[i].xmf3Pos.x;
		if (max.y < pData[i].xmf3Pos.y) max.y = pData[i].xmf3Pos.y;
		if (max.z < pData[i].xmf3Pos.z) max.z = pData[i].xmf3Pos.z;
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(NormalMapVertex) * nSize;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &pData[0];
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &meshInfo.pd3dBuffer);

	meshInfo.bb.m_xv3Maximum = max;
	meshInfo.bb.m_xv3Minimum = min;

	meshInfo.nStrides = sizeof(NormalMapVertex);
	meshInfo.nOffsets = 0;

	return meshInfo;
}


void CMesh::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode        = D3D11_CULL_BACK;
	d3dRasterizerDesc.FillMode		  = D3D11_FILL_SOLID;//D3D11_FILL_WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = true;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

#ifdef PICKING
int CMesh::CheckRayIntersection(XMFLOAT3 *pxv3RayPosition, XMFLOAT3 *pxv3RayDirection, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	//�� ��ǥ���� ������ ������(pxv3RayPosition)�� ������ �־��� �� �޽����� �浹 �˻縦 �Ѵ�.
	int nIntersections = 0;
	BYTE *pbPositions = (BYTE *)m_pxv3Positions + m_pnVertexOffsets[0];

	int nOffset = (m_d3dPrimitiveTopology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? 3 : 1;
	//�޽��� ������Ƽ��(�ﰢ��)���� �����̴�. �ﰢ�� ����Ʈ�� ��� (������ ���� / 3) �Ǵ� (�ε����� ���� / 3), �ﰢ�� ��Ʈ���� ��� (������ ���� - 2) �Ǵ� (�ε����� ���� - 2)�̴�.
	int nPrimitives = (m_d3dPrimitiveTopology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nVertices / 3) : (m_nVertices - 2);
	if (m_nIndices > 0) nPrimitives = (m_d3dPrimitiveTopology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nIndices / 3) : (m_nIndices - 2);

	XMFLOAT3 v0, v1, v2;
	float fuHitBaryCentric, fvHitBaryCentric, fHitDistance, fNearHitDistance = FLT_MAX;
	/*�޽��� ��� ������Ƽ��(�ﰢ��)�鿡 ���Ͽ� ��ŷ �������� �浹�� �˻��Ѵ�. �浹�ϴ� ��� �ﰢ���� ã�� ������ ������(�����δ� ī�޶� ��ǥ���� ����)�� ���� ����� �ﰢ���� ã�´�.*/
	for (int i = 0; i < nPrimitives; i++)
	{
		v0 = *(XMFLOAT3 *)(pbPositions + ((m_pnIndices) ? (m_pnIndices[(i*nOffset) + 0]) : ((i*nOffset) + 0)) * m_pnVertexStrides[0]);
		v1 = *(XMFLOAT3 *)(pbPositions + ((m_pnIndices) ? (m_pnIndices[(i*nOffset) + 1]) : ((i*nOffset) + 1)) * m_pnVertexStrides[0]);
		v2 = *(XMFLOAT3 *)(pbPositions + ((m_pnIndices) ? (m_pnIndices[(i*nOffset) + 2]) : ((i*nOffset) + 2)) * m_pnVertexStrides[0]);
		//XMV

		if (D3DXIntersectTri(&v0, &v1, &v2, pxv3RayPosition, pxv3RayDirection, &fuHitBaryCentric, &fvHitBaryCentric, &fHitDistance))
		{
			if (fHitDistance < fNearHitDistance)
			{
				fNearHitDistance = fHitDistance;
				if (pd3dxIntersectInfo)
				{
					pd3dxIntersectInfo->m_dwFaceIndex = i;
					pd3dxIntersectInfo->m_fU = fuHitBaryCentric;
					pd3dxIntersectInfo->m_fV = fvHitBaryCentric;
					pd3dxIntersectInfo->m_fDistance = fHitDistance;
				}
			}
			nIntersections++;
		}
	}
	return(nIntersections);
}
#endif


float CAnimatedMesh::SetOneCycleTime(float fCycleTime)
{
	return (m_fFramePerTime = m_pvcMeshBuffers.size() / fCycleTime);
}

void CAnimatedMesh::SetFramePerTime(float fFramePerTime)
{
	m_fFramePerTime = fFramePerTime;
}

void CAnimatedMesh::SetAnimationIndex(int iIndex)
{
	m_iIndex = (iIndex) % m_pvcMeshBuffers.size();
	m_bTerminal = false;
}

CAnimatedMesh::CAnimatedMesh(ID3D11Device * pd3dDevice) : CMesh(pd3dDevice)
{
	m_fFramePerTime = 30.0f;
	m_fNowFrameTime = 0.0f;
	m_iIndex = 0;
	m_bTerminal = false;
	m_bStop = false;
	m_pvcMeshBuffers.reserve(30);
}

CAnimatedMesh::CAnimatedMesh(CAnimatedMesh & mesh) : CMesh(nullptr)
{
	m_fFramePerTime = 30.0f;
	m_fNowFrameTime = 0.0f;
	m_iIndex = 0;
	m_bTerminal = false;
	m_bStop = false;
	m_pvcMeshBuffers.reserve(30);

	operator=(mesh);
}

CAnimatedMesh::~CAnimatedMesh()
{
	for (auto it = m_pvcMeshBuffers.begin(); it != m_pvcMeshBuffers.end(); ++it)
	 	it->pd3dBuffer->Release();
	m_pvcMeshBuffers.clear();
}

void CAnimatedMesh::operator=(CAnimatedMesh & mesh)
{
	auto buffers = mesh.GetBuffers();

	for (auto & buffer : buffers)
	{
		buffer.pd3dBuffer->AddRef();
		m_pvcMeshBuffers.push_back(buffer);
	}

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_nVertices = 0;

	//���� ����
	m_bcBoundingCube = mesh.m_bcBoundingCube;

	m_pnVertexOffsets = new UINT[1];
	m_pnVertexOffsets[0] = m_pvcMeshBuffers[0].nOffsets;

	m_pnVertexStrides = new UINT[1];
	m_pnVertexStrides[0] = m_pvcMeshBuffers[0].nStrides;

	m_nBuffers = 1;
	m_nVertices = m_pvcMeshBuffers[0].nVertexes;

	m_pd3dRasterizerState = mesh.GetRasterizerState();
	if (m_pd3dRasterizerState) m_pd3dRasterizerState->AddRef();
}

void CAnimatedMesh::Animate(float fFrameTime)
{
	if (m_bStop) return;

	m_fNowFrameTime += fFrameTime * m_fFramePerTime;
	m_iIndex = (int(m_fNowFrameTime)) % m_pvcMeshBuffers.size();

	if (false == m_bTerminal) { m_bTerminal = (m_fNowFrameTime > m_pvcMeshBuffers.size() - 1); }
}

void CAnimatedMesh::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState)
{
	//�޽��� ������ ���� ���� ���� ���۷� ǥ���ȴ�.
	ID3D11Buffer * ppd3dBuffer[] = { m_pvcMeshBuffers[m_iIndex].pd3dBuffer };

	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, ppd3dBuffer, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	if (!(uRenderState & RS_SHADOWMAP)) pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	if (m_pd3dIndexBuffer)
		pd3dDeviceContext->DrawIndexed(m_nIndices, m_nStartIndex, m_nBaseVertex);
	else
		pd3dDeviceContext->Draw(m_nVertices, m_nStartVertex);
}

void CAnimatedMesh::CreateRasterizerState(ID3D11Device * pd3dDevice)
{
}


#pragma region BasicMesh

CMeshDiffused::CMeshDiffused(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_pd3dColorBuffer = nullptr;
}

CMeshDiffused::~CMeshDiffused()
{
	if (m_pd3dColorBuffer) m_pd3dColorBuffer->Release();
}

CCubeMeshDiffused::CCubeMeshDiffused(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth, XMFLOAT4 xmcolor) : CMeshDiffused(pd3dDevice)
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	//������ü �޽��� 2���� ���� ����(��ġ ���� ���ۿ� ���� ����)�� �����ȴ�.
	//������ü �޽��� ���� ����(��ġ ���� ����)�� �����Ѵ�.

	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, -fz);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, -fz);
	m_pxv3Positions[2] = XMFLOAT3(+fx, +fy, +fz);
	m_pxv3Positions[3] = XMFLOAT3(-fx, +fy, +fz);
	m_pxv3Positions[4] = XMFLOAT3(-fx, -fy, -fz);
	m_pxv3Positions[5] = XMFLOAT3(+fx, -fy, -fz);
	m_pxv3Positions[6] = XMFLOAT3(+fx, -fy, +fz);
	m_pxv3Positions[7] = XMFLOAT3(-fx, -fy, +fz);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	//������ü �޽��� ���� ����(���� ����)�� �����Ѵ�.
	XMFLOAT4 pXMFLOAT4s[8];
	XMVECTOR xmvColor = XMLoadFloat4(&xmcolor);
	for (int i = 0; i < 8; i++)
		XMStoreFloat4(&pXMFLOAT4s[i], xmvColor + XMVectorSet(RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR));

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4)* m_nVertices;
	d3dBufferData.pSysMem = pXMFLOAT4s;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	/*�ε��� ���۴� ������ü�� 6���� ��(�簢��)�� ���� ���� ������ ���´�. �ﰢ�� ����Ʈ�� ������ü�� ǥ���� ���̹Ƿ� �� ���� 2���� �ﰢ���� ������ �� �ﰢ���� 3���� ������ �ʿ��ϴ�. ��, �ε��� ���۴� ��ü 36(=6*2*3)���� �ε����� ������.*/
	m_nIndices = 36;

	m_pnIndices = new UINT[m_nIndices];
	//�� �ո�(Front) �簢���� ���� �ﰢ��
	m_pnIndices[0] = 3; m_pnIndices[1] = 1; m_pnIndices[2] = 0;
	//�� �ո�(Front) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[3] = 2; m_pnIndices[4] = 1; m_pnIndices[5] = 3;
	//�� ����(Top) �簢���� ���� �ﰢ��
	m_pnIndices[6] = 0; m_pnIndices[7] = 5; m_pnIndices[8] = 4;
	//�� ����(Top) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[9] = 1; m_pnIndices[10] = 5; m_pnIndices[11] = 0;
	//�� �޸�(Back) �簢���� ���� �ﰢ��
	m_pnIndices[12] = 3; m_pnIndices[13] = 4; m_pnIndices[14] = 7;
	//�� �޸�(Back) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[15] = 0; m_pnIndices[16] = 4; m_pnIndices[17] = 3;
	//�� �Ʒ���(Bottom) �簢���� ���� �ﰢ��
	m_pnIndices[18] = 1; m_pnIndices[19] = 6; m_pnIndices[20] = 5;
	//�� �Ʒ���(Bottom) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[21] = 2; m_pnIndices[22] = 6; m_pnIndices[23] = 1;
	//�� ����(Left) �簢���� ���� �ﰢ��
	m_pnIndices[24] = 2; m_pnIndices[25] = 7; m_pnIndices[26] = 6;
	//�� ����(Left) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[27] = 3; m_pnIndices[28] = 7; m_pnIndices[29] = 2;
	//�� ����(Right) �簢���� ���� �ﰢ��
	m_pnIndices[30] = 6; m_pnIndices[31] = 4; m_pnIndices[32] = 5;
	//�� ����(Right) �簢���� �Ʒ��� �ﰢ��
	m_pnIndices[33] = 7; m_pnIndices[34] = 4; m_pnIndices[35] = 6;

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	//�ε��� ���۸� �����Ѵ�.
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	//CreateRasterizerState(pd3dDevice);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshDiffused::~CCubeMeshDiffused()
{
}


CWireCube::CWireCube(ID3D11Device * pd3dDevice, float fWidth, float fHeight, float fDepth, XMFLOAT4 xmcolor) : CCubeMeshDiffused(pd3dDevice, fWidth, fHeight, fDepth, xmcolor)
{
	CreateRasterizerState(pd3dDevice);
}

CWireCube::~CWireCube()
{
}

void CWireCube::CreateRasterizerState(ID3D11Device * pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = true;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}


/////////////////////////////////////////

CSphereMeshDiffused::CSphereMeshDiffused(ID3D11Device *pd3dDevice, float fRadius, int nSlices, int nStacks, XMFLOAT4 xmcolor) : CMeshDiffused(pd3dDevice)
{
	m_nVertices = (nSlices * nStacks) * 3 * 2;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	//�� �޽��� 2���� ���� ����(��ġ ���� ���ۿ� ���� ����)�� �����ȴ�.
	//�� �޽��� ���� ����(��ġ ���� ����)�� �����Ѵ�.
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;
	for (int j = 0, k = 0; j < nStacks; j++)
	{
		phi_j = float(PI / nStacks) * j;
		phi_jj = float(PI / nStacks) * (j + 1);
		fRadius_j = fRadius * sinf(phi_j);
		fRadius_jj = fRadius * sinf(phi_jj);
		y_j = fRadius*cosf(phi_j);
		y_jj = fRadius*cosf(phi_jj);
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = float(2 * PI / nSlices) * i;
			theta_ii = float(2 * PI / nSlices) * (i + 1);
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_j*cosf(theta_i), y_j, fRadius_j*sinf(theta_i));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_jj*cosf(theta_ii), y_jj, fRadius_jj*sinf(theta_ii));
			m_pxv3Positions[k++] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	//�� �޽��� ���� ����(���� ����)�� �����Ѵ�.
	XMFLOAT4 pXMFLOAT4s[8];
	XMVECTOR xmvColor = XMLoadFloat4(&xmcolor);
	for (unsigned int i = 0; i < m_nVertices; i++)
		XMStoreFloat4(&pXMFLOAT4s[i], xmvColor + XMVectorSet(RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR, RANDOM_COLOR));

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4)* m_nVertices;
	d3dBufferData.pSysMem = pXMFLOAT4s;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);

	delete [] pXMFLOAT4s;

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fRadius, -fRadius, -fRadius);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fRadius, +fRadius, +fRadius);
}

CSphereMeshDiffused::~CSphereMeshDiffused()
{
}

CMeshTextured::CMeshTextured(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
}

CMeshTextured::~CMeshTextured()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
}

CMeshDetailTextured::CMeshDetailTextured(ID3D11Device *pd3dDevice) : CMeshTextured(pd3dDevice)
{
	m_pd3dDetailTexCoordBuffer = nullptr;
}

CMeshDetailTextured::~CMeshDetailTextured()
{
	if (m_pd3dDetailTexCoordBuffer) m_pd3dDetailTexCoordBuffer->Release();
}

////////////////////////////
CMeshTexturedIlluminated::CMeshTexturedIlluminated(ID3D11Device *pd3dDevice) : CMeshIlluminated(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
}

CMeshTexturedIlluminated::~CMeshTexturedIlluminated()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
}

CMeshDetailTexturedIlluminated::CMeshDetailTexturedIlluminated(ID3D11Device *pd3dDevice) : CMeshIlluminated(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
	m_pd3dDetailTexCoordBuffer = nullptr;
}

CMeshDetailTexturedIlluminated::~CMeshDetailTexturedIlluminated()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
	if (m_pd3dDetailTexCoordBuffer) m_pd3dDetailTexCoordBuffer->Release();
}


CMeshSplatTexturedIlluminated::CMeshSplatTexturedIlluminated(ID3D11Device *pd3dDevice) : CMeshIlluminated(pd3dDevice)
{
	m_pd3dTexCoordBuffer = nullptr;
	m_pd3dAlphaTexCoordBuffer = nullptr;
}

CMeshSplatTexturedIlluminated::~CMeshSplatTexturedIlluminated()
{
	if (m_pd3dTexCoordBuffer) m_pd3dTexCoordBuffer->Release();
	if (m_pd3dAlphaTexCoordBuffer) m_pd3dAlphaTexCoordBuffer->Release();
}

CCubeMeshIlluminated::CCubeMeshIlluminated(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshIlluminated(pd3dDevice)
{
	m_nVertices = 8;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, -fz);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, -fz);
	m_pxv3Positions[2] = XMFLOAT3(+fx, +fy, +fz);
	m_pxv3Positions[3] = XMFLOAT3(-fx, +fy, +fz);
	m_pxv3Positions[4] = XMFLOAT3(-fx, -fy, -fz);
	m_pxv3Positions[5] = XMFLOAT3(+fx, -fy, -fz);
	m_pxv3Positions[6] = XMFLOAT3(+fx, -fy, +fz);
	m_pxv3Positions[7] = XMFLOAT3(-fx, -fy, +fz);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	m_nIndices = 36;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 3; m_pnIndices[1] = 1; m_pnIndices[2] = 0;
	m_pnIndices[3] = 2; m_pnIndices[4] = 1; m_pnIndices[5] = 3;
	m_pnIndices[6] = 0; m_pnIndices[7] = 5; m_pnIndices[8] = 4;
	m_pnIndices[9] = 1; m_pnIndices[10] = 5; m_pnIndices[11] = 0;
	m_pnIndices[12] = 3; m_pnIndices[13] = 4; m_pnIndices[14] = 7;
	m_pnIndices[15] = 0; m_pnIndices[16] = 4; m_pnIndices[17] = 3;
	m_pnIndices[18] = 1; m_pnIndices[19] = 6; m_pnIndices[20] = 5;
	m_pnIndices[21] = 2; m_pnIndices[22] = 6; m_pnIndices[23] = 1;
	m_pnIndices[24] = 2; m_pnIndices[25] = 7; m_pnIndices[26] = 6;
	m_pnIndices[27] = 3; m_pnIndices[28] = 7; m_pnIndices[29] = 2;
	m_pnIndices[30] = 6; m_pnIndices[31] = 4; m_pnIndices[32] = 5;
	m_pnIndices[33] = 7; m_pnIndices[34] = 4; m_pnIndices[35] = 6;

	XMFLOAT3 pxv3Normals[8];
	CalculateVertexNormal(pxv3Normals); // ���

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshIlluminated::~CCubeMeshIlluminated()
{
}
#pragma endregion BasicMesh



CMeshIlluminated::CMeshIlluminated(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_pd3dNormalBuffer = nullptr;
}

CMeshIlluminated::~CMeshIlluminated()
{
	if (m_pd3dNormalBuffer) m_pd3dNormalBuffer->Release();
}

void CMeshIlluminated::CalculateVertexNormal(XMFLOAT3 *pxv3Normals)
{
	switch (m_d3dPrimitiveTopology)
	{
		/*������Ƽ�갡 �ﰢ�� ����Ʈ�� �� �ε��� ���۰� �ִ� ���� ���� ��츦 �����Ͽ� ������ ���� ���͸� ����Ѵ�. �ε��� ���۸� ������� �ʴ� ��� �� ������ ���� ���ʹ� �� ������ ���Ե� �ﰢ���� ���� ���ͷ� ����Ѵ�. �ε��� ���۸� ����ϴ� ��� �� ������ ���� ���ʹ� �� ������ ���Ե� �ﰢ������ ���� ������ �������(���Ͽ�) ����Ѵ�.*/
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST:
		if (m_pnIndices)
			SetAverageVertexNormal(pxv3Normals, (m_nIndices / 3), 3, false);
		else
			SetTriAngleListVertexNormal(pxv3Normals);
		break;
		/*������Ƽ�갡 �ﰢ�� ��Ʈ���� �� �� ������ ���� ���ʹ� �� ������ ���Ե� �ﰢ������ ���� ������ �������(���Ͽ�) ����Ѵ�.*/
	case D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP:
		SetAverageVertexNormal(pxv3Normals, (m_nIndices) ? (m_nIndices - 2) : (m_nVertices - 2), 1, true);
		break;
	default:
		break;
	}
}
void CMeshIlluminated::SetTriAngleListVertexNormal(XMFLOAT3 *pxv3Normals)
{
	/*�ﰢ��(������Ƽ��)�� ������ ���ϰ� �� �ﰢ���� ���� ���͸� ����ϰ� �ﰢ���� �����ϴ� �� ������ ���� ���ͷ� �����Ѵ�.*/
	int nPrimitives = m_nVertices / 3;
	for (int i = 0; i < nPrimitives; i++)
	{
		XMFLOAT3 xv3Normal = CalculateTriAngleNormal((i * 3 + 0), (i * 3 + 1), (i * 3 + 2));
		pxv3Normals[i * 3 + 0] = pxv3Normals[i * 3 + 1] = pxv3Normals[i * 3 + 2] = xv3Normal;
	}
}

XMFLOAT3 CMeshIlluminated::CalculateTriAngleNormal(UINT nIndex0, UINT nIndex1, UINT nIndex2)
{
	XMVECTOR xv3Normal;
	XMVECTOR xv3P0 = XMLoadFloat3(&m_pxv3Positions[nIndex0]);
	XMVECTOR xv3P1 = XMLoadFloat3(&m_pxv3Positions[nIndex1]);
	XMVECTOR xv3P2 = XMLoadFloat3(&m_pxv3Positions[nIndex2]);
	XMVECTOR xv3Edge1 = xv3P1 - xv3P0;
	XMVECTOR xv3Edge2 = xv3P2 - xv3P0;

	xv3Normal = XMVector3Cross(xv3Edge1, xv3Edge2);
	xv3Normal = XMVector3Normalize(xv3Normal);

	XMFLOAT3 xmf3Normal;
	XMStoreFloat3(&xmf3Normal, xv3Normal);
	return(xmf3Normal);
}

void CMeshIlluminated::SetAverageVertexNormal(XMFLOAT3 *pxv3Normals, int nPrimitives, int nOffset, bool bStrip)
{
	for (unsigned int j = 0; j < m_nVertices; j++)
	{
		XMVECTOR xvSumOfNormal = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		for (int i = 0; i < nPrimitives; i++)
		{
			UINT nIndex0 = (bStrip) ? (((i % 2) == 0) ? (i*nOffset + 0) : (i*nOffset + 1)) : (i*nOffset + 0);
			if (m_pnIndices) nIndex0 = m_pnIndices[nIndex0];
			UINT nIndex1 = (bStrip) ? (((i % 2) == 0) ? (i*nOffset + 1) : (i*nOffset + 0)) : (i*nOffset + 1);
			if (m_pnIndices) nIndex1 = m_pnIndices[nIndex1];
			UINT nIndex2 = (m_pnIndices) ? m_pnIndices[i*nOffset + 2] : (i*nOffset + 2);
			if ((nIndex0 == j) || (nIndex1 == j) || (nIndex2 == j))
			{
				xvSumOfNormal += XMLoadFloat3(&CalculateTriAngleNormal(nIndex0, nIndex1, nIndex2));
			}
		}
		xvSumOfNormal = XMVector3Normalize(xvSumOfNormal);
		XMStoreFloat3(&pxv3Normals[j], xvSumOfNormal);
		//		pxv3Normals[j] = xv3SumOfNormal;
	}
}

CCubeMeshTextured::CCubeMeshTextured(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshTextured(pd3dDevice)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 pxv2TexCoords[36];
	int i = 0;

	//������ü�� �� ��(�ﰢ�� 2��)�� �ϳ��� �ؽ��� �̹��� ��ü�� ���εǵ��� �ؽ��� ��ǥ�� �����Ѵ�.
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	/*������ü�� �� �鿡 �ؽ��ĸ� �����Ϸ��� �ε����� ����� �� �����Ƿ� �ε��� ���۴� �������� �ʴ´�.*/

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshTextured::~CCubeMeshTextured()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
CSphereMeshTextured::CSphereMeshTextured(ID3D11Device *pd3dDevice, float fRadius, int nSlices, int nStacks) : CMeshTextured(pd3dDevice)
{
	m_nVertices = (nSlices * nStacks) * 3 * 2;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *xmf2TexCoords = new XMFLOAT2[m_nVertices];

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;
	for (int j = 0, k = 0; j < nStacks; j++)
	{
		phi_j = float(PI / nStacks) * j;
		phi_jj = float(PI / nStacks) * (j + 1);
		fRadius_j = fRadius * sinf(phi_j);
		fRadius_jj = fRadius * sinf(phi_jj);
		y_j = fRadius * cosf(phi_j);
		y_jj = fRadius * cosf(phi_jj);
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = float(2 * PI / nSlices) * i;
			theta_ii = float(2 * PI / nSlices) * (i + 1);
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_i), y_j, fRadius_j*sinf(theta_i));
			xmf2TexCoords[k++] = XMFLOAT2(float(i) / float(nSlices), float(j) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k++] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k++] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k++] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_ii), y_jj, fRadius_jj*sinf(theta_ii));
			xmf2TexCoords[k++] = XMFLOAT2(float(i + 1) / float(nSlices), float(j + 1) / float(nStacks));
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k++] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = xmf2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	delete[] xmf2TexCoords;

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fRadius, -fRadius, -fRadius);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fRadius, +fRadius, +fRadius);
}

CSphereMeshTextured::~CSphereMeshTextured()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

CCubeMeshTexturedIlluminated::CCubeMeshTexturedIlluminated(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fDepth) : CMeshTexturedIlluminated(pd3dDevice)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	XMFLOAT2 pxv2TexCoords[36];
	int i = 0;

	//������ü�� �� ��(�ﰢ�� 2��)�� �ϳ��� �ؽ��� �̹��� ��ü�� ���εǵ��� �ؽ��� ��ǥ�� �����Ѵ�.
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	//�ո�

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	// �Ʒ���
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	// ���� ��
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(-fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	m_pxv3Positions[i] = XMFLOAT3(+fx, +fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, +fz);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fy, -fz);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	//���� ���͸� �����ϱ� ���� ���� �ڵ带 �߰��Ѵ�.
	XMFLOAT3 pxv3Normals[36];
	CalculateVertexNormal(pxv3Normals);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	//������ ��ġ ����, ���� ����, �ؽ��� ��ǥ�� ���´�.
	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -fz);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +fz);
}

CCubeMeshTexturedIlluminated::~CCubeMeshTexturedIlluminated()
{
}

CSphereMeshTexturedIlluminated::CSphereMeshTexturedIlluminated(ID3D11Device *pd3dDevice, float fRadius, int nSlices, int nStacks) : CMeshTexturedIlluminated(pd3dDevice)
{
	m_nVertices = (nSlices * nStacks) * 3 * 2;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT3 *pxv3Normals = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *xmf2TexCoords = new XMFLOAT2[m_nVertices];

	float theta_i, theta_ii, phi_j, phi_jj, fRadius_j, fRadius_jj, y_j, y_jj;
	for (int j = 0, k = 0; j < nStacks; j++)
	{
		phi_j = float(PI / nStacks) * j;
		phi_jj = float(PI / nStacks) * (j + 1);
		fRadius_j = fRadius * sinf(phi_j);
		fRadius_jj = fRadius * sinf(phi_jj);
		y_j = fRadius*cosf(phi_j);
		y_jj = fRadius*cosf(phi_jj);
		for (int i = 0; i < nSlices; i++)
		{
			theta_i = float(2 * PI / nSlices) * i;
			theta_ii = float(2 * PI / nSlices) * (i + 1);
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_i), y_j, fRadius_j*sinf(theta_i));
			xmf2TexCoords[k] = XMFLOAT2(float(i) / float(nSlices), float(j) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_i), y_jj, fRadius_jj*sinf(theta_i));
			xmf2TexCoords[k] = XMFLOAT2(float(i) / float(nSlices), float(j + 1) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_jj*cosf(theta_ii), y_jj, fRadius_jj*sinf(theta_ii));
			xmf2TexCoords[k] = XMFLOAT2(float(i + 1) / float(nSlices), float(j + 1) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
			m_pxv3Positions[k] = XMFLOAT3(fRadius_j*cosf(theta_ii), y_j, fRadius_j*sinf(theta_ii));
			xmf2TexCoords[k] = XMFLOAT2(float(i + 1) / float(nSlices), float(j) / float(nStacks));
			Chae::XMFloat3Normalize(&pxv3Normals[k], &m_pxv3Positions[k]); k++;
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = xmf2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	delete[] pxv3Normals;
	delete[] xmf2TexCoords;

	//������ ��ġ ����, ���� ����, �ؽ��� ��ǥ�� ���´�.
	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fRadius, -fRadius, -fRadius);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fRadius, +fRadius, +fRadius);
}

CSphereMeshTexturedIlluminated::~CSphereMeshTexturedIlluminated()
{
}

CPlaneMesh::CPlaneMesh(ID3D11Device * pd3dDevice, int fx, int fy) : CMeshTextured(pd3dDevice)
{
	m_nVertices = 4;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	XMFLOAT2 pxv2TexCoords[4];
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	int i = 0;
	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[2] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	//m_pxv3Positions[3] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[3] = XMFLOAT3(+fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	//m_pxv3Positions[5] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer,  m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, 0);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, 0);
}

CPlaneMesh::~CPlaneMesh()
{
}

CDoublePlaneMesh::CDoublePlaneMesh(ID3D11Device * pd3dDevice, int fx, int fy) : CMeshTextured(pd3dDevice)
{
	m_nVertices = 4;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	XMFLOAT2 pxv2TexCoords[4];
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	int i = 0;
	m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[2] = XMFLOAT3(-fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	m_pxv3Positions[3] = XMFLOAT3(+fx, -fy, 0); pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer,  m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, 0);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, 0);


	// �ε��� ���� ����
	m_nIndices = 12;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 0; m_pnIndices[1] = 2; m_pnIndices[2] = 1; 
	m_pnIndices[3] = 2; m_pnIndices[4] = 3; m_pnIndices[5] = 1;
	m_pnIndices[6] = 0; m_pnIndices[7] = 1; m_pnIndices[8] = 2;
	m_pnIndices[9] = 2; m_pnIndices[10] = 1; m_pnIndices[11] = 3;

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_nStartIndex = 0;

	CreateRasterizerState(pd3dDevice);
}

CDoublePlaneMesh::~CDoublePlaneMesh()
{
}

CLoadMeshByChae::CLoadMeshByChae(ID3D11Device * pd3dDevice, char * tMeshName, float fScale) : CMeshTexturedIlluminated(pd3dDevice)
{
	int nReadBytes;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//D3D11_PRIMITIVE_TOPOLOGY_

	FILE * pFile = NULL;
	::fopen_s(&pFile,tMeshName, "rb");

	m_nBuffers = 0;

	//���� ����
	nReadBytes = ::fread(&m_nVertices, sizeof(UINT), 1, pFile);
	//int a;
	//::fread(&a, 4, 1, pFile);
	NormalMapVertex * pChaeInfo = new NormalMapVertex[m_nVertices];
	m_pxv3Positions = new XMFLOAT3[m_nVertices];


	nReadBytes = ::fread(&pChaeInfo[0], sizeof(NormalMapVertex), m_nVertices, pFile);
	
	nReadBytes = ::fread(&m_nIndices, sizeof(UINT), 1, pFile);
	m_pnIndices = new UINT[m_nIndices];
	nReadBytes = ::fread(&m_pnIndices[0], sizeof(UINT), m_nIndices, pFile);
	::fclose(pFile);

	XMFLOAT3 min{ FLT_MAX, FLT_MAX , FLT_MAX }, max{ FLT_MIN, FLT_MIN, FLT_MIN };

	XMVECTOR xmvScale = XMVectorSet(fScale, fScale, fScale, 1);
	XMMATRIX xmtxRotate = XMMatrixIdentity();// XMMatrixRotationX(90);

	for (unsigned int i = 0; i < m_nVertices; ++i)
	{
		XMVECTOR xmvPos = XMVectorSet(pChaeInfo[i].xmf3Pos.x, pChaeInfo[i].xmf3Pos.y, pChaeInfo[i].xmf3Pos.z, 1);
		xmvPos *= xmvScale;
		xmvPos = XMVector3TransformCoord(xmvPos, xmtxRotate);

		XMStoreFloat3(&m_pxv3Positions[i], xmvPos);
		XMStoreFloat3(&pChaeInfo[i].xmf3Pos, xmvPos);

		if (min.x > m_pxv3Positions[i].x) min.x = m_pxv3Positions[i].x;
		if (min.y > m_pxv3Positions[i].y) min.y = m_pxv3Positions[i].y;
		if (min.z > m_pxv3Positions[i].z) min.z = m_pxv3Positions[i].z;

		if (max.x < m_pxv3Positions[i].x) max.x = m_pxv3Positions[i].x;
		if (max.y < m_pxv3Positions[i].y) max.y = m_pxv3Positions[i].y;
		if (max.z < m_pxv3Positions[i].z) max.z = m_pxv3Positions[i].z;

//		pChaeInfo[i].xmf3Normal = pChaeInfo[i].xmf3Normal;
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(NormalMapVertex) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;


	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pChaeInfo;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer};
	UINT pnBufferStrides[1] = { sizeof(NormalMapVertex)};
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	delete [] pChaeInfo;

	m_bcBoundingCube.m_xv3Minimum = min;
	m_bcBoundingCube.m_xv3Maximum = max;


	// �ε��� ���� ����
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_nStartIndex = 0;

	CreateRasterizerState(pd3dDevice);
}

CLoadMeshByChae::~CLoadMeshByChae()
{
}

CLoadAnimatedMeshByADFile::CLoadAnimatedMeshByADFile(ID3D11Device * pd3dDevice, const char * fileName, float fScale, 
	vector<wstring> & vcFileName, int iStartIndex, int iCutLastIndex) : CAnimatedMesh(pd3dDevice)
{
	int nReadBytes;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//D3D11_PRIMITIVE_TOPOLOGY_

	FILE * pFile = NULL;
	::fopen_s(&pFile, fileName, "rb");

	m_nBuffers = 0;

	//�ؽ��� ����
	__int32 nTextures = 0;
	nReadBytes = ::fread(&nTextures, sizeof(__int32), 1, pFile);

	wstring * wstringArray = new wstring[nTextures];
	wchar_t temp[256];
	wstring file;
	for (int i = 0; i < nTextures; ++i)
	{
		__int32 fileNameSize = 0;
		nReadBytes = ::fread(&fileNameSize, sizeof(__int32), 1, pFile);
		nReadBytes = ::fread(temp, sizeof(wchar_t), fileNameSize, pFile);
		temp[fileNameSize] = '\0';
		file = temp;
		//wcout << temp << endl;

		vcFileName.push_back(file);
	}
	m_nVertices = 0;


	//���� ����
	__int32 sz = 0;
	__int32 nType = 0;

	MeshBuffer MeshInfo;
	
	vector<NormalMapVertex> pVertexInfo;
	NormalMapVertex ZeroVertex;
	ZeroMemory(&ZeroVertex, sizeof(ZeroVertex));

	for (UINT nIndex = 0;;++nIndex)
	{
		nReadBytes = ::fread(&nType, sizeof(__int32), 1, pFile);
		if (nReadBytes == 0) break;
		nReadBytes = ::fread(&sz, sizeof(__int32), 1, pFile);

		if (pVertexInfo.size() < sz) pVertexInfo.resize(sz, ZeroVertex);

		if (nType == 0)
		{
			nReadBytes = ::fread(&pVertexInfo[0], sizeof(NormalMapVertex), sz, pFile);
			if (nIndex < iStartIndex) // ���� �ε����� �ƴϸ� �������� �Ѿ��.
				continue;
			MeshInfo = CMesh::GetNormalMapVertexBuffer(pd3dDevice, &pVertexInfo[0], sz, fScale);
		}
		else if (nType == 1)
		{
			V3T2N3 temp;
			for (int index = 0; index < sz; ++index)
			{
				nReadBytes = ::fread(&pVertexInfo[index], sizeof(V3T2N3), 1, pFile);
				//pVertexInfo[index] = temp;
			}
			if (nIndex < iStartIndex) // ���� �ε����� �ƴϸ� �������� �Ѿ��.
				continue;
			MeshInfo = CMesh::GetNormalMapVertexBufferAndCalculateTangent(pd3dDevice, &pVertexInfo[0], sz, fScale);
		}
		else
			break;

		if (nIndex == iStartIndex)
		{
			m_bcBoundingCube = MeshInfo.bb;
			cout << "Max : " << m_bcBoundingCube.m_xv3Maximum;
			cout << "Min : " << m_bcBoundingCube.m_xv3Minimum;
			cout << endl;

			m_pnVertexOffsets = new UINT[1];
			m_pnVertexOffsets[0] = MeshInfo.nOffsets;

			m_pnVertexStrides = new UINT[1];
			m_pnVertexStrides[0] = MeshInfo.nStrides;

			m_nBuffers = 1;
			m_nVertices = sz;
		}
		//pMesh->AddRef();
		
		m_pvcMeshBuffers.push_back(MeshInfo);
	}
	::fclose(pFile);
	pVertexInfo.clear();

	int iLastIndex = m_pvcMeshBuffers.size() - 1;
	for (int i = 0; i < iCutLastIndex; ++i, --iLastIndex)
	{
		m_pvcMeshBuffers[iLastIndex].pd3dBuffer->Release();
		m_pvcMeshBuffers.pop_back();
	}

	CreateRasterizerState(pd3dDevice);
}

CLoadAnimatedMeshByADFile::~CLoadAnimatedMeshByADFile()
{
}

void CLoadAnimatedMeshByADFile::CreateRasterizerState(ID3D11Device * pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;//WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = false;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

CLoadMeshByFbxcjh::CLoadMeshByFbxcjh(ID3D11Device * pd3dDevice, char * tMeshName, float fScale, vector<wstring> & vcFileName) : CMeshTexturedIlluminated(pd3dDevice)
{
	int nReadBytes;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	//D3D11_PRIMITIVE_TOPOLOGY_

	FILE * pFile = NULL;
	::fopen_s(&pFile, tMeshName, "rb");

	m_nBuffers = 0;

#ifdef _FBXCJH_USE_TX_READ
	//�ؽ��� ����
	UINT nTextures = 0;
	nReadBytes = ::fread(&nTextures, sizeof(UINT), 1, pFile);

	//wstring * wstringArray = new wstring[nTextures];
	wchar_t temp[256];
	wstring fileName;
	for (int i = 0; i < nTextures; ++i)
	{
		UINT fileNameSize = 0;
		nReadBytes = ::fread(&fileNameSize, sizeof(UINT), 1, pFile);
		nReadBytes = ::fread(temp, sizeof(wchar_t), fileNameSize, pFile);
		temp[fileNameSize] = '\0';
		fileName = temp;
		//wcout << temp << endl;

		vcFileName.push_back(fileName);
	}
#endif
	//���� ����
	UINT nMeshes = 0;
	nReadBytes = ::fread(&nMeshes, sizeof(UINT), 1, pFile);
	m_nVertices = 0;
	for (int i = 0; i < nMeshes; ++i)
	{
		UINT sz = 0;
		nReadBytes = ::fread(&sz, sizeof(UINT), 1, pFile);
		m_nVertices += sz;
	}

	NormalMapVertex * pChaeInfo = new NormalMapVertex[m_nVertices];
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	for (int i = 0; i < nMeshes; ++i)
	{
		nReadBytes = ::fread(&pChaeInfo[0], sizeof(NormalMapVertex), m_nVertices, pFile);
	}


	//nReadBytes = ::fread(&m_nIndices, sizeof(UINT), 1, pFile);
	//m_pnIndices = new UINT[m_nIndices];
	//nReadBytes = ::fread(&m_pnIndices[0], sizeof(UINT), m_nIndices, pFile);
	::fclose(pFile);

	XMFLOAT3 min{ FLT_MAX, FLT_MAX, FLT_MAX }, max{ FLT_MIN, FLT_MIN, FLT_MIN };


	XMVECTOR xmvScale = XMVectorSet(fScale, fScale, fScale, 1);
	XMMATRIX xmtxRotate = XMMatrixIdentity();// XMMatrixRotationX(90);

	for (unsigned int i = 0; i < m_nVertices; ++i)
	{
		XMVECTOR xmvPos = XMVectorSet(pChaeInfo[i].xmf3Pos.x, pChaeInfo[i].xmf3Pos.y, pChaeInfo[i].xmf3Pos.z, 1);
		xmvPos *= xmvScale;
		xmvPos = XMVector3TransformCoord(xmvPos, xmtxRotate);

		XMStoreFloat3(&m_pxv3Positions[i], xmvPos);
		XMStoreFloat3(&pChaeInfo[i].xmf3Pos, xmvPos);

		if (min.x > m_pxv3Positions[i].x) min.x = m_pxv3Positions[i].x;
		if (min.y > m_pxv3Positions[i].y) min.y = m_pxv3Positions[i].y;
		if (min.z > m_pxv3Positions[i].z) min.z = m_pxv3Positions[i].z;

		if (max.x < m_pxv3Positions[i].x) max.x = m_pxv3Positions[i].x;
		if (max.y < m_pxv3Positions[i].y) max.y = m_pxv3Positions[i].y;
		if (max.z < m_pxv3Positions[i].z) max.z = m_pxv3Positions[i].z;
		//		pChaeInfo[i].xmf3Normal = pChaeInfo[i].xmf3Normal;
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(NormalMapVertex) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pChaeInfo;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(NormalMapVertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	delete[] pChaeInfo;

	//Chae::XMFloat3Mul(&min, &min, fScale);
	//Chae::XMFloat3Mul(&max, &max, fScale);
	m_bcBoundingCube.m_xv3Minimum = min;
	m_bcBoundingCube.m_xv3Maximum = max;

	//CreateRasterizerState(pd3dDevice);
}

CLoadMeshByFbxcjh::~CLoadMeshByFbxcjh()
{
}

void CLoadMeshByFbxcjh::CreateRasterizerState(ID3D11Device * pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;//WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = false;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CLoadMeshByChae::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;//WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = false;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

CLoadMesh::CLoadMesh(ID3D11Device *pd3dDevice, wchar_t * tMeshName) : CMeshTexturedIlluminated(pd3dDevice)
{
	int nReadBytes;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	FILE * pFile = NULL;
	::_wfopen_s(&pFile, tMeshName, L"rb, ccs=UNICODE");

	//���� ����
	nReadBytes = ::fread(&m_nVertices, sizeof(UINT), 1, pFile);

	//��ġ ����
	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	nReadBytes = ::fread(m_pxv3Positions, sizeof(XMFLOAT3), m_nVertices, pFile);

	//�������� ����
	m_xmf3Normal = new XMFLOAT3[m_nVertices];
	nReadBytes = ::fread(m_xmf3Normal, sizeof(XMFLOAT3), m_nVertices, pFile);

	//�ؽ�ó ��ǥ �Է�
	m_xmf2TexCoords = new XMFLOAT2[m_nVertices];
	nReadBytes = ::fread(m_xmf2TexCoords, sizeof(XMFLOAT2), m_nVertices, pFile);

	// �ε��� ����
	nReadBytes = ::fread(&m_nIndices, sizeof(UINT), 1, pFile);

	// �ε��� ����
	m_pnIndices = new UINT[m_nIndices];
	nReadBytes = ::fread(m_pnIndices, sizeof(UINT), m_nIndices, pFile);
	::fclose(pFile);
}

CLoadMesh::~CLoadMesh()
{
}

CLoadMeshCommon::CLoadMeshCommon(ID3D11Device *pd3dDevice, wchar_t * tMeshName, float xScale, float yScale, float zScale) : CLoadMesh(pd3dDevice, tMeshName)
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;


	float min[3], max[3], temp[3];
	min[0] = max[0] = m_pxv3Positions[0].x *= xScale;
	min[1] = max[1] = m_pxv3Positions[0].y *= yScale;
	min[2] = max[2] = m_pxv3Positions[0].z *= zScale;

	for (unsigned int i = 1; i < m_nVertices; ++i) 
	{
		temp[0] = m_pxv3Positions[i].x *= xScale;
		temp[1] = m_pxv3Positions[i].y *= yScale;
		temp[2] = m_pxv3Positions[i].z *= zScale;

		for (int j = 0; j < 3; ++j) {
			min[j] > temp[j] ? min[j] = temp[j] : (max[j] < temp[j] ? max[j] = temp[j] : void(0));
		}
	}

	//�������� ����
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	//�������� ���� ����
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = m_xmf3Normal;
	//CalculateVertexNormal(pdxvNormal); // ���
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);

	// �ؽ��� ��ǥ ����
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = m_xmf2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	//������ ��ġ ����, ���� ����, �ؽ��� ��ǥ�� ���´�.
	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	if (m_pd3dTexCoordBuffer) delete[] m_pd3dTexCoordBuffer;
	if (m_pd3dNormalBuffer) delete[] m_pd3dNormalBuffer;


	// �ε��� ���� ����
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(min[0], min[1], min[2]);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(max[0], max[1], max[2]);
}

CLoadMeshCommon::~CLoadMeshCommon()
{
}
