#pragma once

#include <cstdint>

#include <d3d11_1.h>
#include <vector>
#include "d3d8types.hpp"
#include "d3d8to11_resource.h"

class Direct3DDevice8;
class Direct3DSurface8;

class __declspec(uuid("B4211CFA-51B9-4A9F-AB78-DB99B2BB678E")) Direct3DBaseTexture8;

class Direct3DBaseTexture8 : public Direct3DResource8
{
public:
	Direct3DBaseTexture8() = default;

	Direct3DBaseTexture8(const Direct3DBaseTexture8&)     = delete;
	Direct3DBaseTexture8(Direct3DBaseTexture8&&) noexcept = delete;

	Direct3DBaseTexture8& operator=(const Direct3DBaseTexture8&)     = delete;
	Direct3DBaseTexture8& operator=(Direct3DBaseTexture8&&) noexcept = delete;

	virtual DWORD STDMETHODCALLTYPE SetLOD(DWORD LODNew) = 0;
	virtual DWORD STDMETHODCALLTYPE GetLOD() = 0;
	virtual DWORD STDMETHODCALLTYPE GetLevelCount() = 0;

protected:
	~Direct3DBaseTexture8() = default;
};

class __declspec(uuid("E4CDD575-2866-4F01-B12E-7EECE1EC9358")) Direct3DTexture8;

// the destructor cannot be virtual because that would change the layout of the vtable
// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class Direct3DTexture8 : public Direct3DBaseTexture8
{
public:
	Direct3DTexture8(const Direct3DTexture8&)     = delete;
	Direct3DTexture8(Direct3DTexture8&&) noexcept = delete;

	Direct3DTexture8& operator=(const Direct3DTexture8&)     = delete;
	Direct3DTexture8& operator=(Direct3DTexture8&&) noexcept = delete;

	void create_native(ID3D11Texture2D* view_of = nullptr);

	Direct3DTexture8(Direct3DDevice8* Device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);
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

	[[nodiscard]] UINT get_width() const
	{
		return m_width;
	}

	[[nodiscard]] UINT get_height() const
	{
		return m_height;
	}

	[[nodiscard]] UINT get_level_count() const
	{
		return m_level_count;
	}

	[[nodiscard]] DWORD get_d3d8_usage() const
	{
		return m_usage;
	}

	[[nodiscard]] D3DFORMAT get_d3d8_format() const
	{
		return m_format;
	}

	[[nodiscard]] D3DPOOL get_d3d8_pool() const
	{
		return m_pool;
	}

	[[nodiscard]] ID3D11Texture2D* get_native_texture() const
	{
		return m_texture.Get();
	}

	[[nodiscard]] ID3D11ShaderResourceView* get_native_srv() const
	{
		return m_srv.Get();
	}

	[[nodiscard]] const D3D11_TEXTURE2D_DESC& get_native_desc() const
	{
		return m_desc;
	}

	[[nodiscard]] bool is_render_target() const;
	[[nodiscard]] bool is_depth_stencil() const;
	[[nodiscard]] bool is_block_compressed() const;

private:
	void get_level_offset(UINT level, size_t* offset, size_t* size) const;
	bool convert(UINT level);
	[[nodiscard]] bool should_convert() const;

	Direct3DDevice8* const m_device8;

	uint8_t m_flags = 0;

	ComPtr<ID3D11Texture2D> m_texture;
	ComPtr<ID3D11ShaderResourceView> m_srv;

	D3D11_TEXTURE2D_DESC m_desc {};

	std::vector<ComPtr<Direct3DSurface8>> m_surfaces;

	std::unordered_map<UINT, D3DLOCKED_RECT> m_locked_rects;
	std::vector<uint8_t> m_texture_buffer;

	UINT      m_width;
	UINT      m_height;
	UINT      m_level_count;
	DWORD     m_usage;
	D3DFORMAT m_format;
	D3DPOOL   m_pool;
};
