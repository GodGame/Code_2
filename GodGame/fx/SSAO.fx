#include "Define.fx"

struct VS_SCENE_INPUT
{
	float3 pos			: POSITION;
	float2 tex			: TEXCOORD;
};

struct PS_SCENE_INPUT
{
	float4 pos			: SV_POSITION;
	float2 tex			: TEXCOORD;
};

Texture2D gtxtSSAO	   : register(t25);

struct SSAO_PSIN
{
	float4 posH			: SV_POSITION;
	float3 ToFarPlane	: NORMAL;
	float2 Tex			: TEXCOORD;
};

#define NUM_SSAO_OFFSET 14

cbuffer cbPerFrame : register(b6)
{
	float4x4 gViewToTexSpace; // ���� ��� * �ؽ��� ���
	float4 gOffsetVectors[NUM_SSAO_OFFSET];
	float4 gFrustumCorners[4];
};

cbuffer ssaobuffer
{
	// ��������
	static float gOcclusionRadius = 0.5f;
	static float gOcclusionFadeStart = 0.1f;
	static float gOcclusionFadeEnd = 1.0f;
	static float gSurfaceEpsilon = 0.01f;
};
// ǥ���� q�� p�� �󸶳� ���������� distZ(���� ����)�� ���
float OcclusionFunction(float distZ)
{
	float occlusion = 0.0f;

	if (distZ > gSurfaceEpsilon)
	{
		float fadeLength = gOcclusionFadeEnd - gOcclusionFadeStart;
		// distZ�� gOcclusionFadeStart ���� gOcclusionFadeEnd�� �����Կ� ����
		// ���󵵸� 1���� 0���� ���� ����
		occlusion = saturate((gOcclusionFadeEnd - distZ) / fadeLength);
	}
	//else
	//	occlusion = 1.0f;
	return occlusion;
}

SSAO_PSIN VSSCeneSpaceAmbient(VS_SCENE_INPUT input)
{
	SSAO_PSIN output = (SSAO_PSIN)0;
	output.posH = float4(input.pos, 1.0f);
	output.ToFarPlane = gFrustumCorners[max(sign(input.tex.x), 0) + max(sign(input.tex.y * 2), 0)].xyz;
	output.Tex = input.tex;

	return output;
}

float4 PSSCeneSpaceAmbient(SSAO_PSIN input) :SV_Target
{
	// P-- ���� �ֺ��� ���� ����ϰ��� �ϴ� �ȼ��� �ش��ϴ� ��
	// n-- p������ ���� ����
	// q-- p �ֺ��� �� ������ ��(ǥ����)
	// r-- p�� ���� ���ɼ��� �ִ� ������ ������

	// �� �ȼ��� �þ� ���� ������ z ������ �����´�. ���� �������Ǵ� ȭ�� ��ü �簢�� �ؽ��� ��ǥ�� �̹� uv�ȿ� �ִ�.
	int3 uvm = int3(input.posH.xy, 0);	// (u, v, level)
	float4 normal = gtxtNormal.Load(uvm);

	// ������ �þ߰��� ��ġ (x, y, z) �籸��
	// �켱 p = t * input.ToFarPlane�� �����ϴ� t�� ���Ѵ�.
	// p.z = t * input.ToFarPlane.z ==> t = p.z / input.ToFarPlane.z
	float3 p = (normal.w / input.ToFarPlane.z) * input.ToFarPlane;
	//return p.xxx
	// ������ ���͸� �����ؼ� [0, 1] -> [-1, 1] ����
	float3 randVec = gtxtRandom.SampleLevel(gSamplerState, 4.0f * input.Tex, 0.0f).rgb;
	randVec = normalize(randVec);

	float occlusionSum = 0.0f;
	// p �ֺ��� �̿� ǥ�������� n���� �ݱ����� �����Ѵ�.
	[unroll]
	for (int i = 0; i < NUM_SSAO_OFFSET; ++i)
	{
		// �̸� ����� �� ��� ������ ���͵��� ���� ���� �Ǿ��ִ�.
		// (��, �������� ���͵��� ���� �������� �������� �ʴ�.)
		// �̵��� �ϳ��� ������ ���� �������� �ݻ��Ű�� ������ ���͵��� ����� ����.
		float3 offset = reflect(gOffsetVectors[i].xyz, randVec);

		// ������ ���Ͱ� (p, n)���� ���ǵ� ����� ������ ���ϰ� ������ �ݴ�� �����´�.
		float flip = sign(dot(offset, normal.xyz));

		// p�ֺ����� ���� ������ �̳��� ������ �� q�� �����Ѵ�.
		float3 q = p + flip * gOcclusionRadius * offset;
		//return float4(p, 1);
		// q�� �����ؼ� ���� �ؽ��� ��ǥ�� ���Ѵ�.
		float4 ProjQ = mul(float4(q, 1.0f), gViewToTexSpace);
		//ProjQ /= (ProjQ.w);// *gfDepthFar);
		//float fDepth = (ProjQ.w * gfDepthFar);
		//return float4(ProjQ.xyxy);//float4(ProjQ.x / 1280, ProjQ.y / 960, 0, 0);

		// �������� q���� ���������� ������ ���� ����� �ȼ��� ���̸� ���Ѵ�.
		// (�̰��� q�� ���̴� �ƴϴ�. q�� �׳� p ��ó�� ���� ���̸�, ��� ��ü�� �ƴ� ������� �ִ� ���� �� �ִ�.)
		// ���� ����� ���̴� ���� �ʿ��� �����Ѵ�
		float rz = gtxtNormal.Load(int3(ProjQ.xy, 0)).a;
		//occlusionSum += rz;
		//continue;
		//return rz;
		// ������ �þ� ���� ��ġ r = (rx, ry, rz)�� �籸���Ѵ�.
		// r�� q�� ������ �������� �����Ƿ�, r = t * q�� �����ϴ� t�� �����Ѵ�.
		// r.z = t*q.z ==> t = r.z / q.z
		float3 r = (rz / q.z) * q;

		//return float4(r, 1);
		/*
		r�� p�� �������� ����.
		   ���� dot(n, normalize(r - p))�� ������ ������ r�� ��� plane(p, n) �������� �󸶳� �տ� �ִ����� ��Ÿ����.
		   �� �տ� ���� ����, ������ ����ġ�� �� ũ�� ��´�. �̷��� �ϸ� r�� �ü��� ������ ��� (p, n)�� ������
		   ���� ������ ���� �� ���� ������ r�� p�� �����ٰ� �߸� �����ϴ� ��Ȳ�� �����ȴ�.

		   ���󵵴� ���� �� p�� ������ r ������ �Ÿ��� �����Ѵ�.
		   r�� p���� �ʹ� �ָ� ������ p�� ������ �ʴ� ������ �����Ѵ�.
		*/
		float distZ = p.z - r.z;
		float dp = max(dot(normal.xyz, normalize(r - p)), 0.0f);
		float occlusion = dp * OcclusionFunction(distZ);
		//return occlusion;
		occlusionSum += occlusion;
	}
	occlusionSum /= NUM_SSAO_OFFSET;

	float access = 1.0f - occlusionSum;

	//SSAO�� �� �� �ظ��� ȿ���� ������ �ŵ� ������ �̿��� ��� ��ȭ�Ѵ�.
	return saturate(pow(access, 4.0f));
}