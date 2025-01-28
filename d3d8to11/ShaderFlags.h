#pragma once

#include <cstdint>

#include "d3d8types.h" // for D3DFVF

struct ShaderFlags
{
	using type = uint64_t;

	enum T : type
	{
		none,
		rs_lighting             = 0b00000100'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_specular             = 0b00001000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_alpha_test           = 0b00010000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_alpha                = 0b00100000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_fog                  = 0b01000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_oit                  = 0b10000000'00000000'00000000'00000000'00000000'00000000'00000000'00000000,
		rs_fog_mode_mask        = 0b00000000'00000000'00000011'00000000'00000000'00000000'00000000'00000000,
		rs_alpha_test_mode_mask = 0b00000000'00000000'00000000'11110000'00000000'00000000'00000000'00000000,
		fvf_position            = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'00001110,
		fvf_fields              = 0b00000000'00000000'00000000'00000000'00000000'00000000'00000000'11110000,
		fvf_texcount            = 0b00000000'00000000'00000000'00000000'00000000'00000000'00001111'00000000,
		fvf_lastbeta            = 0b00000000'00000000'00000000'00000000'00000000'00000000'00010000'00000000,
		fvf_texfmt              = 0b00000000'00000000'00000000'00000000'11111111'11111111'00000000'00000000,
		stage_count_mask        = 0b00000000'00000000'00000000'00001111'00000000'00000000'00000000'00000000,
		fvf_mask                = fvf_position | fvf_fields | fvf_texcount | fvf_lastbeta | fvf_texfmt,
		rs_mask                 = rs_lighting | rs_specular | rs_alpha | rs_alpha_test | rs_fog | rs_oit | rs_fog_mode_mask | rs_alpha_test_mode_mask,
		mask                    = rs_mask | fvf_mask | stage_count_mask,
	};

	static constexpr type stage_count_shift        = 32;
	static constexpr type rs_alpha_test_mode_shift = 36;
	static constexpr type rs_fog_mode_shift        = 40;

	static constexpr type light_sanitize_flags = rs_lighting | rs_specular |
		D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL | D3DFVF_XYZRHW;

	static constexpr type vs_mask = rs_lighting | rs_specular | fvf_mask;
	static constexpr type ps_mask = stage_count_mask | rs_mask | light_sanitize_flags;

	static constexpr type uber_vs_mask = fvf_mask;
	static constexpr type uber_ps_mask = stage_count_mask | (light_sanitize_flags & ~rs_mask);

	static type sanitize(type flags);
};
