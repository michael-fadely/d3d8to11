/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8types.hpp"

UINT calc_texture_size(UINT Width, UINT Height, UINT Depth, D3DFORMAT Format)
{
	switch (static_cast<DWORD>(Format))
	{
		default:
		case D3DFMT_UNKNOWN:
			return 0;
		case D3DFMT_R3G3B2:
		case D3DFMT_A8:
		case D3DFMT_P8:
		case D3DFMT_L8:
		case D3DFMT_A4L4:
			return Width * Height * Depth;
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
		case D3DFMT_A4R4G4B4:
		case D3DFMT_A8R3G3B2:
		case D3DFMT_X4R4G4B4:
		case D3DFMT_A8P8:
		case D3DFMT_A8L8:
		case D3DFMT_V8U8:
		case D3DFMT_L6V5U5:
		case D3DFMT_D16_LOCKABLE:
		case D3DFMT_D15S1:
		case D3DFMT_D16:
		case D3DFMT_UYVY:
		case D3DFMT_YUY2:
			return Width * 2 * Height * Depth;
		case D3DFMT_R8G8B8:
			return Width * 3 * Height * Depth;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
		case D3DFMT_A2B10G10R10:
		case D3DFMT_G16R16:
		case D3DFMT_X8L8V8U8:
		case D3DFMT_Q8W8V8U8:
		case D3DFMT_V16U16:
		case D3DFMT_W11V11U10:
		case D3DFMT_A2W10V10U10:
		case D3DFMT_D32:
		case D3DFMT_D24S8:
		case D3DFMT_D24X8:
		case D3DFMT_D24X4S4:
			return Width * 4 * Height * Depth;
		case D3DFMT_DXT1:
			assert(Depth <= 1);
			return ((Width + 3) >> 2) * ((Height + 3) >> 2) * 8;
		case D3DFMT_DXT2:
		case D3DFMT_DXT3:
		case D3DFMT_DXT4:
		case D3DFMT_DXT5:
			assert(Depth <= 1);
			return ((Width + 3) >> 2) * ((Height + 3) >> 2) * 16;
	}
}

bool supports_palettes()
{
	HDC hDC = GetDC(nullptr);
	bool hasPalette = (GetDeviceCaps(hDC, RASTERCAPS) & RC_PALETTE) != 0;
	ReleaseDC(nullptr, hDC);
	return hasPalette;
}
