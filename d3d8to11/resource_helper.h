#pragma once
#include "d3d8to11.hpp"

bool are_lock_flags_valid(DWORD Usage, DWORD Flags);

D3D11_MAP d3dlock_to_map_type(DWORD Flags);
