#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class Unknown : public IUnknown
{
	volatile ULONG ref_count = 0;

public:
	Unknown() = default;

	Unknown(const Unknown&) = delete;
	Unknown(Unknown&&) noexcept = delete;

	Unknown& operator=(const Unknown&) = delete;
	Unknown& operator=(Unknown&&) noexcept = delete;

	virtual HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObject) override;
	virtual ULONG __stdcall AddRef() override;
	virtual ULONG __stdcall Release() override;

protected:
	~Unknown() = default;
};
