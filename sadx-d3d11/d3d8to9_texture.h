#pragma once

#include <d3d11_1.h>
#include <vector>
#include "d3d8types.hpp"
#include "d3d8to9_resource.h"
#include "dynarray.h"

class Direct3DDevice8;
class Direct3DSurface8;

class __declspec(uuid("B4211CFA-51B9-4A9F-AB78-DB99B2BB678E")) Direct3DBaseTexture8;

class Direct3DBaseTexture8 : public Direct3DResource8
{
public:
	virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) = 0;
	virtual DWORD STDMETHODCALLTYPE GetLOD() = 0;
	virtual DWORD STDMETHODCALLTYPE GetLevelCount() = 0;
};

class __declspec(uuid("E4CDD575-2866-4F01-B12E-7EECE1EC9358")) Direct3DTexture8;

class Direct3DTexture8 : public Direct3DBaseTexture8
{
public:
	Direct3DTexture8(const Direct3DTexture8&)            = delete;
	Direct3DTexture8& operator=(const Direct3DTexture8&) = delete;

	void create_native(ID3D11Texture2D* view_of = nullptr);

	Direct3DTexture8(Direct3DDevice8* device_, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);
	~Direct3DTexture8() = default;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8** ppDevice) override;
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags) override;
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData) override;
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid) override;
	virtual DWORD STDMETHODCALLTYPE SetPriority(DWORD PriorityNew) override;
	virtual DWORD STDMETHODCALLTYPE GetPriority() override;
	virtual void STDMETHODCALLTYPE PreLoad() override;
	virtual D3DRESOURCETYPE STDMETHODCALLTYPE GetType() override;

	virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) override;
	virtual DWORD STDMETHODCALLTYPE GetLOD() override;
	virtual DWORD STDMETHODCALLTYPE GetLevelCount() override;

	virtual HRESULT STDMETHODCALLTYPE GetLevelDesc(UINT Level, D3DSURFACE_DESC8* pDesc);
	virtual HRESULT STDMETHODCALLTYPE GetSurfaceLevel(UINT Level, Direct3DSurface8** ppSurfaceLevel);
	virtual HRESULT STDMETHODCALLTYPE LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE UnlockRect(UINT Level);
	virtual HRESULT STDMETHODCALLTYPE AddDirtyRect(const RECT* pDirtyRect);

	ComPtr<ID3D11Texture2D> texture;
	ComPtr<ID3D11ShaderResourceView> srv;

	UINT      width_;
	UINT      height_;
	UINT      levels_;
	DWORD     usage_;
	D3DFORMAT format_;
	D3DPOOL   pool_;

	D3D11_TEXTURE2D_DESC desc {};

	bool is_render_target = false;
	bool is_depth_stencil = false;

private:
	dynarray<ComPtr<Direct3DSurface8>> surfaces;

	bool convert(UINT level);
	Direct3DDevice8* const device8;

	std::unordered_map<UINT, D3DLOCKED_RECT> locked_rects;
	std::unordered_map<UINT, std::vector<uint8_t>> texture_levels;
};
