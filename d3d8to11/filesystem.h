#pragma once

#include <filesystem>

namespace d3d8to11::filesystem
{
using fs_string = std::filesystem::path::string_type;

void initialize();

[[nodiscard]] bool extended_length_paths_supported();

[[nodiscard]] bool should_extend_length(const std::filesystem::path& path);

[[nodiscard]] bool is_extended_length(const std::filesystem::path& path);

[[nodiscard]] std::filesystem::path as_extended_length(const std::filesystem::path& path);

[[nodiscard]] std::filesystem::path without_extended_length_prefix(const std::filesystem::path& path);
}
