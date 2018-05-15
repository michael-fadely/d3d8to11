#include "stdafx.h"
#include <string>
#include "ShaderIncluder.h"

ShaderIncluder::ShaderIncluder(Direct3DDevice8* device)
{
	this->device = device;
}

HRESULT ShaderIncluder::Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes)
{
	std::string str = pFileName;
	str = globals::get_system_path(str);

	const auto& buffer = device->get_shader_source(str);
	*ppData = reinterpret_cast<LPCVOID>(buffer.data());
	*pBytes = static_cast<UINT>(buffer.size());
	return S_OK;
}

HRESULT ShaderIncluder::Close(LPCVOID pData)
{
	return S_OK;
}
