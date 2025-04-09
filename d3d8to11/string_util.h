#pragma once

#include <string>

namespace d3d8to11
{
void trim_left(std::string& str);

void trim_right(std::string& str);

void trim(std::string& str);

bool equals_case_insensitive(const std::string_view& a, const std::string_view& b);
}
