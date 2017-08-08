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
