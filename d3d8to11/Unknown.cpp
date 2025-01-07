#include "pch.h"
#include "Unknown.h"

HRESULT Unknown::QueryInterface(const IID& riid, void** ppvObject)
{
	return -1;
}

ULONG Unknown::AddRef()
{
	return static_cast<ULONG>(InterlockedAdd(reinterpret_cast<LONG volatile*>(&ref_count), 1));
}

ULONG Unknown::Release()
{
	return static_cast<ULONG>(InterlockedAdd(reinterpret_cast<LONG volatile*>(&ref_count), -1));
}
