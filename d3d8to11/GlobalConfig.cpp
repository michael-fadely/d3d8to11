#include "pch.h"

#include <string>
#include <vector>

#include "ini_file.h"
#include "tstring.h"

#include "GlobalConfig.h"

using d3d8to11::tstring;

namespace
{
const std::string default_config_dir(".d3d8to11");

constexpr LPCTSTR config_dir_env_name = TEXT("D3D8TO11_CONFIG_DIR"); // all-encompassing fallback if other things aren't set
constexpr LPCTSTR config_file_path_env_name = TEXT("D3D8TO11_CONFIG_FILE_PATH");
constexpr LPCTSTR shader_cache_dir_env_name = TEXT("D3D8TO11_SHADER_CACHE_DIR");
constexpr LPCTSTR shader_source_dir_env_name = TEXT("D3D8TO11_SHADER_SOURCE_DIR");

tstring read_environment_variable(LPCTSTR variable_name)
{
	if (variable_name == nullptr)
	{
		return {};
	}

	const DWORD buffer_size = GetEnvironmentVariable(variable_name, nullptr, 0);

	if (buffer_size == 0)
	{
		return {};
	}

	// TODO: (carefully) write directly to the result string
	std::vector<TCHAR> buffer(buffer_size);
	const DWORD chars_written = GetEnvironmentVariable(variable_name, buffer.data(), buffer_size);

	if (chars_written >= buffer_size)
	{
		return {};
	}

	tstring result(buffer.data());
	buffer = {};
	return result;
}
}

namespace d3d8to11
{
void GlobalConfig::read_config()
{
	set_paths();

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
	{
		std::filesystem::path env_config_dir = read_environment_variable(config_dir_env_name);

		if (!env_config_dir.empty() && std::filesystem::exists(env_config_dir))
		{
			// TODO: logging: say that we're using the env var
			m_config_dir = std::move(env_config_dir);
		}
		else
		{
			// TODO: logging: say we're using the default dir (working dir/.d3d8to11)
			m_config_dir = default_config_dir;

			// TODO: logging: say we're *creating* the default dir
			if (!std::filesystem::exists(m_config_dir))
			{
				std::filesystem::create_directory(m_config_dir);
			}
		}
	}

	{
		std::filesystem::path env_config_file_path = read_environment_variable(config_file_path_env_name);

		if (!env_config_file_path.empty() && std::filesystem::exists(env_config_file_path.parent_path()))
		{
			// TODO: logging: say we're using the env var
			m_config_file_path = std::move(env_config_file_path);
		}
		else
		{
			// TODO: logging: say we're using the default path (config_dir/config.ini)
			m_config_file_path = m_config_dir / "config.ini";
		}
	}

	{
		std::filesystem::path env_shader_cache_dir = read_environment_variable(shader_cache_dir_env_name);

		if (!env_shader_cache_dir.empty() && std::filesystem::exists(env_shader_cache_dir))
		{
			// TODO: logging: say we're using the env var
			m_shader_cache_dir = std::move(env_shader_cache_dir);
		}
		else
		{
			// TODO: logging: say we're using the default path (config_dir/shader cache)
			m_shader_cache_dir = m_config_dir / "shader cache";

			if (!std::filesystem::exists(m_shader_cache_dir))
			{
				std::filesystem::create_directory(m_shader_cache_dir);
			}
		}
	}

	m_shader_cache_variants_file_path = m_shader_cache_dir / "permutations.bin";

	{
		std::filesystem::path env_shader_source_dir = read_environment_variable(shader_source_dir_env_name);

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
