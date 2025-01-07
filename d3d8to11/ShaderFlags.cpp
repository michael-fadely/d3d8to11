#include "pch.h"
#include "ShaderFlags.h"

ShaderFlags::type ShaderFlags::sanitize(type flags)
{
	flags &= mask;

	if (flags & rs_lighting && (flags & D3DFVF_XYZRHW))
	{
		flags ^= rs_lighting;
	}

	if (flags & rs_oit && !(flags & rs_alpha))
	{
		flags ^= rs_oit;
	}

	if (flags & rs_alpha_test_mode_mask)
	{
		if (!(flags & rs_alpha_test))
		{
			flags &= ~rs_alpha_test_mode_mask;
		}
	}
	else if (flags & rs_alpha_test)
	{
		flags ^= rs_alpha_test;
	}

	if (flags & rs_fog_mode_mask)
	{
		if (!(flags & rs_fog))
		{
			flags &= ~rs_fog_mode_mask;
		}
	}
	else if (flags & rs_fog)
	{
		flags ^= rs_fog;
	}

	return flags;
}
