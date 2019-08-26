#pragma once
#include <d3d11_1.h>
#include <vector>
#include <unordered_map>
#include <mutex>

class ShaderIncluder : public ID3DInclude
{
public:
	~ShaderIncluder() = default;
	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;
	HRESULT __stdcall Close(LPCVOID pData) override;

protected:
	std::recursive_mutex shader_sources_mutex;
	std::unordered_map<std::string, std::vector<uint8_t>> shader_sources;

public:
	std::vector<uint8_t> get_shader_source(const std::string& path);
};
