#include "stdafx.h"
#include "ShaderFlags.h"

ShaderFlags::type ShaderFlags::sanitize(type flags)
{
	flags &= mask;

	if (flags & rs_lighting && !(flags & D3DFVF_NORMAL))
	{
		flags &= ~rs_lighting;
	}

	if (flags & rs_oit && !(flags & rs_alpha))
	{
		flags &= ~rs_oit;
	}

	return flags;
}
