#include <string>
#include <vector>
#include <mutex>
#include <fstream>

#include "ShaderIncluder.h"

HRESULT ShaderIncluder::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
	const std::string str = pFileName;

	const auto& buffer = get_shader_source(str);
	*ppData = reinterpret_cast<LPCVOID>(buffer.data());
	*pBytes = static_cast<UINT>(buffer.size());
	return S_OK;
}

HRESULT ShaderIncluder::Close(LPCVOID pData)
{
	return S_OK;
}

std::vector<uint8_t>& ShaderIncluder::get_shader_source(const std::string& path)
{
	std::lock_guard<std::recursive_mutex> lock(shader_sources_mutex);

	const auto it = shader_sources.find(path);

	if (it != shader_sources.end())
	{
		return it->second;
	}

	std::ifstream file(path, std::ios::binary | std::ios::ate);
	const auto size = static_cast<size_t>(file.tellg());
	file.seekg(0, std::ios::beg);
	std::vector<uint8_t> buffer(size);
	file.read(reinterpret_cast<char*>(buffer.data()), size);
	file.close();

	shader_sources[path] = std::move(buffer);
	return shader_sources[path];
}
