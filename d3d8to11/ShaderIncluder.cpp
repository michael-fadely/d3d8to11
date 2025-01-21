#include "pch.h"
#include <fstream>

#include "filesystem.h"

#include "ShaderIncluder.h"

namespace fs = d3d8to11::filesystem;

HRESULT ShaderIncluder::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) noexcept
{
	std::filesystem::path file_path(pFileName);

	if (!file_path.is_absolute())
	{
		switch (IncludeType)
		{
			case D3D_INCLUDE_LOCAL:
			{
				{
					std::shared_lock directories_lock(m_directories_mutex);
					file_path = m_base_directory / file_path;
				}

				if (fs::should_extend_length(file_path))
				{
					if (!fs::extended_length_paths_supported())
					{
						// TODO: provide some error output/logging about how the path is too long
						return S_FALSE;
					}

					file_path = fs::as_extended_length(file_path);
				}

				if (!std::filesystem::exists(file_path))
				{
					return S_FALSE;
				}

				break;
			}

			case D3D_INCLUDE_SYSTEM:
			{
				if (m_include_directories.empty())
				{
					S_FALSE;
				}

				std::shared_lock directories_lock(m_directories_mutex);

				bool found = false;

				for (const std::filesystem::path& include_dir : m_include_directories)
				{
					std::filesystem::path include_file_path = include_dir / file_path;

					if (fs::should_extend_length(include_file_path))
					{
						if (!fs::extended_length_paths_supported())
						{
							// TODO: provide some error output/logging about how the path is too long
							return S_FALSE;
						}

						include_file_path = fs::as_extended_length(include_file_path);
					}

					if (std::filesystem::exists(include_file_path))
					{
						found = true;
						file_path = std::move(include_file_path);
						break;
					}
				}

				if (!found)
				{
					return S_FALSE;
				}

				break;
			}

			default:
				return S_FALSE;
		}
	}

	try
	{
		const auto buffer = get_shader_source(file_path);
		*ppData = reinterpret_cast<LPCVOID>(buffer.data());
		*pBytes = static_cast<UINT>(buffer.size());
	}
	catch (std::exception&)
	{
		return S_FALSE;
	}

	return S_OK;
}

HRESULT ShaderIncluder::Close(LPCVOID /*pData*/) noexcept
{
	return S_OK;
}

void ShaderIncluder::set_base_directory(std::filesystem::path dir)
{
	if (fs::is_extended_length(dir))
	{
		dir = fs::without_extended_length_prefix(dir);
	}

	std::unique_lock directories_lock(m_directories_mutex);
	m_base_directory = std::move(dir);
}

void ShaderIncluder::add_include_directory(std::filesystem::path dir)
{
	if (fs::is_extended_length(dir))
	{
		dir = fs::without_extended_length_prefix(dir);
	}

	std::unique_lock directories_lock(m_directories_mutex);
	m_include_directories.emplace_back(std::move(dir));
}

std::span<const uint8_t> ShaderIncluder::get_shader_source(const std::filesystem::path& file_path)
{
	const std::filesystem::path file_path_key =
		fs::is_extended_length(file_path)
		? fs::without_extended_length_prefix(file_path)
		: file_path;

	std::lock_guard sources_lock(m_sources_mutex);

	const auto it = m_shader_sources.find(file_path_key);

	if (it != m_shader_sources.end())
	{
		return it->second;
	}

	std::ifstream file;

	if (fs::should_extend_length(file_path))
	{
		if (!fs::extended_length_paths_supported())
		{
			// TODO: provide some error output/logging about how the path is too long
			// TODO: better error state in general
			return {};
		}

		file.open(fs::as_extended_length(file_path), std::ios::binary | std::ios::ate);
	}
	else
	{
		file.open(file_path, std::ios::binary | std::ios::ate);
	}

	// TODO: return some kind of error if the file couldn't be opened

	const auto size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);
	std::vector<uint8_t> buffer(size);
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

	const std::span<const uint8_t> result(buffer);
	m_shader_sources[file_path_key] = std::move(buffer);
	return result;
}

void ShaderIncluder::clear_shader_source_cache()
{
	std::lock_guard sources_lock(m_sources_mutex);
	m_shader_sources.clear();
}

void ShaderIncluder::shrink_to_fit()
{
	std::unique_lock directories_lock(m_directories_mutex);
	m_include_directories.shrink_to_fit();
}
