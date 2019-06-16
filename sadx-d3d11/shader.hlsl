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
	uint  colorOp;               // D3DTOP
	uint  colorArg1;             // D3DTA
	uint  colorArg2;             // D3DTA
	uint  alphaOp;               // D3DTOP
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
	FVF_TEXCOORD ## N ## _TYPE tex ## N : TEXCOORD ## N

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

struct FVFTexCoordOut
{
	nointerpolation uint componentCount;
	nointerpolation bool divByLast;
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

	FVFTexCoordOut uvmeta[8] : TEXCOORDMETA;
	float4 uv[8] : TEXCOORD;
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

	float textureFactor;
};

cbuffer TextureStages : register(b3)
{
	TextureStage textureStages[TEXTURE_STAGE_COUNT];
}

Texture2D<float4> textures[TEXTURE_STAGE_COUNT];
SamplerState samplers[TEXTURE_STAGE_COUNT];

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

		#ifdef FVF_SPECULAR
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
			break;
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
				float phi        = clamp(lights[i].Phi, theta, M_PI);
				float phi2       = phi / 2.0f;
				float cos_phi2   = cos(phi2);

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
	result.diffuse.rgb  = saturate(diffuse.rgb + ambient.rgb);
	result.specular.rgb = specular.rgb;
#endif

#if FVF_TEXCOUNT > 0
	result.uvmeta[0].componentCount = FVF_TEXCOORD0_SIZE;

	#if FVF_TEXCOORD0_SIZE == 1
		result.uv[0].x = input.tex0;
	#elif FVF_TEXCOORD0_SIZE == 2
		result.uv[0].xy = input.tex0.xy;
	#elif FVF_TEXCOORD0_SIZE == 3
		result.uv[0].xyz = input.tex0.xyz;
	#elif FVF_TEXCOORD0_SIZE == 4
		result.uv[0].xyzw = input.tex0.xyzw;
	#endif

#endif

	return result;
}

bool compare(uint mode, float a, float b)
{
	switch (mode)
	{
		case CMP_ALWAYS:
			return true;
		case CMP_NEVER:
			return false;
		case CMP_LESS:
			return a < b;
		case CMP_EQUAL:
			return a == b;
		case CMP_LESSEQUAL:
			return a <= b;
		case CMP_GREATER:
			return a > b;
		case CMP_NOTEQUAL:
			return a != b;
		case CMP_GREATEREQUAL:
			return a >= b;
		default:
			return false;
	}
}

float4 getArg(uint stageNum, in TextureStage stage, uint textureArg, float4 current, float4 texel, float4 tempreg, float4 inputDiffuse, float4 inputSpecular)
{
	float4 result;

	switch (textureArg & TA_SELECTMASK)
	{
		case TA_DIFFUSE:
		#ifdef FVF_DIFFUSE
			result = inputDiffuse;
		#else
			result = float4(1, 1, 1, 1);
		#endif
			break;

		case TA_CURRENT:
			result = !stageNum ? inputDiffuse : current;
			break;

		case TA_TEXTURE:
			result = texel; // TODO: check if texture is bound
			break;

		case TA_TFACTOR:
			result = float4(textureFactor, textureFactor, textureFactor, textureFactor);
			break;

		case TA_SPECULAR:
		#ifdef FVF_SPECULAR
			result = inputSpecular;
		#else
			result = float4(1, 1, 1, 1);
		#endif
			break;

		case TA_TEMP:
			result = tempreg;
			break;
	}

	uint modifiers = textureArg & ~TA_SELECTMASK;

	if (modifiers & TA_COMPLEMENT)
	{
		result = 1.0 - result;
	}

	if (modifiers & TA_ALPHAREPLICATE)
	{
		result.xyz = result.aaa;
	}

	return result;
}

float4 textureOp(uint colorOp, float4 colorArg1, float4 colorArg2, float4 colorArg0, float4 texel, float4 current, float4 inputDiffuse)
{
	if (colorOp == TOP_SELECTARG1)
	{
		return colorArg1;
	}

	if (colorOp == TOP_SELECTARG2)
	{
		return colorArg2;
	}

	float4 result = (float4)0;

	switch (colorOp)
	{
		default:
			break;

		case TOP_MODULATE:
			result = colorArg1 * colorArg2;
			break;
		case TOP_MODULATE2X:
			result = 2 * (colorArg1 * colorArg2);
			break;
		case TOP_MODULATE4X:
			result = 4 * (colorArg1 * colorArg2);
			break;
		case TOP_ADD:
			result = colorArg1 + colorArg2;
			break;
		case TOP_ADDSIGNED:
			result = (colorArg1 + colorArg2) - 0.5;
			break;
		case TOP_ADDSIGNED2X:
			result = 2 * ((colorArg1 + colorArg2) - 0.5);
			break;
		case TOP_SUBTRACT:
			result = colorArg1 - colorArg2;
			break;
		case TOP_ADDSMOOTH:
			result = (colorArg1 + colorArg2) - (colorArg1 * colorArg2);
			break;
		case TOP_BLENDDIFFUSEALPHA:
		{
			float alpha = inputDiffuse.a;
			result = (colorArg1 * alpha) + (colorArg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDTEXTUREALPHA:
		{
			float alpha = texel.a;
			result = (colorArg1 * alpha) + (colorArg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDFACTORALPHA:
		{
			float alpha = textureFactor;
			result = (colorArg1 * alpha) + (colorArg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDTEXTUREALPHAPM:
		{
			float alpha = texel.a;
			result = colorArg1 + (colorArg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDCURRENTALPHA:
		{
			float alpha = current.a;
			result = (colorArg1 * alpha) + (colorArg2 * (1 - alpha));
			break;
		}

		case TOP_PREMODULATE: // TODO: NOT SUPPORTED
			break;

		case TOP_MODULATEALPHA_ADDCOLOR:
			result = float4(colorArg1.rgb + (colorArg2.rgb * colorArg1.a), colorArg1.a * colorArg2.a);
			break;
		case TOP_MODULATECOLOR_ADDALPHA:
			result = float4(colorArg1.rgb * colorArg2.rgb, colorArg1.a + colorArg2.a);
			break;
		case TOP_MODULATEINVALPHA_ADDCOLOR:
			result = float4(colorArg1.rgb + (colorArg2.rgb * (1 - colorArg1.a)), colorArg1.a * colorArg2.a);
			break;
		case TOP_MODULATEINVCOLOR_ADDALPHA:
			result = float4((1 - colorArg1.rgb) * colorArg2.rgb, colorArg1.a + colorArg2.a);
			break;

		case TOP_BUMPENVMAP: // TODO: NOT SUPPORTED
			break;
		case TOP_BUMPENVMAPLUMINANCE: // TODO: NOT SUPPORTED
			break;
		case TOP_DOTPRODUCT3: // TODO: NOT SUPPORTED
			break;

		case TOP_MULTIPLYADD:
			result = colorArg1 + colorArg2 * colorArg0;
			break;

		case TOP_LERP:
			result = lerp(colorArg2, colorArg0, colorArg1.r);
			break;
	}

	return result;
}

#ifndef RS_ALPHA
[earlydepthstencil]
#endif
float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
#if TEXTURE_STAGE_COUNT > 0
	float4 current = (float4)0;
	float4 tempreg = (float4)0;

	float4 samples[TEXTURE_STAGE_COUNT];

	for (uint s = 0; s < TEXTURE_STAGE_COUNT; s++)
	{
		uint coordIndex = textureStages[s].texCoordIndex & TSS_TCI_COORD_MASK;
		float4 texcoord = input.uv[coordIndex];
		samples[s] = textures[s].Sample(samplers[s], texcoord);
	}

	bool colorDone = false;
	bool alphaDone = false;

	for (uint i = 0; i < TEXTURE_STAGE_COUNT; i++)
	{
		TextureStage stage = textureStages[i];

		if (stage.colorOp <= TOP_DISABLE && stage.alphaOp <= TOP_DISABLE)
		{
			break;
		}

		float4 texel = samples[i];

		if (stage.colorOp <= TOP_DISABLE)
		{
			colorDone = true;
		}

		if (!colorDone)
		{
			float4 colorArg1 = getArg(i, stage, stage.colorArg1, current, texel, tempreg, input.diffuse, input.specular);
			float4 colorArg2 = getArg(i, stage, stage.colorArg2, current, texel, tempreg, input.diffuse, input.specular);
			float4 colorArg0 = getArg(i, stage, stage.colorArg0, current, texel, tempreg, input.diffuse, input.specular);

			current.rgb = textureOp(stage.colorOp, colorArg1, colorArg2, colorArg0, texel, current, input.diffuse).rgb;
		}

		if (stage.alphaOp <= TOP_DISABLE)
		{
			alphaDone = true;
		}

		if (!alphaDone)
		{
			float4 alphaArg1 = getArg(i, stage, stage.alphaArg1, current, texel, tempreg, input.diffuse, input.specular);
			float4 alphaArg2 = getArg(i, stage, stage.alphaArg2, current, texel, tempreg, input.diffuse, input.specular);
			float4 alphaArg0 = getArg(i, stage, stage.alphaArg0, current, texel, tempreg, input.diffuse, input.specular);

			current.a = textureOp(stage.alphaOp, alphaArg1, alphaArg2, alphaArg0, texel, current, input.diffuse).a;
		}

		// TODO: D3DTTFF_PROJECTED
		/*
			When texture projection is enabled for a texture stage, all four floating point
			values must be written to the corresponding texture register. Each texture coordinate
			is divided by the last element before being passed to the rasterizer. For example,
			if this flag is specified with the D3DTTFF_COUNT3 flag, the first and second texture
			coordinates are divided by the third coordinate before being passed to the rasterizer.
		*/
	}

	float4 result = saturate(current);
#else
	float4 result = (float4)1;
#endif

#ifdef RS_SPECULAR
	result.rgb = saturate(result.rgb + input.specular.rgb);
	result.rgb = saturate(result.rgb + input.emissive.rgb);
#endif

#ifdef RS_ALPHA
	if (alphaReject == true)
	{
		uint alpha = floor(result.a * 255);
		uint threshold = floor(alphaRejectThreshold * 255);
		clip(compare(alphaRejectMode, alpha, threshold) ? 1 : -1);
	}
#endif

#ifdef RS_FOG
	float factor = CalcFogFactor(input.fog);
	result.rgb = (factor * result + (1.0 - factor) * fogColor).rgb;
#endif

	return result;
}
