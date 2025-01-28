/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "pch.h"
#include "d3d8to11.hpp"
#include "not_implemented.h"

using namespace d3d8to11;

// IDirect3DSurface8
Direct3DSurface8::Direct3DSurface8(Direct3DDevice8* device, Direct3DTexture8* parent, UINT level)
	: m_parent(parent),
	  m_device8(device),
	  m_level(level)
{
	auto width  = m_parent->get_width();
	auto height = m_parent->get_height();

	for (size_t i = 0; i < m_level && width > 1 && height > 1; ++i)
	{
		width  = std::max(1u, width / 2);
		height = std::max(1u, height / 2);
	}

	m_desc8.Format          = m_parent->get_d3d8_format();
	m_desc8.Type            = m_parent->GetType();
	m_desc8.Usage           = m_parent->get_d3d8_usage();
	m_desc8.Pool            = m_parent->get_d3d8_pool();
	m_desc8.Size            = calc_texture_size(width, height, 1, m_parent->get_d3d8_format());
	m_desc8.MultiSampleType = D3DMULTISAMPLE_NONE;
	m_desc8.Width           = width;
	m_desc8.Height          = height;

	if (m_parent->is_render_target())
	{
		m_rt_desc.Format             = m_parent->get_native_desc().Format;
		m_rt_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
		m_rt_desc.Texture2D.MipSlice = m_level;
	}

	if (m_parent->is_depth_stencil())
	{
		m_depth_vdesc.Format        = typeless_to_depth(m_parent->get_native_desc().Format);
		m_depth_vdesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	}
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::QueryInterface(REFIID riid, void** ppvObj)
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
	const auto result = Unknown::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetDevice(Direct3DDevice8** ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	m_device8->AddRef();

	*ppDevice = m_device8;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetContainer(REFIID riid, void** ppContainer)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::GetDesc(D3DSURFACE_DESC8* pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*pDesc = m_desc8;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::LockRect(D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
{
	return m_parent->LockRect(m_level, pLockedRect, pRect, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DSurface8::UnlockRect()
{
	return m_parent->UnlockRect(m_level);
}

void Direct3DSurface8::create_native()
{
	ID3D11Device* device = m_device8->get_native_device();

	if (m_parent)
	{
		if (m_parent->is_render_target())
		{
			auto hr = device->CreateRenderTargetView(m_parent->get_native_texture(), /*&rt_desc*/ nullptr, &m_render_target);

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateRenderTargetView failed");
			}

			m_render_target->GetDesc(&m_rt_desc);
		}

		if (m_parent->is_depth_stencil())
		{
			auto hr = device->CreateDepthStencilView(m_parent->get_native_texture(), &m_depth_vdesc, &m_depth_stencil);

			if (FAILED(hr))
			{
				throw std::runtime_error("CreateDepthStencilView failed");
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};

			// create a shader resource view with a readable pixel format
			auto srv_format = typeless_to_float(m_depth_vdesc.Format);

			// if float didn't work, it's probably int we want
			if (srv_format == DXGI_FORMAT_UNKNOWN)
			{
				srv_format = typeless_to_unorm(m_depth_vdesc.Format);
			}

			srv_desc.Format                    = srv_format;
			srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
			srv_desc.Texture2D.MostDetailedMip = 0;
			srv_desc.Texture2D.MipLevels       = 1;

			hr = device->CreateShaderResourceView(m_parent->get_native_texture(), &srv_desc, &m_depth_srv);

			if (FAILED(hr))
			{
				throw std::runtime_error("failed to create depth srv");
			}
		}
	}
}
