#pragma once

#include <sstream>
#include <debugapi.h>

inline void output_not_implemented(const char* function, const char* file, size_t line)
{
	std::stringstream ss;

	ss << "NOT IMPLEMENTED: "
	   << function << " (" << file << ":" << line << ")\n";

	OutputDebugStringA(ss.str().c_str());
}

#define NOT_IMPLEMENTED_RETURN                                \
	output_not_implemented(__FUNCTION__, __FILE__, __LINE__); \
	return D3DERR_INVALIDCALL

#define NOT_IMPLEMENTED \
	output_not_implemented(__FUNCTION__, __FILE__, __LINE__)
