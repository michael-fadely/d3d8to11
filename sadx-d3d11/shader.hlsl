#define NODE_WRITE
#include "d3d8to11.hlsl"

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

#if !defined(RS_OIT) && !defined(RS_ALPHA)
[earlydepthstencil]
#endif
float4 ps_main(VS_OUTPUT input) : SV_TARGET
{
	float4 diffuse;
	float4 specular;

	get_input_colors(input, diffuse, specular);

	float4 result;

	result = handle_texture_stages(input, diffuse, specular);
	result = apply_fog(result, input.fog);

	bool standard_blending = is_standard_blending();

	do_alpha_reject(result, standard_blending);
	do_oit(result, input, standard_blending);

	return result;
}
