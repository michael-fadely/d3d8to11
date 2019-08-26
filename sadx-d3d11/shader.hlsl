#define NODE_WRITE
#include "d3d8to11.hlsl"

VS_OUTPUT vs_main(VS_INPUT input)
{
	return fixed_func_vs(input);
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
