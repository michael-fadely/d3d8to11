/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#pragma once

#include <d3d11_1.h>
#include "d3d8types.hpp"

//class __declspec(uuid("928C088B-76B9-4C6B-A536-A590853876CD")) Direct3DSwapChain8;
//class __declspec(uuid("3EE5B968-2ACA-4C34-8BB5-7E0C3D19B750")) Direct3DCubeTexture8;
//class __declspec(uuid("4B8AAAFA-140F-42BA-9131-597EAFAA2EAD")) Direct3DVolumeTexture8;
//class __declspec(uuid("BD7349F5-14F1-42E4-9C79-972380DB40C0")) Direct3DVolume8;

#include "d3d8types.hpp"
#include "d3d8to11_base.h"
#include "d3d8to11_device.h"
#include "d3d8to11_texture.h"
#include "d3d8to11_surface.h"
#include "d3d8to11_index_buffer.h"
#include "d3d8to11_vertex_buffer.h"

#include <filesystem>

extern "C" Direct3D8* WINAPI Direct3DCreate8(UINT SDKVersion);

namespace d3d8to11
{
	extern const std::filesystem::path storage_directory;
	extern const std::filesystem::path config_file_path;
	extern const std::filesystem::path permutation_file_path;

	DXGI_FORMAT to_dxgi(D3DFORMAT value);
	D3DFORMAT to_d3d8(DXGI_FORMAT value);
	D3D11_PRIMITIVE_TOPOLOGY to_d3d11(D3DPRIMITIVETYPE value);
	DXGI_FORMAT to_typeless(DXGI_FORMAT format);
	DXGI_FORMAT typeless_to_depth(DXGI_FORMAT format);
	DXGI_FORMAT typeless_to_uint(DXGI_FORMAT format);
	DXGI_FORMAT typeless_to_sint(DXGI_FORMAT format);
	DXGI_FORMAT typeless_to_snorm(DXGI_FORMAT format);
	DXGI_FORMAT typeless_to_float(DXGI_FORMAT format);
	DXGI_FORMAT typeless_to_unorm(DXGI_FORMAT format, bool srgb = false);
	size_t dxgi_stride(DXGI_FORMAT format);
	D3D11_FILTER to_d3d11(D3DTEXTUREFILTERTYPE min, D3DTEXTUREFILTERTYPE mag, D3DTEXTUREFILTERTYPE mip);
	bool is_block_compressed(DXGI_FORMAT value);
}

// TODO: Direct3DSwapChain8
#if 0

class Direct3DSwapChain8 : public Unknown
{
	Direct3DSwapChain8(const Direct3DSwapChain8 &) = delete;
	Direct3DSwapChain8 &operator=(const Direct3DSwapChain8 &) = delete;

public:
	Direct3DSwapChain8(Direct3DDevice8 *device, IDirect3DSwapChain9 *ProxyInterface);
	~Direct3DSwapChain8();

	IDirect3DSwapChain9 *GetProxyInterface() const { return ProxyInterface; }

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion);
	virtual HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, Direct3DSurface8 **ppBackBuffer);

private:
	Direct3DDevice8 *const Device;
	IDirect3DSwapChain9 *const ProxyInterface;
};

class Direct3DCubeTexture8 : public Direct3DBaseTexture8
{
	Direct3DCubeTexture8(const Direct3DCubeTexture8 &) = delete;
	Direct3DCubeTexture8 &operator=(const Direct3DCubeTexture8 &) = delete;

public:
	Direct3DCubeTexture8(Direct3DDevice8 *device, IDirect3DCubeTexture9 *ProxyInterface);
	~Direct3DCubeTexture8();

	IDirect3DCubeTexture9 *GetProxyInterface() const { return ProxyInterface; }

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8 **ppDevice) override;
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags) override;
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData) override;
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) override;
	virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) override;
	virtual DWORD STDMETHODCALLTYPE GetPriority() override;
	virtual void STDMETHODCALLTYPE PreLoad() override;
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override;

	virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) override;
	virtual DWORD STDMETHODCALLTYPE GetLOD() override;
	virtual DWORD STDMETHODCALLTYPE GetLevelCount() override;

	virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, D3DSURFACE_DESC8 *pDesc);
	virtual HRESULT STDMETHODCALLTYPE GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, Direct3DSurface8 **ppCubeMapSurface);
	virtual HRESULT STDMETHODCALLTYPE LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level);
	virtual HRESULT STDMETHODCALLTYPE AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT *pDirtyRect);

private:
	Direct3DDevice8 *const Device;
	IDirect3DCubeTexture9 *const ProxyInterface;
};

class Direct3DVolumeTexture8 : public Direct3DBaseTexture8
{
	Direct3DVolumeTexture8(const Direct3DVolumeTexture8 &) = delete;
	Direct3DVolumeTexture8 &operator=(const Direct3DVolumeTexture8 &) = delete;

public:
	Direct3DVolumeTexture8(Direct3DDevice8 *device, IDirect3DVolumeTexture9 *ProxyInterface);
	~Direct3DVolumeTexture8();

	IDirect3DVolumeTexture9 *GetProxyInterface() const { return ProxyInterface; }

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8 **ppDevice) override;
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags) override;
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData) override;
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) override;
	virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) override;
	virtual DWORD STDMETHODCALLTYPE GetPriority() override;
	virtual void STDMETHODCALLTYPE PreLoad() override;
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override;

	virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) override;
	virtual DWORD STDMETHODCALLTYPE GetLOD() override;
	virtual DWORD STDMETHODCALLTYPE GetLevelCount() override;

	virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, D3DVOLUME_DESC8 *pDesc);
	virtual HRESULT STDMETHODCALLTYPE GetVolumeLevel(UINT Level, Direct3DVolume8 **ppVolumeLevel);
	virtual HRESULT STDMETHODCALLTYPE LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE UnlockBox(UINT Level);
	virtual HRESULT STDMETHODCALLTYPE AddDirtyBox(const D3DBOX *pDirtyBox);

private:
	Direct3DDevice8 *const Device;
	IDirect3DVolumeTexture9 *const ProxyInterface;
};

class Direct3DVolume8 : public Unknown
{
	Direct3DVolume8(const Direct3DVolume8 &) = delete;
	Direct3DVolume8 &operator=(const Direct3DVolume8 &) = delete;

public:
	Direct3DVolume8(Direct3DDevice8 *Device, IDirect3DVolume9 *ProxyInterface);
	~Direct3DVolume8();

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8 **ppDevice);
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid);
	virtual HRESULT STDMETHODCALLTYPE GetContainer(REFIID riid, void **ppContainer);
	virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DVOLUME_DESC8 *pDesc);
	virtual HRESULT STDMETHODCALLTYPE LockBox(D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE UnlockBox();

private:
	Direct3DDevice8 *const Device;
};

#endif
