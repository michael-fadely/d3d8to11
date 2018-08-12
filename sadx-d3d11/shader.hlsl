#define NODE_WRITE
#include "include.hlsli"

#define D3DLIGHT_POINT 1
#define D3DLIGHT_SPOT 2
#define D3DLIGHT_DIRECTIONAL 3

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
	float4 diffuse  : COLOR0;
	float4 specular : COLOR1;
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
	uint   bufferLength;
};

cbuffer PerModelBuffer : register(b1)
{
	matrix worldMatrix;
	matrix wvMatrixInvT;
	matrix textureMatrix;

	uint diffuseSource;
	bool colorVertex;

	Light lights[LIGHT_COUNT];
	Material material;
};

cbuffer PerPixelBuffer : register(b2)
{
	uint   srcBlend;
	uint   destBlend;
	uint   fogMode;
	float  fogStart;
	float  fogEnd;
	float  fogDensity;
	float4 fogColor;
};

Texture2D<float4> DiffuseMap     : register(t0);
SamplerState      DiffuseSampler : register(s0);

// From FixedFuncEMU.fx
// Copyright (c) 2005 Microsoft Corporation. All rights reserved.
//
// Calculates fog factor based upon distance
//
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

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT result = (VS_OUTPUT)0;

#ifdef FVF_RHW
	float4 p = input.position;
	float2 screen = screenDimensions - 0.5;

	p.xy = round(p.xy);
	p.x = ((p.x / screen.x) * 2) - 1;
	p.y = -(((p.y / screen.y) * 2) - 1);

	result.position = p;
#else
	input.position.w = 1;
	result.position = input.position;

	result.position = mul(worldMatrix, result.position);
	result.position = mul(viewMatrix, result.position);

	result.fog = result.position.z;

	result.position = mul(projectionMatrix, result.position);
#endif

	result.depth = result.position.zw;

	if (colorVertex == true && diffuseSource == CS_COLOR1
	#ifdef FVF_DIFFUSE
		&& any(input.diffuse)
	#endif
		)
	{
	#ifdef FVF_DIFFUSE
		result.diffuse = input.diffuse;
	#else
		result.diffuse = float4(1, 1, 1, 1);
	#endif
	}
	else
	{
		result.diffuse = saturate(material.Diffuse);
	}

#if defined(RS_LIGHTING) && defined(FVF_NORMAL)/* && !defined(FVF_RHW)*/
	float4 diffuse  = float4(0, 0, 0, 0);
	float4 ambient  = float4(0, 0, 0, 0);
	float4 specular = float4(0, 0, 0, 0);

	float4 Cd = saturate(result.diffuse);
	float3 N  = normalize(mul((float3x3)worldMatrix, input.normal));

	#ifdef RS_SPECULAR
		float4 Cs = max(0, material.Specular);
		float P = max(0, material.Power);

		float4 worldPosition = mul(worldMatrix, input.position);
		float3 viewDir = normalize(viewPosition - worldPosition.xyz);
	#endif

	for (uint i = 0; i < LIGHT_COUNT; ++i)
	{
		if (lights[i].Enabled == false)
		{
			break;
		}

		float4 Ld = lights[i].Diffuse;
		float3 Ldir = normalize(-lights[i].Direction);
		float NdotLdir = saturate(dot(N, Ldir));

		ambient += lights[i].Ambient;
		diffuse += /*Cd * */Ld * NdotLdir; // applying Cd after the fact for better vertex color

		#ifdef RS_SPECULAR
			float4 Ls = lights[i].Specular;
			float3 H = normalize(viewDir + Ldir);
			specular += Ls * pow(dot(N, H), P);
		#endif
	}

	#ifdef RS_SPECULAR
		specular = saturate(specular) * saturate(Cs);
		result.specular = float4(specular.rgb, 0);
	#endif

	result.diffuse.rgb = Cd.rgb * saturate(saturate(diffuse.rgb) + ambient.rgb);
#endif

#if FVF_TEXCOUNT > 0
	#ifdef TCI_CAMERASPACENORMAL
		result.tex0 = (float2)mul(float4(input.normal, 1), wvMatrixInvT);
		result.tex0 = (float2)mul(textureMatrix, float4(result.tex0, 0, 1));
	#else
		result.tex0 = input.tex0;
	#endif
#endif

	return result;
}

//[earlydepthstencil]
float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
	float4 result;

#if FVF_TEXCOUNT >= 1
	result = DiffuseMap.Sample(DiffuseSampler, input.tex0);
#else
	result = float4(1, 0, 0, 1);
#endif

	return result;

	result *= input.diffuse;
	result += input.specular;

#ifdef RS_FOG
	float factor = CalcFogFactor(input.fog);
	result.rgb = (factor * result + (1.0 - factor) * fogColor).rgb;
#endif

#ifdef RS_ALPHA
	if (result.a < 1.0f / 255.0f)
	{
		clip(-1);
	}

	#if !defined(FVF_RHW)
	if ((srcBlend == BLEND_SRCALPHA || srcBlend == BLEND_ONE) &&
	    (destBlend == BLEND_INVSRCALPHA || destBlend == BLEND_ZERO))
	{
		if (result.a > 254.0f / 255.0f)
		{
			return result;
		}
	}
	#endif

	#ifdef OIT
		#ifndef DISABLE_PER_PIXEL_LIMIT
			uint fragmentCount;
			InterlockedAdd(FragListCount[input.position.xy], 1, fragmentCount);

			if (fragmentCount >= MAX_FRAGMENTS)
			{
				clip(-1);
			}
		#endif

		uint newIndex = FragListNodes.IncrementCounter();

		// if per-pixel fragment limiting is enabled, this check is unnecessary
		if (newIndex >= bufferLength)
		{
			clip(-1);
		}

		uint oldIndex;
		InterlockedExchange(FragListHead[input.position.xy], newIndex, oldIndex);

		OitNode n;

		n.depth = input.depth.x / input.depth.y;
		n.color = float4_to_unorm(result);
		n.flags = (srcBlend << 8) | destBlend;
		n.next  = oldIndex;

		FragListNodes[newIndex] = n;
		clip(-1);
	#endif
#endif

	return result;
}
