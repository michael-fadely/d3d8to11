#pragma once

#include <string>
#include <string_view>

namespace d3d8to11
{
#ifdef UNICODE
using tstring = std::wstring;
using tstring_view = std::wstring_view;
#else
using tstring = std::string;
using tstring_view = std::string_view;
#endif
}
