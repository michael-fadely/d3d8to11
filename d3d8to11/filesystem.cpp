#include "pch.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "filesystem.h"

static_assert(std::is_same_v<std::wstring, d3d8to11::fs_string>);

namespace
{
const std::wstring EXTENDED_LENGTH_PREFIX(LR"(\\?\)");
}

namespace d3d8to11
{
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

// WIP: this won't work with most api functions unless extended-length paths are enabled in the registry
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
