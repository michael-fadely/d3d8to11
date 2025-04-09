#include "pch.h"

#include <cctype>
#include <ranges>

#include "string_util.h"

// TODO: handle locale differences

namespace d3d8to11
{
void trim_left(std::string& str)
{
	str.erase(str.begin(), std::ranges::find_if(str, [](int ch)
	{
		return !std::isspace(ch);
	}));
}

void trim_right(std::string& str)
{
	str.erase(std::ranges::find_if(std::ranges::reverse_view(str), [](int ch)
	{
		return !std::isspace(ch);
	}).base(), str.end());
}

void trim(std::string& str)
{
	trim_left(str);
	trim_right(str);
}

bool equals_case_insensitive(const std::string_view& a, const std::string_view& b)
{
	auto fn = [](unsigned char ca, unsigned char cb) { return std::tolower(ca) == std::tolower(cb); };
	return std::ranges::equal(a, b, fn);
}
}
