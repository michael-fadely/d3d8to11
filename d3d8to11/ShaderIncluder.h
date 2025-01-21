#pragma once
#include <d3d11_1.h>

#include <filesystem>
#include <mutex>
#include <span>
#include <unordered_map>
#include <vector>

class ShaderIncluder : public ID3DInclude
{
public:
	ShaderIncluder()          = default;
	virtual ~ShaderIncluder() = default;

	ShaderIncluder(const ShaderIncluder&)     = delete;
	ShaderIncluder(ShaderIncluder&&) noexcept = delete;

	ShaderIncluder& operator=(const ShaderIncluder&)     = delete;
	ShaderIncluder& operator=(ShaderIncluder&&) noexcept = delete;

	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) noexcept override;
	HRESULT __stdcall Close(LPCVOID pData) noexcept override;

	void set_base_directory(std::filesystem::path dir);
	void add_include_directory(std::filesystem::path dir);
	std::span<const uint8_t> get_shader_source(const std::filesystem::path& file_path);
	void clear_shader_source_cache();
	void shrink_to_fit();

private:
	std::shared_mutex m_directories_mutex;
	std::filesystem::path m_base_directory;
	std::vector<std::filesystem::path> m_include_directories;

	std::recursive_mutex m_sources_mutex;
	std::unordered_map<std::filesystem::path, std::vector<uint8_t>> m_shader_sources;
};
