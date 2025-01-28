#pragma once

#include <cstdint>

#include <dirty_t.h>

#include "hash_combine.h"

struct SamplerSettings : dirty_impl
{
	dirty_t<D3DTEXTUREADDRESS> address_u, address_v, address_w;
	dirty_t<D3DTEXTUREFILTERTYPE> filter_mag, filter_min, filter_mip;
	dirty_t<float> mip_lod_bias;
	dirty_t<uint32_t> max_mip_level, max_anisotropy;

	SamplerSettings();

	bool operator==(const SamplerSettings& s) const;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

template <>
struct std::hash<SamplerSettings>
{
	std::size_t operator()(const SamplerSettings& s) const noexcept
	{
		size_t h = std::hash<size_t>()(s.address_u.data());

		hash_combine(h, static_cast<size_t>(s.address_v.data()));
		hash_combine(h, static_cast<size_t>(s.address_w.data()));
		hash_combine(h, static_cast<size_t>(s.filter_mag.data()));
		hash_combine(h, static_cast<size_t>(s.filter_min.data()));
		hash_combine(h, static_cast<size_t>(s.filter_mip.data()));
		hash_combine(h, s.mip_lod_bias.data());
		hash_combine(h, static_cast<size_t>(s.max_mip_level.data()));
		hash_combine(h, static_cast<size_t>(s.max_anisotropy.data()));

		return h;
	}
};
