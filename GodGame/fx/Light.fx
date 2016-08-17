#define MAX_LIGHTS		3 
#define POINT_LIGHT		1.0f
#define SPOT_LIGHT		2.0f
#define DIRECTIONAL_LIGHT	3.0f

#define _WITH_LOCAL_VIEWER_HIGHLIGHTING
#define _WITH_THETA_PHI_CONES
#define _WITH_REFLECT


//������ ���� ����ü�� �����Ѵ�.
struct MATERIAL
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular; //a = power
	float4 m_cEmissive;
};

//������ ���� ����ü�� �����Ѵ�.
struct LIGHT
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular;
	float3 m_vPosition;
	float m_fRange;
	float3 m_vDirection;
	float m_nType;
	float3 m_vAttenuation;
	float m_fFalloff;
	float m_fTheta; //cos(m_fTheta)
	float m_fPhi; //cos(m_fPhi)
	float m_bEnable;
	float padding;
};

cbuffer directWorldLight
{
	static const float4 directWolrdAmbient = float4(0.1f, 0.1f, 0.1f, 1.0f);
	static const float4 directWorldDiffsue = float4(0.3f, 0.3f, 0.3f, 1.0f);
	static const float4 directWorldSpecular = float4(0.0f, 0.0f, 0.0f, 0.0f);
	static const float3 directWorldDirection = float3(-0.707f, -0.707f, 0.0f);
};

//������ ���� ������۸� �����Ѵ�. 
cbuffer cbLight : register(b0)
{
	LIGHT gLights[MAX_LIGHTS];
	float4 gcLightGlobalAmbient;
	float4 gvCameraPosition;
};

//������ ���� ������۸� �����Ѵ�. 
cbuffer cbMaterial : register(b1)
{
	MATERIAL gMaterial;
};
cbuffer cbAmbient
{
	static float4 gAmbient = float4(0.1f, 0.1f, 0.1f, 0.1f);
};

struct LIGHTEDCOLOR
{
	float4 m_cAmbient;
	float4 m_cDiffuse;
	float4 m_cSpecular;
};

#if 0
/*���⼺ ������ ȿ���� ����ϴ� �Լ��̴�. ���⼺ ������ ��������� �Ÿ��� ���� ������ ���� ������ �ʴ´�.*/
LIGHTEDCOLOR DirectionalLight(int i, float3 vNormal, float3 vToCamera)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float3 vToLight = -gLights[i].m_vDirection;
		float fDiffuseFactor = dot(vToLight, vNormal);
	//������ ������ ���� ���Ϳ� �̷�� ������ ������ �� ���� ������ ������ ����Ѵ�.
	if (fDiffuseFactor > 0.0f)
	{
		//������ ����ŧ�� �Ŀ��� 0�� �ƴ� ���� ����ŧ�� ������ ������ ����Ѵ�.
		if (gMaterial.m_cSpecular.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
			output.m_cSpecular = gMaterial.m_cSpecular * (gLights[i].m_cSpecular * fSpecularFactor);
		}
		output.m_cDiffuse = gMaterial.m_cDiffuse * (gLights[i].m_cDiffuse * fDiffuseFactor);
	}
	output.m_cAmbient = gMaterial.m_cAmbient * gLights[i].m_cAmbient;
	return(output);
}

//�� ������ ȿ���� ����ϴ� �Լ��̴�.
LIGHTEDCOLOR PointLight(int i, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float3 vToLight = gLights[i].m_vPosition - vPosition;
		float fDistance = length(vToLight);
	//��������� �Ÿ��� ������ ��ȿ�Ÿ����� ���� ���� ������ ������ ����Ѵ�.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//������ ������ ���� ���Ϳ� �̷�� ������ ������ �� ���� ������ ������ ����Ѵ�.
		if (fDiffuseFactor > 0.0f)
		{
			//������ ����ŧ�� �Ŀ��� 0�� �ƴ� ���� ����ŧ�� ������ ������ ����Ѵ�.
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
					float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
					float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
				output.m_cSpecular = gMaterial.m_cSpecular * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = gMaterial.m_cDiffuse * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
		//��������� �Ÿ��� ���� ������ ������ ����Ѵ�.
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gMaterial.m_cAmbient * (gLights[i].m_cAmbient * fAttenuationFactor);
		output.m_cDiffuse *= fAttenuationFactor;
		output.m_cSpecular *= fAttenuationFactor;
	}
	return(output);
}

//���� ������ ȿ���� ����ϴ� �Լ��̴�.
LIGHTEDCOLOR SpotLight(int i, float3 vPosition, float3 vNormal, float3 vToCamera)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;
	float3 vToLight = gLights[i].m_vPosition - vPosition;
	float fDistance = length(vToLight);
	//��������� �Ÿ��� ������ ��ȿ�Ÿ����� ���� ���� ������ ������ ����Ѵ�.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//������ ������ ���� ���Ϳ� �̷�� ������ ������ �� ���� ������ ������ ����Ѵ�.
		if (fDiffuseFactor > 0.0f)
		{
			//������ ����ŧ�� �Ŀ��� 0�� �ƴ� ���� ����ŧ�� ������ ������ ����Ѵ�.
			if (gMaterial.m_cSpecular.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), gMaterial.m_cSpecular.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), gMaterial.m_cSpecular.a);
#endif
				output.m_cSpecular = gMaterial.m_cSpecular * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = gMaterial.m_cDiffuse * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[i].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[i].m_fPhi) / (gLights[i].m_fTheta - gLights[i].m_fPhi)), 0.0f), gLights[i].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gMaterial.m_cAmbient * (gLights[i].m_cAmbient * fAttenuationFactor * fSpotFactor);
		output.m_cDiffuse *= fAttenuationFactor * fSpotFactor;
		output.m_cSpecular *= fAttenuationFactor * fSpotFactor;
	}
	return(output);
}

float4 Lighting(float3 vPosition, float3 vNormal)
{
	int i;
	float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
		float3 vToCamera = normalize(vCameraPosition - vPosition);
		LIGHTEDCOLOR LightedColor = (LIGHTEDCOLOR)0;
	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		//Ȱ��ȭ�� ���� ���Ͽ� ������ ������ ����Ѵ�.
		if (gLights[i].m_bEnable == 1.0f)
		{
			//������ ������ ���� ������ ������ ����Ѵ�.
			if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			{
				LightedColor = DirectionalLight(i, vNormal, vToCamera);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
			}
			if (gLights[i].m_nType == POINT_LIGHT)
			{
				LightedColor = PointLight(i, vPosition, vNormal, vToCamera);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
			}
			if (gLights[i].m_nType == SPOT_LIGHT)
			{
				LightedColor = SpotLight(i, vPosition, vNormal, vToCamera);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular);
			}
		}
	}
	//�۷ι� �ֺ� ������ ������ ���� ���� ���Ѵ�.
	cColor += (gcLightGlobalAmbient * gMaterial.m_cAmbient);
	
	//���� ������ ���İ��� ������ ��ǻ�� ������ ���İ����� �����Ѵ�.
	cColor.a = gMaterial.m_cDiffuse.a;
	return(cColor);
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////

/*���⼺ ������ ȿ���� ����ϴ� �Լ��̴�. ���⼺ ������ ��������� �Ÿ��� ���� ������ ���� ������ �ʴ´�.*/
LIGHTEDCOLOR DirectionalLight(int i, float3 vNormal, float3 vToCamera, float4 vDiffuse, float4 vSpec)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float3 vToLight = -gLights[i].m_vDirection;
	float fDiffuseFactor = dot(vToLight, vNormal);
	//������ ������ ���� ���Ϳ� �̷�� ������ ������ �� ���� ������ ������ ����Ѵ�.
	if (fDiffuseFactor > 0.0f)
	{
		//������ ����ŧ�� �Ŀ��� 0�� �ƴ� ���� ����ŧ�� ������ ������ ����Ѵ�.
		if (vSpec.a != 0.0f)
		{
#ifdef _WITH_REFLECT
			float3 vReflect = reflect(-vToLight, vNormal);
			float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), vSpec.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
			float3 vHalf = normalize(vToCamera + vToLight);
#else
			float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
			float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), vSpec.a);
#endif
			output.m_cSpecular = vSpec * (directWorldSpecular * fSpecularFactor);
		}
		output.m_cDiffuse = float4(vDiffuse.rgb, 1) * (directWorldDiffsue * fDiffuseFactor);
	}
	output.m_cAmbient = directWolrdAmbient; // gAmbient *
	return(output);
}

//�� ������ ȿ���� ����ϴ� �Լ��̴�.
LIGHTEDCOLOR PointLight(int i, float3 vToLight, float3 vNormal, float3 vToCamera, float4 vDiffuse, float4 vSpec)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float fDistance = length(vToLight);
	//��������� �Ÿ��� ������ ��ȿ�Ÿ����� ���� ���� ������ ������ ����Ѵ�.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//������ ������ ���� ���Ϳ� �̷�� ������ ������ �� ���� ������ ������ ����Ѵ�.
		if (fDiffuseFactor > 0.0f)
		{
			//������ ����ŧ�� �Ŀ��� 0�� �ƴ� ���� ����ŧ�� ������ ������ ����Ѵ�.
			if (vSpec.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), vSpec.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
				float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), vSpec.a);
#endif
				output.m_cSpecular = vSpec * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = float4(vDiffuse.rgb, 1) * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
		//��������� �Ÿ��� ���� ������ ������ ����Ѵ�.
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gAmbient * (gLights[i].m_cAmbient * fAttenuationFactor);
		output.m_cDiffuse *= fAttenuationFactor;
		output.m_cSpecular *= fAttenuationFactor;
	}
	return(output);
}

//���� ������ ȿ���� ����ϴ� �Լ��̴�.
LIGHTEDCOLOR SpotLight(int i, float3 vToLight, float3 vNormal, float3 vToCamera, float4 vDiffuse, float4 vSpec)
{
	LIGHTEDCOLOR output = (LIGHTEDCOLOR)0;

	float fDistance = length(vToLight);
	//��������� �Ÿ��� ������ ��ȿ�Ÿ����� ���� ���� ������ ������ ����Ѵ�.
	if (fDistance <= gLights[i].m_fRange)
	{
		vToLight /= fDistance;
		float fDiffuseFactor = dot(vToLight, vNormal);
		//������ ������ ���� ���Ϳ� �̷�� ������ ������ �� ���� ������ ������ ����Ѵ�.
		if (fDiffuseFactor > 0.0f)
		{
			//������ ����ŧ�� �Ŀ��� 0�� �ƴ� ���� ����ŧ�� ������ ������ ����Ѵ�.
			if (vSpec.a != 0.0f)
			{
#ifdef _WITH_REFLECT
				float3 vReflect = reflect(-vToLight, vNormal);
				float fSpecularFactor = pow(max(dot(vReflect, vToCamera), 0.0f), vSpec.a);
#else
#ifdef _WITH_LOCAL_VIEWER_HIGHLIGHTING
				float3 vHalf = normalize(vToCamera + vToLight);
#else
				float3 vHalf = float3(0.0f, 1.0f, 0.0f);
#endif
					float fSpecularFactor = pow(max(dot(vHalf, vNormal), 0.0f), vSpec.a);
#endif
				output.m_cSpecular = vSpec * (gLights[i].m_cSpecular * fSpecularFactor);
			}
			output.m_cDiffuse = float4(vDiffuse.rgb, 1) * (gLights[i].m_cDiffuse * fDiffuseFactor);
		}
#ifdef _WITH_THETA_PHI_CONES
		float fAlpha = max(dot(-vToLight, gLights[i].m_vDirection), 0.0f);
		float fSpotFactor = pow(max(((fAlpha - gLights[i].m_fPhi) / (gLights[i].m_fTheta - gLights[i].m_fPhi)), 0.0f), gLights[i].m_fFalloff);
#else
		float fSpotFactor = pow(max(dot(-vToLight, gLights[i].m_vDirection), 0.0f), gLights[i].m_fFalloff);
#endif
		float fAttenuationFactor = 1.0f / dot(gLights[i].m_vAttenuation, float3(1.0f, fDistance, fDistance*fDistance));
		output.m_cAmbient = gAmbient * (gLights[i].m_cAmbient * fAttenuationFactor * fSpotFactor);
		output.m_cDiffuse *= fAttenuationFactor * fSpotFactor;
		output.m_cSpecular *= fAttenuationFactor * fSpotFactor;
	}
	return(output);
}

float4 HemisphericLight(float3 vNormal, float3 vPos)
{
	const static float4 cModel = { 1.0f, 1.0f, 1.0f, 1.0f };
	const static float4 cGround = { 0.2f, 0.2f, 0.2f, 1.0f };
	const static float3 vFromSky = { 0.0f, 1.0f, 0.0f };

	//float3 normal = normalize(vNormal);
	float3 fromCamera = normalize(vPos - gvCameraPosition.xyz);
	//float3 vReflected = reflect(vNormal, fromCamera);

	//return (0.9f * lerp(cGround, cModel, dot(vNormal, vFromSky) * 0.5f + 0.5f));
	return (0.9f * lerp(cGround, cModel, dot(-vNormal, fromCamera) * 0.5f + 0.5f));
}

float CookTorrenceSF(float3 vNormal,float3 vToCamera, float3 vToLight, float fm, float fFRI)
{
	float3 N = normalize(vNormal);
	float3 L = normalize(vToLight);
	float3 E = normalize(vToCamera);
	float3 H = normalize(vToLight + vToCamera);

	float NH = saturate(dot(N, H));
	float EH = saturate(dot(E, H));
	float NE = saturate(dot(N, E));
	float NL = saturate(dot(N, L));
	
	float NH2 = NH * NH;
	float m2 = fm * fm;

	float D = (0.25f * m2 * NH2 * NH2) * (exp(-((1 - NH2) / (m2 * NH2))));
	float G = min(1.0f, min((2 * NH * NL) / EH, (2 * NH * NE) / EH));
	float F = fFRI + (1 - fFRI) * pow((1 - NE), 5.0f);
	float fSF = (F * D * G) / (3.1415926535 * NL * NE);

	return fSF;
}

float4 Lighting(float3 vPos, float3 vNormal, float4 vDiff, float4 vSpecular)
{
	int i;
	float4 vDiffuse = vDiff;
	float fShadowFactor = vDiff.a;
	float4 vSpec = float4(vSpecular.xyz, vSpecular.w);
//	float3 vCameraPosition = gvCameraPosition.xyz;
	float3 vToCamera = normalize(gvCameraPosition.xyz - vPos);
	LIGHTEDCOLOR LightedColor = (LIGHTEDCOLOR)0;

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	[unroll]
	for (i = 0; i < MAX_LIGHTS; i++)
	{
		//Ȱ��ȭ�� ���� ���Ͽ� ������ ������ ����Ѵ�.
		if (gLights[i].m_bEnable == 1.0f)
		{
			float3 vToLight = gLights[i].m_vPosition - vPos;
			//vSpec.w = CookTorrenceSF(vNormal, vToCamera, vToLight, 0.85f, 0.01f);
			//vSpec.w = max(0.5, vSpec.w);
			//vSpec.w = 1.0f;

			//������ ������ ���� ������ ������ ����Ѵ�.
			//if (gLights[i].m_nType == DIRECTIONAL_LIGHT)
			//{
			//	LightedColor = DirectionalLight(i, vNormal, vToCamera, vDiffuse, vSpec);
			//	cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);
			//}
			if (gLights[i].m_nType == POINT_LIGHT)
			{
				LightedColor = PointLight(i, vToLight, vNormal, vToCamera, vDiffuse, vSpec);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);
			}
			else if (gLights[i].m_nType == SPOT_LIGHT)
			{
				LightedColor = SpotLight(i, vToLight, vNormal, vToCamera, vDiffuse, vSpec);
				cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse + LightedColor.m_cSpecular );
			}
		}
	}

	LightedColor = DirectionalLight(0, vNormal, vToCamera, vDiffuse, vSpec);
	cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);
	//�۷ι� �ֺ� ������ ������ ���� ���� ���Ѵ�.
	cColor += (gcLightGlobalAmbient * vDiffuse * fShadowFactor/** HemisphericLight( vNormal, vPos)*//*gAmbient*/ );
	//cColor *= HemisphericLight(vNormal, vPos);
	//���� ������ ���İ��� ������ ��ǻ�� ������ ���İ����� �����Ѵ�.
	//cColor.a = gMaterial.m_cDiffuse.a;
	return(cColor);
}

float4 DirectLighting(float3 vPos, float3 vNormal, float4 vDiff, float4 vSpecular)
{
	float4 vDiffuse = vDiff;
	float fShadowFactor = vDiff.a;
	float4 vSpec = float4(vSpecular.xyz, vSpecular.w * 255.0f);
	//float3 vCameraPosition = float3(gvCameraPosition.x, gvCameraPosition.y, gvCameraPosition.z);
	float3 vToCamera = normalize(gvCameraPosition.xyz - vPos);
	LIGHTEDCOLOR LightedColor = (LIGHTEDCOLOR)0;

	float4 cColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	LightedColor = DirectionalLight(0, vNormal, vToCamera, vDiffuse, vSpec);
	cColor += (LightedColor.m_cAmbient + LightedColor.m_cDiffuse  * fShadowFactor + LightedColor.m_cSpecular * fShadowFactor);

	return cColor;
}


float3 NoStripAverageVertexNormal()
{
	float3 result = float3(0, 0, 0);
	return result;
}

float4 ToneMapping(float4 LinearColor)
{
	return (LinearColor * (6.2 * LinearColor + 0.5)) / (LinearColor * (6.2 * LinearColor + 1.7) + 0.06);
}

float4 GammaToneMapping(float4 GammaColor)
{
	float4 color = max(0, pow(GammaColor, 2.2) - 0.004);
	color = (color * (6.2 * color + 0.5)) / (color * (6.2 * color + 1.7) + 0.06);
	return color;
}
