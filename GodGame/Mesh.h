#pragma once
#include "MeshType.h"


#define SPLAT_TERRAIN
/*���� �޽��� �� ������ �� ���� �ؽ��� ��ǥ�� ���´�.*/
#ifdef DETAIL_TERRAIN
class CHeightMapGridMesh : public CMeshDetailTexturedIlluminated
#else
#ifdef SPLAT_TERRAIN
class CHeightMapGridMesh : public CMeshSplatTexturedIlluminated
#endif
#endif
{
protected:
	//������ ũ��(����: x-����, ����: z-����)�̴�.
	int m_nWidth;
	int m_nLength;
	/*������ ������(����: x-����, ����: z-����, ����: y-����) �����̴�. ���� ���� �޽��� �� ������ x-��ǥ, y-��ǥ, z-��ǥ�� ������ ������ x-��ǥ, y-��ǥ, z-��ǥ�� ���� ���� ���´�. ��, ���� ������ x-�� ������ ������ 1�� �ƴ϶� ������ ������ x-��ǥ�� �ȴ�. �̷��� �ϸ� ���� ���ڸ� ����ϴ��� ū ���ڸ� ������ �� �ִ�.*/
	XMFLOAT3 m_xv3Scale;

public:
	CHeightMapGridMesh(ID3D11Device *pd3dDevice, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), void *pContext = nullptr);
	virtual ~CHeightMapGridMesh();

	XMFLOAT3 GetScale() { return(m_xv3Scale); }
	int GetWidth() { return(m_nWidth); }
	int GetLength() { return(m_nLength); }

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	//virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext);
	//������ ����(����)�� ���̸� �����Ѵ�.
	virtual float OnGetHeight(int x, int z, void *pContext);
	//������ ����(����)�� ������ �����Ѵ�.
	virtual XMFLOAT4 OnGetColor(int x, int z, void *pContext);
};

class CWaterGridMesh : public CMeshSplatTexturedIlluminated
{
public:
	CWaterGridMesh(ID3D11Device *pd3dDevice, int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), void *pContext = nullptr);
	virtual ~CWaterGridMesh();

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
};

class CTerrainPartMesh : public CMesh
{
protected:
	/*������ ������(����: x-����, ����: z-����, ����: y-����) �����̴�. ���� ���� �޽��� �� ������ x-��ǥ, y-��ǥ, z-��ǥ�� ������ ������ x-��ǥ, y-��ǥ, z-��ǥ�� ���� ���� ���´�. ��, ���� ������ x-�� ������ ������ 1�� �ƴ϶� ������ ������ x-��ǥ�� �ȴ�. �̷��� �ϸ� ���� ���ڸ� ����ϴ��� ū ���ڸ� ������ �� �ִ�.*/
	XMFLOAT3 m_xv3Scale;

public:
	CTerrainPartMesh(ID3D11Device *pd3dDevice, float xStart, float zStart, int nWidth, int nLength, XMFLOAT3 xv3Scale = XMFLOAT3(1.0f, 1.0f, 1.0f), void *pContext = nullptr);
	virtual ~CTerrainPartMesh();

	XMFLOAT3 GetScale() { return(m_xv3Scale); }

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
	//virtual void RenderReflected(ID3D11DeviceContext *pd3dDeviceContext);
	//������ ����(����)�� ���̸� �����Ѵ�.
	virtual float OnGetHeight(int x, int z, void *pContext);
	//������ ����(����)�� ������ �����Ѵ�.
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
};

class CSkyBoxMesh : public CMeshTextured
{
protected:
	ID3D11DepthStencilState *m_pd3dDepthStencilState;

	CTexture *m_pSkyboxTexture;

public:
	CSkyBoxMesh(ID3D11Device *pd3dDevice, UINT uImageNum, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 20.0f);
	virtual ~CSkyBoxMesh();

	void OnChangeSkyBoxTextures(ID3D11Device *pd3dDevice, int nIndex = 0);

	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
};

class CBillBoardVertex : public CMesh
{
public:
	CBillBoardVertex(ID3D11Device *pd3dDevice, float fWSize, float fHSize);
	~CBillBoardVertex();
};

class CPoint2DMesh : public CMesh
{
public:
	CPoint2DMesh(ID3D11Device *pd3dDevice, float fWidth, float fHeight, float fxSize, float fySize);
	CPoint2DMesh(ID3D11Device *pd3dDevice, XMFLOAT4 & info);
	~CPoint2DMesh();

	void BuildMesh(ID3D11Device *pd3dDevice, XMFLOAT4 & info);
};

class CPointCubeMesh : public CMesh
{
protected:
	ID3D11Buffer * m_pd3dSizeBuffer;
public:
	CPointCubeMesh(ID3D11Device *pd3dDevice, float fSize);
	~CPointCubeMesh();
};


class CPointSphereMesh : public CMesh
{
protected:
	ID3D11Buffer * m_pd3dSizeBuffer;
public:
	CPointSphereMesh(ID3D11Device *pd3dDevice, float nSlices, float fSize);
	~CPointSphereMesh();
};

class CBezierMesh : public CMesh
{
public:
	CBezierMesh(ID3D11Device *pd3dDevice);
	~CBezierMesh();
};

class CNormalCube : public CMesh
{
public:
	CNormalCube(ID3D11Device *pd3dDevice, float fWidth, float fHeight);
	~CNormalCube();
	virtual void CreateRasterizerState(ID3D11Device *pd3dDevice);
	virtual void Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState);
};