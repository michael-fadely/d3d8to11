#pragma once

#include "d3d8to9_resource.h"

class Direct3DDevice8;

class __declspec(uuid("0E689C9A-053D-44A0-9D92-DB0E3D750F86")) Direct3DIndexBuffer8;

class Direct3DIndexBuffer8 : public Direct3DResource8
{
public:
	Direct3DIndexBuffer8(const Direct3DIndexBuffer8&)            = delete;
	Direct3DIndexBuffer8& operator=(const Direct3DIndexBuffer8&) = delete;

	void create_native();

	Direct3DIndexBuffer8(Direct3DDevice8* Device, UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);
	~Direct3DIndexBuffer8() = default;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8** ppDevice) override;
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags) override;
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) override;
	virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) override;
	virtual DWORD STDMETHODCALLTYPE GetPriority() override;
	virtual void STDMETHODCALLTYPE PreLoad() override;
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override;

	virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE** ppbData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE Unlock();
	virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DINDEXBUFFER_DESC* pDesc);

	ComPtr<ID3D11Buffer> buffer_resource;
	D3D11_MAPPED_SUBRESOURCE mapped {};
	D3DINDEXBUFFER_DESC desc8 {};

private:
	bool locked      = false;
	UINT lock_offset = 0;
	UINT lock_size   = 0;
	DWORD lock_flags = 0;
	std::vector<uint8_t> buffer;
	Direct3DDevice8* const device8;
};
