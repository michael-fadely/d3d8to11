#include "pch.h"

#include "SamplerSettings.h"

SamplerSettings::SamplerSettings()
{
	address_u = D3DTADDRESS_WRAP;
	address_v = D3DTADDRESS_WRAP;
	address_w = D3DTADDRESS_WRAP;
	filter_mag = D3DTEXF_POINT;
	filter_min = D3DTEXF_POINT;
	filter_mip = D3DTEXF_NONE;
	mip_lod_bias = 0.0f;
	max_mip_level = 0;
	max_anisotropy = 1;
}

bool SamplerSettings::operator==(const SamplerSettings& s) const
{
	return address_u.data() == s.address_u.data() &&
	       address_v.data() == s.address_v.data() &&
	       address_w.data() == s.address_w.data() &&
	       filter_mag.data() == s.filter_mag.data() &&
	       filter_min.data() == s.filter_min.data() &&
	       filter_mip.data() == s.filter_mip.data() &&
	       mip_lod_bias.data() == s.mip_lod_bias.data() &&
	       max_mip_level.data() == s.max_mip_level.data() &&
	       max_anisotropy.data() == s.max_anisotropy.data();
}

bool SamplerSettings::dirty() const
{
	return address_u.dirty() ||
	       address_v.dirty() ||
	       address_w.dirty() ||
	       filter_mag.dirty() ||
	       filter_min.dirty() ||
	       filter_mip.dirty() ||
	       mip_lod_bias.dirty() ||
	       max_mip_level.dirty() ||
	       max_anisotropy.dirty();
}

void SamplerSettings::clear()
{
	address_u.clear();
	address_v.clear();
	address_w.clear();
	filter_mag.clear();
	filter_min.clear();
	filter_mip.clear();
	mip_lod_bias.clear();
	max_mip_level.clear();
	max_anisotropy.clear();
}

void SamplerSettings::mark()
{
	address_u.mark();
	address_v.mark();
	address_w.mark();
	filter_mag.mark();
	filter_min.mark();
	filter_mip.mark();
	mip_lod_bias.mark();
	max_mip_level.mark();
	max_anisotropy.mark();
}
