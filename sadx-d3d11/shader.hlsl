#define NODE_WRITE
#include "include.hlsli"

#ifdef SPEEDY_SPEED_BOY
	#define TSS_UNROLL [loop]
#else
	#define TSS_UNROLL [unroll(TEXTURE_STAGE_COUNT / 4)]
#endif

#ifndef LIGHT_COUNT
	#define LIGHT_COUNT 8
#endif

#ifndef FVF_TEXCOUNT
	#define FVF_TEXCOUNT 0
#endif

#ifdef RS_LIGHTING
	#ifdef FVF_RHW
		#undef RS_LIGHTING
	#endif
#endif

// Enhancements

#define RADIAL_FOG

#if 0
	#define PIXEL_LIGHTING
#else
	#define VERTEX_LIGHTING
#endif

struct Material
{
	float4 diffuse;  /* Diffuse color RGBA */
	float4 ambient;  /* Ambient color RGB */
	float4 specular; /* Specular 'shininess' */
	float4 emissive; /* Emissive color RGB */
	float  power;    /* Sharpness if specular highlight */
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
	bool   enabled;
	uint   type;         /* Type of light source */
	float4 diffuse;      /* Diffuse color of light */
	float4 specular;     /* Specular color of light */
	float4 ambient;      /* Ambient color of light */
	float3 position;     /* Position in world space */
	float3 direction;    /* Direction in world space */
	float  range;        /* Cutoff range */
	float  falloff;      /* Falloff */
	float  attenuation0; /* Constant attenuation */
	float  attenuation1; /* Linear attenuation */
	float  attenuation2; /* Quadratic attenuation */
	float  theta;        /* Inner angle of spotlight cone */
	float  phi;          /* Outer angle of spotlight cone */
};

struct TextureStage
{
	bool   bound;                  // indicates if the texture is bound
	matrix transform;              // texture coordinate transformation
	uint   color_op;               // D3DTOP
	uint   color_arg1;             // D3DTA
	uint   color_arg2;             // D3DTA
	uint   alpha_op;               // D3DTOP
	uint   alpha_arg1;             // D3DTA
	uint   alpha_arg2;             // D3DTA
	float  bump_env_mat00;
	float  bump_env_mat01;
	float  bump_env_mat10;
	float  bump_env_mat11;
	uint   tex_coord_index;         // D3DTSS_TCI
	float  bump_env_lscale;
	float  bump_env_loffset;
	uint   texture_transform_flags;
	uint   color_arg0;             // D3DTA
	uint   alpha_arg0;             // D3DTA
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
	nointerpolation uint component_count;
	nointerpolation bool div_by_last;
};

struct VS_OUTPUT
{
	float4 position   : SV_POSITION;
	float3 w_position : POSITION;
	float3 normal     : NORMAL0;
	float3 w_normal   : NORMAL1;
	float4 ambient    : COLOR0;
	float4 diffuse    : COLOR1;
	float4 specular   : COLOR2;
	float4 emissive   : COLOR3;
	float2 depth      : DEPTH;
	float  fog        : FOG;

	FVFTexCoordOut uvmeta[8] : TEXCOORDMETA;
	float4 uv[8] : TEXCOORD;
};

cbuffer PerSceneBuffer : register(b0)
{
	matrix view_matrix;
	matrix projection_matrix;
	float2 screen_dimensions;
	float3 view_position;
	uint   buffer_len;
};

cbuffer PerModelBuffer : register(b1)
{
	matrix world_matrix;
	matrix wv_matrix_inv_t;

	MaterialSources material_sources;
	float4 global_ambient;
	bool color_vertex;

	Light lights[LIGHT_COUNT];
	Material material;
};

cbuffer PerPixelBuffer : register(b2)
{
	uint   src_blend;
	uint   dst_blend;
	uint   fog_mode;
	float  fog_start;
	float  fog_end;
	float  fog_density;
	float4 fog_color;

	bool  alpha_reject;
	uint  alpha_reject_mode;
	float alpha_reject_threshold;

	float4 texture_factor;
};

cbuffer TextureStages : register(b3)
{
	TextureStage texture_stages[TEXTURE_STAGE_COUNT];
}

Texture2D<float4> textures[TEXTURE_STAGE_COUNT];
SamplerState samplers[TEXTURE_STAGE_COUNT];

// From FixedFuncEMU.fx, originally calc_fog
// Copyright (c) 2005 Microsoft Corporation. All rights reserved.
// Calculates fog factor based upon distance
float calc_fog(float d)
{
	float fog_coeff = 1.0;

	if (FOGMODE_LINEAR == fog_mode)
	{
		fog_coeff = (fog_end - d) / (fog_end - fog_start);
	}
	else if (FOGMODE_EXP == fog_mode)
	{
		fog_coeff = 1.0 / pow(E, d*fog_density);
	}
	else if (FOGMODE_EXP2 == fog_mode)
	{
		fog_coeff = 1.0 / pow(E, d * d * fog_density * fog_density);
	}

	return clamp(fog_coeff, 0, 1);
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

void perform_lighting(in float4 in_ambient,     in float4 in_diffuse, in float4 in_specular,
                      in float3 world_position, in float3 world_normal,
                     out float4 out_diffuse,   out float4 out_specular)
{
	float4 ambient  = float4(0, 0, 0, 0);
	float4 diffuse  = float4(0, 0, 0, 0);
	float4 specular = float4(0, 0, 0, 0);

	float4 c_a = in_ambient;
	float4 c_d = in_diffuse;
	float4 c_s = in_specular;

	out_diffuse  = c_d;
	out_specular = c_s;

	#ifdef RS_SPECULAR
		float p = max(0, material.power);
		float3 view_dir = normalize(view_position - world_position.xyz);
	#endif

	for (uint i = 0; i < LIGHT_COUNT; ++i)
	{
		if (lights[i].enabled == false)
		{
			break;
		}

		// exceptions to the naming style for the sake of the formula
		float Atten = 1.0f;
		float Spot = 1.0f;

		switch (lights[i].type)
		{
			default:
				Atten = 1.0f;
				Spot = 1.0f;
				break;

			case LIGHT_POINT:
			{
				float d = length(world_position.xyz - lights[i].position);

				if (d <= lights[i].range)
				{
					float d2 = d * d;
					Atten = 1.0f / (lights[i].attenuation0 + lights[i].attenuation1 * d + lights[i].attenuation2 * d2);
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
				float3 Ldcs = normalize(-mul(mul((float3x3)world_matrix, lights[i].direction), (float3x3)view_matrix));
				float3 Ldir = normalize(world_position.xyz - lights[i].position);

				float rho = dot(Ldcs, Ldir);

				float theta      = clamp(lights[i].theta, 0.0f, M_PI);
				float theta2     = theta / 2.0f;
				float cos_theta2 = cos(theta2);
				float phi        = clamp(lights[i].phi, theta, M_PI);
				float phi2       = phi / 2.0f;
				float cos_phi2   = cos(phi2);

				float falloff  = lights[i].falloff;

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

		// exceptions to the naming style for the sake of the formula
		float4 Ld       = lights[i].diffuse;
		float3 Ldir     = normalize(-lights[i].direction);

	#ifdef FVF_NORMAL
		float3 N = world_normal;
		float NdotLdir = saturate(dot(N, Ldir));
	#else
		float NdotLdir = 0;
	#endif

		// Diffuse Lighting = sum[c_d*Ld*(N.Ldir)*Atten*Spot]
		diffuse += c_d * Ld * NdotLdir * Atten * Spot;

		// sum(Atteni*Spoti*Lai)
		ambient += Atten * Spot * lights[i].ambient;

		#ifdef RS_SPECULAR
			float4 Ls = lights[i].specular;
			float3 H = normalize(view_dir + normalize(Ldir));

			#ifdef FVF_NORMAL
				float NdotH = max(0, dot(N, H));
			#else
				float NdotH = 0;
			#endif

			specular += Ls * pow(NdotH, p) * Atten * Spot;
		#endif
	}

	// Ambient Lighting = c_a*[Ga + sum(Atteni*Spoti*Lai)]
	ambient = saturate(c_a * saturate(global_ambient + saturate(ambient)));

	/*
		Diffuse components are clamped to be from 0 to 255, after all lights are processed and interpolated separately.
		The resulting diffuse lighting value is a combination of the ambient, diffuse and emissive light values.
	*/
	diffuse = saturate(diffuse);

	#ifdef RS_SPECULAR
		specular = float4(saturate(c_s * saturate(specular)).rgb, 0);
	#endif

	out_diffuse.rgb = saturate(diffuse.rgb + ambient.rgb);
	out_specular.rgb = specular.rgb;
}

VS_OUTPUT vs_main(VS_INPUT input)
{
	VS_OUTPUT result = (VS_OUTPUT)0;

	float3 world_position = (float3)0;

#ifdef FVF_RHW
	float4 p = input.position;
	float2 screen = screen_dimensions - 0.5;

	p.xy = round(p.xy);
	p.x = ((p.x / screen.x) * 2) - 1;
	p.y = -(((p.y / screen.y) * 2) - 1);

	result.position = p;
#else
	input.position.w = 1;
	result.position = mul(world_matrix, input.position);

	result.w_position = result.position.xyz;

	result.position = mul(view_matrix, result.position);

	#ifdef RADIAL_FOG
		result.fog = length(view_position - result.w_position);
	#else
		result.fog = result.position.z;
	#endif

	result.position = mul(projection_matrix, result.position);
#endif

	result.depth = result.position.zw;

#ifdef FVF_NORMAL
	result.normal   = input.normal;
	result.w_normal = normalize(mul((float3x3)world_matrix, input.normal));
#endif

	if (color_vertex == true)
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
		switch (material_sources.ambient)
		{
			case CS_MATERIAL:
				result.ambient = black_not_lit(material.ambient);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.ambient = input.diffuse;
			#else
				result.ambient = black_not_lit(material.ambient);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.ambient = input.specular;
			#else
				result.ambient = black_not_lit(material.ambient);
			#endif
				break;

			default:
				result.ambient = float4(1, 0, 0, 1);
				break;
		}

		switch (material_sources.diffuse)
		{
			case CS_MATERIAL:
				result.diffuse = white_not_lit(material.diffuse);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.diffuse = input.diffuse;
			#else
				result.diffuse = white_not_lit(material.diffuse);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.diffuse = input.specular;
			#else
				result.diffuse = white_not_lit(material.diffuse);
			#endif
				break;

			default:
				result.diffuse = float4(1, 0, 0, 1);
				break;
		}

		switch (material_sources.specular)
		{
			case CS_MATERIAL:
				result.specular = black_not_lit(material.specular);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.specular = input.diffuse;
			#else
				result.specular = black_not_lit(material.specular);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.specular = input.specular;
			#else
				result.specular = black_not_lit(material.specular);
			#endif
				break;

			default:
				result.specular = float4(1, 0, 0, 1);
				break;
		}

		switch (material_sources.emissive)
		{
			case CS_MATERIAL:
				result.emissive = black_not_lit(material.emissive);
				break;

			case CS_COLOR1:
			#ifdef FVF_DIFFUSE
				result.emissive = input.diffuse;
			#else
				result.emissive = black_not_lit(material.emissive);
			#endif
				break;

			case CS_COLOR2:
			#ifdef FVF_SPECULAR
				result.emissive = input.specular;
			#else
				result.emissive = black_not_lit(material.emissive);
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
		result.diffuse = white_not_lit(material.diffuse);
		result.specular = black_not_lit(material.specular);
	}

#if defined(VERTEX_LIGHTING) && RS_LIGHTING == 1
	float4 diffuse;
	float4 specular;

	perform_lighting(result.ambient, result.diffuse, result.specular, result.w_position, result.w_normal,
	                 diffuse, specular);

	result.diffuse.rgb  = saturate(result.emissive.rgb + diffuse.rgb);
	result.specular.rgb = specular.rgb;
#endif

#if FVF_TEXCOUNT > 0
	result.uvmeta[0].component_count = FVF_TEXCOORD0_SIZE;

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

#if FVF_TEXCOUNT > 1
	result.uvmeta[1].component_count = FVF_TEXCOORD1_SIZE;

	#if FVF_TEXCOORD1_SIZE == 1
		result.uv[1].x = input.tex1;
	#elif FVF_TEXCOORD1_SIZE == 2
		result.uv[1].xy = input.tex1.xy;
	#elif FVF_TEXCOORD1_SIZE == 3
		result.uv[1].xyz = input.tex1.xyz;
	#elif FVF_TEXCOORD1_SIZE == 4
		result.uv[1].xyzw = input.tex1.xyzw;
	#endif
#endif

#if FVF_TEXCOUNT > 2
	result.uvmeta[2].component_count = FVF_TEXCOORD2_SIZE;

	#if FVF_TEXCOORD2_SIZE == 1
		result.uv[2].x = input.tex2;
	#elif FVF_TEXCOORD2_SIZE == 2
		result.uv[2].xy = input.tex2.xy;
	#elif FVF_TEXCOORD2_SIZE == 3
		result.uv[2].xyz = input.tex2.xyz;
	#elif FVF_TEXCOORD2_SIZE == 4
		result.uv[2].xyzw = input.tex2.xyzw;
	#endif
#endif

#if FVF_TEXCOUNT > 3
	result.uvmeta[3].component_count = FVF_TEXCOORD3_SIZE;

	#if FVF_TEXCOORD3_SIZE == 1
		result.uv[3].x = input.tex3;
	#elif FVF_TEXCOORD3_SIZE == 2
		result.uv[3].xy = input.tex3.xy;
	#elif FVF_TEXCOORD3_SIZE == 3
		result.uv[3].xyz = input.tex3.xyz;
	#elif FVF_TEXCOORD3_SIZE == 4
		result.uv[3].xyzw = input.tex3.xyzw;
	#endif
#endif

#if FVF_TEXCOUNT > 4
	result.uvmeta[4].component_count = FVF_TEXCOORD4_SIZE;

	#if FVF_TEXCOORD4_SIZE == 1
		result.uv[4].x = input.tex4;
	#elif FVF_TEXCOORD4_SIZE == 2
		result.uv[4].xy = input.tex4.xy;
	#elif FVF_TEXCOORD4_SIZE == 3
		result.uv[4].xyz = input.tex4.xyz;
	#elif FVF_TEXCOORD4_SIZE == 4
		result.uv[4].xyzw = input.tex4.xyzw;
	#endif
#endif

#if FVF_TEXCOUNT > 5
	result.uvmeta[5].component_count = FVF_TEXCOORD5_SIZE;

	#if FVF_TEXCOORD5_SIZE == 1
		result.uv[5].x = input.tex5;
	#elif FVF_TEXCOORD5_SIZE == 2
		result.uv[5].xy = input.tex5.xy;
	#elif FVF_TEXCOORD5_SIZE == 3
		result.uv[5].xyz = input.tex5.xyz;
	#elif FVF_TEXCOORD5_SIZE == 4
		result.uv[5].xyzw = input.tex5.xyzw;
	#endif
#endif

#if FVF_TEXCOUNT > 6
	result.uvmeta[6].component_count = FVF_TEXCOORD6_SIZE;

	#if FVF_TEXCOORD6_SIZE == 1
		result.uv[6].x = input.tex6;
	#elif FVF_TEXCOORD6_SIZE == 2
		result.uv[6].xy = input.tex6.xy;
	#elif FVF_TEXCOORD6_SIZE == 3
		result.uv[6].xyz = input.tex6.xyz;
	#elif FVF_TEXCOORD6_SIZE == 4
		result.uv[6].xyzw = input.tex6.xyzw;
	#endif
#endif

#if FVF_TEXCOUNT > 7
	result.uvmeta[7].component_count = FVF_TEXCOORD7_SIZE;

	#if FVF_TEXCOORD7_SIZE == 1
		result.uv[7].x = input.tex7;
	#elif FVF_TEXCOORD7_SIZE == 2
		result.uv[7].xy = input.tex7.xy;
	#elif FVF_TEXCOORD7_SIZE == 3
		result.uv[7].xyz = input.tex7.xyz;
	#elif FVF_TEXCOORD7_SIZE == 4
		result.uv[7].xyzw = input.tex7.xyzw;
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

float4 get_arg(uint stageNum, in TextureStage stage, uint textureArg, float4 current, float4 texel, float4 tempreg, float4 in_diffuse, float4 in_specular)
{
	float4 result = (float4)0;

	switch (textureArg & TA_SELECTMASK)
	{
		case TA_DIFFUSE:
		#ifdef FVF_DIFFUSE
			result = in_diffuse;
		#else
			result = float4(1, 1, 1, 1);
		#endif
			break;

		case TA_CURRENT:
			result = !stageNum ? in_diffuse : current;
			break;

		case TA_TEXTURE:
			result = texel; // TODO: check if texture is bound
			break;

		case TA_TFACTOR:
			result = texture_factor;
			break;

		case TA_SPECULAR:
		#ifdef FVF_SPECULAR
			result = in_specular;
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

float4 texture_op(uint color_op, float4 color_arg1, float4 color_arg2, float4 color_arg0, float4 texel, float4 current, float4 in_diffuse)
{
	if (color_op == TOP_SELECTARG1)
	{
		return color_arg1;
	}

	if (color_op == TOP_SELECTARG2)
	{
		return color_arg2;
	}

	float4 result = (float4)0;

	switch (color_op)
	{
		default:
			result = float4(1, 0, 0, 1);
			break;

		case TOP_MODULATE:
			result = color_arg1 * color_arg2;
			break;
		case TOP_MODULATE2X:
			result = 2 * (color_arg1 * color_arg2);
			break;
		case TOP_MODULATE4X:
			result = 4 * (color_arg1 * color_arg2);
			break;
		case TOP_ADD:
			result = color_arg1 + color_arg2;
			break;
		case TOP_ADDSIGNED:
			result = (color_arg1 + color_arg2) - 0.5;
			break;
		case TOP_ADDSIGNED2X:
			result = 2 * ((color_arg1 + color_arg2) - 0.5);
			break;
		case TOP_SUBTRACT:
			result = color_arg1 - color_arg2;
			break;
		case TOP_ADDSMOOTH:
			result = (color_arg1 + color_arg2) - (color_arg1 * color_arg2);
			break;
		case TOP_BLENDDIFFUSEALPHA:
		{
			float alpha = in_diffuse.a;
			result = (color_arg1 * alpha) + (color_arg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDTEXTUREALPHA:
		{
			float alpha = texel.a;
			result = (color_arg1 * alpha) + (color_arg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDFACTORALPHA:
		{
			float alpha = texture_factor.a;
			result = (color_arg1 * alpha) + (color_arg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDTEXTUREALPHAPM:
		{
			float alpha = texel.a;
			result = color_arg1 + (color_arg2 * (1 - alpha));
			break;
		}
		case TOP_BLENDCURRENTALPHA:
		{
			float alpha = current.a;
			result = (color_arg1 * alpha) + (color_arg2 * (1 - alpha));
			break;
		}

		case TOP_PREMODULATE: // TODO: NOT SUPPORTED
			return float4(1, 0, 0, 1);
			break;

		case TOP_MODULATEALPHA_ADDCOLOR:
			result = float4(color_arg1.rgb + (color_arg2.rgb * color_arg1.a), color_arg1.a * color_arg2.a);
			break;
		case TOP_MODULATECOLOR_ADDALPHA:
			result = float4(color_arg1.rgb * color_arg2.rgb, color_arg1.a + color_arg2.a);
			break;
		case TOP_MODULATEINVALPHA_ADDCOLOR:
			result = float4(color_arg1.rgb + (color_arg2.rgb * (1 - color_arg1.a)), color_arg1.a * color_arg2.a);
			break;
		case TOP_MODULATEINVCOLOR_ADDALPHA:
			result = float4((1 - color_arg1.rgb) * color_arg2.rgb, color_arg1.a + color_arg2.a);
			break;

		case TOP_BUMPENVMAP: // TODO: NOT SUPPORTED
			return float4(1, 0, 0, 1);
			break;
		case TOP_BUMPENVMAPLUMINANCE: // TODO: NOT SUPPORTED
			return float4(1, 0, 0, 1);
			break;
		case TOP_DOTPRODUCT3: // TODO: NOT SUPPORTED
			return float4(1, 0, 0, 1);
			break;

		case TOP_MULTIPLYADD:
			result = color_arg1 + color_arg2 * color_arg0;
			break;

		case TOP_LERP:
			result = lerp(color_arg2, color_arg0, color_arg1.r);
			break;
	}

	return result;
}

float4 fix_coord_components(uint count, float4 coords)
{
	float4 result = coords;

	switch (count)
	{
		case 1:
			result.yzw = float3(0, 0, 1);
			break;

		case 2:
			result.zw = float2(0, 1);
			break;

		case 3:
			result.w = 1;
			break;

		default:
			break;
	}

	return result;
}

#if !defined(RS_OIT) && !defined(RS_ALPHA)
[earlydepthstencil]
#endif
float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
	float4 diffuse = input.diffuse;
	float4 specular = input.specular;

#if defined(PIXEL_LIGHTING) && RS_LIGHTING == 1
	perform_lighting(input.ambient, input.diffuse, input.specular, input.w_position, normalize(input.w_normal),
	                 diffuse, specular);

	diffuse = saturate(diffuse + input.emissive);
#endif

#if TEXTURE_STAGE_COUNT > 0
	float4 current = diffuse;
	float4 tempreg = (float4)0;

	float4 samples[TEXTURE_STAGE_COUNT];

	[unroll]
	for (uint s = 0; s < TEXTURE_STAGE_COUNT; s++)
	{
		if (texture_stages[s].bound)
		{
			uint coord_flags = texture_stages[s].tex_coord_index;

			uint coord_index = coord_flags & TSS_TCI_COORD_MASK;
			uint coord_caps = coord_flags & TSS_TCI_SELECT_MASK;
			uint component_count = input.uvmeta[s].component_count;

			float4 texcoord = input.uv[coord_index];

			switch (coord_caps)
			{
				case TSS_TCI_PASSTHRU:
					break;

				case TSS_TCI_CAMERASPACENORMAL:
					texcoord = float4(mul(input.normal, (float3x3)wv_matrix_inv_t), 1);
					texcoord = mul(texture_stages[s].transform, fix_coord_components(component_count, texcoord));
					break;

				case TSS_TCI_CAMERASPACEPOSITION:
					// TODO: TSS_TCI_CAMERASPACEPOSITION
					break;

				case TSS_TCI_CAMERASPACEREFLECTIONVECTOR:
					// TODO: TSS_TCI_CAMERASPACEREFLECTIONVECTOR
					break;

				default:
					return float4(1, 0, 0, 1);
			}

			samples[s] = textures[s].Sample(samplers[s], texcoord.xy);
		}
		else
		{
			samples[s] = float4(1, 1, 1, 1);
		}
	}

	bool color_done = false;
	bool alpha_done = false;

	TSS_UNROLL for (uint i = 0; i < TEXTURE_STAGE_COUNT; i++)
	{
		TextureStage stage = texture_stages[i];

		if (stage.color_op <= TOP_DISABLE && stage.alpha_op <= TOP_DISABLE)
		{
			break;
		}

		float4 texel = samples[i];

		if (stage.color_op <= TOP_DISABLE)
		{
			color_done = true;
		}

		if (!color_done)
		{
			float4 color_arg1 = get_arg(i, stage, stage.color_arg1, current, texel, tempreg, diffuse, specular);
			float4 color_arg2 = get_arg(i, stage, stage.color_arg2, current, texel, tempreg, diffuse, specular);
			float4 color_arg0 = get_arg(i, stage, stage.color_arg0, current, texel, tempreg, diffuse, specular);

			current.rgb = texture_op(stage.color_op, color_arg1, color_arg2, color_arg0, texel, current, diffuse).rgb;
		}

		if (stage.alpha_op <= TOP_DISABLE)
		{
			alpha_done = true;
		}

		if (!alpha_done)
		{
			float4 alpha_arg1 = get_arg(i, stage, stage.alpha_arg1, current, texel, tempreg, diffuse, specular);
			float4 alpha_arg2 = get_arg(i, stage, stage.alpha_arg2, current, texel, tempreg, diffuse, specular);
			float4 alpha_arg0 = get_arg(i, stage, stage.alpha_arg0, current, texel, tempreg, diffuse, specular);

			current.a = texture_op(stage.alpha_op, alpha_arg1, alpha_arg2, alpha_arg0, texel, current, diffuse).a;
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
	result.rgb = saturate(result.rgb + specular.rgb);
#endif

#ifdef RS_ALPHA
	if (alpha_reject == true)
	{
		uint alpha = floor(result.a * 255);
		uint threshold = floor(alpha_reject_threshold * 255);
		clip(compare(alpha_reject_mode, alpha, threshold) ? 1 : -1);
	}

	#if !defined(FVF_RHW)
	if ((src_blend == BLEND_SRCALPHA || src_blend == BLEND_ONE) &&
	    (dst_blend == BLEND_INVSRCALPHA || dst_blend == BLEND_ZERO))
	{
		if (result.a > 254.0f / 255.0f)
		{
			return result;
		}
	}
	#endif

	#ifdef RS_OIT
		#ifndef DISABLE_PER_PIXEL_LIMIT
			uint frag_count;
			InterlockedAdd(FragListCount[input.position.xy], 1, frag_count);

			if (frag_count >= MAX_FRAGMENTS)
			{
				clip(-1);
			}
		#endif

		uint new_index = FragListNodes.IncrementCounter();

		// if per-pixel fragment limiting is enabled, this check is unnecessary
		if (new_index >= buffer_len)
		{
			clip(-1);
		}

		uint old_index;
		InterlockedExchange(FragListHead[input.position.xy], new_index, old_index);

		OitNode n;

		n.depth = input.depth.x / input.depth.y;
		n.color = float4_to_unorm(result);
		n.flags = (src_blend << 8) | dst_blend;
		n.next  = old_index;

		FragListNodes[new_index] = n;
		clip(-1);
	#endif
#endif

#ifdef RS_FOG
	float factor = calc_fog(input.fog);
	result.rgb = (factor * result + (1.0 - factor) * fog_color).rgb;
#endif

	return result;
}
