#include "stdafx.h"
#include "MyInline.h"
#include "ShaderType.h"
#include <D3Dcompiler.h>

ID3D11Buffer *CShader::m_pd3dcbWorldMatrix         = nullptr;
ID3D11Buffer *CIlluminatedShader::m_pd3dcbMaterial = nullptr;

BYTE *ReadCompiledEffectFile(WCHAR *pszFileName, int *pnReadBytes)
{
	FILE *pFile     = nullptr;
	::_wfopen_s(&pFile, pszFileName, L"rb");
	::fseek(pFile, 0, SEEK_END);
	int nFileSize   = ::ftell(pFile);
	BYTE *pByteCode = new BYTE[nFileSize];
	::rewind(pFile);
	*pnReadBytes    = (int)::fread(pByteCode, sizeof(BYTE), nFileSize, pFile);
	::fclose(pFile);
	return(pByteCode);
}

CShader::CShader()
{
	m_ppObjects          = nullptr;
	m_nObjects           = 0;

	m_pd3dVertexShader   = nullptr;
	m_pd3dVertexLayout   = nullptr;
	m_pd3dPixelShader    = nullptr;
	m_pd3dGeometryShader = nullptr;
	m_pd3dHullShader     = nullptr;
	m_pd3dDomainShader   = nullptr;
	//m_p3dComputeShader = nullptr;
}

CShader::~CShader()
{
	if (m_pd3dVertexShader  ) m_pd3dVertexShader->Release();
	if (m_pd3dVertexLayout  ) m_pd3dVertexLayout->Release();
	if (m_pd3dPixelShader   ) m_pd3dPixelShader->Release();
	if (m_pd3dGeometryShader) m_pd3dGeometryShader->Release();
	if (m_pd3dHullShader    ) m_pd3dHullShader->Release();
	if (m_pd3dDomainShader  ) m_pd3dDomainShader->Release();
	//if (m_p3dComputeShader) m_p3dComputeShader->Release();
}

void CShader::BuildObjects(ID3D11Device *pd3dDevice, BUILD_RESOURCES_MGR & SceneMgr)
{
}

void CShader::ReleaseObjects()
{
	if (m_ppObjects)
	{
		for (int j = 0; j < m_nObjects; j++)
			if (m_ppObjects[j])
			{
				m_ppObjects[j]->Release();
			}
		delete[] m_ppObjects;
	}
}
void CShader::AnimateObjects(float fTimeElapsed)
{
	for (int j = 0; j < m_nObjects; j++)
	{
		m_ppObjects[j]->Animate(fTimeElapsed);
	}
}
void CShader::OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	if (!(uRenderState & NOT_PSUPDATE))
		pd3dDeviceContext->PSSetShader(m_pd3dPixelShader, nullptr, 0);
	pd3dDeviceContext->IASetInputLayout(m_pd3dVertexLayout);
	pd3dDeviceContext->VSSetShader(m_pd3dVertexShader, nullptr, 0);

	pd3dDeviceContext->GSSetShader(m_pd3dGeometryShader, nullptr, 0);
	pd3dDeviceContext->HSSetShader(m_pd3dHullShader, nullptr, 0);
	pd3dDeviceContext->DSSetShader(m_pd3dDomainShader, nullptr, 0);
	//pd3dDeviceContext->CSSetShader(m_p3dComputeShader, nullptr, 0);
}
void CShader::Render(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState, CCamera *pCamera)
{
	OnPrepareRender(pd3dDeviceContext, uRenderState);

	for (int j = 0; j < m_nObjects; j++)
	{
#ifdef _QUAD_TREE
		if (m_ppObjects[j]->IsVisible())
#else
		if (m_ppObjects[j]->IsVisible(pCamera))
#endif
		{
			//ī�޶��� ����ü�� ���ԵǴ� ��ü�鸸�� �������Ѵ�.
			//if (m_ppObjects[j]->IsVisible(pCamera)) {
			m_ppObjects[j]->Render(pd3dDeviceContext, uRenderState, pCamera);
			//}
		}
	}
}
void CShader::GetGameMessage(CShader * byObj, eMessage eMSG, void * extra)
{
}
void CShader::SendGameMessage(CShader * toObj, eMessage eMSG, void * extra)
{
}
void CShader::MessageObjToObj(CShader * byObj, CShader * toObj, eMessage eMSG, void * extra)
{
}

#pragma region CREATE_SHADER_FROM_FILE

void CShader::CreateVertexShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11VertexShader **ppd3dVertexShader, D3D11_INPUT_ELEMENT_DESC *pd3dInputLayout, UINT nElements, ID3D11InputLayout **ppd3dVertexLayout)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	/*����(pszFileName)���� ���̴� �Լ�(pszShaderName)�� �������Ͽ� �����ϵ� ���̴� �ڵ��� �޸� �ּ�(pd3dShaderBlob)�� ��ȯ�Ѵ�.*/
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		//�����ϵ� ���̴� �ڵ��� �޸� �ּҿ��� ����-���̴��� �����Ѵ�.
		ASSERT_S(pd3dDevice->CreateVertexShader(pd3dShaderBlob->GetBufferPointer(), pd3dShaderBlob->GetBufferSize(), nullptr, ppd3dVertexShader));
		//�����ϵ� ���̴� �ڵ��� �޸� �ּҿ� �Է� ���̾ƿ����� ���� ���̾ƿ��� �����Ѵ�.
		ASSERT_S(pd3dDevice->CreateInputLayout(pd3dInputLayout, nElements, pd3dShaderBlob->GetBufferPointer(), pd3dShaderBlob->GetBufferSize(), ppd3dVertexLayout));
		pd3dShaderBlob->Release();
	}
}

void CShader::CreatePixelShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11PixelShader **ppd3dPixelShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	/*����(pszFileName)���� ���̴� �Լ�(pszShaderName)�� �������Ͽ� �����ϵ� ���̴� �ڵ��� �޸� �ּ�(pd3dShaderBlob)�� ��ȯ�Ѵ�.*/
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		//�����ϵ� ���̴� �ڵ��� �޸� �ּҿ��� �ȼ�-���̴��� �����Ѵ�.
		ASSERT_S(pd3dDevice->CreatePixelShader(pd3dShaderBlob->GetBufferPointer(), pd3dShaderBlob->GetBufferSize(), nullptr, ppd3dPixelShader));
		pd3dShaderBlob->Release();
	}
}

void CShader::CreateGeometryShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dGeometryShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dGeometryShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		ASSERT_S(pd3dDevice->CreateGeometryShader(pd3dGeometryShaderBlob->GetBufferPointer(), pd3dGeometryShaderBlob->GetBufferSize(), nullptr, ppd3dGeometryShader));
		pd3dGeometryShaderBlob->Release();
	}
}

void CShader::CreateGeometryStreamOutShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11GeometryShader **ppd3dGeometryShader,
	D3D11_SO_DECLARATION_ENTRY * pSODeclaration, UINT NumEntries, UINT *pBufferStrides, UINT NumStrides, UINT RasterizedStream)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dGeometryShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dGeometryShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		ASSERT_S(pd3dDevice->CreateGeometryShaderWithStreamOutput(pd3dGeometryShaderBlob->GetBufferPointer(), pd3dGeometryShaderBlob->GetBufferSize(),
			pSODeclaration, NumEntries, pBufferStrides, NumStrides, RasterizedStream, nullptr, ppd3dGeometryShader));
		pd3dGeometryShaderBlob->Release();
	}
}

void CShader::CreateHullShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11HullShader **ppd3dHullShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dHullShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dHullShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		ASSERT_S(pd3dDevice->CreateHullShader(pd3dHullShaderBlob->GetBufferPointer(), pd3dHullShaderBlob->GetBufferSize(), nullptr, ppd3dHullShader));
		pd3dHullShaderBlob->Release();
	}
}

void CShader::CreateDomainShaderFromFile(ID3D11Device *pd3dDevice, WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11DomainShader **ppd3dDomainShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dDomainShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dDomainShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		ASSERT_S(pd3dDevice->CreateDomainShader(pd3dDomainShaderBlob->GetBufferPointer(), pd3dDomainShaderBlob->GetBufferSize(), nullptr, ppd3dDomainShader));
		pd3dDomainShaderBlob->Release();
	}
}
void CShader::CreateComputeShaderFromFile(ID3D11Device * pd3dDevice, WCHAR * pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderModel, ID3D11ComputeShader ** ppd3dComputeShader)
{
	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob *pd3dDomainShaderBlob = nullptr, *pd3dErrorBlob = nullptr;
	if (SUCCEEDED(D3DX11CompileFromFile(pszFileName, nullptr, nullptr, pszShaderName, pszShaderModel, dwShaderFlags, 0, nullptr, &pd3dDomainShaderBlob, &pd3dErrorBlob, nullptr)))
	{
		ASSERT_S(pd3dDevice->CreateComputeShader(pd3dDomainShaderBlob->GetBufferPointer(), pd3dDomainShaderBlob->GetBufferSize(), nullptr, ppd3dComputeShader));
		pd3dDomainShaderBlob->Release();
	}
}
#pragma endregion CREATE_SHADER_FROM_FILE

void CShader::CreateShader(ID3D11Device *pd3dDevice)
{
	/*IA �ܰ迡 ������ �Է�-���̾ƿ��� �����Ѵ�. ���� ������ �� ���Ұ� CVertex Ŭ������ ��� ����
	(XMFLOAT3 ��, �Ǽ� �� ��)�̹Ƿ� �Է�-���̾ƿ��� �Ǽ�(32-��Ʈ) 3���� �����Ǹ� �ø�ƽ�� ��POSITION���̰� ���� ���������� ǥ���Ѵ�.*/
	D3D11_INPUT_ELEMENT_DESC d3dInputLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputLayout);

	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VS", "vs_5_0", &m_pd3dVertexShader, d3dInputLayout, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PS", "ps_5_0", &m_pd3dPixelShader);
	CreateShaderVariables(pd3dDevice);
}

void CShader::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	////���� ��ȯ ����� ���� ��� ���۸� �����Ѵ�.
	ASSERT(nullptr != (m_pd3dcbWorldMatrix = ViewMgr.GetBuffer("cs_float4x4")));
	m_pd3dcbWorldMatrix->AddRef();
}

void CShader::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, XMFLOAT4X4 & xmtxWorld)
{
	//���� ��ȯ ����� ��� ���ۿ� �����Ѵ�.
	MapMatrixConstantBuffer(pd3dDeviceContext, xmtxWorld, m_pd3dcbWorldMatrix);
	//��� ���۸� ����̽��� ����(CB_SLOT_WORLD_MATRIX)�� �����Ѵ�.
	pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);
	//pd3dDeviceContext->HSSetConstantBuffers(CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);
	//pd3dDeviceContext->DSSetConstantBuffers(CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);
}

void CShader::ReleaseShaderVariables()
{
	//���� ��ȯ ����� ���� ��� ���۸� ��ȯ�Ѵ�.
	if (m_pd3dcbWorldMatrix) m_pd3dcbWorldMatrix->Release();
}
#ifdef PICKING
CGameObject *CShader::PickObjectByRayIntersection(XMFLOAT3 *pxv3PickPosition, XMFLOAT4X4 *pxmtxView, MESHINTERSECTINFO *pd3dxIntersectInfo)
{
	int nIntersected = 0;
	float fNearHitDistance = FLT_MAX;
	CGameObject *pSelectedObject = nullptr;
	MESHINTERSECTINFO d3dxIntersectInfo;
	/*���̴� ��ü�� ���ԵǴ� ��� ��ü�鿡 ���Ͽ� ��ŷ ������ �����ϰ� ��ü�� �ٿ�� �ڽ����� ������ �˻��Ѵ�. �����ϴ� ��ü�� �߿� ī�޶�� ���� ����� ��ü�� ������ ��ü�� ��ȯ�Ѵ�.*/
	for (int i = 0; i < m_nObjects; i++)
	{
		nIntersected = m_ppObjects[i]->PickObjectByRayIntersection(pxv3PickPosition, pxmtxView, &d3dxIntersectInfo);
		if ((nIntersected > 0) && (d3dxIntersectInfo.m_fDistance < fNearHitDistance))
		{
			fNearHitDistance = d3dxIntersectInfo.m_fDistance;
			pSelectedObject = m_ppObjects[i];
			if (pd3dxIntersectInfo) *pd3dxIntersectInfo = d3dxIntersectInfo;
		}
	}
	return(pSelectedObject);
}
#endif

CGameObject * CShader::GetObj(int num) {
	if (m_ppObjects[num])
		return m_ppObjects[num];
	return nullptr;
}


void CShader::EntityAllStaticObjects(const char * name)
{
#ifdef _LOAD_DATA
	if (name) LoadData(name);
#endif

	CQuadTreeManager & pMgr = QUADMgr;
	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->UpdateBoundingBox();
		pMgr.InsertStaticEntity(m_ppObjects[i]);
	}

#ifdef _SAVE_DATA
	if (name) SaveData(name);
#endif
}
void CShader::EntityAllDynamicObjects(const char * name)
{
#ifdef _LOAD_DATA
	if (name) LoadData(name);
#endif

	CQuadTreeManager & pMgr = QUADMgr;
	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->SetCollide(true);
		m_ppObjects[i]->UpdateBoundingBox();
		pMgr.InsertDynamicEntity(m_ppObjects[i]);
	}

#ifdef _SAVE_DATA
	if (name) SaveData(name);
#endif
}

void CShader::SaveData(const char * name)
{
	static char filename[128];

	FILE * file;
	sprintf(filename, "../Assets/Data/Map01/%s.data", name);
	file = fopen(filename, "wb");

	fwrite(&m_nObjects, sizeof(UINT), 1, file);
	XMFLOAT3 pos;
	for (int i = 0; i < m_nObjects; ++i)
	{
		pos = m_ppObjects[i]->GetPosition();
		fwrite(&pos, sizeof(XMFLOAT3), 1, file);
	}
	fclose(file);
}

void CShader::LoadData(const char * name)
{
	static char filename[128];

	FILE * file;
	sprintf(filename, "../Assets/Data/Map01/%s.data", name);
	file = fopen(filename, "rb");

	UINT nObjects;
	fread(&nObjects, sizeof(UINT), 1, file);
	
	XMFLOAT3 pos;
	for (int i = 0; i < nObjects; ++i)
	{
		fread(&pos, sizeof(XMFLOAT3), 1, file);
		m_ppObjects[i]->SetPosition(pos);
	}
	fclose(file);
}
#if 0
void CShader::EntityAllStaticObjects()
{
	CQuadTreeManager & pMgr = QUADMgr;
	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->UpdateBoundingBox();
		pMgr.InsertStaticEntity(m_ppObjects[i]);
	}
}
void CShader::EntityAllDynamicObjects()
{
	CQuadTreeManager & pMgr = QUADMgr;
	for (int i = 0; i < m_nObjects; ++i)
	{
		m_ppObjects[i]->SetCollide(true);
		m_ppObjects[i]->UpdateBoundingBox();
		pMgr.InsertDynamicEntity(m_ppObjects[i]);
	}
}
#endif

ID3D11ShaderResourceView * CShader::CreateRandomTexture1DSRV(ID3D11Device * pd3dDevice)
{
	XMFLOAT4 RandomValue[1024];
	for (int i = 0; i < 1024; ++i)
		RandomValue[i] = XMFLOAT4(Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f), Chae::RandomFloat(-1.0f, 1.0f));
	D3D11_SUBRESOURCE_DATA d3dSubresourceData;
	d3dSubresourceData.pSysMem          = RandomValue;
	d3dSubresourceData.SysMemPitch      = sizeof(XMFLOAT4) * 1024;
	d3dSubresourceData.SysMemSlicePitch = 0;

	D3D11_TEXTURE1D_DESC d3dTextureDesc;
	ZeroMemory(&d3dTextureDesc, sizeof(D3D11_TEXTURE1D_DESC));
	d3dTextureDesc.Width                = 1024;
	d3dTextureDesc.MipLevels            = 1;
	d3dTextureDesc.Format               = DXGI_FORMAT_R32G32B32A32_FLOAT;
	d3dTextureDesc.Usage                = D3D11_USAGE_IMMUTABLE;
	d3dTextureDesc.BindFlags            = D3D11_BIND_SHADER_RESOURCE;
	d3dTextureDesc.ArraySize            = 1;

	ID3D11Texture1D * pd3dTexture;
	pd3dDevice->CreateTexture1D(&d3dTextureDesc, &d3dSubresourceData, &pd3dTexture);

	ID3D11ShaderResourceView * pd3dsrvTexutre;
	pd3dDevice->CreateShaderResourceView(pd3dTexture, nullptr, &pd3dsrvTexutre);
	pd3dTexture->Release();
	return(pd3dsrvTexutre);
}

#pragma region BasicShader
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Illuminated

CIlluminatedShader::CIlluminatedShader()
{
}

CIlluminatedShader::~CIlluminatedShader()
{
}

void CIlluminatedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSLightingColor", "ps_5_0", &m_pd3dPixelShader);
}
void CIlluminatedShader::CreateShaderVariables(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	d3dBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	d3dBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	d3dBufferDesc.ByteWidth      = sizeof(MATERIAL);
	pd3dDevice->CreateBuffer(&d3dBufferDesc, nullptr, &m_pd3dcbMaterial);
}

void CIlluminatedShader::ReleaseShaderVariables()
{
	if (m_pd3dcbMaterial) m_pd3dcbMaterial->Release();
}

void CIlluminatedShader::UpdateShaderVariable(ID3D11DeviceContext *pd3dDeviceContext, MATERIAL *pMaterial)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	pd3dDeviceContext->Map(m_pd3dcbMaterial, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	MATERIAL *pcbMaterial = (MATERIAL *)d3dMappedResource.pData;
	memcpy(pcbMaterial, pMaterial, sizeof(MATERIAL));
	pd3dDeviceContext->Unmap(m_pd3dcbMaterial, 0);
	pd3dDeviceContext->PSSetConstantBuffers(CB_PS_SLOT_MATERIAL, 1, &m_pd3dcbMaterial);
}
#pragma endregion Illuminated
///////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region diffuse
CDiffusedShader::CDiffusedShader()
{
}

CDiffusedShader::~CDiffusedShader()
{
}

void CDiffusedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSDiffusedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSDiffusedColor", "ps_5_0", &m_pd3dPixelShader);
}

#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region Texture
CTexturedShader::CTexturedShader()
{
}

CTexturedShader::~CTexturedShader()
{
}

void CTexturedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTexturedColor", "ps_5_0", &m_pd3dPixelShader);
}
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region DetailTexutre
CDetailTexturedShader::CDetailTexturedShader()
{
}

CDetailTexturedShader::~CDetailTexturedShader()
{
}

void CDetailTexturedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSDetailTexturedColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSDetailTexturedColor", "ps_5_0", &m_pd3dPixelShader);
}
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region TextureIlluminated
CTexturedIlluminatedShader::CTexturedIlluminatedShader()
{
}

CTexturedIlluminatedShader::~CTexturedIlluminatedShader()
{
}

void CTexturedIlluminatedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
}
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma region DetailTextureIlluminate
CDetailTexturedIlluminatedShader::CDetailTexturedIlluminatedShader()
{
}

CDetailTexturedIlluminatedShader::~CDetailTexturedIlluminatedShader()
{
}

void CDetailTexturedIlluminatedShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSDetailTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSDetailTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
}
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma endregion BasicShader

//////////////////////////////////////////////////////////////////////////

CInstanceShader::CInstanceShader()
{
	m_nInstanceBufferOffset = 0;
	m_nInstanceBufferStride = 0;

}

CInstanceShader::~CInstanceShader()
{
}

ID3D11Buffer *CInstanceShader::CreateInstanceBuffer(ID3D11Device *pd3dDevice, int nObjects, UINT nBufferStride, void *pBufferData)
{
	ID3D11Buffer *pd3dInstanceBuffer = nullptr;
	D3D11_BUFFER_DESC d3dBufferDesc;
	ZeroMemory(&d3dBufferDesc, sizeof(D3D11_BUFFER_DESC));
	/*������ �ʱ�ȭ �����Ͱ� ������ ���� ���۷� �����Ѵ�. ��, ���߿� ������ �Ͽ� ������ ä��ų� �����Ѵ�.*/
	d3dBufferDesc.Usage          = (pBufferData) ? D3D11_USAGE_DEFAULT : D3D11_USAGE_DYNAMIC;
	d3dBufferDesc.ByteWidth      = nBufferStride * nObjects;
	d3dBufferDesc.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	d3dBufferDesc.CPUAccessFlags = (pBufferData) ? 0 : D3D11_CPU_ACCESS_WRITE;

	D3D11_SUBRESOURCE_DATA d3dBufferData;
	ZeroMemory(&d3dBufferData, sizeof(D3D11_SUBRESOURCE_DATA));
	d3dBufferData.pSysMem = pBufferData;
	pd3dDevice->CreateBuffer(&d3dBufferDesc, (pBufferData) ? &d3dBufferData : nullptr, &pd3dInstanceBuffer);
	return(pd3dInstanceBuffer);
}

#pragma region SplatTextureIlluminated
CSplatLightingShader::CSplatLightingShader()
{
	m_pd3dSplatBlendState = nullptr;
}

CSplatLightingShader::~CSplatLightingShader()
{
	if (m_pd3dSplatBlendState) m_pd3dSplatBlendState->Release();
}

void CSplatLightingShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	UINT nElements = ARRAYSIZE(d3dInputElements);
	CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSSplatTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSSplatTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
}
#pragma endregion
////////////////////////////////////////////////////////////////////////////////////////////////////////

CNormalMapShader::CNormalMapShader() : CShader()
{
}

CNormalMapShader::~CNormalMapShader()
{
}

void CNormalMapShader::CreateShader(ID3D11Device *pd3dDevice)
{
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage          = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth      = sizeof(CB_DISPLACEMENT);
	bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbBump);

	bd.ByteWidth = sizeof(CB_DISPLACEMENT);
	pd3dDevice->CreateBuffer(&bd, nullptr, &m_pd3dcbBump);

	//D3D11_INPUT_ELEMENT_DESC d3dInputElements[] =
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TANGENT", 0, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	//};
	//UINT nElements = ARRAYSIZE(d3dInputElements);
	//CreateVertexShaderFromFile(pd3dDevice, L"fx/Effect.fx", "VSTexturedLightingColor", "vs_5_0", &m_pd3dVertexShader, d3dInputElements, nElements, &m_pd3dVertexLayout);
	//CreatePixelShaderFromFile(pd3dDevice, L"fx/Effect.fx", "PSTexturedLightingColor", "ps_5_0", &m_pd3dPixelShader);
}

void CNormalMapShader::OnPrepareRender(ID3D11DeviceContext *pd3dDeviceContext, UINT uRenderState)
{
	CShader::OnPrepareRender(pd3dDeviceContext, uRenderState);

	UpdateBumpInfo(pd3dDeviceContext);
}

void CNormalMapShader::UpdateBumpInfo(ID3D11DeviceContext *pd3dDeviceContext, SETSHADER setInfo /*= ( SET_SHADER_DS | SET_SHADER_PS )*/)
{
	D3D11_MAPPED_SUBRESOURCE d3dMappedResource;
	/*��� ������ �޸� �ּҸ� �����ͼ� ī�޶� ��ȯ ��İ� ���� ��ȯ ����� �����Ѵ�. ���̴����� ����� ��� ���� �ٲ�� �Ϳ� �����϶�.*/
	pd3dDeviceContext->Map(m_pd3dcbBump, 0, D3D11_MAP_WRITE_DISCARD, 0, &d3dMappedResource);
	CB_DISPLACEMENT *pcbBump = (CB_DISPLACEMENT *)d3dMappedResource.pData;
	pcbBump->m_xv3BumpScale  = m_Bump.m_xv3BumpScale;
	pcbBump->m_fBumpMax      = m_Bump.m_fBumpMax;
	pd3dDeviceContext->Unmap(m_pd3dcbBump, 0);

	//��� ���۸� ����(VS_SLOT_CAMERA)�� �����Ѵ�.
	if (setInfo & SET_SHADER_VS) pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_DISPLACEMENT, 1, &m_pd3dcbBump);
	if (setInfo & SET_SHADER_GS) pd3dDeviceContext->GSSetConstantBuffers(CB_SLOT_DISPLACEMENT, 1, &m_pd3dcbBump);
	if (setInfo & SET_SHADER_HS) pd3dDeviceContext->HSSetConstantBuffers(CB_SLOT_DISPLACEMENT, 1, &m_pd3dcbBump);
	if (setInfo & SET_SHADER_DS) pd3dDeviceContext->DSSetConstantBuffers(CB_SLOT_DISPLACEMENT, 1, &m_pd3dcbBump);
}
