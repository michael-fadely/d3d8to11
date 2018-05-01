/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8to9.hpp"

#ifndef D3D8TO9NOLOG
 // Very simple logging for the purpose of debugging only.
std::ofstream LOG;
#endif

extern "C" Direct3D8 *WINAPI Direct3DCreate8(UINT SDKVersion)
{
#ifndef D3D8TO9NOLOG
	static bool LogMessageFlag = true;

	if (!LOG.is_open())
	{
		LOG.open("d3d8.log", std::ios::trunc);
	}

	if (!LOG.is_open() && LogMessageFlag)
	{
		LogMessageFlag = false;
		MessageBox(nullptr, TEXT("Failed to open debug log file \"d3d8.log\"!"), nullptr, MB_ICONWARNING);
	}

	LOG << "Redirecting '" << "Direct3DCreate8" << "(" << SDKVersion << ")' ..." << std::endl;
	LOG << "> Passing on to 'Direct3DCreate9':" << std::endl;
#endif

	auto result = new Direct3D8();

	try
	{
		result->create_native();
		return result;
	}
	catch (std::exception& ex)
	{
		delete result;
		PrintDebug("%s\n", ex.what());
		return nullptr;
	}
}

DXGI_FORMAT to_dxgi(D3DFORMAT value)
{
	switch (value)
	{
		case D3DFMT_UNKNOWN:
			return DXGI_FORMAT_UNKNOWN;

		case D3DFMT_D24X8:
			return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		case D3DFMT_D24S8:
			return DXGI_FORMAT_D24_UNORM_S8_UINT;

		case D3DFMT_D32:
			return DXGI_FORMAT_D32_FLOAT;

		case D3DFMT_A8R8G8B8:
			return DXGI_FORMAT_B8G8R8A8_UNORM;// & DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

		case D3DFMT_X8R8G8B8:
			return DXGI_FORMAT_B8G8R8X8_UNORM; // & DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

		case D3DFMT_R5G6B5:
			return DXGI_FORMAT_B5G6R5_UNORM;

		case D3DFMT_A1R5G5B5:
			return DXGI_FORMAT_B5G5R5A1_UNORM;

		case D3DFMT_A4R4G4B4:
			return DXGI_FORMAT_B4G4R4A4_UNORM;

		case D3DFMT_A8:
			return DXGI_FORMAT_A8_UNORM;

		case D3DFMT_A2B10G10R10:
			return DXGI_FORMAT_R10G10B10A2_UNORM;

		case D3DFMT_G16R16:
			return DXGI_FORMAT_R16G16_UNORM;

		case D3DFMT_L8:
			return DXGI_FORMAT_R8_UNORM;

		case D3DFMT_A8L8:
			return DXGI_FORMAT_R8G8_UNORM;

		case D3DFMT_V8U8:
			return DXGI_FORMAT_R8G8_SNORM;

		case D3DFMT_Q8W8V8U8:
			return DXGI_FORMAT_R8G8B8A8_SNORM;

		case D3DFMT_V16U16:
			return DXGI_FORMAT_R16G16_SNORM;

		case D3DFMT_DXT1:
			return DXGI_FORMAT_BC1_UNORM; // & DXGI_FORMAT_BC1_UNORM_SRGB;

		case D3DFMT_DXT2:
			return DXGI_FORMAT_BC1_UNORM; // & DXGI_FORMAT_BC1_UNORM_SRGB;

		case D3DFMT_DXT3:
			return DXGI_FORMAT_BC2_UNORM; // & DXGI_FORMAT_BC2_UNORM_SRGB;

		case D3DFMT_DXT4:
			return DXGI_FORMAT_BC2_UNORM; // & DXGI_FORMAT_BC2_UNORM_SRGB;

		case D3DFMT_DXT5:
			return DXGI_FORMAT_BC3_UNORM; // & DXGI_FORMAT_BC3_UNORM_SRGB;

		case D3DFMT_D16:
		case D3DFMT_D16_LOCKABLE:
			return DXGI_FORMAT_D16_UNORM;

		case D3DFMT_INDEX16:
			return DXGI_FORMAT_R16_UINT;

		case D3DFMT_INDEX32:
			return DXGI_FORMAT_R32_UINT;

		case /*FourCC 'ATI1'*/ MAKEFOURCC('A', 'T', 'I', '1'):
			return DXGI_FORMAT_BC4_UNORM;

		case /*FourCC 'ATI2'*/ MAKEFOURCC('A', 'T', 'I', '2'):
			return DXGI_FORMAT_BC5_UNORM;

		default:
			break;
	}

	throw std::runtime_error("unsupported D3DFORMAT");
}

D3DFORMAT to_d3d8(DXGI_FORMAT value)
{
	switch (value)
	{
		case DXGI_FORMAT_UNKNOWN:
			return D3DFMT_UNKNOWN;

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			return D3DFMT_A8R8G8B8;

		case DXGI_FORMAT_B8G8R8X8_UNORM:
		case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			return D3DFMT_X8R8G8B8;

		case DXGI_FORMAT_B5G6R5_UNORM:
			return D3DFMT_R5G6B5;

		case DXGI_FORMAT_B5G5R5A1_UNORM:
			return D3DFMT_A1R5G5B5;

		case DXGI_FORMAT_B4G4R4A4_UNORM:
			return D3DFMT_A4R4G4B4;

		case DXGI_FORMAT_A8_UNORM:
			return D3DFMT_A8;

		case DXGI_FORMAT_R10G10B10A2_UNORM:
			return D3DFMT_A2B10G10R10;

		case DXGI_FORMAT_R16G16_UNORM:
			return D3DFMT_G16R16;

		case DXGI_FORMAT_R8_UNORM:
			return D3DFMT_L8;

		case DXGI_FORMAT_R8G8_UNORM:
			return D3DFMT_A8L8;

		case DXGI_FORMAT_R8G8_SNORM:
			return D3DFMT_V8U8;

		case DXGI_FORMAT_R8G8B8A8_SNORM:
			return D3DFMT_Q8W8V8U8;

		case DXGI_FORMAT_R16G16_SNORM:
			return D3DFMT_V16U16;

		case DXGI_FORMAT_BC1_UNORM:
		case DXGI_FORMAT_BC1_UNORM_SRGB:
			return D3DFMT_DXT1;

			/*case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			return D3DFMT_DXT2;*/

		case DXGI_FORMAT_BC2_UNORM:
		case DXGI_FORMAT_BC2_UNORM_SRGB:
			return D3DFMT_DXT3;

			/*case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			return D3DFMT_DXT4;*/

		case DXGI_FORMAT_BC3_UNORM:
		case DXGI_FORMAT_BC3_UNORM_SRGB:
			return D3DFMT_DXT5;

		case DXGI_FORMAT_D16_UNORM:
			return D3DFMT_D16;
			//return D3DFMT_D16_LOCKABLE;

		case DXGI_FORMAT_R16_UINT:
			return D3DFMT_INDEX16;

		case DXGI_FORMAT_R32_UINT:
			return D3DFMT_INDEX32;

		case DXGI_FORMAT_BC4_UNORM:
			return /*FourCC 'ATI1'*/ static_cast<D3DFORMAT>(MAKEFOURCC('A', 'T', 'I', '1'));

		case DXGI_FORMAT_BC5_UNORM:
			return /*FourCC 'ATI2'*/ static_cast<D3DFORMAT>(MAKEFOURCC('A', 'T', 'I', '2'));

		default:
			break;
	}

	throw std::runtime_error("unsupported DXGI_FORMAT");
}

D3D11_PRIMITIVE_TOPOLOGY to_d3d11(D3DPRIMITIVETYPE value)
{
	switch (value)
	{
		case D3DPT_POINTLIST:
			return D3D11_PRIMITIVE_TOPOLOGY_POINTLIST;
		case D3DPT_LINELIST:
			return D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		case D3DPT_LINESTRIP:
			return D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case D3DPT_TRIANGLELIST:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN:
			return D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
		//case D3DPT_TRIANGLEFAN:
			//return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
		default:
			return D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}
