#pragma once

#include <debugapi.h>
#include <format>

inline void output_not_implemented(const char* function, const char* file, size_t line)
{
	const std::string str = std::format("NOT IMPLEMENTED: {} ({}:{})\n", function, file, line);
	OutputDebugStringA(str.c_str());
}

#define NOT_IMPLEMENTED_RETURN                                \
	output_not_implemented(__FUNCTION__, __FILE__, __LINE__); \
	return D3DERR_INVALIDCALL

#define NOT_IMPLEMENTED \
	output_not_implemented(__FUNCTION__, __FILE__, __LINE__)
