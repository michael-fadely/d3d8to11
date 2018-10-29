/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8to9.hpp"

// IDirect3DSurface8
Direct3DSurface8::Direct3DSurface8(Direct3DDevice8* device, Direct3DTexture8* parent_, UINT level_)
	: Device(device),
	  parent(parent_),
	  level(level_)
{
	auto width = parent->Width;
	auto height = parent->Height;

	for (size_t i = 0; i < level && width > 1 && height > 1; ++i)
	{
		width = std::max(1u, width / 2);
		height = std::max(1u, height / 2);
	}

	desc8.Format          = parent->Format;
	desc8.Type            = parent->GetType();
	desc8.Usage           = parent->Usage;
	desc8.Pool            = parent->Pool;
	desc8.Size            = calc_texture_size(width, height, 1, parent->Format);
	desc8.MultiSampleType = D3DMULTISAMPLE_NONE;
	desc8.Width           = width;
	desc8.Height          = height;

	if (parent->is_render_target)
	{
		rt_desc.Format             = parent->desc.Format;
		rt_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
		rt_desc.Texture2D.MipSlice = level;
	}

	if (parent->is_depth_stencil)
	{
		depth_vdesc.Format        = typeless_to_depth(parent->desc.Format);
		depth_vdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	}
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::QueryInterface(REFIID riid, void **ppvObj)
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

ULONG STDMETHODCALLTYPE Direct3DSurface8::AddRef()
{
	return Unknown::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DSurface8::Release()
{
	auto result = Unknown::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::FreePrivateData(REFGUID refguid)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->FreePrivateData(refguid);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetContainer(REFIID riid, void **ppContainer)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->GetContainer(riid, ppContainer);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetDesc(D3DSURFACE_DESC8 *pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*pDesc = desc8;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::LockRect(D3DLOCKED_RECT *pLockedRect, const RECT *pRect, DWORD Flags)
{
	return parent->LockRect(level, pLockedRect, pRect, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::UnlockRect()
{
	return parent->UnlockRect(level);
}

void Direct3DSurface8::create_native()
{
	auto device = Device->device;

	if (parent)
	{
		if (parent->is_render_target)
		{
			auto hr = device->CreateRenderTargetView(parent->texture.Get(), &rt_desc, &render_target);

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateRenderTargetView failed");
			}
		}

		if (parent->is_depth_stencil)
		{

			auto hr = device->CreateDepthStencilView(parent->texture.Get(), &depth_vdesc, &depth_stencil);

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateDepthStencilView failed");
			}
		}
	}
}
