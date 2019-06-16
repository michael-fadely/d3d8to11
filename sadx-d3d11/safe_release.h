#pragma once

template <typename T>
void safe_release(T** ptr)
{
	if (ptr && *ptr)
	{
		(*ptr)->Release();
		*ptr = nullptr;
	}
}

template <typename T>
ULONG safe_addref(T* ptr)
{
	if (ptr)
	{
		return ptr->AddRef();
	}

	return 0;
}
