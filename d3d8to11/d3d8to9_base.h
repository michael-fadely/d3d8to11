#pragma once

#include <d3d11_1.h>
#include <vector>

#include "Unknown.h"
#include "d3d8types.hpp"

class Direct3DDevice8;
class __declspec(uuid("1DD9E8DA-1C77-4D40-B0CF-98FEFDFF9512")) Direct3D8;

class Direct3D8 : public Unknown
{
public:
	Direct3D8(const Direct3D8&)            = delete;
	Direct3D8& operator=(const Direct3D8&) = delete;

	Direct3D8()  = default;
	~Direct3D8() = default;

	void create_native();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE RegisterSoftwareDevice(void* pInitializeFunction);
	virtual UINT STDMETHODCALLTYPE GetAdapterCount();
	virtual HRESULT STDMETHODCALLTYPE GetAdapterIdentifier(UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER8* pIdentifier);
	virtual UINT STDMETHODCALLTYPE GetAdapterModeCount(UINT Adapter);
	virtual HRESULT STDMETHODCALLTYPE EnumAdapterModes(UINT Adapter, UINT Mode, D3DDISPLAYMODE* pMode);
	virtual HRESULT STDMETHODCALLTYPE GetAdapterDisplayMode(UINT Adapter, D3DDISPLAYMODE* pMode);
	virtual HRESULT STDMETHODCALLTYPE CheckDeviceType(UINT Adapter, D3DDEVTYPE CheckType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed);
	virtual HRESULT STDMETHODCALLTYPE CheckDeviceFormat(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat);
	virtual HRESULT STDMETHODCALLTYPE CheckDeviceMultiSampleType(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType);
	virtual HRESULT STDMETHODCALLTYPE CheckDepthStencilMatch(UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat);
	virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS8* pCaps);
	virtual HMONITOR STDMETHODCALLTYPE GetAdapterMonitor(UINT Adapter);
	virtual HRESULT STDMETHODCALLTYPE CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS8* pPresentationParameters, Direct3DDevice8** ppReturnedDeviceInterface);

private:
	static const UINT max_adapters                = 8;
	UINT current_adapter_count                    = 0;
	UINT current_adapter_mode_count[max_adapters] = {};
	std::vector<DXGI_MODE_DESC> current_adapter_modes[max_adapters];
};
