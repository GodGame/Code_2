#include "Common.fx"
#include "Light.fx"

//#define LUMCOLOR

struct VS_INPUT
{
	float3	position	: POSITION;
	//float4 color		: COLOR;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 color : COLOR0;
};

struct VS_INSTANCED_COLOR_INPUT
{
	float3 position                    : POSITION;
	float4 color                       : COLOR0;
	column_major float4x4 mtxTransform : INSTANCEPOS;
	float4 instanceColor               : INSTANCECOLOR;
};

struct VS_INSTANCED_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float4 color                       : COLOR0;
	//�ý��� ���� ������ ���� ���̴��� ���޵Ǵ� ��ü �ν��Ͻ��� ID�� �ȼ� ���̴��� �����Ѵ�.
	float4 instanceID                  : INDEX;
};

//������ ������ ���� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_DIFFUSED_COLOR_INPUT
{
	float3 position                    : POSITION;
	float4 color                       : COLOR0;
};

//������ ������ ���� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_DIFFUSED_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float4 color                       : COLOR0;
};

//�ν��Ͻ��� �ϸ鼭 ������ ������ ���� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_DIFFUSED_COLOR_INPUT
{
	float3 position                    : POSITION;
	float4 color                       : COLOR0;
	float4x4 mtxTransform              : INSTANCEPOS;
};

struct VS_INSTANCED_DIFFUSED_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float4 positionW                   : POSITION;
	float4 color                       : COLOR0;
};

//������ ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_LIGHTING_COLOR_INPUT
{
	float3 position                    : POSITION;
	float3 normal                      : NORMAL;
};

//������ ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_LIGHTING_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	//������ǥ�迡�� ������ ��ġ�� ���� ���͸� ��Ÿ����.
	float3 positionW                   : POSITION;
	float3 normalW                     : NORMAL;
};

//�ν��Ͻ��� �ϸ鼭 ������ ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_LIGHTING_COLOR_INPUT
{
	float3 position                    : POSITION;
	float3 normal                      : NORMAL;
	float4x4 mtxTransform              : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 ������ ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_LIGHTING_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float3 positionW                   : POSITION;
	float3 normalW                     : NORMAL;
};

//�ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_COLOR_INPUT
{
	float3 position                    : POSITION;
	float2 texCoord                    : TEXCOORD0;
};

//�ؽ��ĸ� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	//float4 shadowPos                 : SHADOW;
	float3 posW		                   : POSITION;
	float2 texCoord                    : TEXCOORD0;
};

// ��ī�̹ڽ� ���� ���̴�
struct VS_SKYBOX_CUBEMAP_INPUT
{
	float3	position                   : POSITION;
};

struct VS_SKYBOX_CUBEMAP_OUTPUT
{
	float3	positionL                  : POSITION;
	float4	position                   : SV_POSITION;
};

//�ν��Ͻ��� �ϸ鼭 �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_COLOR_INPUT
{
	float3 position                    : POSITION;
	float2 texCoord                    : TEXCOORD0;
	float4x4 mtxTransform              : INSTANCEPOS;
};

//�ν��Ͻ��� �ϸ鼭 �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float2 texCoord                    : TEXCOORD0;
};
//-------------------------------------------------------------------------------------------------------------------------
//������ �ؽ��ĸ� ����ϴ� ��� ���� ���̴��� �Է°� ����� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_COLOR_INPUT
{
	float3 position                    : POSITION;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordDetail              : TEXCOORD1;
};

struct VS_DETAIL_TEXTURED_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordDetail              : TEXCOORD1;
};

//-------------------------------------------------------------------------------------------------------------------------
//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position                    : POSITION;
	float3 normal                      : NORMAL;
	float2 texCoord                    : TEXCOORD0;
};

//�ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float3 positionW                   : POSITION;
	//float4 shadowPos                 : SHADOW;
	float3 normalW                     : NORMAL;
	float2 texCoord                    : TEXCOORD0;
};
//--------------------------------------------------------------------------------------------------------------------
struct VS_DETAIL_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position                    : POSITION;
	float3 normal                      : NORMAL;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordDetail              : TEXCOORD1;
};

//������ �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_DETAIL_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float3 positionW                   : POSITION;
	float3 normalW                     : NORMAL;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordDetail              : TEXCOORD1;
};

struct VS_SPLAT_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position                    : POSITION;
	float3 normal                      : NORMAL;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordAlpha               : TEXCOORD1;
};

struct VS_SPLAT_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float3 positionW                   : POSITION;
	//float4 shadowPos                 : SHADOW;
	float3 normalW                     : NORMAL;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordAlpha               : TEXCOORD1;
};

struct VS_DETAIL_NOT_NORMAL_INPUT
{
	float3 position                    : POSITION;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordAlpha               : TEXCOORD1;
};

struct VS_DETAIL_NOT_NORMAL_OUTPUT
{
	float4 position                    : SV_POSITION;
	float3 positionW                   : POSITION;
	float2 texCoordBase                : TEXCOORD0;
	float2 texCoordAlpha               : TEXCOORD1;
};

//--------------------------------------------------------------------------------------------------------------------
//�ν��Ͻ�, �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� �Է��� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_LIGHTING_COLOR_INPUT
{
	float3 position                    : POSITION;
	float3 normal                      : NORMAL;
	float2 texCoord                    : TEXCOORD0;
	float4x4 mtxTransform              : INSTANCEPOS;
};

//�ν��Ͻ�, �ؽ��Ŀ� ������ ���� ����ϴ� ��� ���� ���̴��� ����� ���� ����ü�̴�.
struct VS_INSTANCED_TEXTURED_LIGHTING_COLOR_OUTPUT
{
	float4 position                    : SV_POSITION;
	float3 positionW                   : POSITION;
	float3 normalW                     : NORMAL;
	float2 texCoord                    : TEXCOORD0;
};
////////////////////////////////////////////////////////////////////////////////////////////////////

struct FLOAT3_POS
{
	float3 pos : POSITION;
};

struct HCS_EDGE4_IN2
{
	float fTessEdges[4]   : SV_TessFactor;
	float fTessInsides[2] : SV_InsideTessFactor;
};

struct DS_BEZIER_OUTPUT
{
	float4 pos : SV_POSITION;
};

float4 BernsteinCoefficient(float t)
{
	float tlnv = 1.0f - t;
	return float4(tlnv * tlnv * tlnv, 3.0f * t * tlnv * tlnv, 3.0f * t * t * tlnv, t* t* t);
}

struct FLOAT3_POS_FLOAT2_TEX
{
	float3 pos : POSITION;
	float2 tex : TEXCOORD0;
};

float3 CubicBezierSum(OutputPatch<FLOAT3_POS_FLOAT2_TEX, 16> patch, float4 u, float4 v)
{
	float3 sum = float3(0.0f, 0.0f, 0.0f);
	sum = v.x * (u.x * patch[0].pos + u.y * patch[1].pos + u.z * patch[2].pos + u.w * patch[3].pos);
	sum += v.y * (u.x * patch[4].pos + u.y * patch[5].pos + u.z * patch[6].pos + u.w * patch[7].pos);
	sum += v.z * (u.x * patch[8].pos + u.y * patch[9].pos + u.z * patch[10].pos + u.w * patch[11].pos);
	sum += v.w * (u.x * patch[12].pos + u.y * patch[13].pos + u.z * patch[14].pos + u.w * patch[15].pos);
	return sum;
}

///////////////////////////////////////////////////

float CalculateTessFactor(float3 p)
{
	float fDistToCamera = distance(p, gf3CameraPos);
	float s = saturate((fDistToCamera - gCameraMin) / (gCameraMax - gCameraMin));
	return pow(2, lerp(6.0f, 2.0f, s));
}

#define PSNORMAL

struct DETAIL_TERRAIN
{
	float4 posH : SV_POSITION;
	float3 posW : POSITION;
	float2 tex  : TEXCOORD0;
	float2 texDetail : TEXCOORD1;
#ifdef DSNORMAL
	float3 normalW: NORMAL;
#endif
};

//////////////////////////////////////////////////////////

struct MODEL_NORMALMAP
{
	float3 pos            : POSITION;
	float2 tex            : TEXCOORD;
	float3 normal         : NORMAL;
	float3 tangent        : TANGENT;
};

struct WORLD_NORMALMAP
{
	float3 posW           : POSITION;
	float2 tex            : TEXCOORD;
	float3 normalW        : NORMAL;
	float3 tangentW       : TANGENT;
};

struct PS_WORLD_NORMALMAP
{
	float4 pos            : SV_POSITION;
	float3 posW           : POSITION;
	//float4 shadowPos    : SHADOW;
	float3 normalW        : NORMAL;
	float3 tangentW       : TANGENT;
	float2 tex            : TEXCOORD;
};

struct HCS_EDGE3_IN1
{
	float fTessEdges[3]   : SV_TessFactor;
	float fTessInsides[1] : SV_InsideTessFactor;
};


struct VS_NORMALMAP_INST_STATIC
{
	float3 pos            : POSITION;
	float2 tex            : TEXCOORD;
	float3 normal         : NORMAL;
	float3 tangent        : TANGENT;
	float4 instanceW      : INSTANCE;
};
