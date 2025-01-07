/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "pch.h"
#include "d3d8to11.hpp"
#include "not_implemented.h"
#include "resource_helper.h"

void Direct3DVertexBuffer8::create_native()
{
	const auto& device = device8->device;

	D3D11_BUFFER_DESC desc = {};

	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.ByteWidth = desc8.Size;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	if (FAILED(device->CreateBuffer(&desc, nullptr, &buffer_resource)))
	{
		throw std::runtime_error("vertex buffer_resource creation failed");
	}
}

// IDirect3DVertexBuffer8
Direct3DVertexBuffer8::Direct3DVertexBuffer8(Direct3DDevice8* Device, UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool)
	: device8(Device)
{
	desc8.Type  = D3DRTYPE_VERTEXBUFFER;
	desc8.Size  = Length;
	desc8.Usage = Usage;
	desc8.FVF   = FVF;
	desc8.Pool  = Pool;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
	    riid == __uuidof(IUnknown) ||
	    riid == __uuidof(Direct3DResource8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Direct3DVertexBuffer8::AddRef()
{
	return Direct3DResource8::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DVertexBuffer8::Release()
{
	const auto result = Direct3DResource8::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::GetDevice(Direct3DDevice8** ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	device8->AddRef();

	*ppDevice = device8;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DVertexBuffer8::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DVertexBuffer8::GetPriority()
{
	NOT_IMPLEMENTED_RETURN;
}

void STDMETHODCALLTYPE Direct3DVertexBuffer8::PreLoad()
{
	NOT_IMPLEMENTED;
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DVertexBuffer8::GetType()
{
	return D3DRTYPE_VERTEXBUFFER;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags)
{
	if (!ppbData)
	{
		return D3DERR_INVALIDCALL;
	}

	if (OffsetToLock >= desc8.Size)
	{
		return D3DERR_INVALIDCALL;
	}

	const uint64_t remainder = desc8.Size - OffsetToLock;

	if (SizeToLock > remainder)
	{
		return D3DERR_INVALIDCALL;
	}

	if (!are_lock_flags_valid(desc8.Usage, Flags))
	{
		return D3DERR_INVALIDCALL;
	}

	const auto map_type = d3dlock_to_map_type(Flags);

	const auto& context = device8->context;
	D3D11_MAPPED_SUBRESOURCE mapped_resource {};
	const HRESULT result = context->Map(buffer_resource.Get(), 0, map_type, 0, &mapped_resource);

	if (result != S_OK)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppbData = static_cast<BYTE*>(mapped_resource.pData) + OffsetToLock;

	++lock_count;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::Unlock()
{
	if (!lock_count)
	{
		return D3DERR_INVALIDCALL;
	}

	const auto& context = device8->context;
	context->Unmap(buffer_resource.Get(), 0);
	--lock_count;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVertexBuffer8::GetDesc(D3DVERTEXBUFFER_DESC* pDesc)
{
	if (!pDesc)
	{
		return D3DERR_INVALIDCALL;
	}

	*pDesc = desc8;
	return D3D_OK;
}
