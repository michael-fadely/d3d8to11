/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "pch.h"

#include <d3d11_1.h>
#include <wrl/client.h>

#include "d3d8to11_base.h"

#include "not_implemented.h"

using namespace Microsoft::WRL;
using namespace d3d8to11;

static constexpr D3DFORMAT ADAPTER_FORMATS[] = {
	D3DFMT_A8R8G8B8,
	D3DFMT_X8R8G8B8,
	D3DFMT_R5G6B5,
	D3DFMT_X1R5G5B5,
	D3DFMT_A1R5G5B5
};

void Direct3D8::create_native()
{
	ComPtr<IDXGIFactory2> factory;

	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory.ReleaseAndGetAddressOf()))))
	{
		throw std::runtime_error("CreateDXGIFactory failed");
	}

	HRESULT result = S_OK;

	for (UINT adapter_index = 0; adapter_index < MAX_ADAPTERS && result == S_OK; adapter_index++)
	{
		ComPtr<IDXGIAdapter> adapter;
		result = factory->EnumAdapters(adapter_index, &adapter);

		if (result != S_OK)
		{
			continue;
		}

		ComPtr<IDXGIOutput> output;

		if (FAILED(adapter->EnumOutputs(0, &output)))
		{
			continue;
		}

		++m_current_adapter_count;

		for (const D3DFORMAT format : ADAPTER_FORMATS)
		{
			const DXGI_FORMAT dxgi_format = d3d8to11::to_dxgi(format);

			UINT count = 0;
			auto hr = output->GetDisplayModeList(dxgi_format, 0, &count, nullptr);

			if (FAILED(hr))
			{
				throw std::runtime_error("GetDisplayModeList failed");
			}

			if (!count)
			{
				continue;
			}

			std::vector<DXGI_MODE_DESC> modes(count);

			hr = output->GetDisplayModeList(dxgi_format, 0, &count, modes.data());

			if (FAILED(hr))
			{
				continue;
			}

			auto& stored_modes = m_current_adapter_modes[adapter_index];
			stored_modes.insert(stored_modes.end(), modes.begin(), modes.end());
			m_current_adapter_mode_count[adapter_index] += static_cast<UINT>(modes.size());
		}
	}
}

HRESULT STDMETHODCALLTYPE Direct3D8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
	    riid == __uuidof(IUnknown))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Direct3D8::AddRef()
{
	return Unknown::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3D8::Release()
{
	const ULONG LastRefCount = Unknown::Release();

	if (LastRefCount == 0)
	{
		delete this;
	}

	return LastRefCount;
}

HRESULT STDMETHODCALLTYPE Direct3D8::RegisterSoftwareDevice(void* pInitializeFunction)
{
	//return ProxyInterface->RegisterSoftwareDevice(pInitializeFunction);
	// TODO
	NOT_IMPLEMENTED_RETURN;
}

UINT STDMETHODCALLTYPE Direct3D8::GetAdapterCount()
{
	return m_current_adapter_count;
}

HRESULT STDMETHODCALLTYPE Direct3D8::GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier)
{
#if 1
	// TODO
	if (pIdentifier)
	{
		*pIdentifier = {};
		return D3D_OK; // HACK
	}

	return D3DERR_INVALIDCALL;
#else
	if (pIdentifier == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DADAPTER_IDENTIFIER9 AdapterIndentifier;

	if ((Flags & D3DENUM_NO_WHQL_LEVEL) == 0)
	{
		Flags |= D3DENUM_WHQL_LEVEL;
	}
	else
	{
		Flags ^= D3DENUM_NO_WHQL_LEVEL;
	}

	const HRESULT hr = ProxyInterface->GetAdapterIdentifier(Adapter, Flags, &AdapterIndentifier);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertAdapterIdentifier(AdapterIndentifier, *pIdentifier);

	return D3D_OK;
#endif
}

UINT STDMETHODCALLTYPE Direct3D8::GetAdapterModeCount(UINT Adapter)
{
	return m_current_adapter_mode_count[Adapter];
}

HRESULT STDMETHODCALLTYPE Direct3D8::EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
	if (pMode == nullptr || !(Adapter < m_current_adapter_count && Mode < m_current_adapter_mode_count[Adapter]))
	{
		return D3DERR_INVALIDCALL;
	}

	auto& mode = m_current_adapter_modes[Adapter];

	pMode->Format      = to_d3d8(mode.at(Mode).Format);
	pMode->Height      = mode.at(Mode).Height;
	pMode->RefreshRate = mode.at(Mode).RefreshRate.Numerator / mode.at(Mode).RefreshRate.Denominator;
	pMode->Width       = mode.at(Mode).Width;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3D8::GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode)
{
	if (!pMode)
	{
		return D3DERR_INVALIDCALL;
	}

	const auto& element = *(m_current_adapter_modes[Adapter].end() - 1);

	pMode->Format      = to_d3d8(element.Format);
	pMode->Width       = element.Width;
	pMode->Height      = element.Height;
	pMode->RefreshRate = element.RefreshRate.Numerator / element.RefreshRate.Denominator;

	return D3D_OK;
	//return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	const auto& modes = m_current_adapter_modes[Adapter];

	const DXGI_FORMAT format = to_dxgi(BackBufferFormat);

	for (const DXGI_MODE_DESC& mode : modes)
	{
		if (mode.Format == format)
		{
			return D3D_OK;
		}
	}

	return D3DERR_NOTAVAILABLE;
}

HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat)
{
#ifdef _DEBUG
	//printf(__FUNCTION__ " RType: %u, CheckFormat: %u\n", RType, CheckFormat);
#endif

	if (RType == D3DRTYPE_TEXTURE)
	{
		if (CheckFormat == D3DFMT_A8L8 || CheckFormat == D3DFMT_L8 || CheckFormat == D3DFMT_A8 || CheckFormat == D3DFMT_R8G8B8)
		{
			return D3DERR_NOTAVAILABLE;
		}
	}

	if (RType == D3DRTYPE_SURFACE && CheckFormat == D3DFMT_D32)
	{
		return D3DERR_NOTAVAILABLE;
	}

	// TODO
	return D3D_OK;
	//return ProxyInterface->CheckDeviceFormat(Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat);
}

HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType)
{
	// TODO
	NOT_IMPLEMENTED_RETURN;
	//return ProxyInterface->CheckDeviceMultiSampleType(Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, nullptr);
}

HRESULT STDMETHODCALLTYPE Direct3D8::CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat)
{
	// TODO
	return D3D_OK; // HACK
	//return ProxyInterface->CheckDepthStencilMatch(Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat);
}

HRESULT STDMETHODCALLTYPE Direct3D8::GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps)
{
	if (!pCaps)
	{
		return D3DERR_INVALIDCALL;
	}

	// TODO: Get capabilities from D3D11 and convert flags. This is all hard-coded.
	*pCaps = {};

	// TODO: D3DPTEXTURECAPS_ALPHAPALETTE
	// TODO: D3DPTEXTURECAPS_CUBEMAP
	// TODO: D3DPTEXTURECAPS_VOLUMEMAP
	// TODO: D3DPTEXTURECAPS_MIPVOLUMEMAP
	// TODO: D3DPTEXTURECAPS_MIPCUBEMAP

	pCaps->MaxTextureWidth          = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	pCaps->MaxTextureHeight         = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
	pCaps->Caps                     = 0;
	pCaps->Caps2                    = D3DCAPS2_CANRENDERWINDOWED |
	                                  D3DCAPS2_FULLSCREENGAMMA |
	                                  D3DCAPS2_DYNAMICTEXTURES;
	pCaps->Caps3                    = D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD;
	pCaps->PresentationIntervals    = D3DPRESENT_INTERVAL_ONE |
	                                  D3DPRESENT_INTERVAL_TWO |
	                                  D3DPRESENT_INTERVAL_THREE |
	                                  D3DPRESENT_INTERVAL_FOUR |
	                                  D3DPRESENT_INTERVAL_IMMEDIATE;
	pCaps->CursorCaps               = D3DCURSORCAPS_COLOR;
	pCaps->DevCaps                  = D3DDEVCAPS_EXECUTESYSTEMMEMORY |
	                                  D3DDEVCAPS_EXECUTEVIDEOMEMORY |
	                                  D3DDEVCAPS_TLVERTEXSYSTEMMEMORY |
	                                  D3DDEVCAPS_TLVERTEXVIDEOMEMORY |
	                                  D3DDEVCAPS_TEXTUREVIDEOMEMORY |
	                                  D3DDEVCAPS_DRAWPRIMTLVERTEX |
	                                  D3DDEVCAPS_CANRENDERAFTERFLIP |
	                                  D3DDEVCAPS_TEXTURENONLOCALVIDMEM |
	                                  D3DDEVCAPS_DRAWPRIMITIVES2 |
	                                  D3DDEVCAPS_DRAWPRIMITIVES2EX |
	                                  D3DDEVCAPS_HWTRANSFORMANDLIGHT |
	                                  D3DDEVCAPS_CANBLTSYSTONONLOCAL |
	                                  D3DDEVCAPS_HWRASTERIZATION |
	                                  D3DDEVCAPS_PUREDEVICE;
	pCaps->PrimitiveMiscCaps        = D3DPMISCCAPS_MASKZ |
	                                  D3DPMISCCAPS_CULLNONE |
	                                  D3DPMISCCAPS_CULLCW |
	                                  D3DPMISCCAPS_CULLCCW |
	                                  D3DPMISCCAPS_COLORWRITEENABLE |
	                                  D3DPMISCCAPS_CLIPTLVERTS |
	                                  D3DPMISCCAPS_TSSARGTEMP |
	                                  D3DPMISCCAPS_BLENDOP;
	pCaps->RasterCaps               = /*D3DPRASTERCAPS_DITHER |*/      // not true
	                                  D3DPRASTERCAPS_ZTEST |
	                                  D3DPRASTERCAPS_FOGVERTEX |
	                                  D3DPRASTERCAPS_FOGTABLE |        // not true
	                                  D3DPRASTERCAPS_ANTIALIASEDGES |  // not true
	                                  D3DPRASTERCAPS_MIPMAPLODBIAS |
	                                  D3DPRASTERCAPS_ZBIAS |           // not true
	                                  D3DPRASTERCAPS_FOGRANGE |
	                                  D3DPRASTERCAPS_ANISOTROPY |
	                                  D3DPRASTERCAPS_WFOG |
	                                  D3DPRASTERCAPS_ZFOG |
	                                  D3DPRASTERCAPS_COLORPERSPECTIVE |
	                                  D3DPRASTERCAPS_STRETCHBLTMULTISAMPLE;
	pCaps->ZCmpCaps                 = D3DPCMPCAPS_NEVER |
	                                  D3DPCMPCAPS_LESS |
	                                  D3DPCMPCAPS_EQUAL |
	                                  D3DPCMPCAPS_LESSEQUAL |
	                                  D3DPCMPCAPS_GREATER |
	                                  D3DPCMPCAPS_NOTEQUAL |
	                                  D3DPCMPCAPS_GREATEREQUAL |
	                                  D3DPCMPCAPS_ALWAYS;
	pCaps->SrcBlendCaps             = D3DPBLENDCAPS_ZERO |
	                                  D3DPBLENDCAPS_ONE |
	                                  D3DPBLENDCAPS_SRCCOLOR |
	                                  D3DPBLENDCAPS_INVSRCCOLOR |
	                                  D3DPBLENDCAPS_SRCALPHA |
	                                  D3DPBLENDCAPS_INVSRCALPHA |
	                                  D3DPBLENDCAPS_DESTALPHA |
	                                  D3DPBLENDCAPS_INVDESTALPHA |
	                                  D3DPBLENDCAPS_DESTCOLOR |
	                                  D3DPBLENDCAPS_INVDESTCOLOR |
	                                  D3DPBLENDCAPS_SRCALPHASAT |
	                                  D3DPBLENDCAPS_BOTHSRCALPHA |
	                                  D3DPBLENDCAPS_BOTHINVSRCALPHA;
	pCaps->DestBlendCaps            = pCaps->SrcBlendCaps;
	pCaps->AlphaCmpCaps             = D3DPCMPCAPS_NEVER |
	                                  D3DPCMPCAPS_LESS |
	                                  D3DPCMPCAPS_EQUAL |
	                                  D3DPCMPCAPS_LESSEQUAL |
	                                  D3DPCMPCAPS_GREATER |
	                                  D3DPCMPCAPS_NOTEQUAL |
	                                  D3DPCMPCAPS_GREATEREQUAL |
	                                  D3DPCMPCAPS_ALWAYS;
	pCaps->ShadeCaps                = D3DPSHADECAPS_COLORGOURAUDRGB |
	                                  D3DPSHADECAPS_SPECULARGOURAUDRGB |
	                                  D3DPSHADECAPS_ALPHAGOURAUDBLEND |
	                                  D3DPSHADECAPS_FOGGOURAUD;
	pCaps->TextureCaps              = D3DPTEXTURECAPS_PERSPECTIVE |
	                                  D3DPTEXTURECAPS_ALPHA |
	                                  D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE |
	                                  D3DPTEXTURECAPS_PROJECTED |
	                                  D3DPTEXTURECAPS_MIPMAP;
	pCaps->TextureFilterCaps        = D3DPTFILTERCAPS_MINFPOINT |
	                                  D3DPTFILTERCAPS_MINFLINEAR |
	                                  D3DPTFILTERCAPS_MINFANISOTROPIC |
	                                  D3DPTFILTERCAPS_MIPFPOINT |
	                                  D3DPTFILTERCAPS_MIPFLINEAR |
	                                  D3DPTFILTERCAPS_MAGFPOINT |
	                                  D3DPTFILTERCAPS_MAGFLINEAR;
	pCaps->CubeTextureFilterCaps    = pCaps->TextureFilterCaps & ~D3DPTFILTERCAPS_MINFANISOTROPIC;
	pCaps->VolumeTextureFilterCaps  = pCaps->TextureFilterCaps & ~D3DPTFILTERCAPS_MINFANISOTROPIC;
	pCaps->TextureAddressCaps       = D3DPTADDRESSCAPS_WRAP |
	                                  D3DPTADDRESSCAPS_MIRROR |
	                                  D3DPTADDRESSCAPS_CLAMP |
	                                  D3DPTADDRESSCAPS_BORDER |
	                                  D3DPTADDRESSCAPS_INDEPENDENTUV |  // is this true?
	                                  D3DPTADDRESSCAPS_MIRRORONCE;
	pCaps->VolumeTextureAddressCaps = pCaps->TextureAddressCaps;
	pCaps->LineCaps                 = 0; // TODO: LineCaps
	pCaps->MaxTextureRepeat         = 0xFFFFFFFF; // TODO: MaxTextureRepeat
	pCaps->MaxTextureAspectRatio    = 0xFFFFFFFF; // TODO: MaxTextureAspectRatio
	pCaps->MaxAnisotropy            = 16;
	pCaps->StencilCaps              = D3DSTENCILCAPS_KEEP |
	                                  D3DSTENCILCAPS_ZERO |
	                                  D3DSTENCILCAPS_REPLACE |
	                                  D3DSTENCILCAPS_INCRSAT |
	                                  D3DSTENCILCAPS_DECRSAT |
	                                  D3DSTENCILCAPS_INVERT |
	                                  D3DSTENCILCAPS_INCR |
	                                  D3DSTENCILCAPS_DECR;
	pCaps->FVFCaps                  = D3DFVFCAPS_DONOTSTRIPELEMENTS |
	                                  D3DFVFCAPS_PSIZE |
	                                  (FVF_TEXCOORD_MAX & D3DFVFCAPS_TEXCOORDCOUNTMASK);
	pCaps->TextureOpCaps            = D3DTEXOPCAPS_DISABLE |
	                                  D3DTEXOPCAPS_SELECTARG1 |
	                                  D3DTEXOPCAPS_SELECTARG2 |
	                                  D3DTEXOPCAPS_MODULATE |
	                                  D3DTEXOPCAPS_MODULATE2X |
	                                  D3DTEXOPCAPS_MODULATE4X |
	                                  D3DTEXOPCAPS_ADD |
	                                  D3DTEXOPCAPS_ADDSIGNED |
	                                  D3DTEXOPCAPS_ADDSIGNED2X |
	                                  D3DTEXOPCAPS_SUBTRACT |
	                                  D3DTEXOPCAPS_ADDSMOOTH |
	                                  D3DTEXOPCAPS_BLENDDIFFUSEALPHA |
	                                  D3DTEXOPCAPS_BLENDTEXTUREALPHA |
	                                  D3DTEXOPCAPS_BLENDFACTORALPHA |
	                                  D3DTEXOPCAPS_BLENDTEXTUREALPHAPM |
	                                  D3DTEXOPCAPS_BLENDCURRENTALPHA |
	                                  D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR |
	                                  D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA |
	                                  D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA |
	                                  D3DTEXOPCAPS_BUMPENVMAP |           // not true
	                                  D3DTEXOPCAPS_BUMPENVMAPLUMINANCE |  // not true
	                                  D3DTEXOPCAPS_DOTPRODUCT3 |
	                                  D3DTEXOPCAPS_MULTIPLYADD |
	                                  D3DTEXOPCAPS_LERP;
	pCaps->VertexProcessingCaps     = D3DVTXPCAPS_DIRECTIONALLIGHTS |
	                                  D3DVTXPCAPS_LOCALVIEWER |
	                                  D3DVTXPCAPS_MATERIALSOURCE7 |
	                                  D3DVTXPCAPS_POSITIONALLIGHTS |
	                                  D3DVTXPCAPS_TEXGEN;
	pCaps->MaxActiveLights          = LIGHT_COUNT;
	pCaps->MaxTextureBlendStages    = TEXTURE_STAGE_MAX;
	pCaps->MaxSimultaneousTextures  = TEXTURE_STAGE_MAX;

	return D3D_OK;
}

HMONITOR STDMETHODCALLTYPE Direct3D8::GetAdapterMonitor(UINT Adapter)
{
	// TODO
	NOT_IMPLEMENTED;
	return nullptr;
	//return ProxyInterface->GetAdapterMonitor(Adapter);
}

HRESULT STDMETHODCALLTYPE Direct3D8::CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags,
                                                  D3DPRESENT_PARAMETERS8* pPresentationParameters, Direct3DDevice8** ppReturnedDeviceInterface)
{
	if (pPresentationParameters == nullptr || ppReturnedDeviceInterface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppReturnedDeviceInterface = nullptr;

	const D3DPRESENT_PARAMETERS8 present_params = *pPresentationParameters;

	// Get multisample quality level
	if (present_params.MultiSampleType != D3DMULTISAMPLE_NONE)
	{
	#if 1
		throw;
	#else // TODO
		DWORD QualityLevels = 0;
		if (ProxyInterface->CheckDeviceMultiSampleType(Adapter,
			DeviceType, PresentParams.BackBufferFormat, PresentParams.Windowed,
			PresentParams.MultiSampleType, &QualityLevels) == D3D_OK &&
			ProxyInterface->CheckDeviceMultiSampleType(Adapter,
				DeviceType, PresentParams.AutoDepthStencilFormat, PresentParams.Windowed,
				PresentParams.MultiSampleType, &QualityLevels) == D3D_OK)
		{
			PresentParams.MultiSampleQuality = (QualityLevels != 0) ? QualityLevels - 1 : 0;
		}
	#endif
	}

	auto device = new Direct3DDevice8(this, Adapter, DeviceType, hFocusWindow, BehaviorFlags, present_params);
	device->AddRef();

	try
	{
		device->create_native();
		*ppReturnedDeviceInterface = device;
	}
	catch (std::exception& ex)
	{
		delete device;
		printf(__FUNCTION__ " %s\n", ex.what());
		return D3DERR_INVALIDCALL;
	}

	// Set default vertex declaration
	device->SetVertexShader(D3DFVF_XYZ);

	return D3D_OK;
}
