#pragma once

#include <filesystem>

namespace d3d8to11
{
using fs_string = std::filesystem::path::string_type;

bool should_extend_length(const std::filesystem::path& path);

bool is_extended_length(const std::filesystem::path& path);

std::filesystem::path as_extended_length(const std::filesystem::path& path);

std::filesystem::path without_extended_length_prefix(const std::filesystem::path& path);
}
