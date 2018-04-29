/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8to9.hpp"

// TODO: instead of storing in map/vector, let d3d do the work if possible

void Direct3DTexture8::create_native()
{
	auto device  = Device->device;
	auto context = Device->context;

	if (!Levels)
	{
		++Levels;
		auto width  = Width;
		auto height = Height;

		while (width != 1 && height != 1)
		{
			++Levels;
			width  = std::max(1u, width / 2);
			height = std::max(1u, height / 2);
		}
	}

	uint32_t usage = D3D11_USAGE_DEFAULT;

	if (Usage == D3DUSAGE_DYNAMIC)
	{
		usage = D3D11_USAGE_DYNAMIC;
	}

	if (Usage == D3DUSAGE_RENDERTARGET)
	{
		throw std::runtime_error("D3DUSAGE_RENDERTARGET not supported");
	}

	desc.ArraySize      = 1;
	desc.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage          = static_cast<D3D11_USAGE>(usage);
	desc.Format         = to_dxgi(Format);
	desc.Width          = Width;
	desc.Height         = Height;
	desc.MipLevels      = Levels;
	desc.SampleDesc     = { 1, 0 };

	if (FAILED(device->CreateTexture2D(&desc, nullptr, &texture)))
	{
		throw std::runtime_error("CreateTexture2D failed");
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};

	srv_desc.Format              = desc.Format;
	srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MipLevels = Levels;

	if (FAILED(device->CreateShaderResourceView(texture.Get(), &srv_desc, &srv)))
	{
		throw std::runtime_error("CreateShaderResourceView failed");
	}
}
// IDirect3DTexture8
Direct3DTexture8::Direct3DTexture8(Direct3DDevice8 *device_, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool) :
	Device(device_)
{
	this->Width  = Width;
	this->Height = Height;
	this->Levels = Levels;
	this->Usage  = Usage;
	this->Format = Format;
	this->Pool   = Pool;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	// TODO
	throw;
	//return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DTexture8::AddRef()
{
	return Direct3DBaseTexture8::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DTexture8::Release()
{
	auto result = Direct3DBaseTexture8::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
#if 1
	// not used in SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
#if 1
	// not used in SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::FreePrivateData(REFGUID refguid)
{
#if 1
	// not used in SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->FreePrivateData(refguid);
#endif
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::SetPriority(DWORD PriorityNew)
{
#if 1
	// not used in SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->SetPriority(PriorityNew);
#endif
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetPriority()
{
#if 1
	// not used in SADX
	return -1;
#else
	return ProxyInterface->GetPriority();
#endif
}

void STDMETHODCALLTYPE Direct3DTexture8::PreLoad()
{
	// not used in SADX
#if 0
	ProxyInterface->PreLoad();
#endif
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DTexture8::GetType()
{
	return D3DRTYPE_TEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::SetLOD(DWORD LODNew)
{
#if 1
	// not used in SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->SetLOD(LODNew);
#endif
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetLOD()
{
#if 1
	// not used in SADX
	return 0;
#else
	return ProxyInterface->GetLOD();
#endif
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetLevelCount()
{
	return desc.MipLevels;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC8 *pDesc)
{
	if (pDesc == nullptr || Level > GetLevelCount())
	{
		return D3DERR_INVALIDCALL;
	}

	auto width = Width;
	auto height = Height;

	for (size_t i = 0; i < Level && width > 1 && height > 1; ++i)
	{
		width = std::max(1u, width / 2);
		height = std::max(1u, height / 2);
	}

	pDesc->Format          = Format;
	pDesc->Type            = GetType();
	pDesc->Usage           = Usage;
	pDesc->Pool            = Pool;
	pDesc->Size            = CalcTextureSize(width, height, 1, Format);
	pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
	pDesc->Width           = width;
	pDesc->Height          = height;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetSurfaceLevel(UINT Level, Direct3DSurface8 **ppSurfaceLevel)
{
	if (!ppSurfaceLevel)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurfaceLevel = nullptr;

	if (Level >= desc.MipLevels)
	{
		return D3DERR_INVALIDCALL;
	}

	try
	{
		*ppSurfaceLevel = new Direct3DSurface8(Device, this, Level);
	}
	catch (std::exception&)
	{
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::LockRect(UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags)
{
	if (pRect)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Level > desc.MipLevels)
	{
		return D3DERR_INVALIDCALL;
	}

	if (locked_rects.find(Level) != locked_rects.end())
	{
		return D3DERR_INVALIDCALL;
	}

	auto format = to_d3d8(desc.Format);

	auto width = Width;
	auto height = Height;

	for (size_t i = 0; i < Level && width > 1 && height > 1; ++i)
	{
		width = std::max(1u, width / 2);
		height = std::max(1u, height / 2);
	}

	D3DLOCKED_RECT rect;
	rect.Pitch = CalcTextureSize(width, 1, 1, format);

	auto it = texture_levels.find(Level);

	if (it != texture_levels.end())
	{
		auto& v = it->second;
		rect.pBits = v.data();
	}
	else
	{
		std::vector<uint8_t> v(CalcTextureSize(width, height, 1, format));
		rect.pBits = v.data();
		texture_levels[Level] = std::move(v);
	}

	locked_rects[Level] = rect;
	*pLockedRect = rect;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::UnlockRect(UINT Level)
{
	auto it = locked_rects.find(Level);

	if (it == locked_rects.end())
	{
		return D3DERR_INVALIDCALL;
	}

	auto& rect = it->second;
	auto context = Device->context;

	context->UpdateSubresource(texture.Get(), Level, nullptr, rect.pBits, rect.Pitch, 0);

	locked_rects.erase(it);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::AddDirtyRect(const RECT *pDirtyRect)
{
#if 1
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->AddDirtyRect(pDirtyRect);
#endif
}

#if 0
// IDirect3DCubeTexture8
Direct3DCubeTexture8::Direct3DCubeTexture8(Direct3DDevice8 *device, IDirect3DCubeTexture9 *ProxyInterface) :
	ProxyInterface(ProxyInterface),
	Device(device)
{
	Device->address_table->SaveAddress(this, ProxyInterface);
}

Direct3DCubeTexture8::~Direct3DCubeTexture8()
{
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DCubeTexture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DCubeTexture8::Release()
{
	return ProxyInterface->Release();
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::FreePrivateData(REFGUID refguid)
{
	return ProxyInterface->FreePrivateData(refguid);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::SetPriority(DWORD PriorityNew)
{
	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetPriority()
{
	return ProxyInterface->GetPriority();
}

void STDMETHODCALLTYPE Direct3DCubeTexture8::PreLoad()
{
	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DCubeTexture8::GetType()
{
	return D3DRTYPE_CUBETEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::SetLOD(DWORD LODNew)
{
	return ProxyInterface->SetLOD(LODNew);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetLOD()
{
	return ProxyInterface->GetLOD();
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetLevelCount()
{
	return ProxyInterface->GetLevelCount();
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC8 *pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DSURFACE_DESC SurfaceDesc;

	const HRESULT hr = ProxyInterface->GetLevelDesc(Level, &SurfaceDesc);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertSurfaceDesc(SurfaceDesc, *pDesc);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, Direct3DSurface8 **ppCubeMapSurface)
{
	if (ppCubeMapSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppCubeMapSurface = nullptr;

	IDirect3DSurface9 *SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetCubeMapSurface(FaceType, Level, &SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppCubeMapSurface = Device->address_table->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags)
{
	return ProxyInterface->LockRect(FaceType, Level, pLockedRect, pRect, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level)
{
	return ProxyInterface->UnlockRect(FaceType, Level);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT *pDirtyRect)
{
	return ProxyInterface->AddDirtyRect(FaceType, pDirtyRect);
}

#endif

#if 0
// IDirect3DVolumeTexture8
Direct3DVolumeTexture8::Direct3DVolumeTexture8(Direct3DDevice8 *device, IDirect3DVolumeTexture9 *ProxyInterface) :
	ProxyInterface(ProxyInterface),
	Device(device)
{
	Device->address_table->SaveAddress(this, ProxyInterface);
}

Direct3DVolumeTexture8::~Direct3DVolumeTexture8()
{
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DVolumeTexture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DVolumeTexture8::Release()
{
	return ProxyInterface->Release();
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::FreePrivateData(REFGUID refguid)
{
	return ProxyInterface->FreePrivateData(refguid);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::SetPriority(DWORD PriorityNew)
{
	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetPriority()
{
	return ProxyInterface->GetPriority();
}

void STDMETHODCALLTYPE Direct3DVolumeTexture8::PreLoad()
{
	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DVolumeTexture8::GetType()
{
	return D3DRTYPE_VOLUMETEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::SetLOD(DWORD LODNew)
{
	return ProxyInterface->SetLOD(LODNew);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLOD()
{
	return ProxyInterface->GetLOD();
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLevelCount()
{
	return ProxyInterface->GetLevelCount();
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLevelDesc(UINT Level, D3DVOLUME_DESC8 *pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DVOLUME_DESC VolumeDesc;

	const HRESULT hr = ProxyInterface->GetLevelDesc(Level, &VolumeDesc);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertVolumeDesc(VolumeDesc, *pDesc);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetVolumeLevel(UINT Level, Direct3DVolume8 **ppVolumeLevel)
{
	if (ppVolumeLevel == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppVolumeLevel = nullptr;

	IDirect3DVolume9 *VolumeInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetVolumeLevel(Level, &VolumeInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppVolumeLevel = Device->address_table->FindAddress<Direct3DVolume8>(VolumeInterface);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags)
{
	return ProxyInterface->LockBox(Level, pLockedVolume, pBox, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::UnlockBox(UINT Level)
{
	return ProxyInterface->UnlockBox(Level);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::AddDirtyBox(const D3DBOX *pDirtyBox)
{
	return ProxyInterface->AddDirtyBox(pDirtyBox);
}

#endif
