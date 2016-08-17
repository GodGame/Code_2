#include "stdafx.h"
#include "MyInline.h"
#include "Mesh.h"
#include "Object.h"

/////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////
CHeightMapGridMesh::CHeightMapGridMesh(ID3D11Device *pd3dDevice, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale, void *pContext) 
#ifdef DETAIL_TERRAIN
: CMeshDetailTexturedIlluminated(pd3dDevice)
#else
#ifdef SPLAT_TERRAIN
: CMeshSplatTexturedIlluminated(pd3dDevice)
#endif
#endif
{
	m_nVertices = nWidth * nLength;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT3 *pxv3Normals = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *pxv2TexCoords = new XMFLOAT2[m_nVertices];
	XMFLOAT2 *pxv2AlphaTexCoords = new XMFLOAT2[m_nVertices];

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xv3Scale = xv3Scale;

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	int cxHeightMap = pHeightMap->GetHeightMapWidth();
	int czHeightMap = pHeightMap->GetHeightMapLength();
	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	/*xStart�� zStart�� ������ ���� ��ġ(x-��ǥ�� z-��ǥ)�� ��Ÿ����. ������ ���ڵ��� ������ �迭�� ���� ���̱� ������ �������� �� ������ ���� ��ġ�� ��Ÿ���� ������ �ʿ��ϴ�. <�׸� 18>�� ������ ����(����)�� �����ϴ� ������ �����ش�.*/
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			//������ ���̿� ������ ���� �����κ��� ���Ѵ�.
			fHeight = OnGetHeight(x, z, pContext) ;
			m_pxv3Positions[i] = XMFLOAT3((x*m_xv3Scale.x), fHeight, (z*m_xv3Scale.z));
			pxv3Normals[i] = pHeightMap->GetHeightMapNormal(x, z);
			pxv2TexCoords[i] = XMFLOAT2(float(x) / float(m_xv3Scale.x*0.25f), float(z) / float(m_xv3Scale.z*0.25f));
			pxv2AlphaTexCoords[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
#ifdef DETAIL_TERRAIN
			pxv3DetailTexCoords[i] = XMFLOAT2(float(x) / float(m_xv3Scale.x*0.5f), float(z) / float(m_xv3Scale.z*0.5f));
#endif
			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
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
	//��������
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferData.pSysMem = pxv3Normals;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dNormalBuffer);
	//�ؽ���
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);
#ifdef DETAIL_TERRAIN
	//�������ؽ���
	d3dBufferData.pSysMem = pxv3DetailTexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dDetailTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[4] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dDetailTexCoordBuffer };
	UINT pnBufferStrides[4] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[4] = { 0, 0, 0, 0 };
	AssembleToVertexBuffer(4, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxv3Normals) delete[] pxv3Normals;
	if (pxv2TexCoords) delete[] pxv2TexCoords;
	if (pxv3DetailTexCoords) delete[] pxv3DetailTexCoords;
#else
#ifdef SPLAT_TERRAIN
	d3dBufferData.pSysMem = pxv2AlphaTexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dAlphaTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[4] = { m_pd3dPositionBuffer, m_pd3dNormalBuffer, m_pd3dTexCoordBuffer, m_pd3dAlphaTexCoordBuffer };
	UINT pnBufferStrides[4] = { sizeof(XMFLOAT3), sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[4] = { 0, 0, 0, 0 };
	AssembleToVertexBuffer(4, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxv3Normals) delete[] pxv3Normals;
	if (pxv2TexCoords) delete[] pxv2TexCoords;
	if (pxv2AlphaTexCoords) delete[] pxv2AlphaTexCoords;
	if (m_pxv3Positions) delete[] m_pxv3Positions;
	m_pxv3Positions = nullptr;
	//if (pxv3DetailTexCoords) delete[] pxv3DetailTexCoords;
	// ���ε� ���� ����
#endif
#endif

	m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_pnIndices = new UINT[m_nIndices];
	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			//Ȧ�� ��° ���̹Ƿ�(z = 0, 2, 4, ...) �ε����� ���� ������ ���ʿ��� ������ �����̴�.
			for (int x = 0; x < nWidth; x++)
			{
				//ù ��° ���� �����ϰ� ���� �ٲ� ������(x == 0) ù ��° �ε����� �߰��Ѵ�.
				if ((x == 0) && (z > 0)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				//�Ʒ�, ���� ������ �ε����� �߰��Ѵ�.
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			//¦�� ��° ���̹Ƿ�(z = 1, 3, 5, ...) �ε����� ���� ������ �����ʿ��� ���� �����̴�.
			for (int x = nWidth - 1; x >= 0; x--)
			{
				//���� �ٲ� ������(x == (nWidth-1)) ù ��° �ε����� �߰��Ѵ�.
				if (x == (nWidth - 1)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				//�Ʒ�, ���� ������ �ε����� �߰��Ѵ�.
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(xStart*m_xv3Scale.x, fMinHeight, zStart*m_xv3Scale.z);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3((xStart + nWidth)*m_xv3Scale.x, fMaxHeight, (zStart + nLength)*m_xv3Scale.z);
	//CreateRasterizerState(pd3dDevice);
}

CHeightMapGridMesh::~CHeightMapGridMesh()
{
}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void *pContext)
{
	//���� �� ��ü�� ���� �� �̹����� �ȼ� ���� ������ ���̷� ��ȯ�Ѵ�. 
	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	BYTE *pHeightMapImage = pHeightMap->GetHeightMapImage();
	//printf("HEIGHT : %d \t", *pHeightMapImage);
	XMFLOAT3 xv3Scale = pHeightMap->GetScale();
	int cxTerrain = pHeightMap->GetHeightMapWidth();
	float fHeight = pHeightMapImage[x + (z*cxTerrain)] * xv3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void *pContext)
{
	//������ ���� ����(�������� ��������� ����)�̴�.
	//XMVECTOR xv3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	XMVECTOR xv3LightDirection = XMVectorSet(-1.0f, 1.0f, 1.0f, 0.0f);
	xv3LightDirection = XMVector3Normalize(xv3LightDirection);

	XMFLOAT3 xmf3LightDirecton;
	XMStoreFloat3(&xmf3LightDirecton, xv3LightDirection);

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	XMFLOAT3 xv3Scale = pHeightMap->GetScale();

	/*���� (x, z)���� ������ �ݻ�Ǵ� ���� ���� (x, z)�� ���� ���Ϳ� ������ ���� ������ ����(cos)�� ������ 3���� �� (x+1, z), (x, z+1), (x+1, z+1)�� ���� ���Ϳ� ������ ���� ������ ������ ����Ͽ� ���Ѵ�. ���� (x, z)�� ������ ���� ����(����)�� �ݻ�Ǵ� ���� ���� ���̴�.*/

	float fScale = Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x, z), &xmf3LightDirecton);
	fScale += Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x + 1, z), &xmf3LightDirecton);
	fScale += Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x + 1, z + 1), &xmf3LightDirecton);
	fScale += Chae::XMFloat3Dot(&pHeightMap->GetHeightMapNormal(x, z + 1), &xmf3LightDirecton);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;

	//������ ����(����)�̴�. 
	XMVECTOR vIncidentLight = XMVectorSet(0.9f, 0.8f, 0.4f, 1.0f);

	XMFLOAT4 xmf4Light;
	XMStoreFloat4(&xmf4Light, fScale * vIncidentLight);
	return(xmf4Light);
}

void CHeightMapGridMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}
/////////////////////////////////////////////////////////////////////////
CWaterGridMesh::CWaterGridMesh(ID3D11Device * pd3dDevice, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale, void * pContext) : CMeshSplatTexturedIlluminated(pd3dDevice)
{
	m_nVertices = nWidth * nLength;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *pxv2TexCoords = new XMFLOAT2[m_nVertices];
	XMFLOAT2 *pxv2AlphaTexCoords = new XMFLOAT2[m_nVertices];

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	int cxHeightMap = pHeightMap->GetHeightMapWidth();
	int czHeightMap = pHeightMap->GetHeightMapLength();
	float fHeight = 0.0f;
	/*xStart�� zStart�� ������ ���� ��ġ(x-��ǥ�� z-��ǥ)�� ��Ÿ����. ������ ���ڵ��� ������ �迭�� ���� ���̱� ������ �������� �� ������ ���� ��ġ�� ��Ÿ���� ������ �ʿ��ϴ�. <�׸� 18>�� ������ ����(����)�� �����ϴ� ������ �����ش�.*/
	for (int i = 0, z = zStart; z < (zStart + nLength); z++)
	{
		for (int x = xStart; x < (xStart + nWidth); x++, i++)
		{
			m_pxv3Positions[i] = XMFLOAT3((x*xv3Scale.x), fHeight, (z*xv3Scale.z));
			pxv2TexCoords[i] = XMFLOAT2(float(x) / float(xv3Scale.x * 0.5f), float(z) / float(xv3Scale.z * 0.5f));
			pxv2AlphaTexCoords[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));

			//	pxv2AlphaTexCoords[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			//pxv2TexCoords[i] = XMFLOAT2(float(x) / float(xv3Scale.x * 0.25f), float(z) / float(xv3Scale.z * 0.25f));
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

	//�ؽ���
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	d3dBufferData.pSysMem = pxv2AlphaTexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dAlphaTexCoordBuffer);

	ID3D11Buffer *pd3dBuffers[3] = { m_pd3dPositionBuffer, m_pd3dTexCoordBuffer, m_pd3dAlphaTexCoordBuffer };
	UINT pnBufferStrides[3] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[3] = { 0, 0, 0 };
	AssembleToVertexBuffer(3, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxv2TexCoords) delete[] pxv2TexCoords;
	if (pxv2AlphaTexCoords) delete[] pxv2AlphaTexCoords;
	if (m_pxv3Positions) delete[] m_pxv3Positions;
	m_pxv3Positions = nullptr;

	m_nIndices = ((nWidth * 2)*(nLength - 1)) + ((nLength - 1) - 1);
	m_pnIndices = new UINT[m_nIndices];
	for (int j = 0, z = 0; z < nLength - 1; z++)
	{
		if ((z % 2) == 0)
		{
			//Ȧ�� ��° ���̹Ƿ�(z = 0, 2, 4, ...) �ε����� ���� ������ ���ʿ��� ������ �����̴�.
			for (int x = 0; x < nWidth; x++)
			{
				//ù ��° ���� �����ϰ� ���� �ٲ� ������(x == 0) ù ��° �ε����� �߰��Ѵ�.
				if ((x == 0) && (z > 0)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				//�Ʒ�, ���� ������ �ε����� �߰��Ѵ�.
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
		else
		{
			//¦�� ��° ���̹Ƿ�(z = 1, 3, 5, ...) �ε����� ���� ������ �����ʿ��� ���� �����̴�.
			for (int x = nWidth - 1; x >= 0; x--)
			{
				//���� �ٲ� ������(x == (nWidth-1)) ù ��° �ε����� �߰��Ѵ�.
				if (x == (nWidth - 1)) m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				//�Ʒ�, ���� ������ �ε����� �߰��Ѵ�.
				m_pnIndices[j++] = (UINT)(x + (z * nWidth));
				m_pnIndices[j++] = (UINT)((x + (z * nWidth)) + nWidth);
			}
		}
	}

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(xStart*xv3Scale.x, fHeight, zStart*xv3Scale.z);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3((xStart + nWidth)*xv3Scale.x, fHeight, (zStart + nLength)*xv3Scale.z);
}

CWaterGridMesh::~CWaterGridMesh()
{
}

void CWaterGridMesh::Render(ID3D11DeviceContext * pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}

/////////////////////////////////////////////////////////////////////////
CTerrainPartMesh::CTerrainPartMesh(ID3D11Device *pd3dDevice, float xStart, float zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale, void *pContext) : CMesh(pd3dDevice)
{
	m_nVertices = 4;//nWidth * nLength;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	CTexture2Vertex * pxvInfo = new CTexture2Vertex[m_nVertices];

	m_xv3Scale = xv3Scale;

	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	int cxHeightMap = pHeightMap->GetHeightMapWidth();
	int czHeightMap = pHeightMap->GetHeightMapLength();
	float fHeight = 0.0f, fMinHeight = 0, fMaxHeight = 256 * xv3Scale.y;

	int index = 0;
	float xSize = xv3Scale.x, zSize = xv3Scale.z;
	float xPos, zPos;
	
	for (float z = 0.0; z < 2.0; ++z){
		for (float x = 0.0; x < 2.0; ++x){
			xPos = nWidth * (xStart + x), zPos = nLength * (zStart + z);
			m_pxv3Positions[index] = XMFLOAT3(xPos, 100, zPos);
			pxvInfo[index++] = { m_pxv3Positions[index], XMFLOAT2((xStart + x) / xSize, (zSize - z - zStart) / zSize)};
			printf("%f %f \n", xStart / xSize, (zSize - zStart) / zSize);
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(CTexture2Vertex)* m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pxvInfo; //m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	if (pxvInfo) delete[] pxvInfo;

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(CTexture2Vertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(m_pxv3Positions[0].x, 0, m_pxv3Positions[0].z);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(m_pxv3Positions[3].x, fMaxHeight, m_pxv3Positions[3].z);
	CreateRasterizerState(pd3dDevice);
}

CTerrainPartMesh::~CTerrainPartMesh()
{
}

float CTerrainPartMesh::OnGetHeight(int x, int z, void *pContext)
{
	//���� �� ��ü�� ���� �� �̹����� �ȼ� ���� ������ ���̷� ��ȯ�Ѵ�. 
	CHeightMap *pHeightMap = (CHeightMap *)pContext;
	BYTE *pHeightMapImage = pHeightMap->GetHeightMapImage();
	//printf("HEIGHT : %d \t", *pHeightMapImage);
	XMFLOAT3 xv3Scale = pHeightMap->GetScale();
	int cxTerrain = pHeightMap->GetHeightMapWidth();
	float fHeight = pHeightMapImage[x + (z*cxTerrain)] * xv3Scale.y;
	return(fHeight);
}

void CTerrainPartMesh::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;//WIREFRAME;
	d3dRasterizerDesc.DepthClipEnable = false;
	pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CTerrainPartMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}

CSkyBoxMesh::CSkyBoxMesh(ID3D11Device *pd3dDevice, UINT uImageNum, float fWidth, float fHeight, float fDepth) : CMeshTextured(pd3dDevice)
{
	//��ī�� �ڽ��� 6���� ��(�簢��), �簢���� ���� 4��, �׷��Ƿ� 24���� ������ �ʿ��ϴ�.
	m_nVertices = 24;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	XMFLOAT2 *pxv2TexCoords = new XMFLOAT2[m_nVertices];

	int i = 0;
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;
	// Front Quad 
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Back Quad
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Left Quad
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Right Quad
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Top Quad
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, +fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);
	// Bottom Quad
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, +fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 0.0f);
	m_pxv3Positions[i] = XMFLOAT3(+fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(1.0f, 1.0f);
	m_pxv3Positions[i] = XMFLOAT3(-fx, -fx, -fx);
	pxv2TexCoords[i++] = XMFLOAT2(0.0f, 1.0f);

	D3D11_BUFFER_DESC d3dBufferDesc;
	::ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage          = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth      = sizeof(XMFLOAT3)* m_nVertices;
	d3dBufferDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA d3dBufferData;
	::ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem        = m_pxv3Positions;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	d3dBufferDesc.ByteWidth      = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem        = pxv2TexCoords;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dTexCoordBuffer);

	delete[] pxv2TexCoords;

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1]      = { sizeof(XMFLOAT3)};
	UINT pnBufferOffsets[1]      = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	//�ﰢ�� ��Ʈ������ �簢�� 1���� �׸��� ���� �ε����� 4���� �ʿ��ϴ�.
	m_nIndices = 4;
	m_pnIndices = new UINT[m_nIndices];

	m_pnIndices[0] = 0;
	m_pnIndices[1] = 1;
	m_pnIndices[2] = 3;
	m_pnIndices[3] = 2;

	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage          = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth      = sizeof(UINT)* m_nIndices;
	d3dBufferDesc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem        = m_pnIndices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);

	D3D11_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
	ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D11_DEPTH_STENCIL_DESC));
	//��ī�� �ڽ� �簢������ ���� ���� �˰����� �������� �ʰ� ���� ���۸� �������� �ʴ´�.
	d3dDepthStencilDesc.DepthEnable                  = false;
	d3dDepthStencilDesc.DepthWriteMask               = D3D11_DEPTH_WRITE_MASK_ZERO;
	d3dDepthStencilDesc.DepthFunc                    = D3D11_COMPARISON_NEVER;
	d3dDepthStencilDesc.StencilEnable                = false;
	d3dDepthStencilDesc.StencilReadMask              = 0xFF;
	d3dDepthStencilDesc.StencilWriteMask             = 0xFF;
	d3dDepthStencilDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	d3dDepthStencilDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
	d3dDepthStencilDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_DECR;
	d3dDepthStencilDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_KEEP;
	d3dDepthStencilDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;
	pd3dDevice->CreateDepthStencilState(&d3dDepthStencilDesc, &m_pd3dDepthStencilState);

	ID3D11SamplerState *pd3dSamplerState = nullptr;
	D3D11_SAMPLER_DESC d3dSamplerDesc;
	ZeroMemory(&d3dSamplerDesc, sizeof(D3D11_SAMPLER_DESC));
	d3dSamplerDesc.AddressU              = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressV              = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.AddressW              = D3D11_TEXTURE_ADDRESS_CLAMP;
	d3dSamplerDesc.Filter                = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	d3dSamplerDesc.ComparisonFunc        = D3D11_COMPARISON_NEVER;
	d3dSamplerDesc.MinLOD                = 0;
	d3dSamplerDesc.MaxLOD                = 0;
	pd3dDevice->CreateSamplerState(&d3dSamplerDesc, &pd3dSamplerState);

	m_pSkyboxTexture = new CTexture(1, 1, TX_SLOT_CUBE_TEXTURE, SS_SLOT_CUBE_SAMPLER_STATE);

	m_pSkyboxTexture->SetSampler(0, pd3dSamplerState);
	pd3dSamplerState->Release();
	//m_pSkyboxTexture->AddRef();

	OnChangeSkyBoxTextures(pd3dDevice, uImageNum);

	//CMesh::CreateRasterizerState(pd3dDevice);
}

CSkyBoxMesh::~CSkyBoxMesh()
{
	if (m_pd3dDepthStencilState) m_pd3dDepthStencilState->Release();
	if (m_pSkyboxTexture) m_pSkyboxTexture->Release();
}

void CSkyBoxMesh::OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, int nIndex)
{
	_TCHAR pstrTextureNames[128];
	_stprintf_s(pstrTextureNames, _T("../Assets/Image/SkyBox/SkyBox_%d.dds"), nIndex, 128);
	ID3D11ShaderResourceView *pd3dsrvTexture = nullptr;
	D3DX11CreateShaderResourceViewFromFile(pd3dDevice, pstrTextureNames, nullptr, nullptr, &pd3dsrvTexture, nullptr);
	m_pSkyboxTexture->SetTexture(0, pd3dsrvTexture); 
	pd3dsrvTexture->Release();

}

void CSkyBoxMesh::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	//CMesh::Render(pd3dDeviceContext);
	pd3dDeviceContext->IASetVertexBuffers(m_nSlot, m_nBuffers, m_ppd3dVertexBuffers, m_pnVertexStrides, m_pnVertexOffsets);
	pd3dDeviceContext->IASetIndexBuffer(m_pd3dIndexBuffer, m_dxgiIndexFormat, m_nIndexOffset);
	pd3dDeviceContext->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	//pd3dDeviceContext->RSSetState(m_pd3dRasterizerState);

	//��ī�� �ڽ��� �׸��� ���� ���÷� ���� ��ü�� ���� ���ٽ� ���� ��ü�� �����Ѵ�.
	m_pSkyboxTexture->UpdateShaderVariable(pd3dDeviceContext);

	//m_pSkyboxTexture->UpdateSamplerShaderVariable(pd3dDeviceContext, 0, 2);
	//m_pSkyboxTexture->UpdateTextureShaderVariable(pd3dDeviceContext, 0, 8);

	pd3dDeviceContext->OMSetDepthStencilState(m_pd3dDepthStencilState, 1);
	for (int i = 0; i < 6; i++)
	{
		pd3dDeviceContext->DrawIndexed(4, 0, i * 4);
	}

	pd3dDeviceContext->OMSetDepthStencilState(nullptr, 1);
}



CBillBoardVertex::CBillBoardVertex(ID3D11Device *pd3dDevice, float fWSize, float fHSize) : CMesh(pd3dDevice)
{
	m_nVertices = 1;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	//������ü �޽��� 2���� ���� ����(��ġ ���� ���ۿ� ���� ����)�� �����ȴ�.
	//������ü �޽��� ���� ����(��ġ ���� ����)�� �����Ѵ�.
	
	m_pxv3Positions = new XMFLOAT3[m_nVertices];

	m_pxv3Positions[0] = XMFLOAT3(0, 0, 0);//XMFLOAT3(1006, 200, 308);;

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
	XMFLOAT2 * pxmf2Size = new XMFLOAT2[m_nVertices];
	
	pxmf2Size[0] = XMFLOAT2(fWSize, fHSize);

	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT2)* m_nVertices;
	d3dBufferData.pSysMem = pxmf2Size;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dColorBuffer);
	
	ID3D11Buffer *pd3dBuffers[2] = { m_pd3dPositionBuffer, m_pd3dColorBuffer };
	UINT pnBufferStrides[2] = { sizeof(XMFLOAT3), sizeof(XMFLOAT2) };
	UINT pnBufferOffsets[2] = { 0, 0 };
	AssembleToVertexBuffer(2, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	if (pxmf2Size) delete[] pxmf2Size;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fWSize, -fHSize, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fWSize, +fHSize, +1);
}

CBillBoardVertex::~CBillBoardVertex()
{
}


CPoint2DMesh::CPoint2DMesh(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fxSize, float fySize) : CMesh(pd3dDevice)
{
	m_nVertices = 1;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	XMFLOAT4 xmfInfo = XMFLOAT4(fWidth, fHeight, fxSize, fySize);

	BuildMesh(pd3dDevice, xmfInfo);
}

CPoint2DMesh::CPoint2DMesh(ID3D11Device * pd3dDevice, XMFLOAT4 & info) : CMesh(pd3dDevice)
{
	m_nVertices = 1;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	BuildMesh(pd3dDevice, info);
}

CPoint2DMesh::~CPoint2DMesh()
{
}

void CPoint2DMesh::BuildMesh(ID3D11Device * pd3dDevice, XMFLOAT4 & info)
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4) *m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = &info;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(FLT_MIN, FLT_MIN, FLT_MIN);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
}


CPointCubeMesh::CPointCubeMesh(ID3D11Device *pd3dDevice, float fSize) : CMesh(pd3dDevice)
{
	m_nVertices = 1;
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;

	float pSize[1] = { fSize };

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(float) *m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pSize;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dSizeBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dSizeBuffer };
	UINT pnBufferStrides[1] = { sizeof(float) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fSize, -fSize, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(fSize, fSize, +1);
}

CPointCubeMesh::~CPointCubeMesh()
{
}



CPointSphereMesh::CPointSphereMesh(ID3D11Device *pd3dDevice,float nSlices, float fSize) : CMesh(pd3dDevice)
{
	m_nVertices = nSlices * nSlices;

	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
	
	XMFLOAT4 * pxmf4StackInfo= new XMFLOAT4[m_nVertices];
	//float *pSize = new float[m_nVertices];
	for (int i = 0, index = 0; i < nSlices; ++i)
	{
		for (int j = 0; j < nSlices; ++j)
		{
			//pSize[index] = fSize;
			pxmf4StackInfo[index].x = i;
			pxmf4StackInfo[index].y = j;
			pxmf4StackInfo[index].z = nSlices;
			pxmf4StackInfo[index++].w = fSize;
		}
	}

	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	d3dBufferDesc.ByteWidth = sizeof(XMFLOAT4) * m_nVertices;
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pxmf4StackInfo;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dSizeBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dSizeBuffer };
	UINT pnBufferStrides[1] = { sizeof(XMFLOAT4) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	//d0elete[] pSize;
	delete[] pxmf4StackInfo;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fSize, -fSize, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(fSize, fSize, +1);
}

CPointSphereMesh::~CPointSphereMesh()
{
}

CBezierMesh::CBezierMesh(ID3D11Device *pd3dDevice) : CMesh(pd3dDevice)
{
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
	//m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	m_nVertices = 16;
	XMFLOAT3 pQuadPatchVertices[16] =
	{
		XMFLOAT3(-10.0f, -20.0f, 15.0f), XMFLOAT3(-5.0f, 0.0f, 15.0f), XMFLOAT3(5.0f, 0.0f, 15.0f), XMFLOAT3(10.0f, 0.0f, 15.0f),
		XMFLOAT3(-15.0f, 0.0f, 5.0f), XMFLOAT3(-5.0f, 0.0f, 5.0f), XMFLOAT3(5.0f, 30.0f, 5.0f), XMFLOAT3(15.0f, 0.0f, 5.0f),
		XMFLOAT3(-15.0f, 0.0f, -5.0f), XMFLOAT3(-5.0f, 0.0f, -5.0f), XMFLOAT3(5.0f, 0.0f, -5.0f), XMFLOAT3(15.0f, 0.0f, -5.0f),
		XMFLOAT3(-10.0f, 20.0f, -15.0f), XMFLOAT3(-5.0f, 0.0f, -15.0f), XMFLOAT3(5.0f, 0.0f, -15.0f), XMFLOAT3(25.0f, 20.0f, -15.0f)
	};

	CTexture2Vertex pInfo[16];
	for (int i = 0; i < 16; ++i)
		pInfo[i] = { pQuadPatchVertices[i], XMFLOAT2(0, 0) };

	D3D11_BUFFER_DESC d3dBufferDesc;
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(CTexture2Vertex)* m_nVertices;		// 1���� �ǹ�
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	d3dBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	d3dBufferData.pSysMem = pInfo;// pQuadPatchVertices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(CTexture2Vertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);


	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-15, -20, -15);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+25, +20, +15);

	CreateRasterizerState(pd3dDevice);
}

CBezierMesh::~CBezierMesh()
{

}

CNormalCube::CNormalCube(ID3D11Device *pd3dDevice, float fWidth, float fHeight) : CMesh(pd3dDevice)
{
	m_nVertices = 4;
	//m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST; 
	m_d3dPrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = 1;

	m_pxv3Positions = new XMFLOAT3[m_nVertices];
	NormalMapVertex * pNormalMap = new NormalMapVertex[m_nVertices];
	int i = 0;

	//������ü�� �� ��(�ﰢ�� 2��)�� �ϳ��� �ؽ��� �̹��� ��ü�� ���εǵ��� �ؽ��� ��ǥ�� �����Ѵ�.
	pNormalMap[0] = { m_pxv3Positions[0] = XMFLOAT3(-fx, +fy, 0), XMFLOAT2(0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	pNormalMap[1] = { m_pxv3Positions[1] = XMFLOAT3(+fx, +fy, 0), XMFLOAT2(1.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	pNormalMap[2] = { m_pxv3Positions[2] = XMFLOAT3(-fx, -fy, 0), XMFLOAT2(0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	//pNormalMap[3] = { m_pxv3Positions[3] = XMFLOAT3(+fx, +fy, 0), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(1.0f, 0.0f) };
	pNormalMap[3] = { m_pxv3Positions[3] = XMFLOAT3(+fx, -fy, 0), XMFLOAT2(1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, -1.0f), XMFLOAT3(1.0f, 0.0f, 0.0f)};
	//pNormalMap[5] = { m_pxv3Positions[5] = XMFLOAT3(-fx, -fy, 0), XMFLOAT3(0.0f, 0.0f, 1.0f), XMFLOAT3(-1.0f, 0.0f, 0.0f), XMFLOAT2(0.0f, 1.0f) };

	D3D11_BUFFER_DESC d3dBufferDesc;
	d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	d3dBufferDesc.ByteWidth = sizeof(NormalMapVertex)* m_nVertices;		// 1���� �ǹ�
	d3dBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = 0;
	d3dBufferDesc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	d3dBufferData.pSysMem = pNormalMap;// pQuadPatchVertices;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dPositionBuffer);

	ID3D11Buffer *pd3dBuffers[1] = { m_pd3dPositionBuffer };
	UINT pnBufferStrides[1] = { sizeof(NormalMapVertex) };
	UINT pnBufferOffsets[1] = { 0 };
	AssembleToVertexBuffer(1, pd3dBuffers, pnBufferStrides, pnBufferOffsets);

	delete[] pNormalMap;

	m_bcBoundingCube.m_xv3Minimum = XMFLOAT3(-fx, -fy, -1);
	m_bcBoundingCube.m_xv3Maximum = XMFLOAT3(+fx, +fy, +1);

	//m_nIndices = 4;
	//m_pnIndices = new UINT[m_nIndices];


	//ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	//d3dBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	//d3dBufferDesc.ByteWidth = sizeof(UINT)* m_nIndices;
	//d3dBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//d3dBufferDesc.CPUAccessFlags = 0;
	//ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	//d3dBufferData.pSysMem = m_pnIndices;
	//pd3dDevice->CreateBuffer(&d3dBufferDesc, &d3dBufferData, &m_pd3dIndexBuffer);


	CreateRasterizerState(pd3dDevice);
}

CNormalCube::~CNormalCube()
{

}


void CNormalCube::CreateRasterizerState(ID3D11Device *pd3dDevice)
{
	D3D11_RASTERIZER_DESC d3dRasterizerDesc;
	ZeroMemory(&d3dRasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
	//�����Ͷ����� �ܰ迡�� �ø�(���� ����)�� ���� �ʵ��� �����Ͷ����� ���¸� �����Ѵ�.
	d3dRasterizerDesc.CullMode = D3D11_CULL_NONE;
	d3dRasterizerDesc.FillMode = D3D11_FILL_SOLID;
	d3dRasterizerDesc.DepthClipEnable = true;

	//pd3dDevice->CreateRasterizerState(&d3dRasterizerDesc, &m_pd3dRasterizerState);
}

void CNormalCube::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CMesh::Render(pd3dDeviceContext, uRenderState);
}
