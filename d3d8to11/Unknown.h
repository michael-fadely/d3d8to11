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

	virtual ~Unknown() = default;

	Unknown& operator=(const Unknown&) = delete;
	Unknown& operator=(Unknown&&) noexcept = delete;

	HRESULT __stdcall QueryInterface(const IID& riid, void** ppvObject) override;
	ULONG __stdcall AddRef() override;
	ULONG __stdcall Release() override;
};
