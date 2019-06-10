#define NODE_WRITE
#include "include.hlsli"

#ifndef LIGHT_COUNT
#define LIGHT_COUNT 8
#endif

#ifndef FVF_TEXCOUNT
	#define FVF_TEXCOUNT 0
#endif

struct Material
{
	float4 Diffuse;  /* Diffuse color RGBA */
	float4 Ambient;  /* Ambient color RGB */
	float4 Specular; /* Specular 'shininess' */
	float4 Emissive; /* Emissive color RGB */
	float  Power;    /* Sharpness if specular highlight */
};

struct MaterialSources
{
	uint diffuse;  /* Diffuse material source. */
	uint specular; /* Specular material source. */
	uint ambient;  /* Ambient material source. */
	uint emissive; /* Emissive material source. */
};

struct Light
{
	bool   Enabled;
	uint   Type;         /* Type of light source */
	float4 Diffuse;      /* Diffuse color of light */
	float4 Specular;     /* Specular color of light */
	float4 Ambient;      /* Ambient color of light */
	float3 Position;     /* Position in world space */
	float3 Direction;    /* Direction in world space */
	float  Range;        /* Cutoff range */
	float  Falloff;      /* Falloff */
	float  Attenuation0; /* Constant attenuation */
	float  Attenuation1; /* Linear attenuation */
	float  Attenuation2; /* Quadratic attenuation */
	float  Theta;        /* Inner angle of spotlight cone */
	float  Phi;          /* Outer angle of spotlight cone */
};

struct TextureStage
{
	uint  colorOp;
	uint  colorArg1;             // D3DTA
	uint  colorArg2;             // D3DTA
	uint  alphaOp;
	uint  alphaArg1;             // D3DTA
	uint  alphaArg2;             // D3DTA
	float bumpEnvMat00;
	float bumpEnvMat01;
	float bumpEnvMat10;
	float bumpEnvMat11;
	uint  texCoordIndex;         // D3DTSS_TCI
	float bumpEnvLScale;
	float bumpEnvLOffset;
	uint  textureTransformFlags;
	uint  colorArg0;             // D3DTA
	uint  alphaArg0;             // D3DTA
};

#define MAKE_TEXN(N) \
	float2 tex ## N : TEXCOORD ## N

struct VS_INPUT
{
	float4 position : POSITION;

#ifdef FVF_NORMAL
	float3 normal   : NORMAL;
#endif

#ifdef FVF_DIFFUSE
	float4 diffuse : COLOR0;
#endif

#ifdef FVF_SPECULAR
	float4 specular : COLOR1;
#endif

#if FVF_TEXCOUNT >= 1
	MAKE_TEXN(0);
#endif
#if FVF_TEXCOUNT >= 2
	MAKE_TEXN(1);
#endif
#if FVF_TEXCOUNT >= 3
	MAKE_TEXN(2);
#endif
#if FVF_TEXCOUNT >= 4
	MAKE_TEXN(3);
#endif
#if FVF_TEXCOUNT >= 5
	MAKE_TEXN(4);
#endif
#if FVF_TEXCOUNT >= 6
	MAKE_TEXN(5);
#endif
#if FVF_TEXCOUNT >= 7
	MAKE_TEXN(6);
#endif
#if FVF_TEXCOUNT >= 8
	MAKE_TEXN(7);
#endif
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 ambient  : COLOR0;
	float4 diffuse  : COLOR1;
	float4 specular : COLOR2;
	float4 emissive : COLOR3;
	float2 depth    : DEPTH;
	float  fog      : FOG;

#if FVF_TEXCOUNT >= 1
	MAKE_TEXN(0);
#endif
#if FVF_TEXCOUNT >= 2
	MAKE_TEXN(1);
#endif
#if FVF_TEXCOUNT >= 3
	MAKE_TEXN(2);
#endif
#if FVF_TEXCOUNT >= 4
	MAKE_TEXN(3);
#endif
#if FVF_TEXCOUNT >= 5
	MAKE_TEXN(4);
#endif
#if FVF_TEXCOUNT >= 6
	MAKE_TEXN(5);
#endif
#if FVF_TEXCOUNT >= 7
	MAKE_TEXN(6);
#endif
#if FVF_TEXCOUNT >= 8
	MAKE_TEXN(7);
#endif
};

cbuffer PerSceneBuffer : register(b0)
{
	matrix viewMatrix;
	matrix projectionMatrix;
	float2 screenDimensions;
	float3 viewPosition;
};

cbuffer PerModelBuffer : register(b1)
{
	matrix worldMatrix;
	matrix wvMatrixInvT;
	matrix textureMatrix;

	MaterialSources materialSources;
	float4 globalAmbient;
	bool colorVertex;

	Light lights[LIGHT_COUNT];
	Material material;
};

cbuffer PerPixelBuffer : register(b2)
{
	uint   fogMode;
	float  fogStart;
	float  fogEnd;
	float  fogDensity;
	float4 fogColor;

	bool  alphaReject;
	uint  alphaRejectMode;
	float alphaRejectThreshold;
};

cbuffer TextureStages : register(b3)
{
	TextureStage textureStages[TEXTURE_STAGE_COUNT];
}

Texture2D<float4> DiffuseMap     : register(t0);
SamplerState      DiffuseSampler : register(s0);

// From FixedFuncEMU.fx
// Copyright (c) 2005 Microsoft Corporation. All rights reserved.
// Calculates fog factor based upon distance
float CalcFogFactor(float d)
{
	float fogCoeff = 1.0;

	if (FOGMODE_LINEAR == fogMode)
	{
		fogCoeff = (fogEnd - d) / (fogEnd - fogStart);
	}
	else if (FOGMODE_EXP == fogMode)
	{
		fogCoeff = 1.0 / pow(E, d*fogDensity);
	}
	else if (FOGMODE_EXP2 == fogMode)
	{
		fogCoeff = 1.0 / pow(E, d * d * fogDensity * fogDensity);
	}

	return clamp(fogCoeff, 0, 1);
}

float4 white_not_lit(float4 color)
{
#ifdef RS_LIGHTING
	return color;
#else
	return float4(1, 1, 1, 1);
#endif
}

float4 black_not_lit(float4 color)
{
#ifdef RS_LIGHTING
	return color;
#else
	return float4(0, 0, 0, 0);
#endif
}

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT result = (VS_OUTPUT)0;

	float4 worldPosition = (float4)0;

#ifdef FVF_RHW
	float4 p = input.position;
	float2 screen = screenDimensions - 0.5;

	p.xy = round(p.xy);
	p.x = ((p.x / screen.x) * 2) - 1;
	p.y = -(((p.y / screen.y) * 2) - 1);

	result.position = p;
#else
	input.position.w = 1;
	result.position = mul(worldMatrix, input.position);

	worldPosition = result.position;

	result.position = mul(viewMatrix, result.position);

	result.fog = result.position.z;

	result.position = mul(projectionMatrix, result.position);
#endif

	result.depth = result.position.zw;

	if (colorVertex == true)
	{
	#ifndef RS_LIGHTING
		#ifdef FVF_DIFFUSE
			result.diffuse = input.diffuse;
		#else
			result.diffuse = float4(1, 1, 1, 1);
		#endif

		#if defined(RS_SPECULAR) && defined(FVF_SPECULAR)
			result.specular = input.specular;
		#else
			result.specular = float4(0, 0, 0, 0);
		#endif

	#else
		switch (materialSources.ambient)
		{
			case CS_MATERIAL:
				result.ambient = black_not_lit(material.Ambient);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.ambient = input.diffuse;
			#else
				result.ambient = black_not_lit(material.Ambient);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.ambient = input.specular;
			#else
				result.ambient = black_not_lit(material.Ambient);
			#endif
				break;

			default:
				result.ambient = float4(1, 0, 0, 1);
				break;
		}

		switch (materialSources.diffuse)
		{
			case CS_MATERIAL:
				result.diffuse = white_not_lit(material.Diffuse);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.diffuse = input.diffuse;
			#else
				result.diffuse = white_not_lit(material.Diffuse);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.diffuse = input.specular;
			#else
				result.diffuse = white_not_lit(material.Diffuse);
			#endif
				break;

			default:
				result.diffuse = float4(1, 0, 0, 1);
				break;
		}

		switch (materialSources.specular)
		{
			case CS_MATERIAL:
				result.specular = black_not_lit(material.Specular);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.specular = input.diffuse;
			#else
				result.specular = black_not_lit(material.Specular);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.specular = input.specular;
			#else
				result.specular = black_not_lit(material.Specular);
			#endif
				break;

			default:
				result.specular = float4(1, 0, 0, 1);
				break;
		}

		switch (materialSources.emissive)
		{
			case CS_MATERIAL:
				result.emissive = black_not_lit(material.Emissive);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.emissive = input.diffuse;
			#else
				result.emissive = black_not_lit(material.Emissive);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.emissive = input.specular;
			#else
				result.emissive = black_not_lit(material.Emissive);
			#endif
				break;

			default:
				result.emissive = float4(1, 0, 0, 1);
				break;
		}
	#endif
	}
	else
	{
		result.diffuse = white_not_lit(material.Diffuse);
		result.specular = black_not_lit(material.Specular);
	}

#if defined(RS_LIGHTING) && defined(FVF_NORMAL) && !defined(FVF_RHW)
	float4 ambient  = float4(0, 0, 0, 0);
	float4 diffuse  = float4(0, 0, 0, 0);
	float4 specular = float4(0, 0, 0, 0);

	float4 Ca = result.ambient;
	float4 Cd = result.diffuse;
	float4 Cs = result.specular;

	float3 N = normalize(mul((float3x3)worldMatrix, input.normal));

	#ifdef RS_SPECULAR
		float P = max(0, material.Power);
		float3 viewDir = normalize(viewPosition - worldPosition.xyz);
	#endif

	for (uint i = 0; i < LIGHT_COUNT; ++i)
	{
		if (lights[i].Enabled == false)
		{
			continue;
		}

		float Atten;
		float Spot;

		switch (lights[i].Type)
		{
			default:
				Atten = 1.0f;
				Spot = 1.0f;
				break;

			case LIGHT_POINT:
			{
				float d = length(worldPosition.xyz - lights[i].Position);

				if (d <= lights[i].Range)
				{
					float d2 = d * d;
					Atten = 1.0f / (lights[i].Attenuation0 + lights[i].Attenuation1 * d + lights[i].Attenuation2 * d2);
				}
				else
				{
					Atten = 0.0f;
				}

				Spot = 1.0f;
				break;
			}

			case LIGHT_SPOT:
			{
				float3 Ldcs = normalize(-mul(mul((float3x3)worldMatrix, lights[i].Direction), (float3x3)viewMatrix));
				float3 Ldir = normalize(worldPosition.xyz - lights[i].Position);

				float rho = dot(Ldcs, Ldir);

				float theta      = clamp(lights[i].Theta, 0.0f, M_PI);
				float theta2     = theta / 2.0f;
				float cos_theta2 = cos(theta2);

				float phi      = clamp(lights[i].Phi, theta, M_PI);
				float phi2     = phi / 2.0f;
				float cos_phi2 = cos(phi2);

				float falloff  = lights[i].Falloff;

				if (rho > cos_theta2)
				{
					Spot = 1.0f;
				}
				else if (rho <= cos_phi2)
				{
					Spot = 0.0f;
				}
				else
				{
					Spot = pow(max(0, (cos_theta2 - cos_phi2) / (rho - cos_phi2)), falloff);
				}

				break;
			}
		}

		float4 Ld       = lights[i].Diffuse;
		float3 Ldir     = normalize(-lights[i].Direction);
		float  NdotLdir = saturate(dot(N, Ldir));

		// Diffuse Lighting = sum[Cd*Ld*(N.Ldir)*Atten*Spot]
		diffuse += Cd * Ld * NdotLdir * Atten * Spot;

		// sum(Atteni*Spoti*Lai)
		ambient += Atten * Spot * lights[i].Ambient;

		#ifdef RS_SPECULAR
			float4 Ls = lights[i].Specular;
			float3 H = normalize(viewDir + normalize(Ldir));
			specular += Ls * pow(dot(N, H), P) * Atten * Spot;
		#endif
	}

	// Ambient Lighting = Ca*[Ga + sum(Atteni*Spoti*Lai)]
	ambient = saturate(Ca * saturate(globalAmbient + saturate(ambient)));

	/*
		Diffuse components are clamped to be from 0 to 255, after all lights are processed and interpolated separately.
		The resulting diffuse lighting value is a combination of the ambient, diffuse and emissive light values.
	*/
	diffuse = saturate(diffuse);

	#ifdef RS_SPECULAR
		specular = float4(saturate(Cs * saturate(specular)).rgb, 0);
	#endif

	result.ambient.rgb  = ambient.rgb;
	result.diffuse.rgb  = diffuse.rgb;
	result.specular.rgb = specular.rgb;
#endif

	return result;
}

#ifndef RS_ALPHA
[earlydepthstencil]
#endif
float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
	float4 result;

#if FVF_TEXCOUNT > 0
	result = DiffuseMap.Sample(DiffuseSampler, input.tex0);
#else
	result = float4(1, 1, 1, 1);
#endif

	result = saturate(result * saturate(input.diffuse + input.ambient));
	result = saturate(result + input.specular);
	result = saturate(result + input.emissive);

#ifdef RS_ALPHA
	if (alphaReject == true)
	{
		uint alpha = floor(result.a * 255);
		uint threshold = floor(alphaRejectThreshold * 255);

		switch (alphaRejectMode)
		{
			case CMP_NEVER:
				clip(-1);
				break;
			case CMP_LESS:
				clip(alpha < threshold ? 1 : -1);
				break;
			case CMP_EQUAL:
				clip(alpha == threshold ? 1 : -1);
				break;
			case CMP_LESSEQUAL:
				clip(alpha <= threshold ? 1 : -1);
				break;
			case CMP_GREATER:
				clip(alpha > threshold ? 1 : -1);
				break;
			case CMP_NOTEQUAL:
				clip(alpha != threshold ? 1 : -1);
				break;
			case CMP_GREATEREQUAL:
				clip(alpha >= threshold ? 1 : -1);
				break;

			default:
				result = float4(1, 0, 0, 1);
				break;
		}
	}
#endif

#ifdef RS_FOG
	float factor = CalcFogFactor(input.fog);
	result.rgb = (factor * result + (1.0 - factor) * fogColor).rgb;
#endif

	return result;
}
