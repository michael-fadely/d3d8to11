/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"

#include <d3d11_1.h>
#include <wrl/client.h>

#include "d3d8to9_base.h"

using namespace Microsoft::WRL;
using namespace d3d8to11;

static const D3DFORMAT ADAPTER_FORMATS[] = {
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

	for (size_t adapter_index = 0; adapter_index < max_adapters && result == S_OK; adapter_index++)
	{
		ComPtr<IDXGIAdapter> adapter;
		result = factory->EnumAdapters(adapter_index, &adapter);

		if (result != S_OK)
		{
			continue;
		}

		++current_adapter_count;

		ComPtr<IDXGIOutput> output;

		if (FAILED(adapter->EnumOutputs(0, &output)))
		{
			continue;
		}

		for (auto format : ADAPTER_FORMATS)
		{
			const DXGI_FORMAT dxgi = d3d8to11::to_dxgi(format);

			UINT count = 0;
			auto hr = output->GetDisplayModeList(dxgi, 0, &count, nullptr);

			if (FAILED(hr))
			{
				throw std::runtime_error("GetDisplayModeList falied");
			}

			if (!count)
			{
				continue;
			}

			std::vector<DXGI_MODE_DESC> modes(count);

			hr = output->GetDisplayModeList(dxgi, 0, &count, modes.data());

			if (FAILED(hr))
			{
				continue;
			}

			auto& stored_modes = current_adapter_modes[adapter_index];
			stored_modes.insert(stored_modes.end(), modes.begin(), modes.end());
			current_adapter_mode_count[adapter_index] += modes.size();
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
	return D3DERR_INVALIDCALL;
}

UINT STDMETHODCALLTYPE Direct3D8::GetAdapterCount()
{
	return current_adapter_count;
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
	return current_adapter_mode_count[Adapter];
}

HRESULT STDMETHODCALLTYPE Direct3D8::EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode)
{
	if (pMode == nullptr || !(Adapter < current_adapter_count && Mode < current_adapter_mode_count[Adapter]))
	{
		return D3DERR_INVALIDCALL;
	}

	auto& mode = current_adapter_modes[Adapter];

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

	const auto& element = *(current_adapter_modes[Adapter].end() - 1);

	pMode->Format      = to_d3d8(element.Format);
	pMode->Width       = element.Width;
	pMode->Height      = element.Height;
	pMode->RefreshRate = element.RefreshRate.Numerator / element.RefreshRate.Denominator;

	return D3D_OK;
	//return ProxyInterface->GetAdapterDisplayMode(Adapter, pMode);
}

HRESULT STDMETHODCALLTYPE Direct3D8::CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed)
{
	const auto& modes = current_adapter_modes[Adapter];

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
	return D3DERR_INVALIDCALL;
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
	if (pCaps == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

#if 1
	// TODO
	memset(pCaps, 0xFF, sizeof(D3DCAPS8));
	return D3D_OK;
#else
	D3DCAPS9 DeviceCaps;

	const HRESULT hr = ProxyInterface->GetDeviceCaps(Adapter, DeviceType, &DeviceCaps);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertCaps(DeviceCaps, *pCaps);

	return D3D_OK;
#endif
}

HMONITOR STDMETHODCALLTYPE Direct3D8::GetAdapterMonitor(UINT Adapter)
{
	// TODO
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
