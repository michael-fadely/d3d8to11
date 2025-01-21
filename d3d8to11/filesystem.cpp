#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "filesystem.h"

static_assert(std::is_same_v<std::wstring, d3d8to11::filesystem::fs_string>);

namespace
{
const std::wstring EXTENDED_LENGTH_PREFIX(LR"(\\?\)");

bool g_extended_length_paths_supported = false;
}

namespace d3d8to11::filesystem
{
void initialize()
{
	static bool initialized = false;

	if (initialized)
	{
		return;
	}

	// although this only checks if extended-length path support is enabled globally,
	// the chances of it being enabled per-process in a d3d8 application are... low.
	HKEY hkey = nullptr;
	LSTATUS reg_result =
		RegOpenKeyEx(HKEY_LOCAL_MACHINE,
			TEXT(R"(SYSTEM\CurrentControlSet\Control\FileSystem)"),
			0,
			KEY_READ,
			&hkey);

	if (reg_result == ERROR_SUCCESS)
	{
		DWORD dword_size = sizeof(DWORD);
		DWORD long_paths_enabled = 0;
		DWORD key_type = 0;

		reg_result =
			RegQueryValueEx(hkey,
			                TEXT("LongPathsEnabled"),
			                nullptr,
			                &key_type,
			                reinterpret_cast<LPBYTE>(&long_paths_enabled),
			                &dword_size);

		g_extended_length_paths_supported = reg_result == ERROR_SUCCESS && key_type == REG_DWORD && long_paths_enabled == 1;

		RegCloseKey(hkey);
	}

	initialized = true;
}

bool extended_length_paths_supported()
{
	return g_extended_length_paths_supported;
}

bool should_extend_length(const std::filesystem::path& path)
{
	// 1 is added to length() because MAX_PATH usually includes the null terminator
	return !is_extended_length(path) &&
	       path.native().length() + 1 > MAX_PATH;
}

bool is_extended_length(const std::filesystem::path& path)
{
	return path.native().starts_with(EXTENDED_LENGTH_PREFIX);
}

std::filesystem::path as_extended_length(const std::filesystem::path& path)
{
	if (is_extended_length(path))
	{
		return path;
	}

	const std::wstring absolute_path_str = std::filesystem::absolute(path).native();
	std::wstring temp;
	temp.reserve(EXTENDED_LENGTH_PREFIX.length() + absolute_path_str.length());
	temp += EXTENDED_LENGTH_PREFIX;
	temp += absolute_path_str;

	return std::filesystem::path(std::move(temp));
}

std::filesystem::path without_extended_length_prefix(const std::filesystem::path& path)
{
	if (!is_extended_length(path))
	{
		return path;
	}

	return std::filesystem::path(path.native().substr(EXTENDED_LENGTH_PREFIX.length()));
}
}
