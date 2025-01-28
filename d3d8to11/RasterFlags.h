#pragma once

#include <cstdint>

struct RasterFlags
{
	enum T : uint32_t
	{
		cull_none = 1,
		cull_front,
		cull_back,
		fill_wireframe = 2 << 2,
		fill_solid = 3 << 2
	};

	static constexpr uint32_t cull_mask = 0b0011;
	static constexpr uint32_t fill_mask = 0b1100;
};
