#pragma once

#include "d3d8types.hpp"
#include "Unknown.h"

class Direct3DDevice8;

class __declspec(uuid("1B36BB7B-09B7-410A-B445-7D1430D7B33F")) Direct3DResource8;

class Direct3DResource8 : public Unknown
{
public:
	virtual ~Direct3DResource8() = default;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8** ppDevice) = 0;
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags) = 0;
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData) = 0;
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) = 0;
	virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) = 0;
	virtual DWORD STDMETHODCALLTYPE GetPriority() = 0;
	virtual void STDMETHODCALLTYPE PreLoad() = 0;
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType() = 0;
};
