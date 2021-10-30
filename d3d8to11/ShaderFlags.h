#pragma once

#include <cstdint>

struct ShaderFlags
{
	using type = uint64_t;

	enum T : type
	{
		none,
		rs_lighting      = 0b00001000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_specular      = 0b00010000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_alpha         = 0b00100000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_fog           = 0b01000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_oit           = 0b10000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		fvf_position     = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00001110,
		fvf_fields       = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'11110000,
		fvf_texcount     = 0b00000000'00000000'00000000'00000000'00000000'00000000'00001111'00000000,
		fvf_lastbeta     = 0b00000000'00000000'00000000'00000000'00000000'00000000'00010000'00000000,
		fvf_texfmt       = 0b00000000'00000000'00000000'00000000'11111111'11111111'00000000'00000000,
		stage_count_mask = 0b00000000'00000000'00000000'00001111'00000000'00000000'00000000'00000000,
		fvf_mask         = fvf_position | fvf_fields | fvf_texcount | fvf_lastbeta | fvf_texfmt,
		rs_mask          = rs_lighting | rs_specular | rs_alpha | rs_fog | rs_oit,
		mask             = rs_mask | fvf_mask | stage_count_mask,
		count
	};

	static constexpr type stage_count_shift = 32;

	static constexpr type light_sanitize_flags = rs_lighting | rs_specular |
		D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL | D3DFVF_XYZRHW;

#ifdef PER_PIXEL
	// TODO
#else
	static constexpr type vs_mask = rs_lighting | rs_specular | fvf_mask;
	static constexpr type ps_mask = stage_count_mask | rs_mask | light_sanitize_flags;

	static constexpr type uber_vs_mask = fvf_mask;
	static constexpr type uber_ps_mask = stage_count_mask | (light_sanitize_flags & ~rs_mask);
#endif

	static type sanitize(type flags);
};
