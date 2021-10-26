#define NODE_WRITE
#include "d3d8to11.hlsl"

VS_OUTPUT vs_main(VS_INPUT input)
{
#if UBER == 1
	VS_OUTPUT result = fixed_func_vs(input);
	result.diffuse.rgb = float3(0, 1, 0);
	result.specular.rgb = float3(0, 1, 0);
	return result;
#else
	return fixed_func_vs(input);
#endif
}

//#if (!defined(UBER) || UBER == 0) && \
//	(!defined(RS_OIT) || RS_OIT == 0) && \
//	(!defined(RS_ALPHA) || RS_ALPHA == 0)
//[earlydepthstencil]
//#endif
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

#if UBER == 1
	result.rgb = float3(1, 0, 0);
#endif

	return result;
}
