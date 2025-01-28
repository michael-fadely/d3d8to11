#pragma once

#include <d3d11_1.h>

#include <array>
#include <vector>

#include "Unknown.h"
#include "d3d8types.hpp"

class Direct3DDevice8;
class __declspec(uuid("1DD9E8DA-1C77-4D40-B0CF-98FEFDFF9512")) Direct3D8;

// the destructor cannot be virtual because that would change the layout of the vtable
// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class Direct3D8 : public Unknown
{
public:
	Direct3D8(const Direct3D8&)     = delete;
	Direct3D8(Direct3D8&&) noexcept = delete;

	Direct3D8& operator=(const Direct3D8&)     = delete;
	Direct3D8& operator=(Direct3D8&&) noexcept = delete;

	Direct3D8() = default;
	~Direct3D8() = default;

	void create_native();

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
	ULONG STDMETHODCALLTYPE AddRef() override;
	ULONG STDMETHODCALLTYPE Release() override;

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
	static constexpr UINT MAX_ADAPTERS = 8;
	UINT m_current_adapter_count = 0;
	std::array<UINT, MAX_ADAPTERS> m_current_adapter_mode_count {};
	std::array<std::vector<DXGI_MODE_DESC>, MAX_ADAPTERS> m_current_adapter_modes;
};
