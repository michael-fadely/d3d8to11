#pragma once

#include <filesystem>

namespace d3d8to11
{
struct OITConfig
{
	bool enabled = false;
};

class GlobalConfig
{
public:
	GlobalConfig() = default;

	void read_config();
	void save_config();

	[[nodiscard]] const std::filesystem::path& get_shader_cache_dir();
	[[nodiscard]] const std::filesystem::path& get_shader_cache_variants_file_path();
	[[nodiscard]] const std::filesystem::path& get_shader_source_dir();

	[[nodiscard]] OITConfig& get_oit_config();

private:
	void set_paths();

	std::filesystem::path m_config_dir;
	std::filesystem::path m_config_file_path;
	std::filesystem::path m_shader_cache_dir;
	std::filesystem::path m_shader_cache_variants_file_path;
	std::filesystem::path m_shader_source_dir;

	OITConfig m_oit_config;
};
}
