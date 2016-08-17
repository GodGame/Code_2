Texture2D     gtxtTexture            : register(t0);
SamplerState  gSamplerState          : register(s0);
			  
Texture2D     gtxtDetailTexture      : register(t1);
SamplerState  gDetailSamplerState    : register(s1);
			  
Texture2D     gtxtSlpatDetail        : register(t2);
			  
Texture2D     gtxtGridNormalMap       : register(t2);
Texture2D     gtxtGridDisplaceMap     : register(t3);


TextureCube   gtxtSkyBox             : register(t2);
SamplerState  gssSkyBox              : register(s2);
			  
Texture2D     gtxtSFTexture          : register(t2);
			  
//Texture2D   gtxtResult             : register(t16);
Texture2D     gtxtTxColor            : register(t17);
Texture2D     gtxtPos                : register(t18);
Texture2D     gtxtDiffuse            : register(t19);	// ���� ��ǻ�� �Ϲ��� �ؽ��� ����
Texture2D     gtxtSpecular           : register(t20);	// ���� ����ŧ��
Texture2D     gtxtNormal             : register(t21);
Texture1D     gtxtRandom             : register(t22);

Texture2DArray gTextureArray		 : register(t10);

#define	FRAME_BUFFER_WIDTH		1280
#define	FRAME_BUFFER_HEIGHT		960


//ī�޶� ��ȯ ��İ� ���� ��ȯ ����� ���� ���̴� ������ �����Ѵ�(���� 0�� ���).
cbuffer cbViewProjectionMatrix : register(b0)
{
	matrix gmtxViewProjection;
	float4 gf3CameraPos;

	static float gfCameraFar = 1000.0f;
	static float gfDepthFar = 0.001f;
};

//���� ��ȯ ����� ���� ���̴� ������ �����Ѵ�(���� 1�� ���).
cbuffer cbWorldMatrix : register(b1)
{
	matrix gmtxWorld : packoffset(c0);
};

cbuffer cbTerrain
{
	static int gWorldCell     = 256;
	static int gHegiht        = 512;
	static float gCameraMax   = 500.0f;
	static float gCameraMin   = 20.0f;

	static float gScaleHeight = 32.0f;
};

cbuffer cbFixed
{
	static float  gFogStart        = 20.0f;
	static float  gFogRangeInverse = 1 / 400.0f;
	static float4 gFogColor = { 0.1, 0.1, 0.2, 0.0 };
	static float4 vZero     = { 0, 0, 0, 0 };
	static float4 vOne      = { 1, 1, 1, 1 };
};

cbuffer cbQuad 
{
	static float2 gvQuadTexCoord[4] = { float2(0.0f, 1.0f), float2(0.0f, 0.0f), float2(1.0f, 1.0f), float2(1.0f, 0.0f) };
};

#define COLOR_NONE			-1
#define COLOR_WHITE			0
#define COLOR_RED			1
#define COLOR_GREEN			2
#define COLOR_BLUE			3
#define COLOR_BLACK			4
#define COLOR_YELLOW		5
#define COLOR_MAGENT		6
#define COLOR_CYAN			7
#define COLOR_GRAY			8
#define COLOR_BLACK_GRAY    9

cbuffer gcColor
{
	//static float4 gcWhite =  { 1, 1, 1, 1 };
	//static float4 gcRed =    { 1, 0, 0, 1 };
	//static float4 gcGreen =  { 0, 1, 0, 1 };
	//static float4 gcBlue =   { 0, 0, 1, 1 };
	//static float4 gcCyan =   { 0, 1, 1, 1 };
	//static float4 gcYellow = { 1, 1, 0, 1 };
	//static float4 gcMagent = { 1, 0, 1, 1 };
	//static float4 gcGray   = { 0.5, 0.5, 0.5, 1 };
	static float4 gcColors[10] =
	{
		{ 1, 1, 1, 1 },
		{ 1, 0, 0, 1 },
		{ 0, 1, 0, 1 },
		{ 0, 0, 1, 1 },
		{ 0.01, 0.01, 0.01, 1 },
		{ 1, 1, 0, 1 },
		{ 1, 0, 1, 1 },
		{ 0, 1, 1, 1 },
		{ 0.7, 0.7, 0.7, 1 },
		{ 0.2, 0.2, 0.2, 1 },
	};
};

cbuffer cbDisplacement : register(b3)
{
	float3 gBumpScale;
	float  gnBumpMax;
};

cbuffer cbStaticShadow : register(b5)
{
	matrix gmtxStaticShadowTransform : packoffset(c0);
}

cbuffer cbDynamicShadow : register(b6)
{
	matrix gmtxDynamicShadowTransform : packoffset(c0);
}

/*(����) register(b0)���� b�� �������Ͱ� ��� ���۸� ���� ���Ǵ� ���� �ǹ��Ѵ�. 0�� ���������� ��ȣ�̸�
���� ���α׷����� ��� ���۸� ����̽� ���ؽ�Ʈ�� ������ ���� ���� ��ȣ�� ��ġ�ϵ��� �ؾ� �Ѵ�.
pd3dDeviceContext->VSSetConstantBuffers(CB_SLOT_WORLD_MATRIX, 1, &m_pd3dcbWorldMatrix);*/
//����-���̴��� ����� ���� ����ü�̴�.

struct PS_MRT_COLOR_OUT
{
	// �ִ� 8�� ����
	float4 color  : SV_Target0;
	float4 zDepth : SV_Target1;
	float4 colorR : SV_Target2;
	float4 colorG : SV_Target3;
	float4 colorB : SV_Target4;
};

struct PS_MRT_OUT
{
	// �ִ� 8�� ����
	float4 vTxColor : SV_Target0;
	float4 vPos		: SV_Target1;
	float4 vDiffuse : SV_Target2;
	float4 vSpec	: SV_Target3;
	float4 vNormal  : SV_Target4;
	//float4 vDepth : SV_Target4;
};

float4 FogColor(float4 color, float flerp)
{
	return lerp(color, gFogColor, flerp);
}

float4 FogExp(float4 color, float distance, float fFogDestiny)
{
	float fRate = 1.1f - saturate((distance - gFogStart) * gFogRangeInverse);
	float f = exp(-(fRate * fFogDestiny));
	return lerp(color, gFogColor, f);
}

float4 FogLerp(float4 color, float distance)
{
	float f = saturate((distance - gFogStart) * gFogRangeInverse);
	return lerp(color, gFogColor, f);
}

float4 FogLerpSq(float4 color, float distanceSq)
{
	float f = saturate((distanceSq - gFogStart * gFogStart) * gFogRangeInverse * gFogRangeInverse);
	return lerp(color, gFogColor, f);
}

