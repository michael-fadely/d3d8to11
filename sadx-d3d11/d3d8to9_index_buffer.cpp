/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "stdafx.h"
#include "d3d8to9.hpp"

void Direct3DIndexBuffer8::create_native()
{
	auto device = Device->device;

	D3D11_BUFFER_DESC desc = {};

	desc.Usage          = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth      = desc8.Size;
	desc.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	if (FAILED(device->CreateBuffer(&desc, nullptr, &buffer_resource)))
	{
		throw std::runtime_error("index buffer_resource creation failed");
	}

	buffer.resize(desc8.Size);
	buffer.shrink_to_fit();
}
// IDirect3DIndexBuffer8
Direct3DIndexBuffer8::Direct3DIndexBuffer8(Direct3DDevice8* Device, UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool) :
	Device(Device)
{
	desc8.Type   = D3DRTYPE_INDEXBUFFER;
	desc8.Size   = Length;
	desc8.Usage  = Usage;
	desc8.Format = Format;
	desc8.Pool   = Pool;
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::QueryInterface(REFIID riid, void **ppvObj)
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

ULONG STDMETHODCALLTYPE Direct3DIndexBuffer8::AddRef()
{
	return Direct3DResource8::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DIndexBuffer8::Release()
{
	auto result = Direct3DResource8::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::FreePrivateData(REFGUID refguid)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->FreePrivateData(refguid);
#endif
}

DWORD STDMETHODCALLTYPE Direct3DIndexBuffer8::SetPriority(DWORD PriorityNew)
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->SetPriority(PriorityNew);
#endif
}

DWORD STDMETHODCALLTYPE Direct3DIndexBuffer8::GetPriority()
{
#if 1
	// not needed for SADX
	return D3DERR_INVALIDCALL;
#else
	return ProxyInterface->GetPriority();
#endif
}

void STDMETHODCALLTYPE Direct3DIndexBuffer8::PreLoad()
{
	// not needed for SADX
#if 0
	ProxyInterface->PreLoad();
#endif
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DIndexBuffer8::GetType()
{
	return D3DRTYPE_INDEXBUFFER;
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags)
{
	if (!ppbData || locked)
	{
		return D3DERR_INVALIDCALL;
	}

	if ((uint64_t)OffsetToLock + (uint64_t)SizeToLock > (uint64_t)buffer.size())
	{
		return D3DERR_INVALIDCALL;
	}

	locked = true;
	*ppbData = reinterpret_cast<BYTE*>(&buffer[OffsetToLock]);

	this->lock_offset = OffsetToLock;
	this->lock_size   = SizeToLock;
	this->lock_flags  = Flags;

	/*if (Flags & D3DLOCK_DISCARD)
	{
		memset(*ppbData, 0, SizeToLock);
	}*/

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::Unlock()
{
	if (!locked)
	{
		return D3DERR_INVALIDCALL;
	}

	locked = false;
	auto context = Device->context;

	D3D11_MAPPED_SUBRESOURCE mapped {};

	D3D11_MAP map_type = D3D11_MAP_WRITE_DISCARD;

	if (lock_flags & D3DLOCK_NOOVERWRITE)
	{
		map_type = D3D11_MAP_WRITE_NO_OVERWRITE;
	}

	auto hr = context->Map(buffer_resource.Get(), 0, map_type, 0, &mapped);

	if (FAILED(hr))
	{
		return D3DERR_INVALIDCALL;
	}

	if (mapped.pData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	memcpy(reinterpret_cast<uint8_t*>(mapped.pData) + lock_offset, buffer.data() + lock_offset, lock_size);

	context->Unmap(buffer_resource.Get(), 0);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DIndexBuffer8::GetDesc(D3DINDEXBUFFER_DESC *pDesc)
{
	if (!pDesc)
	{
		return D3DERR_INVALIDCALL;
	}

	*pDesc = desc8;
	return D3D_OK;
}
