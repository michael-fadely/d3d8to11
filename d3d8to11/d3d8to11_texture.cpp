/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "pch.h"

#include "d3d8to11.hpp"
#include "not_implemented.h"

#include "d3d8to11_texture.h"

// TODO: let d3d do the memory management work if possible

struct TextureFlags
{
	using type = uint8_t;

	static constexpr type render_target_shift    = 0;
	static constexpr type depth_stencil_shift    = 1;
	static constexpr type block_compressed_shift = 2;

	enum T : type
	{
		none,
		render_target    = 1 << render_target_shift,
		depth_stencil    = 1 << depth_stencil_shift,
		block_compressed = 1 << block_compressed_shift
	};

	static constexpr type renderable_mask = render_target | depth_stencil;
};

void Direct3DTexture8::create_native(ID3D11Texture2D* view_of)
{
	ID3D11Device* device = m_device8->get_native_device();

	m_flags = 0;

	if (d3d8to11::is_block_compressed(d3d8to11::to_dxgi(m_format)))
	{
		m_flags |= TextureFlags::block_compressed;
	}

	if (view_of != nullptr)
	{
		view_of->GetDesc(&m_desc);
		m_texture = view_of;

		m_flags |= static_cast<TextureFlags::type>(!!(m_desc.BindFlags & D3D11_BIND_RENDER_TARGET)) << TextureFlags::render_target_shift;
		m_flags |= static_cast<TextureFlags::type>(!!(m_desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)) << TextureFlags::depth_stencil_shift;
	}
	else
	{
		static_assert(TextureFlags::render_target == D3DUSAGE_RENDERTARGET);
		static_assert(TextureFlags::depth_stencil == D3DUSAGE_DEPTHSTENCIL);

		// these D3DUSAGE values happen to line up!
		m_flags |= static_cast<TextureFlags::type>(m_usage & (D3DUSAGE_RENDERTARGET | D3DUSAGE_DEPTHSTENCIL));

		if (!m_level_count)
		{
			++m_level_count;

			//if (!m_is_render_target && !m_is_depth_stencil)
			if (!(m_flags & TextureFlags::renderable_mask))
			{
				auto width  = m_width;
				auto height = m_height;

				while (width != 1 && height != 1)
				{
					++m_level_count;
					width  = std::max(1u, width / 2);
					height = std::max(1u, height / 2);
				}
			}
		}

		auto format = d3d8to11::to_dxgi(m_format);

		if (!IsWindows8OrGreater())
		{
			switch (format)
			{
				case DXGI_FORMAT_B5G6R5_UNORM:
				case DXGI_FORMAT_B5G5R5A1_UNORM:
				case DXGI_FORMAT_B4G4R4A4_UNORM:
				case DXGI_FORMAT_B8G8R8A8_UNORM:
					format = DXGI_FORMAT_R8G8B8A8_UNORM;
					break;

				default:
					break;
			}
		}

		const D3D11_USAGE usage = D3D11_USAGE_DEFAULT;

		/*if (Usage & D3DUSAGE_DYNAMIC)
		{
			usage = D3D11_USAGE_DYNAMIC;
		}*/

		uint32_t bind_flags = D3D11_BIND_SHADER_RESOURCE;

		if (m_flags & TextureFlags::render_target)
		{
			bind_flags |= D3D11_BIND_RENDER_TARGET;
		}

		if (m_flags & TextureFlags::depth_stencil)
		{
			bind_flags |= D3D11_BIND_DEPTH_STENCIL;
			format = d3d8to11::to_typeless(format);
		}

		m_desc.ArraySize  = 1;
		m_desc.BindFlags  = bind_flags;
		m_desc.Usage      = usage;
		m_desc.Format     = format;
		m_desc.Width      = m_width;
		m_desc.Height     = m_height;
		m_desc.MipLevels  = m_level_count;
		m_desc.SampleDesc = { 1, 0 };

		const auto hr = device->CreateTexture2D(&m_desc, nullptr, &m_texture);

		if (FAILED(hr))
		{
			const std::string message = std::format("CreateTexture2D failed with error {:X}: format: {} width: {} height: {} levels: {}",
			                                        static_cast<uint32_t>(hr), static_cast<uint32_t>(m_desc.Format), m_desc.Width, m_desc.Height, m_desc.MipLevels);

			throw std::runtime_error(message);
		}

		auto srv_format = format;

		if (m_flags & TextureFlags::depth_stencil)
		{
			// create a shader resource view with a readable pixel format
			srv_format = d3d8to11::typeless_to_float(format);

			// if float didn't work, it's probably int we want
			if (srv_format == DXGI_FORMAT_UNKNOWN)
			{
				srv_format = d3d8to11::typeless_to_unorm(format);
			}
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};

		srv_desc.Format              = srv_format;
		srv_desc.ViewDimension       = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = m_level_count;

		if (FAILED(device->CreateShaderResourceView(m_texture.Get(), &srv_desc, &m_srv)))
		{
			throw std::runtime_error("CreateShaderResourceView failed");
		}
	}

	m_surfaces.clear();
	m_surfaces.resize(m_level_count);
	m_surfaces.shrink_to_fit();

	UINT level = 0;
	size_t total_size = 0;

	for (auto& it : m_surfaces)
	{
		it = new Direct3DSurface8(m_device8, this, level++);
		it->create_native();
		total_size += it->get_d3d8_desc().Size;
	}

	m_texture_buffer.resize(total_size);
	m_texture_buffer.shrink_to_fit();
}

// IDirect3DTexture8
Direct3DTexture8::Direct3DTexture8(Direct3DDevice8* Device, UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool)
	: m_device8(Device),
	  m_width(Width),
	  m_height(Height),
	  m_level_count(Levels),
	  m_usage(Usage),
	  m_format(Format),
	  m_pool(Pool)
{
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
	    riid == __uuidof(IUnknown) ||
	    riid == __uuidof(Direct3DResource8) ||
	    riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Direct3DTexture8::AddRef()
{
	return Direct3DBaseTexture8::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DTexture8::Release()
{
	const auto result = Direct3DBaseTexture8::Release();

	if (!result)
	{
		delete this;
	}

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetDevice(Direct3DDevice8** ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	m_device8->AddRef();
	*ppDevice = m_device8;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::FreePrivateData(REFGUID refguid)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::SetPriority(DWORD PriorityNew)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetPriority()
{
	NOT_IMPLEMENTED;
	return -1;
}

void STDMETHODCALLTYPE Direct3DTexture8::PreLoad()
{
	NOT_IMPLEMENTED;
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DTexture8::GetType()
{
	return D3DRTYPE_TEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::SetLOD(DWORD LODNew)
{
	NOT_IMPLEMENTED_RETURN;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetLOD()
{
	NOT_IMPLEMENTED;
	return 0;
}

DWORD STDMETHODCALLTYPE Direct3DTexture8::GetLevelCount()
{
	return m_desc.MipLevels;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC8* pDesc)
{
	if (pDesc == nullptr || Level > GetLevelCount())
	{
		return D3DERR_INVALIDCALL;
	}

	const D3DSURFACE_DESC8& surface_desc8 = m_surfaces[Level]->get_d3d8_desc();

	const auto width  = surface_desc8.Width;
	const auto height = surface_desc8.Height;

	pDesc->Format          = m_format;
	pDesc->Type            = GetType();
	pDesc->Usage           = m_usage;
	pDesc->Pool            = m_pool;
	pDesc->Size            = calc_texture_size(width, height, 1, m_format);
	pDesc->MultiSampleType = D3DMULTISAMPLE_NONE;
	pDesc->Width           = width;
	pDesc->Height          = height;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::GetSurfaceLevel(UINT Level, Direct3DSurface8** ppSurfaceLevel)
{
	if (!ppSurfaceLevel)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurfaceLevel = nullptr;

	if (Level >= m_desc.MipLevels)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSurfaceLevel = m_surfaces[Level].Get();
	(*ppSurfaceLevel)->AddRef();

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::LockRect(UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
{
	if (pRect)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Level >= m_desc.MipLevels)
	{
		return D3DERR_INVALIDCALL;
	}

	if (m_locked_rects.contains(Level))
	{
		return D3DERR_INVALIDCALL;
	}

	const D3DSURFACE_DESC8& surface_desc8 = m_surfaces[Level]->get_d3d8_desc();
	const auto width = surface_desc8.Width;

	size_t level_offset = 0;
	size_t level_size = 0;
	get_level_offset(Level, &level_offset, &level_size);

	D3DLOCKED_RECT rect;
	rect.Pitch = calc_texture_size(width, 1, 1, m_format);
	rect.pBits = &m_texture_buffer[level_offset];

	m_locked_rects[Level] = rect;
	*pLockedRect = rect;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::UnlockRect(UINT Level)
{
	const auto it = m_locked_rects.find(Level);

	if (it == m_locked_rects.end())
	{
		return D3DERR_INVALIDCALL;
	}

	// HACK: fixes Phantasy Star Online: Blue Burst
	// TODO: instead of this, make sure render target data is accessible by the CPU, even if that means we need a staging texture
	//if (!m_is_render_target && !m_is_depth_stencil)
	if (!(m_flags & TextureFlags::renderable_mask))
	{
		ID3D11DeviceContext* context = m_device8->get_native_context();

		// TODO: make this behavior configurable [safe mipmaps]
		if (!Level)
		{
			if (should_convert())
			{
				for (UINT i = 0; i < m_level_count; ++i)
				{
					convert(i);
				}
			}
			else
			{
				for (UINT i = 0; i < m_level_count; ++i)
				{
					const auto desc8 = m_surfaces[i]->get_d3d8_desc();
					const auto width = desc8.Width;

					size_t level_offset = 0;
					size_t level_size = 0;
					get_level_offset(i, &level_offset, &level_size);

					D3DLOCKED_RECT rect;
					rect.Pitch = calc_texture_size(width, 1, 1, m_format);
					rect.pBits = &m_texture_buffer[level_offset];

					context->UpdateSubresource(m_texture.Get(), i, nullptr, rect.pBits, rect.Pitch, 0);
				}
			}
		}
		else if (!convert(Level))
		{
			const auto& rect = it->second;
			context->UpdateSubresource(m_texture.Get(), Level, nullptr, rect.pBits, rect.Pitch, 0);
		}
	}
	
	m_locked_rects.erase(it);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DTexture8::AddDirtyRect(const RECT* pDirtyRect)
{
	NOT_IMPLEMENTED_RETURN;
}

bool Direct3DTexture8::is_render_target() const
{
	return !!(m_flags & TextureFlags::render_target);
}

bool Direct3DTexture8::is_depth_stencil() const
{
	return !!(m_flags & TextureFlags::depth_stencil);
}

bool Direct3DTexture8::is_block_compressed() const
{
	return !!(m_flags & TextureFlags::block_compressed);
}

void Direct3DTexture8::get_level_offset(UINT level, size_t* offset, size_t* size) const
{
	*size = m_surfaces[level]->get_d3d8_desc().Size;
	*offset = 0;

	for (UINT i = 0; i < level; ++i)
	{
		*offset += m_surfaces[i]->get_d3d8_desc().Size;
	}
}

bool Direct3DTexture8::convert(UINT level)
{
	if (IsWindows8OrGreater())
	{
		return false;
	}

	D3DSURFACE_DESC8 level_desc {};
	GetLevelDesc(level, &level_desc);

	size_t level_offset = 0;
	size_t level_size = 0;
	get_level_offset(level, &level_offset, &level_size);

	uint8_t* buffer = &m_texture_buffer[level_offset];
	
	const DXGI_FORMAT format = d3d8to11::to_dxgi(m_format);

	std::vector<uint32_t> rgba;

	switch (format)
	{
		case DXGI_FORMAT_B5G6R5_UNORM:
		{
			rgba.resize(level_size / 2);

			auto b16            = reinterpret_cast<uint16_t*>(buffer);
			uint32_t* b32       = rgba.data();
			const size_t length = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto p16  = b16[i];
				auto& p32 = b32[i];

				auto b = static_cast<uint8_t>(static_cast<float>(p16 & 0b011111) / 32.0f * 255.0f);
				auto g = static_cast<uint8_t>(static_cast<float>((p16 >> 5) & 0b111111) / 64.0f * 255.0f);
				auto r = static_cast<uint8_t>(static_cast<float>((p16 >> 11) & 0b011111) / 32.0f * 255.0f);

				p32 = 255 << 24 | b << 16 | g << 8 | r;
			}
			break;
		}

		case DXGI_FORMAT_B5G5R5A1_UNORM:
		{
			rgba.resize(level_size / 2);

			auto b16      = reinterpret_cast<uint16_t*>(buffer);
			uint32_t* b32 = rgba.data();
			auto length   = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto p16  = b16[i];
				auto& p32 = b32[i];

				auto b = static_cast<uint8_t>(static_cast<float>(p16 & 0b011111) / 32.0f * 255.0f);
				auto g = static_cast<uint8_t>(static_cast<float>((p16 >> 5) & 0b011111) / 32.0f * 255.0f);
				auto r = static_cast<uint8_t>(static_cast<float>((p16 >> 10) & 0b011111) / 32.0f * 255.0f);
				auto a = static_cast<uint8_t>(p16 & (1 << 15) ? 255 : 0);

				p32 = a << 24 | b << 16 | g << 8 | r;
			}
			break;
		}

		case DXGI_FORMAT_B4G4R4A4_UNORM:
		{
			rgba.resize(level_size / 2);

			auto b16      = reinterpret_cast<uint16_t*>(buffer);
			uint32_t* b32 = rgba.data();
			auto length   = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto p16  = b16[i];
				auto& p32 = b32[i];

				auto b = static_cast<uint8_t>(static_cast<float>(p16 & 0xF) / 15.0f * 255.0f);
				auto g = static_cast<uint8_t>(static_cast<float>((p16 >> 4) & 0xF) / 15.0f * 255.0f);
				auto r = static_cast<uint8_t>(static_cast<float>((p16 >> 8) & 0xF) / 15.0f * 255.0f);
				auto a = static_cast<uint8_t>(static_cast<float>((p16 >> 12) & 0xF) / 15.0f * 255.0f);

				p32 = a << 24 | b << 16 | g << 8 | r;
			}
			break;
		}

		case DXGI_FORMAT_B8G8R8A8_UNORM:
		{
			rgba.resize(level_size / 4);

			auto bgr      = reinterpret_cast<uint32_t*>(buffer);
			uint32_t* b32 = rgba.data();
			auto length   = rgba.size();

			for (size_t i = 0; i < length; ++i)
			{
				auto a  = bgr[i];
				auto& b = b32[i];

				b = (a & 0xFF000000) | ((a >> 16) & 0xFF) | ((a >> 8) & 0xFF) << 8 | (a & 0xFF) << 16;
			}

			break;
		}

		default:
			return false;
	}

	m_device8->get_native_context()->UpdateSubresource(m_texture.Get(), level, nullptr, rgba.data(), 4 * level_desc.Width, 0);
	return true;
}

bool Direct3DTexture8::should_convert() const
{
	if (IsWindows8OrGreater())
	{
		return false;
	}

	const DXGI_FORMAT format = d3d8to11::to_dxgi(m_format);

	switch (format)
	{
		case DXGI_FORMAT_B5G6R5_UNORM:
		case DXGI_FORMAT_B5G5R5A1_UNORM:
		case DXGI_FORMAT_B4G4R4A4_UNORM:
		case DXGI_FORMAT_B8G8R8A8_UNORM:
			return true;

		default:
			return false;
	}
}

// TODO: IDirect3DCubeTexture8

#if 0

Direct3DCubeTexture8::Direct3DCubeTexture8(Direct3DDevice8* device, IDirect3DCubeTexture9* ProxyInterface)
	: ProxyInterface(ProxyInterface),
	Device(device)
{
	Device->address_table->SaveAddress(this, ProxyInterface);
}

Direct3DCubeTexture8::~Direct3DCubeTexture8()
{
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DCubeTexture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DCubeTexture8::Release()
{
	return ProxyInterface->Release();
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetDevice(Direct3DDevice8** ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::SetPrivateData(REFGUID refguid, const void* pData, DWORD SizeOfData, DWORD Flags)
{
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData)
{
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::FreePrivateData(REFGUID refguid)
{
	return ProxyInterface->FreePrivateData(refguid);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::SetPriority(DWORD PriorityNew)
{
	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetPriority()
{
	return ProxyInterface->GetPriority();
}

void STDMETHODCALLTYPE Direct3DCubeTexture8::PreLoad()
{
	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DCubeTexture8::GetType()
{
	return D3DRTYPE_CUBETEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::SetLOD(DWORD LODNew)
{
	return ProxyInterface->SetLOD(LODNew);
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetLOD()
{
	return ProxyInterface->GetLOD();
}

DWORD STDMETHODCALLTYPE Direct3DCubeTexture8::GetLevelCount()
{
	return ProxyInterface->GetLevelCount();
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetLevelDesc(UINT Level, D3DSURFACE_DESC8* pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DSURFACE_DESC SurfaceDesc;

	const HRESULT hr = ProxyInterface->GetLevelDesc(Level, &SurfaceDesc);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertSurfaceDesc(SurfaceDesc, *pDesc);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::GetCubeMapSurface(D3DCUBEMAP_FACES FaceType, UINT Level, Direct3DSurface8** ppCubeMapSurface)
{
	if (ppCubeMapSurface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppCubeMapSurface = nullptr;

	IDirect3DSurface9* SurfaceInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetCubeMapSurface(FaceType, Level, &SurfaceInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppCubeMapSurface = Device->address_table->FindAddress<Direct3DSurface8>(SurfaceInterface);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::LockRect(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, const RECT* pRect, DWORD Flags)
{
	return ProxyInterface->LockRect(FaceType, Level, pLockedRect, pRect, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::UnlockRect(D3DCUBEMAP_FACES FaceType, UINT Level)
{
	return ProxyInterface->UnlockRect(FaceType, Level);
}

HRESULT STDMETHODCALLTYPE Direct3DCubeTexture8::AddDirtyRect(D3DCUBEMAP_FACES FaceType, const RECT* pDirtyRect)
{
	return ProxyInterface->AddDirtyRect(FaceType, pDirtyRect);
}

#endif

// TODO: IDirect3DVolumeTexture8

#if 0

Direct3DVolumeTexture8::Direct3DVolumeTexture8(Direct3DDevice8 *device, IDirect3DVolumeTexture9 *ProxyInterface) :
	ProxyInterface(ProxyInterface),
	Device(device)
{
	Device->address_table->SaveAddress(this, ProxyInterface);
}

Direct3DVolumeTexture8::~Direct3DVolumeTexture8()
{
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::QueryInterface(REFIID riid, void **ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
		riid == __uuidof(IUnknown) ||
		riid == __uuidof(Direct3DResource8) ||
		riid == __uuidof(Direct3DBaseTexture8))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return ProxyInterface->QueryInterface(riid, ppvObj);
}

ULONG STDMETHODCALLTYPE Direct3DVolumeTexture8::AddRef()
{
	return ProxyInterface->AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DVolumeTexture8::Release()
{
	return ProxyInterface->Release();
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetDevice(Direct3DDevice8 **ppDevice)
{
	if (ppDevice == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	Device->AddRef();

	*ppDevice = Device;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::SetPrivateData(REFGUID refguid, const void *pData, DWORD SizeOfData, DWORD Flags)
{
	return ProxyInterface->SetPrivateData(refguid, pData, SizeOfData, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetPrivateData(REFGUID refguid, void *pData, DWORD *pSizeOfData)
{
	return ProxyInterface->GetPrivateData(refguid, pData, pSizeOfData);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::FreePrivateData(REFGUID refguid)
{
	return ProxyInterface->FreePrivateData(refguid);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::SetPriority(DWORD PriorityNew)
{
	return ProxyInterface->SetPriority(PriorityNew);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetPriority()
{
	return ProxyInterface->GetPriority();
}

void STDMETHODCALLTYPE Direct3DVolumeTexture8::PreLoad()
{
	ProxyInterface->PreLoad();
}

D3DRESOURCETYPE STDMETHODCALLTYPE Direct3DVolumeTexture8::GetType()
{
	return D3DRTYPE_VOLUMETEXTURE;
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::SetLOD(DWORD LODNew)
{
	return ProxyInterface->SetLOD(LODNew);
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLOD()
{
	return ProxyInterface->GetLOD();
}

DWORD STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLevelCount()
{
	return ProxyInterface->GetLevelCount();
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetLevelDesc(UINT Level, D3DVOLUME_DESC8 *pDesc)
{
	if (pDesc == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	D3DVOLUME_DESC VolumeDesc;

	const HRESULT hr = ProxyInterface->GetLevelDesc(Level, &VolumeDesc);

	if (FAILED(hr))
	{
		return hr;
	}

	ConvertVolumeDesc(VolumeDesc, *pDesc);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::GetVolumeLevel(UINT Level, Direct3DVolume8 **ppVolumeLevel)
{
	if (ppVolumeLevel == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppVolumeLevel = nullptr;

	IDirect3DVolume9 *VolumeInterface = nullptr;

	const HRESULT hr = ProxyInterface->GetVolumeLevel(Level, &VolumeInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppVolumeLevel = Device->address_table->FindAddress<Direct3DVolume8>(VolumeInterface);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::LockBox(UINT Level, D3DLOCKED_BOX *pLockedVolume, const D3DBOX *pBox, DWORD Flags)
{
	return ProxyInterface->LockBox(Level, pLockedVolume, pBox, Flags);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::UnlockBox(UINT Level)
{
	return ProxyInterface->UnlockBox(Level);
}

HRESULT STDMETHODCALLTYPE Direct3DVolumeTexture8::AddDirtyBox(const D3DBOX *pDirtyBox)
{
	return ProxyInterface->AddDirtyBox(pDirtyBox);
}

#endif
