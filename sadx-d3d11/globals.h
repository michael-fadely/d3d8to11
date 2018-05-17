#pragma once

#include <string>

namespace globals
{
	extern HelperFunctions helper_functions;

	std::string get_system_path(const char* path);
	std::string get_system_path(const std::string& path);

	std::wstring get_system_path(const wchar_t* path);
	std::wstring get_system_path(const std::wstring& path);

	extern uint32_t max_fragments;
	extern bool allow_d32;
}
