#define NODE_WRITE
#include "include.hlsli"

#define D3DLIGHT_POINT 1
#define D3DLIGHT_SPOT 2
#define D3DLIGHT_DIRECTIONAL 3

#ifndef LIGHT_COUNT
#define LIGHT_COUNT 8
#endif

struct Material
{
	float4 Diffuse;        /* Diffuse color RGBA */
	float4 Ambient;        /* Ambient color RGB */
	float4 Specular;       /* Specular 'shininess' */
	float4 Emissive;       /* Emissive color RGB */
	float  Power;          /* Sharpness if specular highlight */
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

struct VS_INPUT
{
	float4 position : POSITION;

#ifdef FVF_NORMAL
	float3 normal   : NORMAL;
#endif

#ifdef FVF_DIFFUSE
	float4 diffuse   : COLOR;
#endif

#ifdef FVF_TEX1
	float2 tex      : TEXCOORD;
#endif
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 diffuse  : COLOR0;
	float4 specular : COLOR1;
	float2 tex      : TEXCOORD0;
	float2 depth    : TEXCOORD1;
	float  fog      : FOG;
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

#ifdef FVF_DIFFUSE
	if (colorVertex == true)
	{
		result.diffuse = input.diffuse;
	}
	else
	{
		result.diffuse = float4(1, 1, 1, 1);
	}
#else
	result.diffuse = float4(1, 1, 1, 1);
#endif

	if (diffuseSource == CS_MATERIAL)
	{
		if (colorVertex == true)
		{
			result.diffuse *= material.Diffuse;
		}
		else
		{
			result.diffuse = material.Diffuse;
		}
	}

#if defined(RS_LIGHTING) && defined(FVF_NORMAL) && !defined(FVF_RHW)
	float4 diffuse  = float4(0, 0, 0, 0);
	float4 ambient  = float4(0, 0, 0, 0);
	float4 specular = float4(0, 0, 0, 0);

	float4 Cd = result.diffuse;
	float3 N  = mul((float3x3)worldMatrix, input.normal);

	#ifdef RS_SPECULAR
		float4 Cs = material.Specular;
		float P   = material.Power;

		float4 worldPosition = mul(worldMatrix, input.position);
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
		diffuse += (/*Cd * */Ld * NdotLdir); // applying Cd after the fact for better vertex color

		#ifdef RS_SPECULAR
			float4 Ls = lights[i].Specular;
			float3 H = normalize(normalize(viewPosition - worldPosition) + Ldir);
			specular += Ls * pow(saturate(dot(N, H)), P);
		#endif
	}

	#ifdef RS_SPECULAR
		specular *= Cs;
		result.specular = saturate(specular);
		result.specular.a = 0;
	#endif

	result.diffuse.rgb = Cd.rgb * saturate(saturate(diffuse.rgb) + ambient.rgb);
#endif

#ifdef FVF_TEX1
	#ifdef TCI_CAMERASPACENORMAL
		result.tex = (float2)mul(float4(input.normal, 1), wvMatrixInvT);
		result.tex = (float2)mul(textureMatrix, float4(result.tex, 0, 1));
	#else
		result.tex = input.tex;
	#endif
#endif

	return result;
}

//[earlydepthstencil]
float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
	float4 result;

#ifdef FVF_TEX1
	result = DiffuseMap.Sample(DiffuseSampler, input.tex);
#else
	result = float4(1, 1, 1, 1);
#endif

	result *= input.diffuse;
	result += input.specular;

#ifdef RS_FOG
	float factor = CalcFogFactor(input.fog);
	result.rgb = (factor * result + (1.0 - factor) * fogColor).rgb;
#endif

#ifdef RS_ALPHA
	if (result.a < 1.0f / 255.0f)
	{
		discard;
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
		uint newIndex = FragListNodes.IncrementCounter();
		if (newIndex == FRAGMENT_LIST_NULL)
		{
			discard;
		}

		uint oldIndex;
		InterlockedExchange(FragListHead[uint2(input.position.xy)], newIndex, oldIndex);

		OitNode n;

		n.depth = input.depth.x / input.depth.y;
		n.color = float4_to_unorm(result);
		n.flags = (srcBlend << 8) | destBlend;
		n.next  = oldIndex;

		FragListNodes[newIndex] = n;
		discard;
	#endif
#endif

	return result;
}
