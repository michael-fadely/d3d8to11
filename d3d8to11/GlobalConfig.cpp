#include "pch.h"

#include <string>

#include "filesystem.h"
#include "ini_file.h"
#include "tstring.h"

#include "GlobalConfig.h"

using d3d8to11::tstring;

namespace
{
const std::string DEFAULT_CONFIG_DIR(".d3d8to11");

constexpr LPCTSTR CONFIG_DIR_ENV_NAME = TEXT("D3D8TO11_CONFIG_DIR"); // all-encompassing fallback if other things aren't set
constexpr LPCTSTR CONFIG_FILE_PATH_ENV_NAME = TEXT("D3D8TO11_CONFIG_FILE_PATH");
constexpr LPCTSTR SHADER_CACHE_DIR_ENV_NAME = TEXT("D3D8TO11_SHADER_CACHE_DIR");
constexpr LPCTSTR SHADER_SOURCE_DIR_ENV_NAME = TEXT("D3D8TO11_SHADER_SOURCE_DIR");

tstring read_environment_variable(LPCTSTR variable_name)
{
	if (variable_name == nullptr)
	{
		return {};
	}

	// on "failure", return value is required buffer size *including* null terminator
	const DWORD buffer_size = GetEnvironmentVariable(variable_name, nullptr, 0);

	if (buffer_size <= 1)
	{
		return {};
	}

	tstring result(static_cast<size_t>(buffer_size) - 1, 0);
	const DWORD chars_written = GetEnvironmentVariable(variable_name, result.data(), buffer_size);

	if (!chars_written || chars_written >= buffer_size)
	{
		return {};
	}

	return result;
}
}

namespace d3d8to11
{
void GlobalConfig::read_config()
{
	set_paths();

	if (m_config_file_path.empty())
	{
		return;
	}

	if (std::filesystem::exists(m_config_file_path))
	{
		std::fstream file(m_config_file_path.string(), std::fstream::in);

		ini_file ini;
		ini.read(file);

		auto section = ini.get_section("OIT");

		if (section)
		{
			m_oit_config.enabled = section->get_or("enabled", false);
		}
	}
	else
	{
		save_config();
	}
}

void GlobalConfig::save_config() const
{
	if (m_config_file_path.empty())
	{
		return;
	}

	// this is the same as the old implementation from Direct3DDevice8.
	// it's good enough for now.

	std::fstream file(m_config_file_path.string(), std::fstream::out);

	ini_file ini;

	auto section = std::make_shared<ini_section>();

	ini.set_section("OIT", section);
	section->set("enabled", m_oit_config.enabled);

	ini.write(file);
}

const std::filesystem::path& GlobalConfig::get_shader_cache_dir()
{
	return m_shader_cache_dir;
}

const std::filesystem::path& GlobalConfig::get_shader_cache_variants_file_path()
{
	return m_shader_cache_variants_file_path;
}

const std::filesystem::path& GlobalConfig::get_shader_source_dir()
{
	return m_shader_source_dir;
}

OITConfig& GlobalConfig::get_oit_config()
{
	return m_oit_config;
}

void GlobalConfig::set_paths()
{
	auto maybe_extended_length_or_empty = [](const std::filesystem::path& path) -> std::filesystem::path
	{
		if (!d3d8to11::filesystem::should_extend_length(path))
		{
			return path;
		}

		if (!d3d8to11::filesystem::extended_length_paths_supported())
		{
			// TODO: logging: note that the path was too long
			return TEXT("");
		}

		return d3d8to11::filesystem::as_extended_length(path);
	};

	{
		std::filesystem::path env_config_dir = maybe_extended_length_or_empty(read_environment_variable(CONFIG_DIR_ENV_NAME));

		if (!env_config_dir.empty() && std::filesystem::exists(env_config_dir))
		{
			// TODO: logging: say that we're using the env var
			m_config_dir = std::move(env_config_dir);
		}
		else
		{
			// TODO: logging: say we're using the default dir (working dir/.d3d8to11)
			m_config_dir = DEFAULT_CONFIG_DIR;

			// TODO: logging: say we're *creating* the default dir
			if (!std::filesystem::exists(m_config_dir))
			{
				std::filesystem::create_directory(m_config_dir);
			}
		}
	}

	{
		std::filesystem::path env_config_file_path = maybe_extended_length_or_empty(read_environment_variable(CONFIG_FILE_PATH_ENV_NAME));

		if (!env_config_file_path.empty() && std::filesystem::exists(env_config_file_path.parent_path()))
		{
			// TODO: logging: say we're using the env var
			m_config_file_path = std::move(env_config_file_path);
		}
		else
		{
			// TODO: logging: say we're using the default path (config_dir/config.ini)
			m_config_file_path = maybe_extended_length_or_empty(m_config_dir / "config.ini");
		}
	}

	{
		std::filesystem::path env_shader_cache_dir = maybe_extended_length_or_empty(read_environment_variable(SHADER_CACHE_DIR_ENV_NAME));

		if (!env_shader_cache_dir.empty() && std::filesystem::exists(env_shader_cache_dir))
		{
			// TODO: logging: say we're using the env var
			m_shader_cache_dir = std::move(env_shader_cache_dir);
		}
		else
		{
			// TODO: logging: say we're using the default path (config_dir/shader cache)
			m_shader_cache_dir = maybe_extended_length_or_empty(m_config_dir / "shader cache");

			if (!m_shader_cache_dir.empty() && !std::filesystem::exists(m_shader_cache_dir))
			{
				std::filesystem::create_directory(m_shader_cache_dir);
			}
		}
	}

	// TODO: if m_shader_cache_variants_file_path ends up empty, report some kind of error
	m_shader_cache_variants_file_path = maybe_extended_length_or_empty(m_shader_cache_dir / "permutations.bin");

	{
		std::filesystem::path env_shader_source_dir = maybe_extended_length_or_empty(read_environment_variable(SHADER_SOURCE_DIR_ENV_NAME));

		if (!env_shader_source_dir.empty() && std::filesystem::exists(env_shader_source_dir))
		{
			// TODO: logging: say we're using the env var
			m_shader_source_dir = std::move(env_shader_source_dir);
		}
		else
		{
			// TODO: logging: say we're using the default path (working dir)
			// just set it to empty so that we load from the working directory by default
			m_shader_source_dir = "";
		}
	}
}
}
