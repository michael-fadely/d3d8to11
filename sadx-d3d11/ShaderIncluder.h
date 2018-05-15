#pragma once
#include <d3d11_1.h>

class ShaderIncluder : public ID3DInclude
{
	Direct3DDevice8* device = nullptr;
public:
	~ShaderIncluder() = default;
	explicit ShaderIncluder(Direct3DDevice8* device);
	HRESULT __stdcall Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override;
	HRESULT __stdcall Close(LPCVOID pData) override;
};
