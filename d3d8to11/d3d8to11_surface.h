#pragma once

#include "Unknown.h"

class Direct3DDevice8;
class Direct3DTexture8;

class __declspec(uuid("B96EEBCA-B326-4EA5-882F-2FF5BAE021DD")) Direct3DSurface8;

class Direct3DSurface8 : public Unknown
{
public:
	Direct3DSurface8(const Direct3DSurface8&)     = delete;
	Direct3DSurface8(Direct3DSurface8&&) noexcept = delete;

	Direct3DSurface8& operator=(const Direct3DSurface8&)     = delete;
	Direct3DSurface8& operator=(Direct3DSurface8&&) noexcept = delete;

	Direct3DSurface8(Direct3DDevice8* device, Direct3DTexture8* parent_, UINT level_);
	~Direct3DSurface8() = default;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE GetDevice(Direct3DDevice8** ppDevice);
	virtual HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE FreePrivateData(REFGUID refguid);
	virtual HRESULT STDMETHODCALLTYPE GetContainer(REFIID riid, void** ppContainer);
	virtual HRESULT STDMETHODCALLTYPE GetDesc(D3DSURFACE_DESC8* pDesc);
	virtual HRESULT STDMETHODCALLTYPE LockRect(D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE UnlockRect();

	void create_native();
	ComPtr<ID3D11RenderTargetView> render_target;
	ComPtr<ID3D11DepthStencilView> depth_stencil;
	ComPtr<ID3D11ShaderResourceView> depth_srv;
	Direct3DTexture8* parent;

	Direct3DDevice8* const device8;
	UINT level;
	D3DSURFACE_DESC8 desc8 {};

	D3D11_RENDER_TARGET_VIEW_DESC rt_desc {};
	D3D11_DEPTH_STENCIL_VIEW_DESC depth_vdesc {};
};
