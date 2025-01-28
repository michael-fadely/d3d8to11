#pragma once

#include <cstdint>

#include <dirty_t.h>

#include "hash_combine.h"

struct DepthFlags
{
	using type = uint32_t;

	enum T : type
	{
		comparison_mask = 0xF
	};
};

struct StencilFlags
{
	using type = uint32_t;

	static constexpr type op_mask = 0xF;

	static constexpr type fail_shift  = 0;
	static constexpr type zfail_shift = 4;
	static constexpr type pass_shift  = 8;
	static constexpr type func_shift  = 12;

	static constexpr type rw_mask = 0xFF;

	static constexpr type read_shift  = 16;
	static constexpr type write_shift = 24;
};

struct DepthStencilFlags : dirty_impl
{
	using type = uint32_t;

	enum T : type
	{
		depth_test_enabled  = 1 << 0,
		depth_write_enabled = 1 << 1,
		stencil_enabled     = 1 << 2,
	};

	dirty_t<type> flags = dirty_t<type>(0);
	dirty_t<DepthFlags::type> depth_flags = dirty_t<DepthFlags::type>(0);
	dirty_t<StencilFlags::type> stencil_flags = dirty_t<StencilFlags::type>(0);

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;

	bool operator==(const DepthStencilFlags& rhs) const;
};

template <>
struct std::hash<DepthStencilFlags>
{
	std::size_t operator()(const DepthStencilFlags& s) const noexcept
	{
		size_t h = std::hash<size_t>()(s.flags.data());

		hash_combine(h, static_cast<size_t>(s.depth_flags.data()));
		hash_combine(h, static_cast<size_t>(s.stencil_flags.data()));

		return h;
	}
};
