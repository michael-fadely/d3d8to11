#include "pch.h"
#include "resource_helper.h"

bool are_lock_flags_valid(DWORD Usage, DWORD Flags)
{
	if ((Flags & (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE)) == (D3DLOCK_DISCARD | D3DLOCK_NOOVERWRITE))
	{
		return false;
	}

	if ((Flags & D3DLOCK_READONLY) && ((Flags & D3DLOCK_DISCARD) || (Usage & D3DUSAGE_WRITEONLY)))
	{
		return false;
	}

	return true;
}

D3D11_MAP d3dlock_to_map_type(DWORD Flags)
{
	uint32_t map = D3D11_MAP_WRITE_DISCARD;

	if (Flags & D3DLOCK_READONLY)
	{
		map = D3D11_MAP_READ;
	}
	else if (Flags & D3DLOCK_DISCARD)
	{
		map = D3D11_MAP_WRITE_DISCARD;
	}
	else if (Flags & D3DLOCK_NOOVERWRITE)
	{
		map = D3D11_MAP_WRITE_NO_OVERWRITE;
	}

	return static_cast<D3D11_MAP>(map);
}
