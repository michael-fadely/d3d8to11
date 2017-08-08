#pragma once

#include "d3d8to9_resource.h"
#include "lookup_table.hpp"

class Direct3DDevice8;

class __declspec(uuid("8AEEEAC7-05F9-44D4-B591-000B0DF1CB95")) Direct3DVertexBuffer8;
class Direct3DVertexBuffer8 : public Direct3DResource8, public AddressLookupTableObject
{
public:
	Direct3DVertexBuffer8(const Direct3DVertexBuffer8 &) = delete;
	Direct3DVertexBuffer8 &operator=(const Direct3DVertexBuffer8 &) = delete;

	void create_native();
	Direct3DVertexBuffer8(Direct3DDevice8 *Device, UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool);
	~Direct3DVertexBuffer8() = default;

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

	virtual HRESULT STDMETHODCALLTYPE Lock(UINT OffsetToLock, UINT SizeToLock, BYTE **ppbData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE Unlock();
	virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DVERTEXBUFFER_DESC *pDesc);

	ComPtr<ID3D11Buffer> buffer_resource;
	std::vector<uint8_t> buffer;
	D3DVERTEXBUFFER_DESC desc8 {};

private:
	bool locked = false;
	Direct3DDevice8 *const Device;
};

template <>
struct AddressCacheIndex<Direct3DVertexBuffer8>
{
	static constexpr UINT CacheIndex = 5;
	using Type9 = IUnknown;
};
