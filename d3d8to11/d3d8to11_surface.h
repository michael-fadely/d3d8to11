#pragma once

#include "Unknown.h"

class Direct3DDevice8;
class Direct3DTexture8;

class __declspec(uuid("B96EEBCA-B326-4EA5-882F-2FF5BAE021DD")) Direct3DSurface8;

// the destructor cannot be virtual because that would change the layout of the vtable
// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class Direct3DSurface8 : public Unknown
{
public:
	Direct3DSurface8(const Direct3DSurface8&)     = delete;
	Direct3DSurface8(Direct3DSurface8&&) noexcept = delete;

	Direct3DSurface8& operator=(const Direct3DSurface8&)     = delete;
	Direct3DSurface8& operator=(Direct3DSurface8&&) noexcept = delete;

	Direct3DSurface8(Direct3DDevice8* device, Direct3DTexture8* parent, UINT level);
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

	[[nodiscard]] Direct3DTexture8* get_d3d8_parent() const
	{
		return m_parent;
	}

	[[nodiscard]] UINT get_d3d8_level() const
	{
		return m_level;
	}

	[[nodiscard]] const D3DSURFACE_DESC8& get_d3d8_desc() const
	{
		return m_desc8;
	}

	[[nodiscard]] ID3D11RenderTargetView* get_native_render_target() const
	{
		return m_render_target.Get();
	}

	[[nodiscard]] ID3D11DepthStencilView* get_native_depth_stencil() const
	{
		return m_depth_stencil.Get();
	}

	[[nodiscard]] ID3D11ShaderResourceView* get_native_depth_srv() const
	{
		return m_depth_srv.Get();
	}

private:
	Direct3DTexture8* m_parent;
	Direct3DDevice8* const m_device8;

	UINT m_level;

	ComPtr<ID3D11RenderTargetView> m_render_target;
	ComPtr<ID3D11DepthStencilView> m_depth_stencil;
	ComPtr<ID3D11ShaderResourceView> m_depth_srv;

	D3DSURFACE_DESC8 m_desc8 {};

	D3D11_RENDER_TARGET_VIEW_DESC m_rt_desc {};
	D3D11_DEPTH_STENCIL_VIEW_DESC m_depth_vdesc {};
};
