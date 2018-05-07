#include "stdafx.h"
#include "globals.h"

namespace globals
{
	HelperFunctions helper_functions {};

	std::string get_system_path(const char* path)
	{
		std::string result("SYSTEM\\");
		result.append(path);
		result = helper_functions.GetReplaceablePath(result.c_str());
		return result;
	}

	std::string get_system_path(const std::string& path)
	{
		return get_system_path(path.c_str());
	}

	std::wstring get_system_path(const wchar_t* path)
	{
		return get_system_path(std::wstring(path));
	}

	// this is disgusting
	std::wstring get_system_path(const std::wstring& path)
	{
		auto result = get_system_path(std::string(path.begin(), path.end()));
		return std::wstring(result.begin(), result.end());
	}
}
