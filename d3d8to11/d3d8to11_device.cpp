/**
 * Copyright (C) 2015 Patrick Mours. All rights reserved.
 * License: https://github.com/crosire/d3d8to9#license
 */

#include "pch.h"

#include <d3d11_1.h> // TODO: switch to newer header (11.3, 11.4)
#include <DirectXMath.h>

#include <cstdint>
#include <filesystem>
#include <format>
#include <fstream>
#include <ranges>

#include <CBufferWriter.h>

#include "alignment.h"
#include "d3d8to11.hpp"
#include "globals.h"
#include "ini_file.h"
#include "Material.h"
#include "not_implemented.h"
#include "RasterFlags.h"
#include "safe_release.h"
#include "ShaderFlags.h"
#include "ShaderIncluder.h"
#include "SimpleMath.h"

#include "d3d8to11_device.h"

#pragma comment(lib, "dxguid.lib") // for D3DDebugObjectName

// TODO: provide a wrapper structure that can swap out render targets when OIT is toggled

#define SHADER_ASYNC_COMPILE

using namespace Microsoft::WRL;
using namespace d3d8to11;

static constexpr uint32_t BLEND_COLORMASK_SHIFT = 28;

static const std::unordered_map<uint32_t, std::string> RS_STRINGS = {
	{ D3DRS_ZENABLE,                  "D3DRS_ZENABLE" },
	{ D3DRS_FILLMODE,                 "D3DRS_FILLMODE" },
	{ D3DRS_SHADEMODE,                "D3DRS_SHADEMODE" },
	{ D3DRS_LINEPATTERN,              "D3DRS_LINEPATTERN" },
	{ D3DRS_ZWRITEENABLE,             "D3DRS_ZWRITEENABLE" },
	{ D3DRS_ALPHATESTENABLE,          "D3DRS_ALPHATESTENABLE" },
	{ D3DRS_LASTPIXEL,                "D3DRS_LASTPIXEL" },
	{ D3DRS_SRCBLEND,                 "D3DRS_SRCBLEND" },
	{ D3DRS_DESTBLEND,                "D3DRS_DESTBLEND" },
	{ D3DRS_CULLMODE,                 "D3DRS_CULLMODE" },
	{ D3DRS_ZFUNC,                    "D3DRS_ZFUNC" },
	{ D3DRS_ALPHAREF,                 "D3DRS_ALPHAREF" },
	{ D3DRS_ALPHAFUNC,                "D3DRS_ALPHAFUNC" },
	{ D3DRS_DITHERENABLE,             "D3DRS_DITHERENABLE" },
	{ D3DRS_ALPHABLENDENABLE,         "D3DRS_ALPHABLENDENABLE" },
	{ D3DRS_FOGENABLE,                "D3DRS_FOGENABLE" },
	{ D3DRS_SPECULARENABLE,           "D3DRS_SPECULARENABLE" },
	{ D3DRS_ZVISIBLE,                 "D3DRS_ZVISIBLE" },
	{ D3DRS_FOGCOLOR,                 "D3DRS_FOGCOLOR" },
	{ D3DRS_FOGTABLEMODE,             "D3DRS_FOGTABLEMODE" },
	{ D3DRS_FOGSTART,                 "D3DRS_FOGSTART" },
	{ D3DRS_FOGEND,                   "D3DRS_FOGEND" },
	{ D3DRS_FOGDENSITY,               "D3DRS_FOGDENSITY" },
	{ D3DRS_EDGEANTIALIAS,            "D3DRS_EDGEANTIALIAS" },
	{ D3DRS_ZBIAS,                    "D3DRS_ZBIAS" },
	{ D3DRS_RANGEFOGENABLE,           "D3DRS_RANGEFOGENABLE" },
	{ D3DRS_STENCILENABLE,            "D3DRS_STENCILENABLE" },
	{ D3DRS_STENCILFAIL,              "D3DRS_STENCILFAIL" },
	{ D3DRS_STENCILZFAIL,             "D3DRS_STENCILZFAIL" },
	{ D3DRS_STENCILPASS,              "D3DRS_STENCILPASS" },
	{ D3DRS_STENCILFUNC,              "D3DRS_STENCILFUNC" },
	{ D3DRS_STENCILREF,               "D3DRS_STENCILREF" },
	{ D3DRS_STENCILMASK,              "D3DRS_STENCILMASK" },
	{ D3DRS_STENCILWRITEMASK,         "D3DRS_STENCILWRITEMASK" },
	{ D3DRS_TEXTUREFACTOR,            "D3DRS_TEXTUREFACTOR" },
	{ D3DRS_WRAP0,                    "D3DRS_WRAP0" },
	{ D3DRS_WRAP1,                    "D3DRS_WRAP1" },
	{ D3DRS_WRAP2,                    "D3DRS_WRAP2" },
	{ D3DRS_WRAP3,                    "D3DRS_WRAP3" },
	{ D3DRS_WRAP4,                    "D3DRS_WRAP4" },
	{ D3DRS_WRAP5,                    "D3DRS_WRAP5" },
	{ D3DRS_WRAP6,                    "D3DRS_WRAP6" },
	{ D3DRS_WRAP7,                    "D3DRS_WRAP7" },
	{ D3DRS_CLIPPING,                 "D3DRS_CLIPPING" },
	{ D3DRS_LIGHTING,                 "D3DRS_LIGHTING" },
	{ D3DRS_AMBIENT,                  "D3DRS_AMBIENT" },
	{ D3DRS_FOGVERTEXMODE,            "D3DRS_FOGVERTEXMODE" },
	{ D3DRS_COLORVERTEX,              "D3DRS_COLORVERTEX" },
	{ D3DRS_LOCALVIEWER,              "D3DRS_LOCALVIEWER" },
	{ D3DRS_NORMALIZENORMALS,         "D3DRS_NORMALIZENORMALS" },
	{ D3DRS_DIFFUSEMATERIALSOURCE,    "D3DRS_DIFFUSEMATERIALSOURCE" },
	{ D3DRS_SPECULARMATERIALSOURCE,   "D3DRS_SPECULARMATERIALSOURCE" },
	{ D3DRS_AMBIENTMATERIALSOURCE,    "D3DRS_AMBIENTMATERIALSOURCE" },
	{ D3DRS_EMISSIVEMATERIALSOURCE,   "D3DRS_EMISSIVEMATERIALSOURCE" },
	{ D3DRS_VERTEXBLEND,              "D3DRS_VERTEXBLEND" },
	{ D3DRS_CLIPPLANEENABLE,          "D3DRS_CLIPPLANEENABLE" },
	{ D3DRS_SOFTWAREVERTEXPROCESSING, "D3DRS_SOFTWAREVERTEXPROCESSING" },
	{ D3DRS_POINTSIZE,                "D3DRS_POINTSIZE" },
	{ D3DRS_POINTSIZE_MIN,            "D3DRS_POINTSIZE_MIN" },
	{ D3DRS_POINTSPRITEENABLE,        "D3DRS_POINTSPRITEENABLE" },
	{ D3DRS_POINTSCALEENABLE,         "D3DRS_POINTSCALEENABLE" },
	{ D3DRS_POINTSCALE_A,             "D3DRS_POINTSCALE_A" },
	{ D3DRS_POINTSCALE_B,             "D3DRS_POINTSCALE_B" },
	{ D3DRS_POINTSCALE_C,             "D3DRS_POINTSCALE_C" },
	{ D3DRS_MULTISAMPLEANTIALIAS,     "D3DRS_MULTISAMPLEANTIALIAS" },
	{ D3DRS_MULTISAMPLEMASK,          "D3DRS_MULTISAMPLEMASK" },
	{ D3DRS_PATCHEDGESTYLE,           "D3DRS_PATCHEDGESTYLE" },
	{ D3DRS_PATCHSEGMENTS,            "D3DRS_PATCHSEGMENTS" },
	{ D3DRS_DEBUGMONITORTOKEN,        "D3DRS_DEBUGMONITORTOKEN" },
	{ D3DRS_POINTSIZE_MAX,            "D3DRS_POINTSIZE_MAX" },
	{ D3DRS_INDEXEDVERTEXBLENDENABLE, "D3DRS_INDEXEDVERTEXBLENDENABLE" },
	{ D3DRS_COLORWRITEENABLE,         "D3DRS_COLORWRITEENABLE" },
	{ D3DRS_TWEENFACTOR,              "D3DRS_TWEENFACTOR" },
	{ D3DRS_BLENDOP,                  "D3DRS_BLENDOP" },
	{ D3DRS_POSITIONORDER,            "D3DRS_POSITIONORDER" },
	{ D3DRS_NORMALORDER,              "D3DRS_NORMALORDER" }
};

static const std::array FEATURE_LEVELS =
{
	D3D_FEATURE_LEVEL_12_1,
	D3D_FEATURE_LEVEL_12_0,
	D3D_FEATURE_LEVEL_11_1,
	D3D_FEATURE_LEVEL_11_0
};

size_t Direct3DDevice8::count_texture_stages() const
{
	size_t n = 0;

	for (const auto& stage : m_per_texture.stages)
	{
		if (stage.color_op.data() == D3DTOP_DISABLE && stage.alpha_op.data() == D3DTOP_DISABLE)
		{
			break;
		}

		++n;
	}

	return n;
}

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

void Direct3DDevice8::shader_preprocess(ShaderFlags::type flags, bool is_uber, std::vector<D3D_SHADER_MACRO>& definitions) const
{
	static const std::array texcoord_size_strings = {
		"FVF_TEXCOORD0_SIZE",
		"FVF_TEXCOORD1_SIZE",
		"FVF_TEXCOORD2_SIZE",
		"FVF_TEXCOORD3_SIZE",
		"FVF_TEXCOORD4_SIZE",
		"FVF_TEXCOORD5_SIZE",
		"FVF_TEXCOORD6_SIZE",
		"FVF_TEXCOORD7_SIZE"
	};

	static const std::array texcoord_size_types = {
		"FVF_TEXCOORD0_TYPE",
		"FVF_TEXCOORD1_TYPE",
		"FVF_TEXCOORD2_TYPE",
		"FVF_TEXCOORD3_TYPE",
		"FVF_TEXCOORD4_TYPE",
		"FVF_TEXCOORD5_TYPE",
		"FVF_TEXCOORD6_TYPE",
		"FVF_TEXCOORD7_TYPE"
	};

	static const std::array texcoord_format_types = {
		"float2",
		"float3",
		"float4",
		"float1",
	};

	const ShaderFlags::type sanitized_flags = ShaderFlags::sanitize(flags);

	definitions.clear();
	definitions.emplace_back("OIT_MAX_FRAGMENTS", m_oit_fragments_str.c_str());
	definitions.emplace_back("TEXTURE_STAGE_MAX", TOSTRING(TEXTURE_STAGE_MAX));

	auto uv_format = sanitized_flags >> 16u; // FIXME: magic number
	const auto tex_count = static_cast<size_t>(((sanitized_flags & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT) & 0xF);

	for (size_t i = 0; i < tex_count; i++)
	{
		const auto f = static_cast<size_t>(uv_format & 3u);
		uv_format >>= 2;

		switch (f)
		{
			case D3DFVF_TEXTUREFORMAT2:
				definitions.push_back({ texcoord_size_strings[i], "2" });
				break;

			case D3DFVF_TEXTUREFORMAT3:
				definitions.push_back({ texcoord_size_strings[i], "3" });
				break;

			case D3DFVF_TEXTUREFORMAT4:
				definitions.push_back({ texcoord_size_strings[i], "4" });
				break;

			case D3DFVF_TEXTUREFORMAT1:
				definitions.push_back({ texcoord_size_strings[i], "1" });
				break;

			default:
				continue;
		}

		definitions.push_back({ texcoord_size_types[i], texcoord_format_types[f] });
	}

	{
		const size_t stage_count = (sanitized_flags & ShaderFlags::stage_count_mask) >> ShaderFlags::stage_count_shift;
		const std::string& digit_string = m_digit_strings.at(stage_count);
		definitions.push_back({ "TEXTURE_STAGE_COUNT", digit_string.c_str() });
	}

	if ((sanitized_flags & D3DFVF_POSITION_MASK) == D3DFVF_XYZRHW)
	{
		definitions.push_back({ "FVF_RHW", "1" });
	}

	if ((sanitized_flags & D3DFVF_POSITION_MASK) == D3DFVF_XYZ)
	{
		definitions.push_back({ "FVF_XYZ", "1" });
	}

	if ((sanitized_flags & D3DFVF_NORMAL) != 0)
	{
		definitions.push_back({ "FVF_NORMAL", "1" });
	}

	if ((sanitized_flags & D3DFVF_DIFFUSE) != 0)
	{
		definitions.push_back({ "FVF_DIFFUSE", "1" });
	}

	if ((sanitized_flags & D3DFVF_SPECULAR) != 0)
	{
		definitions.push_back({ "FVF_SPECULAR", "1" });
	}

	if (sanitized_flags & D3DFVF_TEXCOUNT_MASK)
	{
		const size_t texcount = (sanitized_flags & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
		const std::string& digit_string = m_digit_strings.at(texcount);
		definitions.push_back({ "FVF_TEXCOUNT", digit_string.c_str() });
	}
	else
	{
		definitions.push_back({ "FVF_TEXCOUNT", "0" });
	}

	auto one_or_zero = [&](ShaderFlags::type mask)
	{
		return (sanitized_flags & mask) != 0 ? "1" : "0";
	};

	if (is_uber)
	{
		definitions.push_back({ "UBER", "1" });
	}
	else
	{
		definitions.push_back({ "UBER", "0" });

		definitions.push_back({ "RS_LIGHTING", one_or_zero(ShaderFlags::rs_lighting) });
		definitions.push_back({ "RS_SPECULAR", one_or_zero(ShaderFlags::rs_specular) });
		definitions.push_back({ "RS_ALPHA", one_or_zero(ShaderFlags::rs_alpha) });
		definitions.push_back({ "RS_ALPHA_TEST", one_or_zero(ShaderFlags::rs_alpha_test) });
		definitions.push_back({ "RS_FOG", one_or_zero(ShaderFlags::rs_fog) });
		definitions.push_back({ "RS_OIT", one_or_zero(ShaderFlags::rs_oit) });

		const auto alpha_test_mode = static_cast<size_t>((sanitized_flags & ShaderFlags::rs_alpha_test_mode_mask) >> ShaderFlags::rs_alpha_test_mode_shift);
		const auto fog_mode        = static_cast<size_t>((sanitized_flags & ShaderFlags::rs_fog_mode_mask) >> ShaderFlags::rs_fog_mode_shift);

		definitions.push_back({ "RS_ALPHA_TEST_MODE", m_digit_strings.at(alpha_test_mode).c_str() });
		definitions.push_back({ "RS_FOG_MODE", m_digit_strings.at(fog_mode).c_str() });
	}
}

std::vector<D3D_SHADER_MACRO> Direct3DDevice8::shader_preprocess(ShaderFlags::type flags, bool is_uber) const
{
	std::vector<D3D_SHADER_MACRO> definitions;
	shader_preprocess(flags, is_uber, definitions);
	return definitions;
}

void Direct3DDevice8::draw_call_increment()
{
	m_per_model.draw_call = (m_per_model.draw_call.data() + 1) % 65536;
}

static constexpr auto SHADER_COMPILER_FLAGS =
	D3DCOMPILE_PREFER_FLOW_CONTROL |
	D3DCOMPILE_DEBUG
// just keeping optimization on for now
#if 1 || !defined(_DEBUG)
	| D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
;

VertexShader Direct3DDevice8::compile_vertex_shader(ShaderFlags::type flags, bool is_uber)
{
	ComPtr<ID3DBlob> errors;
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3D11VertexShader> shader;

	std::filesystem::path shader_path = d3d8to11::config->get_shader_source_dir() / "shader.hlsl";

	if (d3d8to11::filesystem::should_extend_length(shader_path))
	{
		shader_path = d3d8to11::filesystem::as_extended_length(shader_path);
	}

	const auto shader_source = m_shader_includer.get_shader_source(shader_path);

	std::vector<D3D_SHADER_MACRO> preproc = shader_preprocess(flags, is_uber);
	preproc.push_back({});

	HRESULT hr;

	{
		// unfortunately a necessary evil :(
		const std::string shader_path_string = shader_path.string();
		hr = D3DCompile(shader_source.data(), shader_source.size(), shader_path_string.c_str(), preproc.data(), &m_shader_includer, "vs_main", "vs_5_0", SHADER_COMPILER_FLAGS, 0, &blob, &errors);
	}

	if (errors != nullptr)
	{
		const bool failed = FAILED(hr);

		const std::string str(static_cast<char*>(errors->GetBufferPointer()), 0, errors->GetBufferSize());
		const std::string message = std::format("\n" __FUNCTION__ "\n{} while compiling shader 0x{:016X}:\n{}\n", failed ? "error" : "warning", flags, str);
		OutputDebugStringA(message.c_str());

		if (failed)
		{
			throw std::runtime_error(str);
		}
	}

	hr = m_device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);

	if (FAILED(hr))
	{
		throw std::runtime_error("vertex shader creation failed");
	}

	return VertexShader(std::move(shader), std::move(blob));
}

PixelShader Direct3DDevice8::compile_pixel_shader(ShaderFlags::type flags, bool is_uber)
{
	ComPtr<ID3DBlob> errors;
	ComPtr<ID3DBlob> blob;
	ComPtr<ID3D11PixelShader> shader;

	std::filesystem::path shader_path = d3d8to11::config->get_shader_source_dir() / "shader.hlsl";

	if (d3d8to11::filesystem::should_extend_length(shader_path))
	{
		shader_path = d3d8to11::filesystem::as_extended_length(shader_path);
	}

	const auto shader_source = m_shader_includer.get_shader_source(shader_path);

	std::vector<D3D_SHADER_MACRO> preproc = shader_preprocess(flags, is_uber);
	preproc.push_back({});

	HRESULT hr;

	{
		// unfortunately a necessary evil :(
		const std::string shader_path_string = shader_path.string();
		hr = D3DCompile(shader_source.data(), shader_source.size(), shader_path_string.c_str(), preproc.data(), &m_shader_includer, "ps_main", "ps_5_0", SHADER_COMPILER_FLAGS, 0, &blob, &errors);
	}

	if (errors != nullptr)
	{
		const bool failed = FAILED(hr);

		const std::string str(static_cast<char*>(errors->GetBufferPointer()), 0, errors->GetBufferSize());
		const std::string message = std::format("\n" __FUNCTION__ "\n{} while compiling shader 0x{:016X}:\n{}\n", failed ? "error" : "warning", flags, str);
		OutputDebugStringA(message.c_str());

		if (failed)
		{
			throw std::runtime_error(str);
		}
	}

	hr = m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &shader);

	if (FAILED(hr))
	{
		throw std::runtime_error("pixel shader creation failed");
	}

	return PixelShader(std::move(shader), std::move(blob));
}

void Direct3DDevice8::store_permutation_flags(ShaderFlags::type flags)
{
	if (m_permutation_cache_file.is_open() &&
	    m_permutation_flags.insert(flags).second == true)
	{
		const std::string str = std::format("writing shader permutation to cache: 0x{:016X}\n", flags);
		OutputDebugStringA(str.c_str());
		m_permutation_cache_file.write(reinterpret_cast<const char*>(&flags), sizeof(flags));
		m_permutation_cache_file.flush();
	}
}

VertexShader Direct3DDevice8::get_vertex_shader(ShaderFlags::type flags)
{
	const auto sanitized_flags = ShaderFlags::sanitize(flags);

	const auto base_flags = ShaderFlags::sanitize(sanitized_flags & ShaderFlags::vs_mask);

	{
		const auto it = m_vertex_shaders.find(base_flags);

		if (it != m_vertex_shaders.end())
		{
			return it->second;
		}
	}

	{
		auto enqueue_function = [this](ShaderFlags::type flags) -> VertexShader
		{
			return compile_vertex_shader(flags, false);
		};

#ifdef SHADER_ASYNC_COMPILE
		const auto it = m_compiling_vertex_shaders.find(base_flags);

		if (it == m_compiling_vertex_shaders.end())
		{
			auto compilation_task = m_thread_pool.enqueue(enqueue_function, base_flags);

			// we *could* check *right now* to see if this shader is somehow already done
			// and skip the compiling queue, but nah.
			m_compiling_vertex_shaders[base_flags] = std::move(compilation_task);
			store_permutation_flags(base_flags);
		}
		else
		{
			auto& future = it->second;

			if (is_future_ready(future))
			{
				auto shader = std::move(future.get());
				m_compiling_vertex_shaders.erase(it);
				m_vertex_shaders[base_flags] = shader;
				return shader;
			}
		}
#else
		store_permutation_flags(base_flags);
		auto shader = enqueue_function(base_flags);
		vertex_shaders[base_flags] = shader;
		return shader;
#endif
	}

	const auto uber_flags = ShaderFlags::sanitize(sanitized_flags & ShaderFlags::uber_vs_mask);

	{
		const auto it = m_uber_vertex_shaders.find(uber_flags);

		if (it != m_uber_vertex_shaders.end())
		{
			return it->second;
		}
	}

	auto shader = compile_vertex_shader(uber_flags, true);
	m_uber_vertex_shaders[uber_flags] = shader;
	return shader;
}

PixelShader Direct3DDevice8::get_pixel_shader(ShaderFlags::type flags)
{
	const auto sanitized_flags = ShaderFlags::sanitize(flags);

	const auto base_flags = ShaderFlags::sanitize(sanitized_flags & ShaderFlags::ps_mask);

	{
		const auto it = m_pixel_shaders.find(base_flags);

		if (it != m_pixel_shaders.end())
		{
			return it->second;
		}
	}

	{
		auto enqueue_function = [this](ShaderFlags::type flags) -> PixelShader
		{
			return compile_pixel_shader(flags, false);
		};

#ifdef SHADER_ASYNC_COMPILE
		const auto it = m_compiling_pixel_shaders.find(base_flags);

		if (it == m_compiling_pixel_shaders.end())
		{
			auto compilation_task = m_thread_pool.enqueue(enqueue_function, base_flags);

			// we *could* check *right now* to see if this shader is somehow already done
			// and skip the compiling queue, but nah.
			m_compiling_pixel_shaders[base_flags] = std::move(compilation_task);
			store_permutation_flags(base_flags);
		}
		else
		{
			auto& future = it->second;

			if (is_future_ready(future))
			{
				auto shader = std::move(future.get());
				m_compiling_pixel_shaders.erase(it);
				m_pixel_shaders[base_flags] = shader;
				return shader;
			}
		}
#else
		store_permutation_flags(base_flags);
		auto shader = enqueue_function(base_flags);
		pixel_shaders[base_flags] = shader;
		return shader;
#endif
	}

	const auto uber_flags = ShaderFlags::sanitize(sanitized_flags & ShaderFlags::uber_ps_mask);

	{
		const auto it = m_uber_pixel_shaders.find(uber_flags);

		if (it != m_uber_pixel_shaders.end())
		{
			return it->second;
		}
	}

	auto shader = compile_pixel_shader(uber_flags, true);
	m_uber_pixel_shaders[uber_flags] = shader;
	return shader;
}

void Direct3DDevice8::create_depth_stencil()
{
	m_depth_stencil = new Direct3DTexture8(this, m_present_params.BackBufferWidth, m_present_params.BackBufferHeight, 1,
	                                       D3DUSAGE_DEPTHSTENCIL, m_present_params.AutoDepthStencilFormat, D3DPOOL_DEFAULT);

	m_depth_stencil->create_native();
	m_depth_stencil->GetSurfaceLevel(0, &m_current_depth_stencil);
}

void Direct3DDevice8::create_composite_texture(D3D11_TEXTURE2D_DESC& tex_desc)
{
	tex_desc.Usage     = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_device->CreateTexture2D(&tex_desc, nullptr, &m_oit_composite_texture);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create composite target texture");
	}

	D3D11_RENDER_TARGET_VIEW_DESC view_desc {};

	view_desc.Format             = tex_desc.Format;
	view_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipSlice = 0;

	hr = m_device->CreateRenderTargetView(m_oit_composite_texture.Get(), &view_desc, &m_oit_composite_view);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create composite target view");
	}

	std::string composite_view_name = "m_oit_composite_view";
	m_oit_composite_view->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(composite_view_name.size()), composite_view_name.data());

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};

	srv_desc.Format                    = tex_desc.Format;
	srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels       = 1;

	hr = m_device->CreateShaderResourceView(m_oit_composite_texture.Get(), &srv_desc, &m_oit_composite_srv);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create composite resource view");
	}

	std::string composite_srv_name = "m_oit_composite_srv";
	m_oit_composite_srv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(composite_srv_name.size()), composite_srv_name.data());

	m_oit_composite_wrapper = new Direct3DTexture8(this, tex_desc.Width, tex_desc.Height, tex_desc.MipLevels, D3DUSAGE_RENDERTARGET, m_present_params.BackBufferFormat, D3DPOOL_DEFAULT);
	m_oit_composite_wrapper->create_native(m_oit_composite_texture.Get());
}

void Direct3DDevice8::create_render_target(D3D11_TEXTURE2D_DESC& tex_desc)
{
	tex_desc.Usage     = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_device->CreateTexture2D(&tex_desc, nullptr, &m_render_target_texture);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create render target texture");
	}

	D3D11_RENDER_TARGET_VIEW_DESC view_desc {};

	view_desc.Format             = tex_desc.Format;
	view_desc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
	view_desc.Texture2D.MipSlice = 0;

	hr = m_device->CreateRenderTargetView(m_render_target_texture.Get(), &view_desc, &m_render_target_view);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create render target view");
	}

	std::string view_name = "m_render_target_view";
	m_render_target_view->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(view_name.size()), view_name.data());

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc {};

	srv_desc.Format                    = tex_desc.Format;
	srv_desc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels       = 1;

	hr = m_device->CreateShaderResourceView(m_render_target_texture.Get(), &srv_desc, &m_render_target_srv);

	if (FAILED(hr))
	{
		throw std::runtime_error("Failed to create composite resource view");
	}

	std::string render_target_srv_name = "m_render_target_srv";
	m_render_target_srv->SetPrivateData(WKPDID_D3DDebugObjectName, static_cast<UINT>(render_target_srv_name.size()), render_target_srv_name.data());

	m_render_target_wrapper = new Direct3DTexture8(this, tex_desc.Width, tex_desc.Height, tex_desc.MipLevels, D3DUSAGE_RENDERTARGET, m_present_params.BackBufferFormat, D3DPOOL_DEFAULT);
	m_render_target_wrapper->create_native(m_render_target_texture.Get());
}

void Direct3DDevice8::get_back_buffer()
{
	ID3D11Texture2D* pBackBuffer = nullptr;
	m_swap_chain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));

	D3D11_TEXTURE2D_DESC tex_desc {};
	pBackBuffer->GetDesc(&tex_desc);

	m_back_buffer = new Direct3DTexture8(this, tex_desc.Width, tex_desc.Height, tex_desc.MipLevels, D3DUSAGE_RENDERTARGET, m_present_params.BackBufferFormat, D3DPOOL_DEFAULT);
	m_back_buffer->create_native(pBackBuffer);

	//m_back_buffer->GetSurfaceLevel(0, &current_render_target);

	m_device->CreateRenderTargetView(pBackBuffer, nullptr, &m_back_buffer_view);

	pBackBuffer->Release();

	ComPtr<Direct3DSurface8> ds_surface;
	m_depth_stencil->GetSurfaceLevel(0, &ds_surface);

	create_composite_texture(tex_desc);
	create_render_target(tex_desc);

	if (oit_enabled)
	{
		m_context->OMSetRenderTargets(1, m_oit_composite_view.GetAddressOf(), ds_surface->get_native_depth_stencil());

		// FIXME: this doesn't make sense! If a program is expecting the render target with things in it, this has nothing until ::Present()!
		m_oit_composite_wrapper->GetSurfaceLevel(0, &m_current_render_target);
	}
	else
	{
		// set the composite render target as the back buffer
		m_context->OMSetRenderTargets(1, m_render_target_view.GetAddressOf(), ds_surface->get_native_depth_stencil());
		m_render_target_wrapper->GetSurfaceLevel(0, &m_current_render_target);
	}
}

void Direct3DDevice8::create_native()
{
	m_shader_includer.set_base_directory(d3d8to11::config->get_shader_source_dir());
	m_shader_includer.add_include_directory(d3d8to11::config->get_shader_source_dir());

	oit_enabled = d3d8to11::config->get_oit_config().enabled;

	if (!m_present_params.EnableAutoDepthStencil)
	{
		throw std::runtime_error("manual depth buffer not supported");
	}

	m_palette_flag = supports_palettes();

	/*
	 * BackBufferWidth and BackBufferHeight 
	 * Width and height of the new swap chain's back buffers, in pixels. If Windowed is FALSE (the presentation is full-screen),
	 * then these values must equal the width and height of one of the enumerated display modes found through IDirect3D8::EnumAdapterModes.
	 * If Windowed is TRUE and either of these values is zero, then the corresponding dimension of the client area of the hDeviceWindow
	 * (or the focus window, if hDeviceWindow is NULL) is taken.
	 */

	UINT& width = m_present_params.BackBufferWidth;
	UINT& height = m_present_params.BackBufferHeight;

	if (m_present_params.Windowed && (!width || !height))
	{
		HWND handle = m_present_params.hDeviceWindow ? m_present_params.hDeviceWindow : m_focus_window;

		RECT rect;
		GetClientRect(handle, &rect);

		if (!width)
		{
			width = rect.right - rect.left;
		}

		if (!height)
		{
			height = rect.bottom - rect.top;
		}
	}

	DXGI_SWAP_CHAIN_DESC desc = {};

	desc.BufferCount        = 1;
	desc.BufferDesc.Format  = to_dxgi(m_present_params.BackBufferFormat);
	desc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferDesc.Width   = width;
	desc.BufferDesc.Height  = height;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
	desc.OutputWindow       = m_present_params.hDeviceWindow;
	desc.SampleDesc.Count   = 1;
	desc.Windowed           = m_present_params.Windowed;
	desc.Flags              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	auto feature_level = static_cast<D3D_FEATURE_LEVEL>(0);

#ifdef _DEBUG
	constexpr auto flag = D3D11_CREATE_DEVICE_DEBUG;
#else
	constexpr auto flag = 0;
#endif

	// TODO: use more modern swap chain creation and management
	auto error = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flag,
	                                           FEATURE_LEVELS.data(), static_cast<UINT>(FEATURE_LEVELS.size()),
	                                           D3D11_SDK_VERSION, &desc, &m_swap_chain,
	                                           &m_device, &feature_level, &m_context);

	if (feature_level < D3D_FEATURE_LEVEL_11_0)
	{
		throw std::runtime_error("Device does not meet the minimum required feature level (D3D_FEATURE_LEVEL_11_0).");
	}

	if (error != S_OK)
	{
		throw std::runtime_error("Device creation failed with a known error that I'm too lazy to get the details of.");
	}

	m_device->QueryInterface(__uuidof(ID3D11InfoQueue), &m_info_queue);

	if (m_info_queue)
	{
		OutputDebugStringA("D3D11 debug info queue enabled\n");
		m_info_queue->SetMuteDebugOutput(FALSE);
	}

	m_swap_chain->SetFullscreenState(!m_present_params.Windowed, nullptr);

	create_depth_stencil();
	get_back_buffer();

	D3DVIEWPORT8 vp {};
	vp.Width  = m_present_params.BackBufferWidth;
	vp.Height = m_present_params.BackBufferHeight;
	vp.MaxZ   = 1.0f;
	SetViewport(&vp);

	HRESULT hr = make_cbuffer(m_uber_shader_flags, m_uber_shader_cbuffer);
	if (FAILED(hr))
	{
		throw std::runtime_error("uber-shader CreateBuffer failed");
	}

	hr = make_cbuffer(m_per_scene, m_per_scene_cbuffer);
	if (FAILED(hr))
	{
		throw std::runtime_error("per-scene CreateBuffer failed");
	}

	hr = make_cbuffer(m_per_model, m_per_model_cbuffer);
	if (FAILED(hr))
	{
		throw std::runtime_error("per-model CreateBuffer failed");
	}

	hr = make_cbuffer(m_per_pixel, m_per_pixel_cbuffer);
	if (FAILED(hr))
	{
		throw std::runtime_error("per-pixel CreateBuffer failed");
	}

	hr = make_cbuffer(m_per_texture, m_per_texture_cbuffer);
	if (FAILED(hr))
	{
		throw std::runtime_error("per-texture CreateBuffer failed");
	}

	m_context->VSSetConstantBuffers(0, 1, m_uber_shader_cbuffer.GetAddressOf());
	m_context->PSSetConstantBuffers(0, 1, m_uber_shader_cbuffer.GetAddressOf());

	m_context->VSSetConstantBuffers(1, 1, m_per_scene_cbuffer.GetAddressOf());
	m_context->PSSetConstantBuffers(1, 1, m_per_scene_cbuffer.GetAddressOf());

	m_context->VSSetConstantBuffers(2, 1, m_per_model_cbuffer.GetAddressOf());
	m_context->PSSetConstantBuffers(2, 1, m_per_model_cbuffer.GetAddressOf());

	m_context->VSSetConstantBuffers(3, 1, m_per_pixel_cbuffer.GetAddressOf());
	m_context->PSSetConstantBuffers(3, 1, m_per_pixel_cbuffer.GetAddressOf());

	m_context->VSSetConstantBuffers(4, 1, m_per_texture_cbuffer.GetAddressOf());
	m_context->PSSetConstantBuffers(4, 1, m_per_texture_cbuffer.GetAddressOf());

	{
		const auto& permutation_file_path = d3d8to11::config->get_shader_cache_variants_file_path();
		const bool exists = std::filesystem::exists(permutation_file_path);

		decltype(m_permutation_flags) temp_permutation_flags;
		std::fstream permutation_file;

		if (permutation_file_path.empty())
		{
			OutputDebugStringA("The file path for the shader permutation cache was too long and extended-length paths are not enabled."
			                   " Shaders will not be cached.\n");
		}
		else
		{
			permutation_file.open(permutation_file_path, std::ios::binary | std::ios::in | std::ios::out);

			if (!permutation_file.is_open())
			{
				const std::string str =
					std::format("Unable to open or create shader permutation cache file: \"{}\"\nShaders will not be cached.\n",
					            permutation_file_path.string());

				OutputDebugStringA(str.c_str());
			}
		}

		if (exists)
		{
			OutputDebugStringA("precompiling shaders...\n");

			while (permutation_file.is_open() && !permutation_file.eof())
			{
				ShaderFlags::type flags = 0;
				permutation_file.read(reinterpret_cast<char*>(&flags), sizeof(flags));

				if (permutation_file.gcount() != sizeof(flags))
				{
					break;
				}

				// TODO: if there are duplicate entries (particularly after sanitizing), rewrite the permutation file
				temp_permutation_flags.insert(flags);
			}

			if (permutation_file.is_open() && permutation_file.eof())
			{
				permutation_file.clear();
			}

			auto compile_vertex_shader_wrapper = [this](ShaderFlags::type flags, bool is_uber)
			{
				return compile_vertex_shader(flags, is_uber);
			};

			auto compile_pixel_shader_wrapper = [this](ShaderFlags::type flags, bool is_uber)
			{
				return compile_pixel_shader(flags, is_uber);
			};

			{
				const auto uber_start = std::chrono::high_resolution_clock::now();

				std::unordered_map<ShaderFlags::type, std::future<VertexShader>> uber_vs_tasks;
				std::unordered_map<ShaderFlags::type, std::future<PixelShader>> uber_ps_tasks;

				uber_vs_tasks.reserve(temp_permutation_flags.size());
				uber_ps_tasks.reserve(temp_permutation_flags.size());

				for (ShaderFlags::type flags : temp_permutation_flags)
				{
					const auto sanitized_vs = ShaderFlags::sanitize(flags & ShaderFlags::uber_vs_mask);
					const auto sanitized_ps = ShaderFlags::sanitize(flags & ShaderFlags::uber_ps_mask);

					if (!uber_vs_tasks.contains(sanitized_vs))
					{
						const std::string str = std::format("enqueueing uber vertex shader: 0x{:016X}\n", sanitized_vs);
						OutputDebugStringA(str.c_str());

						uber_vs_tasks[sanitized_vs] = m_thread_pool.enqueue(compile_vertex_shader_wrapper, sanitized_vs, true);
					}

					if (!uber_ps_tasks.contains(sanitized_ps))
					{
						const std::string str = std::format("enqueueing uber pixel shader: 0x{:016X}\n", sanitized_ps);
						OutputDebugStringA(str.c_str());

						uber_ps_tasks[sanitized_ps] = m_thread_pool.enqueue(compile_pixel_shader_wrapper, sanitized_ps, true);
					}
				}

				OutputDebugStringA("waiting for uber shader compilation to finish...\n");

				for (auto& [flags, task] : uber_vs_tasks)
				{
					m_uber_vertex_shaders[flags] = std::move(task.get());
				}

				for (auto& [flags, task] : uber_ps_tasks)
				{
					m_uber_pixel_shaders[flags] = std::move(task.get());
				}

				const auto uber_end = std::chrono::high_resolution_clock::now();
				const auto uber_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(uber_end - uber_start);

				const size_t uber_vs_count = m_uber_vertex_shaders.size();
				const size_t uber_ps_count = m_uber_pixel_shaders.size();

				const std::string str =
					std::format("done ({} vertex shader(s) and {} pixel shader(s) ({} total) in {} ms)\n"
					            "\nenqueueing standard shaders now...\n",
					            uber_vs_count, uber_ps_count, uber_vs_count + uber_ps_count, uber_elapsed.count());

				OutputDebugStringA(str.c_str());
			}

			for (ShaderFlags::type flags : temp_permutation_flags)
			{
				const auto sanitized_vs = ShaderFlags::sanitize(flags & ShaderFlags::vs_mask);
				const auto sanitized_ps = ShaderFlags::sanitize(flags & ShaderFlags::ps_mask);

				if (!m_compiling_vertex_shaders.contains(sanitized_vs))
				{
					const std::string str = std::format("enqueueing standard vertex shader: 0x{:016X}\n", sanitized_vs);
					OutputDebugStringA(str.c_str());

					m_compiling_vertex_shaders[sanitized_vs] = m_thread_pool.enqueue(compile_vertex_shader_wrapper, sanitized_vs, false);
				}

				if (!m_compiling_pixel_shaders.contains(sanitized_ps))
				{
					const std::string str = std::format("enqueueing standard pixel shader: 0x{:016X}\n", sanitized_ps);
					OutputDebugStringA(str.c_str());

					m_compiling_pixel_shaders[sanitized_ps] = m_thread_pool.enqueue(compile_pixel_shader_wrapper, sanitized_ps, false);
				}
			}

			OutputDebugStringA("done\n");
		}

		if (permutation_file.is_open())
		{
			// make sure the next write position is aligned sizeof(ShaderFlags::type)
			// so that we don't write any malformed flags.
			const auto position = align_down(permutation_file.tellg(), sizeof(ShaderFlags::type));
			permutation_file.seekp(position);

			m_permutation_cache_file = std::move(permutation_file);
			m_permutation_flags = std::move(temp_permutation_flags);
		}
	}

	m_blend_flags         = 0;
	m_raster_flags        = 0;
	m_depth_stencil_flags = {};

	// TODO: properly set default for D3DRS_ZENABLE; see below
	// The default value for this render state is D3DZB_TRUE if a depth stencil was created along with the swap chain by setting
	// the EnableAutoDepthStencil member of the D3DPRESENT_PARAMETERS structure to TRUE, and D3DZB_FALSE otherwise. 
	SetRenderState(D3DRS_ZENABLE, TRUE);

	SetRenderState(D3DRS_ZWRITEENABLE,     TRUE);
	SetRenderState(D3DRS_FILLMODE,         D3DFILL_SOLID);
	SetRenderState(D3DRS_SRCBLEND,         D3DBLEND_ONE);
	SetRenderState(D3DRS_DESTBLEND,        D3DBLEND_ZERO);
	SetRenderState(D3DRS_CULLMODE,         D3DCULL_CCW);
	SetRenderState(D3DRS_ZFUNC,            D3DCMP_LESS);
	SetRenderState(D3DRS_ALPHAFUNC,        D3DCMP_ALWAYS);
	SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	SetRenderState(D3DRS_FOGENABLE,        FALSE);
	SetRenderState(D3DRS_SPECULARENABLE,   TRUE);
	SetRenderState(D3DRS_LIGHTING,         TRUE);
	SetRenderState(D3DRS_COLORVERTEX,      TRUE);
	SetRenderState(D3DRS_LOCALVIEWER,      TRUE);
	SetRenderState(D3DRS_BLENDOP,          D3DBLENDOP_ADD);
	SetRenderState(D3DRS_COLORWRITEENABLE, 0xF);

	SetRenderState(D3DRS_AMBIENTMATERIALSOURCE,  D3DMCS_MATERIAL);
	SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE,  D3DMCS_COLOR1);
	SetRenderState(D3DRS_SPECULARMATERIALSOURCE, D3DMCS_COLOR2);
	SetRenderState(D3DRS_EMISSIVEMATERIALSOURCE, D3DMCS_MATERIAL);

	SetRenderState(D3DRS_STENCILENABLE,    FALSE);
	SetRenderState(D3DRS_STENCILFAIL,      D3DSTENCILOP_KEEP);
	SetRenderState(D3DRS_STENCILZFAIL,     D3DSTENCILOP_KEEP);
	SetRenderState(D3DRS_STENCILPASS,      D3DSTENCILOP_KEEP);
	SetRenderState(D3DRS_STENCILFUNC,      D3DCMP_ALWAYS);
	SetRenderState(D3DRS_STENCILREF,       0);
	SetRenderState(D3DRS_STENCILMASK,      0xFFFFFFFF);
	SetRenderState(D3DRS_STENCILWRITEMASK, 0xFFFFFFFF);

	for (auto& state : m_render_state_values)
	{
		state.mark();
	}

	// set all the texture stage states to their defaults

	for (DWORD i = 0; i < TEXTURE_STAGE_MAX; i++)
	{
		SetTextureStageState(i, D3DTSS_COLOROP, !i ? D3DTOP_MODULATE : D3DTOP_DISABLE);
		SetTextureStageState(i, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		SetTextureStageState(i, D3DTSS_COLORARG2, D3DTA_CURRENT);
		SetTextureStageState(i, D3DTSS_ALPHAOP, !i ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE);
		SetTextureStageState(i, D3DTSS_ALPHAARG1, D3DTA_TEXTURE); // If no texture is set for this stage, the default argument is D3DTA_DIFFUSE.
		SetTextureStageState(i, D3DTSS_ALPHAARG2, D3DTA_CURRENT);
		SetTextureStageState(i, D3DTSS_BUMPENVMAT00, 0);
		SetTextureStageState(i, D3DTSS_BUMPENVMAT01, 0);
		SetTextureStageState(i, D3DTSS_BUMPENVMAT10, 0);
		SetTextureStageState(i, D3DTSS_BUMPENVMAT11, 0);
		SetTextureStageState(i, D3DTSS_TEXCOORDINDEX, i);
		SetTextureStageState(i, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		SetTextureStageState(i, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
		SetTextureStageState(i, D3DTSS_BORDERCOLOR, 0);
		SetTextureStageState(i, D3DTSS_MAGFILTER, D3DTEXF_POINT);
		SetTextureStageState(i, D3DTSS_MINFILTER, D3DTEXF_POINT);
		SetTextureStageState(i, D3DTSS_MIPFILTER, D3DTEXF_NONE);
		SetTextureStageState(i, D3DTSS_MIPMAPLODBIAS, 0);
		SetTextureStageState(i, D3DTSS_MAXMIPLEVEL, 0);
		SetTextureStageState(i, D3DTSS_MAXANISOTROPY, 1);
		SetTextureStageState(i, D3DTSS_BUMPENVLSCALE, 0);
		SetTextureStageState(i, D3DTSS_BUMPENVLOFFSET, 0);
		SetTextureStageState(i, D3DTSS_TEXTURETRANSFORMFLAGS, D3DTTFF_DISABLE);
		SetTextureStageState(i, D3DTSS_ADDRESSW, D3DTADDRESS_WRAP);
		SetTextureStageState(i, D3DTSS_COLORARG0, D3DTA_CURRENT);
		SetTextureStageState(i, D3DTSS_ALPHAARG0, D3DTA_CURRENT);
		SetTextureStageState(i, D3DTSS_RESULTARG, D3DTA_CURRENT);
	}

	for (auto& light : m_per_model.lights)
	{
		Light actual_light = {};
		actual_light.diffuse = float4(1.0f, 1.0f, 1.0f, 0.0f);
		actual_light.direction = float3(0.0f, 0.0f, 1.0f);
		light = actual_light;
	}

	D3DMATERIAL8 material {};
	material.Diffuse = { 1.0f, 1.0f, 1.0f, 1.0f };
	SetMaterial(&material);

	m_blend_flags.mark();
	m_raster_flags.mark();
	m_depth_stencil_flags.mark();

	m_fvf_flags = 0;
	m_fvf_flags.mark();

	m_uber_shader_flags.mark();
	m_per_scene.mark();
	m_per_model.mark();
	m_per_pixel.mark();
	m_per_texture.mark();

	oit_load_shaders();
	oit_init();

	update();
}

// IDirect3DDevice8
Direct3DDevice8::Direct3DDevice8(Direct3D8* d3d, UINT adapter, D3DDEVTYPE device_type, HWND focus_window, DWORD behavior_flags, const D3DPRESENT_PARAMETERS8& parameters)
	: m_d3d(d3d),
	  m_adapter(adapter),
	  m_focus_window(focus_window),
	  m_device_type(device_type),
	  m_behavior_flags(behavior_flags),
	  m_present_params(parameters),
	  m_oit_fragments_str(std::to_string(globals::max_fragments)),
	  m_thread_pool(std::max<size_t>(2, std::thread::hardware_concurrency()) - 1)
{
	constexpr size_t max_digit_strings = std::max(static_cast<size_t>(TEXTURE_STAGE_MAX), FVF_TEXCOORD_MAX);

	for (size_t i = 0; i <= max_digit_strings; ++i)
	{
		m_digit_strings[i] = std::to_string(i);
	}
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::QueryInterface(REFIID riid, void** ppvObj)
{
	if (ppvObj == nullptr)
	{
		return E_POINTER;
	}

	if (riid == __uuidof(this) ||
	    riid == __uuidof(IUnknown))
	{
		AddRef();

		*ppvObj = this;

		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE Direct3DDevice8::AddRef()
{
	return Unknown::AddRef();
}

ULONG STDMETHODCALLTYPE Direct3DDevice8::Release()
{
	ULONG LastRefCount = Unknown::Release();

	if (LastRefCount == 0)
	{
		delete this;
	}

	return LastRefCount;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::TestCooperativeLevel()
{
	NOT_IMPLEMENTED;
	return D3D_OK;
}

UINT STDMETHODCALLTYPE Direct3DDevice8::GetAvailableTextureMem()
{
#if 1
	NOT_IMPLEMENTED;
	return UINT_MAX;
#else
	return ProxyInterface->GetAvailableTextureMem();
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::ResourceManagerDiscardBytes(DWORD Bytes)
{
	UNREFERENCED_PARAMETER(Bytes);

#if 1
	NOT_IMPLEMENTED_RETURN;
#else
	return ProxyInterface->EvictManagedResources();
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDirect3D(Direct3D8** ppD3D8)
{
	if (ppD3D8 == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	m_d3d->AddRef();

	*ppD3D8 = m_d3d;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDeviceCaps(D3DCAPS8* pCaps)
{
	return m_d3d->GetDeviceCaps(m_adapter, m_device_type, pCaps);
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDisplayMode(D3DDISPLAYMODE* pMode)
{
	return m_d3d->GetAdapterDisplayMode(m_adapter, pMode);
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters)
{
	if (!pParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	pParameters->BehaviorFlags  = m_behavior_flags;
	pParameters->AdapterOrdinal = m_adapter;
	pParameters->DeviceType     = m_device_type;
	pParameters->hFocusWindow   = m_focus_window;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, Direct3DSurface8* pCursorBitmap)
{
	NOT_IMPLEMENTED_RETURN;
}

void STDMETHODCALLTYPE Direct3DDevice8::SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags)
{
	NOT_IMPLEMENTED;
}

BOOL STDMETHODCALLTYPE Direct3DDevice8::ShowCursor(BOOL bShow)
{
	NOT_IMPLEMENTED;
	return false;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS8* pPresentationParameters, Direct3DSwapChain8** ppSwapChain)
{
#if 1
	NOT_IMPLEMENTED_RETURN;
#else
	if (pPresentationParameters == nullptr || ppSwapChain == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppSwapChain = nullptr;

	D3DPRESENT_PARAMETERS PresentParams;
	ConvertPresentParameters(*pPresentationParameters, PresentParams);

	// Get multisample quality level
	if (PresentParams.MultiSampleType != D3DMULTISAMPLE_NONE)
	{
		DWORD QualityLevels = 0;
		D3DDEVICE_CREATION_PARAMETERS CreationParams;
		ProxyInterface->GetCreationParameters(&CreationParams);

		if (d3d->GetProxyInterface()->CheckDeviceMultiSampleType(CreationParams.AdapterOrdinal,
			CreationParams.DeviceType, PresentParams.BackBufferFormat, PresentParams.Windowed,
			PresentParams.MultiSampleType, &QualityLevels) == S_OK &&
			d3d->GetProxyInterface()->CheckDeviceMultiSampleType(CreationParams.AdapterOrdinal,
				CreationParams.DeviceType, PresentParams.AutoDepthStencilFormat, PresentParams.Windowed,
				PresentParams.MultiSampleType, &QualityLevels) == S_OK)
		{
			PresentParams.MultiSampleQuality = (QualityLevels != 0) ? QualityLevels - 1 : 0;
		}
	}

	IDirect3DSwapChain9 *SwapChainInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateAdditionalSwapChain(&PresentParams, &SwapChainInterface);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppSwapChain = new Direct3DSwapChain8(this, SwapChainInterface);
	(*ppSwapChain)->AddRef();

	return D3D_OK;
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::Reset(D3DPRESENT_PARAMETERS8* pPresentationParameters)
{
	if (!pPresentationParameters)
	{
		return D3DERR_INVALIDCALL;
	}

	// TODO: handle actual device lost state
	// TODO: handle/fix fullscreen toggle

	UINT& width = pPresentationParameters->BackBufferWidth;
	UINT& height = pPresentationParameters->BackBufferHeight;

	if (pPresentationParameters->Windowed && (!width || !height))
	{
		HWND handle = pPresentationParameters->hDeviceWindow
		              ? pPresentationParameters->hDeviceWindow
		              : m_focus_window;

		RECT rect;
		GetClientRect(handle, &rect);

		if (!width)
		{
			width = rect.right - rect.left;
		}

		if (!height)
		{
			height = rect.bottom - rect.top;
		}
	}

	if (pPresentationParameters->BackBufferWidth != m_present_params.BackBufferWidth ||
	    pPresentationParameters->BackBufferHeight != m_present_params.BackBufferHeight ||
	    pPresentationParameters->Windowed != m_present_params.Windowed)
	{
		m_present_params = *pPresentationParameters;

		m_context->OMSetRenderTargets(0, nullptr, nullptr);

		m_back_buffer           = nullptr;
		m_current_depth_stencil = nullptr;
		m_current_render_target = nullptr;
		m_back_buffer_view      = nullptr;

		m_swap_chain->ResizeBuffers(1, m_present_params.BackBufferWidth, m_present_params.BackBufferHeight,
		                            DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);

		create_depth_stencil();
		get_back_buffer();

		D3DVIEWPORT8 vp {};
		GetViewport(&vp);

		vp.Width  = m_present_params.BackBufferWidth;
		vp.Height = m_present_params.BackBufferHeight;

		SetViewport(&vp);

		oit_release();
		oit_init();

		m_shader_flags &= ~ShaderFlags::fvf_mask;
		m_fvf_flags = 0;
		m_fvf_flags.clear();
	}

	return D3D_OK;
}

void Direct3DDevice8::oit_composite()
{
	if (!m_oit_actually_enabled)
	{
		return;
	}

	DWORD CULLMODE, ZENABLE;
	GetRenderState(D3DRS_CULLMODE, &CULLMODE);
	GetRenderState(D3DRS_ZENABLE, &ZENABLE);

	SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	SetRenderState(D3DRS_ZENABLE, FALSE);

	static constexpr auto BLEND_DEFAULT = D3DBLEND_ONE | (D3DBLEND_ONE << 4) | (D3DBLENDOP_ADD << 8) | (0xF << BLEND_COLORMASK_SHIFT);
	auto blend_flags = m_blend_flags.data();
	m_blend_flags = BLEND_DEFAULT;
	update();

	// Unbinds UAV read/write buffers and binds their read-only
	// shader resource views.
	oit_read();

	ID3D11VertexShader* vs;
	ID3D11PixelShader* ps;

	m_context->VSGetShader(&vs, nullptr, nullptr);
	m_context->PSGetShader(&ps, nullptr, nullptr);

	// Switches to the composite to begin the sorting process.
	m_context->VSSetShader(m_oit_composite_vs.shader.Get(), nullptr, 0);
	m_context->PSSetShader(m_oit_composite_ps.shader.Get(), nullptr, 0);

	// Unbind the last vertex & index buffers
	m_context->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	m_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// ...then draw 3 points. The composite shader uses SV_VertexID
	// to generate a full screen triangle, so we don't need a buffer!
	m_context->Draw(3, 0);

	m_context->VSSetShader(vs, nullptr, 0);
	m_context->PSSetShader(ps, nullptr, 0);

	safe_release(&vs);
	safe_release(&ps);

	m_blend_flags = blend_flags;
	SetRenderState(D3DRS_CULLMODE, CULLMODE);
	SetRenderState(D3DRS_ZENABLE, ZENABLE);
	update();

	for (auto& stream : m_stream_sources)
	{
		SetStreamSource(stream.first, stream.second.buffer, stream.second.stride);
	}

	ComPtr<Direct3DIndexBuffer8> index_buffer = std::move(m_current_index_buffer);
	SetIndices(index_buffer.Get(), m_current_base_vertex_index);
}

void Direct3DDevice8::oit_start()
{
	if (!oit_enabled && m_oit_actually_enabled != oit_enabled)
	{
		m_oit_actually_enabled = oit_enabled;
		oit_write();
	}

	m_oit_actually_enabled = oit_enabled;

	if (m_oit_actually_enabled)
	{
		// Restore R/W access to the UAV buffers from the shader.
		oit_write();
	}
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	{
		ComPtr<Direct3DSurface8> rt_surface;
		m_render_target_wrapper->GetSurfaceLevel(0, &rt_surface);

		ComPtr<Direct3DSurface8> bb_surface;
		m_back_buffer->GetSurfaceLevel(0, &bb_surface);

		CopyRects(rt_surface.Get(), nullptr, 0, bb_surface.Get(), nullptr);
	}

	print_info_queue();
	UNREFERENCED_PARAMETER(pDirtyRegion);

	auto interval = m_present_params.FullScreen_PresentationInterval;

	if (interval == D3DPRESENT_INTERVAL_IMMEDIATE)
	{
		interval = 0;
	}

	oit_composite();

	m_per_model.draw_call = 0;

	try
	{
		if (FAILED(m_swap_chain->Present(interval, 0)))
		{
			return D3DERR_INVALIDCALL;
		}
	}
	catch (std::exception&)
	{
	}

	oit_start();

	const auto vk_shift   = GetAsyncKeyState(VK_SHIFT) & (1 << 16);
	const auto vk_control = GetAsyncKeyState(VK_CONTROL) & (1 << 16);
	const auto vk_o       = GetAsyncKeyState('O') & 1;
	const auto vk_r       = GetAsyncKeyState('R') & 1;

	if (vk_control && vk_o)
	{
		if (m_oit_actually_enabled == oit_enabled)
		{
			oit_enabled = !oit_enabled;
			OutputDebugStringA(oit_enabled ? "OIT enabled\n" : "OIT disabled\n");
			d3d8to11::config->get_oit_config().enabled = oit_enabled; // this is ugly :(
		}
	}

	if (vk_control && vk_r)
	{
		if (!this->m_freeing_shaders)
		{
			OutputDebugStringA("clearing cached shaders...\n");

			if (vk_shift)
			{
				oit_load_shaders();
			}
			else
			{
				free_shaders();
				oit_load_shaders();
			}

			update();
			this->m_freeing_shaders = true;
		}
	}
	else
	{
		this->m_freeing_shaders = false;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, Direct3DSurface8** ppBackBuffer)
{
	if (ppBackBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if (iBackBuffer) // TODO: actual backbuffer, actual swapchain
	{
		return D3DERR_INVALIDCALL; // HACK
	}

	auto& bb = m_render_target_wrapper;

	ComPtr<Direct3DSurface8> surface;
	bb->GetSurfaceLevel(0, &surface);

	*ppBackBuffer = surface.Get();
	(*ppBackBuffer)->AddRef();

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRasterStatus(D3DRASTER_STATUS* pRasterStatus)
{
	NOT_IMPLEMENTED_RETURN;
}

void STDMETHODCALLTYPE Direct3DDevice8::SetGammaRamp(DWORD Flags, const D3DGAMMARAMP* pRamp)
{
	NOT_IMPLEMENTED;
}

void STDMETHODCALLTYPE Direct3DDevice8::GetGammaRamp(D3DGAMMARAMP* pRamp)
{
	NOT_IMPLEMENTED;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DTexture8** ppTexture)
{
	if (ppTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppTexture = nullptr;

	if (Pool == D3DPOOL_DEFAULT)
	{
	#if 0
		D3DDEVICE_CREATION_PARAMETERS CreationParams;
		ProxyInterface->GetCreationParameters(&CreationParams);

		if ((Usage & D3DUSAGE_DYNAMIC) == 0 &&
			SUCCEEDED(d3d->GetProxyInterface()->CheckDeviceFormat(CreationParams.AdapterOrdinal, CreationParams.DeviceType, D3DFMT_X8R8G8B8, D3DUSAGE_RENDERTARGET, D3DRTYPE_TEXTURE, Format)))
		{
			Usage |= D3DUSAGE_RENDERTARGET;
		}
		else
	#endif
		{
			Usage |= D3DUSAGE_DYNAMIC;
		}
	}

	auto result = new Direct3DTexture8(this, Width, Height, Levels, Usage, Format, Pool);
	result->AddRef();

	try
	{
		result->create_native();
		*ppTexture = result;
	}
	catch (std::exception& ex)
	{
		delete result;

		const std::string str = std::format("{} {}\n", __FUNCTION__, ex.what());
		OutputDebugStringA(str.c_str());

		print_info_queue();
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DVolumeTexture8** ppVolumeTexture)
{
#if 1
	NOT_IMPLEMENTED_RETURN;
#else
	if (ppVolumeTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppVolumeTexture = nullptr;

	IDirect3DVolumeTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, &TextureInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppVolumeTexture = new Direct3DVolumeTexture8(this, TextureInterface);
	(*ppVolumeTexture)->AddRef();

	return D3D_OK;
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DCubeTexture8** ppCubeTexture)
{
#if 1
	NOT_IMPLEMENTED_RETURN;
#else
	if (ppCubeTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppCubeTexture = nullptr;

	IDirect3DCubeTexture9 *TextureInterface = nullptr;

	const HRESULT hr = ProxyInterface->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, &TextureInterface, nullptr);

	if (FAILED(hr))
	{
		return hr;
	}

	*ppCubeTexture = new Direct3DCubeTexture8(this, TextureInterface);
	(*ppCubeTexture)->AddRef();

	return D3D_OK;
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, Direct3DVertexBuffer8** ppVertexBuffer)
{
	if (ppVertexBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if ((FVF & D3DFVF_XYZRHW) && D3DFVF_XYZRHW != (FVF & (D3DFVF_XYZRHW | D3DFVF_XYZ | D3DFVF_NORMAL)))
	{
		return D3DERR_INVALIDCALL;
	}

	if (Usage & D3DUSAGE_DYNAMIC)
	{
		if (Pool != D3DPOOL_DEFAULT)
		{
			return D3DERR_INVALIDCALL;
		}
	}

	*ppVertexBuffer = nullptr;
	auto result = new Direct3DVertexBuffer8(this, Length, Usage, FVF, Pool);
	result->AddRef();

	try
	{
		result->create_native();
		*ppVertexBuffer = result;
	}
	catch (std::exception& ex)
	{
		delete result;

		const std::string str = std::format("{} {}\n", __FUNCTION__, ex.what());
		OutputDebugStringA(str.c_str());

		print_info_queue();
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DIndexBuffer8** ppIndexBuffer)
{
	if (ppIndexBuffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Usage & D3DUSAGE_DYNAMIC)
	{
		if (Pool != D3DPOOL_DEFAULT)
		{
			return D3DERR_INVALIDCALL;
		}
	}

	if (Format != D3DFMT_INDEX16 && Format != D3DFMT_INDEX32)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppIndexBuffer = nullptr;
	auto result = new Direct3DIndexBuffer8(this, Length, Usage, Format, Pool);
	result->AddRef();

	try
	{
		result->create_native();
		*ppIndexBuffer = result;
	}
	catch (std::exception& ex)
	{
		delete result;

		const std::string str = std::format("{} {}\n", __FUNCTION__, ex.what());
		OutputDebugStringA(str.c_str());

		print_info_queue();
		return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, Direct3DSurface8** ppSurface)
{
	// TODO: high priority: Direct3DDevice8::CreateRenderTarget
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, Direct3DSurface8** ppSurface)
{
	// TODO: high priority: Direct3DDevice8::CreateDepthStencilSurface
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, Direct3DSurface8** ppSurface)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CopyRects(Direct3DSurface8* pSourceSurface, const RECT* pSourceRectsArray, UINT cRects,
                                                     Direct3DSurface8* pDestinationSurface, const POINT* pDestPointsArray)
{
	print_info_queue();

	if (!pSourceSurface || !pDestinationSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	// HACK: all so I can avoid writing the code to copy subregions; they're so rarely used anyway
	// fixes this call in Phantasy Star Online: Blue Burst
	if (pSourceRectsArray && cRects == 1)
	{
		if (pSourceRectsArray->top || pSourceRectsArray->left)
		{
			return D3DERR_INVALIDCALL;
		}

		if (pSourceSurface->get_d3d8_desc().Width  != static_cast<uint32_t>(pSourceRectsArray->right) ||
		    pSourceSurface->get_d3d8_desc().Height != static_cast<uint32_t>(pSourceRectsArray->bottom))
		{
			return D3DERR_INVALIDCALL;
		}

		if (pDestPointsArray && (pDestPointsArray->x != 0 || pDestPointsArray->y != 0))
		{
			return D3DERR_INVALIDCALL;
		}
	}
	else if (pSourceRectsArray || cRects || pDestPointsArray) // TODO
	{
		return D3DERR_INVALIDCALL;
	}

	UINT src_index = 0;
	UINT dst_index = 0;
	ComPtr<ID3D11Resource> src, dst;

	// TODO: MAKE NOT SHIT

	if (pSourceSurface->get_native_render_target())
	{
		pSourceSurface->get_native_render_target()->GetResource(&src);
	}
	else if (pSourceSurface->get_d3d8_parent())
	{
		src = pSourceSurface->get_d3d8_parent()->get_native_texture();
		src_index = pSourceSurface->get_d3d8_level();
	}

	if (pDestinationSurface->get_native_render_target())
	{
		pDestinationSurface->get_native_render_target()->GetResource(&dst);
	}
	else if (pDestinationSurface->get_d3d8_parent())
	{
		dst = pDestinationSurface->get_d3d8_parent()->get_native_texture();
		dst_index = pDestinationSurface->get_d3d8_level();
	}

	m_context->CopySubresourceRegion(dst.Get(), dst_index, 0, 0, 0, src.Get(), src_index, nullptr);
	print_info_queue();
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::UpdateTexture(Direct3DBaseTexture8* pSourceTexture, Direct3DBaseTexture8* pDestinationTexture)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetFrontBuffer(Direct3DSurface8* pDestSurface)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetRenderTarget(Direct3DSurface8* pRenderTarget, Direct3DSurface8* pNewZStencil)
{
	print_info_queue();

	ID3D11RenderTargetView* render_target = nullptr;
	ID3D11DepthStencilView* depth_stencil = nullptr;

	if (pRenderTarget != nullptr)
	{
		render_target = pRenderTarget->get_native_render_target();

		if (!render_target)
		{
			return D3DERR_INVALIDCALL;
		}

		m_current_render_target = pRenderTarget;

		D3DVIEWPORT8 viewport {};
		GetViewport(&viewport);

		viewport.Width  = pRenderTarget->get_d3d8_desc().Width;
		viewport.Height = pRenderTarget->get_d3d8_desc().Height;

		SetViewport(&viewport);
	}

	if (pNewZStencil != nullptr)
	{
		depth_stencil = pNewZStencil->get_native_depth_stencil();

		if (!depth_stencil)
		{
			return D3DERR_INVALIDCALL;
		}

		m_current_depth_stencil = pNewZStencil;
	}

	m_context->OMSetRenderTargets(1, &render_target, depth_stencil);
	print_info_queue();
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRenderTarget(Direct3DSurface8** ppRenderTarget)
{
	if (!ppRenderTarget)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppRenderTarget = m_current_render_target.Get();
	(*ppRenderTarget)->AddRef();
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetDepthStencilSurface(Direct3DSurface8** ppZStencilSurface)
{
	if (!ppZStencilSurface)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppZStencilSurface = m_current_depth_stencil.Get();
	(*ppZStencilSurface)->AddRef();
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::BeginScene()
{
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::EndScene()
{
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	if (Flags & D3DCLEAR_TARGET)
	{
		const float color[] = {
			static_cast<float>((Color >> 16) & 0xFF) / 255.0f,
			static_cast<float>((Color >> 8) & 0xFF) / 255.0f,
			static_cast<float>(Color & 0xFF) / 255.0f,
			static_cast<float>((Color >> 24) & 0xFF) / 255.0f,
		};

		if (m_current_render_target)
		{
			m_context->ClearRenderTargetView(m_current_render_target->get_native_render_target(), color);
		}
	}

	if (Flags & (D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL))
	{
		uint32_t flags = 0;

		if (Flags & D3DCLEAR_ZBUFFER)
		{
			flags |= D3D11_CLEAR_DEPTH;
		}

		if (Flags & D3DCLEAR_STENCIL)
		{
			flags |= D3D11_CLEAR_STENCIL;
		}

		if (m_current_depth_stencil)
		{
			m_context->ClearDepthStencilView(m_current_depth_stencil->get_native_depth_stencil(), flags, Z, static_cast<uint8_t>(Stencil));
		}
	}

	return D3D_OK;
}

void Direct3DDevice8::update_wv_inv_t()
{
	const matrix m = (m_per_model.world_matrix.data() * m_per_scene.view_matrix.data()).Invert();

	// don't need to transpose for reasons
	m_per_model.wv_matrix_inv_t = m;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTransform(D3DTRANSFORMSTATETYPE State, const matrix* pMatrix)
{
	if (!pMatrix)
	{
		return D3DERR_INVALIDCALL;
	}

	switch (static_cast<uint32_t>(State))
	{
		case D3DTS_VIEW:
		{
			m_per_scene.view_matrix = *pMatrix;

			const auto inverse = m_per_scene.view_matrix.data().Invert();
			m_per_scene.view_position = inverse.Translation();

			update_wv_inv_t();
			break;
		}

		case D3DTS_PROJECTION:
			m_per_scene.projection_matrix = *pMatrix;
			break;

		case D3DTS_TEXTURE0:
		case D3DTS_TEXTURE1:
		case D3DTS_TEXTURE2:
		case D3DTS_TEXTURE3:
		case D3DTS_TEXTURE4:
		case D3DTS_TEXTURE5:
		case D3DTS_TEXTURE6:
		case D3DTS_TEXTURE7:
			m_per_texture.stages[State - D3DTS_TEXTURE0].transform = *pMatrix;
			break;

		case D3DTS_WORLD:
			m_per_model.world_matrix = *pMatrix;
			update_wv_inv_t();
			break;

		default:
			return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTransform(D3DTRANSFORMSTATETYPE State, matrix* pMatrix)
{
	if (!pMatrix)
	{
		return D3DERR_INVALIDCALL;
	}

	switch (static_cast<uint32_t>(State))
	{
		case D3DTS_VIEW:
			*pMatrix = m_per_scene.view_matrix;
			break;

		case D3DTS_PROJECTION:
			*pMatrix = m_per_scene.projection_matrix;
			break;

		case D3DTS_TEXTURE0:
		case D3DTS_TEXTURE1:
		case D3DTS_TEXTURE2:
		case D3DTS_TEXTURE3:
		case D3DTS_TEXTURE4:
		case D3DTS_TEXTURE5:
		case D3DTS_TEXTURE6:
		case D3DTS_TEXTURE7:
			*pMatrix = m_per_texture.stages[State - D3DTS_TEXTURE0].transform;
			break;

		case D3DTS_WORLD:
			*pMatrix = m_per_model.world_matrix;
			break;

		default:
			return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::MultiplyTransform(D3DTRANSFORMSTATETYPE State, const matrix* pMatrix)
{
	if (!pMatrix)
	{
		return D3DERR_INVALIDCALL;
	}

	switch (static_cast<uint32_t>(State))
	{
		case D3DTS_VIEW:
			m_per_scene.view_matrix = m_per_scene.view_matrix * *pMatrix;
			break;

		case D3DTS_PROJECTION:
			m_per_scene.projection_matrix = m_per_scene.projection_matrix * *pMatrix;
			break;

		case D3DTS_TEXTURE0:
		case D3DTS_TEXTURE1:
		case D3DTS_TEXTURE2:
		case D3DTS_TEXTURE3:
		case D3DTS_TEXTURE4:
		case D3DTS_TEXTURE5:
		case D3DTS_TEXTURE6:
		case D3DTS_TEXTURE7:
		{
			auto& t = m_per_texture.stages[State - D3DTS_TEXTURE0].transform;
			t = t * *pMatrix;
			break;
		}

		case D3DTS_WORLD:
			m_per_model.world_matrix = m_per_model.world_matrix * *pMatrix;
			break;

		default:
			return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetViewport(const D3DVIEWPORT8* pViewport)
{
	if (!pViewport)
	{
		return D3DERR_INVALIDCALL;
	}

	m_viewport.Width    = static_cast<float>(pViewport->Width);
	m_viewport.Height   = static_cast<float>(pViewport->Height);
	m_viewport.MaxDepth = pViewport->MaxZ;
	m_viewport.MinDepth = pViewport->MinZ;
	m_viewport.TopLeftX = static_cast<float>(pViewport->X);
	m_viewport.TopLeftY = static_cast<float>(pViewport->Y);

	m_context->RSSetViewports(1, &m_viewport);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetViewport(D3DVIEWPORT8* pViewport)
{
	if (!pViewport)
	{
		return D3DERR_INVALIDCALL;
	}

	pViewport->Width  = static_cast<DWORD>(m_viewport.Width);
	pViewport->Height = static_cast<DWORD>(m_viewport.Height);
	pViewport->MaxZ   = m_viewport.MaxDepth;
	pViewport->MinZ   = m_viewport.MinDepth;
	pViewport->X      = static_cast<DWORD>(m_viewport.TopLeftX);
	pViewport->Y      = static_cast<DWORD>(m_viewport.TopLeftY);

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetMaterial(const D3DMATERIAL8* pMaterial)
{
	if (!pMaterial)
	{
		return D3DERR_INVALIDCALL;
	}

	m_material = *pMaterial;
	m_per_model.material = Material(m_material);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetMaterial(D3DMATERIAL8* pMaterial)
{
	if (!pMaterial)
	{
		return D3DERR_INVALIDCALL;
	}

	*pMaterial = m_material;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetLight(DWORD Index, const D3DLIGHT8* pLight)
{
	if (!pLight)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Index >= m_per_model.lights.size())
	{
		return D3DERR_INVALIDCALL;
	}

	Light light = m_per_model.lights[Index].data();
	light.copy(*pLight);
	m_per_model.lights[Index] = light;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetLight(DWORD Index, D3DLIGHT8* pLight)
{
	if (!pLight)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Index >= m_per_model.lights.size())
	{
		return D3DERR_INVALIDCALL;
	}

	const auto& light = m_per_model.lights[Index].data();

	pLight->Type         = static_cast<D3DLIGHTTYPE>(light.type);
	pLight->Diffuse      = { light.diffuse.x, light.diffuse.y, light.diffuse.z, light.diffuse.w };
	pLight->Specular     = { light.diffuse.x, light.diffuse.y, light.diffuse.z, light.diffuse.w };
	pLight->Ambient      = { light.ambient.x, light.ambient.y, light.ambient.z, light.ambient.w };
	pLight->Position     = { light.position.x, light.position.y, light.position.z };
	pLight->Direction    = { light.direction.x, light.direction.y, light.direction.z };
	pLight->Range        = light.range;
	pLight->Falloff      = light.falloff;
	pLight->Attenuation0 = light.attenuation0;
	pLight->Attenuation1 = light.attenuation1;
	pLight->Attenuation2 = light.attenuation2;
	pLight->Theta        = light.theta;
	pLight->Phi          = light.phi;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::LightEnable(DWORD Index, BOOL Enable)
{
	if (Index >= m_per_model.lights.size())
	{
		return D3DERR_INVALIDCALL;
	}

	Light light = m_per_model.lights[Index].data();
	light.enabled = Enable == TRUE;
	m_per_model.lights[Index] = light;

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	if (!pEnable)
	{
		return D3DERR_INVALIDCALL;
	}

	if (Index >= m_per_model.lights.size())
	{
		return D3DERR_INVALIDCALL;
	}

	*pEnable = m_per_model.lights[Index].data().enabled;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetClipPlane(DWORD Index, const float* pPlane)
{
	if (pPlane == nullptr || Index >= MAX_CLIP_PLANES)
	{
		return D3DERR_INVALIDCALL;
	}

	memcpy(m_stored_clip_planes[Index], pPlane, sizeof(m_stored_clip_planes[0]));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetClipPlane(DWORD Index, float* pPlane)
{
	if (pPlane == nullptr || Index >= MAX_CLIP_PLANES)
	{
		return D3DERR_INVALIDCALL;
	}

	memcpy(pPlane, m_stored_clip_planes[Index], sizeof(m_stored_clip_planes[0]));
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	switch (static_cast<DWORD>(State))
	{
		case D3DRS_LINEPATTERN:
		case D3DRS_ZVISIBLE:
		case D3DRS_EDGEANTIALIAS:
		case D3DRS_PATCHSEGMENTS:
		case D3DRS_CLIPPLANEENABLE:
		case D3DRS_ZBIAS:
			return D3DERR_INVALIDCALL;

		case D3DRS_SOFTWAREVERTEXPROCESSING:
			return D3D_OK;

		default:
			break;
	}

	if (State < D3DRS_ZENABLE || State > D3DRS_NORMALORDER)
	{
		return D3DERR_INVALIDCALL;
	}

	// even if we do custom handling for a render state, we
	// store its value so the caller can retrieve it later in
	// Direct3DDevice8::GetRenderState
	auto& ref = m_render_state_values[State];

	auto set_stencil_flags = [&](auto shift)
	{
		auto flags = m_depth_stencil_flags.stencil_flags.data();
		flags &= ~(StencilFlags::op_mask << shift);
		flags |= (Value & StencilFlags::op_mask) << shift;
		m_depth_stencil_flags.stencil_flags = flags;
	};

	auto set_stencil_rw = [&](auto shift)
	{
		auto flags = m_depth_stencil_flags.stencil_flags.data();
		flags &= ~(StencilFlags::rw_mask << shift);
		flags |= (Value & StencilFlags::rw_mask) << shift;
		m_depth_stencil_flags.stencil_flags = flags;
	};

	switch (State)
	{
		default:
		{
			ref = Value;

			if (!ref.dirty())
			{
				break;
			}

			ref.clear();
			const auto it = RS_STRINGS.find(State);

			if (it == RS_STRINGS.end())
			{
				break;
			}

			const std::string str = std::format("{} unhandled render state type: {}; value: {}\n",
			                                    __FUNCTION__, it->second, Value);

			OutputDebugStringA(str.c_str());
			break;
		}

		case D3DRS_COLORWRITEENABLE:
		{
			m_blend_flags = (m_blend_flags.data() & ~(0xF << BLEND_COLORMASK_SHIFT)) | ((Value & 0xF) << BLEND_COLORMASK_SHIFT);
			ref = Value;
			break;
		}

		case D3DRS_TEXTUREFACTOR:
			m_per_pixel.texture_factor = to_color4(Value);
			ref = Value;
			break;

		case D3DRS_FOGSTART:
			m_per_pixel.fog_start = *reinterpret_cast<float*>(&Value);
			ref = Value;
			break;

		case D3DRS_FOGEND:
			m_per_pixel.fog_end = *reinterpret_cast<float*>(&Value);
			ref = Value;
			break;

		case D3DRS_FOGCOLOR:
			m_per_pixel.fog_color = to_color4(Value);
			ref = Value;
			break;

		case D3DRS_FOGTABLEMODE:
			m_shader_flags &= ~ShaderFlags::rs_fog_mode_mask;
			m_shader_flags |= (static_cast<ShaderFlags::type>(Value) << ShaderFlags::rs_fog_mode_shift) & ShaderFlags::rs_fog_mode_mask;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_FOGDENSITY:
			m_per_pixel.fog_density = *reinterpret_cast<float*>(&Value);
			ref = Value;
			break;

		case D3DRS_SPECULARENABLE:
			if (Value != 0)
			{
				m_shader_flags |= ShaderFlags::rs_specular;
			}
			else
			{
				m_shader_flags &= ~ShaderFlags::rs_specular;
			}

			ref = Value;
			ref.clear();
			break;

		case D3DRS_LIGHTING:
			if (Value == 1)
			{
				m_shader_flags |= ShaderFlags::rs_lighting;
			}
			else
			{
				m_shader_flags &= ~ShaderFlags::rs_lighting;
			}

			ref = Value;
			ref.clear();
			break;

		case D3DRS_FOGENABLE:
			if (Value != 0)
			{
				m_shader_flags |= ShaderFlags::rs_fog;
			}
			else
			{
				m_shader_flags &= ~ShaderFlags::rs_fog;
			}

			ref = Value;
			ref.clear();
			break;

		case D3DRS_ALPHATESTENABLE:
		{
			if (Value != 0)
			{
				m_shader_flags |= ShaderFlags::rs_alpha_test;
			}
			else
			{
				m_shader_flags &= ~ShaderFlags::rs_alpha_test;
			}

			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_ALPHAFUNC:
		{
			if (!Value)
			{
				return D3DERR_INVALIDCALL;
			}

			m_shader_flags &= ~ShaderFlags::rs_alpha_test_mode_mask;
			m_shader_flags |= (static_cast<ShaderFlags::type>(Value) << ShaderFlags::rs_alpha_test_mode_shift) & ShaderFlags::rs_alpha_test_mode_mask;

			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_ALPHAREF:
		{
			m_per_pixel.alpha_test_reference = static_cast<float>(Value) / 255.0f;
			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_CULLMODE:
		{
			m_raster_flags = (m_raster_flags.data() & ~RasterFlags::cull_mask) | (Value & 3);
			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_FILLMODE:
		{
			m_raster_flags = (m_raster_flags.data() & ~RasterFlags::fill_mask) | ((Value & 3) << 2);
			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_ZENABLE:
		{
			if (Value)
			{
				m_depth_stencil_flags.flags = m_depth_stencil_flags.flags.data() | DepthStencilFlags::depth_test_enabled;
			}
			else
			{
				m_depth_stencil_flags.flags = m_depth_stencil_flags.flags.data() & ~DepthStencilFlags::depth_test_enabled;
			}

			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_ZFUNC:
		{
			if (!Value)
			{
				return D3DERR_INVALIDCALL;
			}

			m_depth_stencil_flags.depth_flags = (m_depth_stencil_flags.depth_flags.data() & ~DepthFlags::comparison_mask) | Value;
			ref = Value;
			ref.clear();
			break;
		}

		case D3DRS_ZWRITEENABLE:
			if (Value)
			{
				m_depth_stencil_flags.flags = m_depth_stencil_flags.flags.data() | DepthStencilFlags::depth_write_enabled;
			}
			else
			{
				m_depth_stencil_flags.flags = m_depth_stencil_flags.flags.data() & ~DepthStencilFlags::depth_write_enabled;
			}

			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILENABLE:
			if (Value)
			{
				m_depth_stencil_flags.flags = m_depth_stencil_flags.flags.data() | DepthStencilFlags::stencil_enabled;
			}
			else
			{
				m_depth_stencil_flags.flags = m_depth_stencil_flags.flags.data() & ~DepthStencilFlags::stencil_enabled;
			}

			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILFAIL:
			set_stencil_flags(StencilFlags::fail_shift);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILZFAIL:
			set_stencil_flags(StencilFlags::zfail_shift);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILPASS:
			set_stencil_flags(StencilFlags::pass_shift);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILFUNC:
			set_stencil_flags(StencilFlags::func_shift);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILREF:
			ref = Value;
			break;

		case D3DRS_STENCILMASK:
			set_stencil_rw(StencilFlags::read_shift);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_STENCILWRITEMASK:
			set_stencil_rw(StencilFlags::write_shift);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_AMBIENT:
			m_per_model.ambient = to_color4(Value);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_DIFFUSEMATERIALSOURCE:
			m_per_model.material_sources.diffuse = Value;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_SPECULARMATERIALSOURCE:
			m_per_model.material_sources.specular = Value;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_AMBIENTMATERIALSOURCE:
			m_per_model.material_sources.ambient = Value;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_EMISSIVEMATERIALSOURCE:
			m_per_model.material_sources.emissive = Value;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_COLORVERTEX:
			m_per_model.color_vertex = !!Value;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_SRCBLEND:
			m_per_pixel.src_blend = Value;
			m_blend_flags = (m_blend_flags.data() & ~0x0F) | Value;
			ref = Value;
			ref.clear();
			break;

		case D3DRS_DESTBLEND:
			m_per_pixel.dst_blend = Value;
			m_blend_flags = (m_blend_flags.data() & ~0xF0) | (Value << 4);
			ref = Value;
			ref.clear();
			break;

		case D3DRS_ALPHABLENDENABLE:
			m_blend_flags = (m_blend_flags.data() & ~0x8000) | (Value ? 0x8000 : 0);

			if (Value != 1)
			{
				m_shader_flags &= ~ShaderFlags::rs_alpha;
			}
			else
			{
				m_shader_flags |= ShaderFlags::rs_alpha;
			}

			ref = Value;
			ref.clear();
			break;

		case D3DRS_BLENDOP:
			m_blend_flags = (m_blend_flags.data() & ~0xF00) | (Value << 8);
			m_per_pixel.blend_op = Value;

			ref = Value;
			ref.clear();
			break;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	if (pValue == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	switch (static_cast<DWORD>(State))
	{
		case D3DRS_LINEPATTERN:
		case D3DRS_ZVISIBLE:
		case D3DRS_EDGEANTIALIAS:
		case D3DRS_PATCHSEGMENTS:
		case D3DRS_CLIPPLANEENABLE:
		case D3DRS_ZBIAS:
			return D3DERR_INVALIDCALL;

		case D3DRS_SOFTWAREVERTEXPROCESSING:
			*pValue = 0;
			return D3D_OK;

		default:
			break;
	}

	if (State < D3DRS_ZENABLE || State > D3DRS_NORMALORDER)
	{
		return D3DERR_INVALIDCALL;
	}

	*pValue = m_render_state_values[State];
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::BeginStateBlock()
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::EndStateBlock(DWORD* pToken)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::ApplyStateBlock(DWORD Token)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CaptureStateBlock(DWORD Token)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeleteStateBlock(DWORD Token)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD* pToken)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetClipStatus(const D3DCLIPSTATUS8* pClipStatus)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetClipStatus(D3DCLIPSTATUS8* pClipStatus)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTexture(DWORD Stage, Direct3DBaseTexture8** ppTexture)
{
	if (ppTexture == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppTexture = nullptr;

	const auto it = m_textures.find(Stage);

	if (it == m_textures.end())
	{
		return D3DERR_INVALIDCALL;
	}

	auto texture = it->second;
	texture->AddRef();
	*ppTexture = it->second;
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTexture(DWORD Stage, Direct3DBaseTexture8* pTexture)
{
	auto it = m_textures.find(Stage);

	if (pTexture == nullptr)
	{
		m_per_texture.stages[Stage].bound = false;

		ID3D11ShaderResourceView* dummy[1] {};
		m_context->PSSetShaderResources(Stage, 1, &dummy[0]);

		if (it != m_textures.end())
		{
			safe_release(&it->second);
			m_textures.erase(it);
		}

		return D3D_OK;
	}

	switch (pTexture->GetType())
	{
		case D3DRTYPE_TEXTURE:
			break;
		default:
			return D3DERR_INVALIDCALL;
	}

	auto texture = dynamic_cast<Direct3DTexture8*>(pTexture);

	if (it != m_textures.end())
	{
		safe_release(&it->second);
		it->second = texture;
		texture->AddRef();
	}
	else
	{
		m_textures[Stage] = texture;
		texture->AddRef();
	}

	m_per_texture.stages[Stage].bound = true;
	ID3D11ShaderResourceView* texture_srv = texture->get_native_srv();
	m_context->PSSetShaderResources(Stage, 1, &texture_srv);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	if (!pValue)
	{
		return D3DERR_INVALIDCALL;
	}

	switch (Type)
	{
		case D3DTSS_ADDRESSU:
			*pValue = m_sampler_setting_values[Stage].address_u.data();
			break;
		case D3DTSS_ADDRESSV:
			*pValue = m_sampler_setting_values[Stage].address_v.data();
			break;
		case D3DTSS_MAGFILTER:
			*pValue = m_sampler_setting_values[Stage].filter_mag.data();
			break;
		case D3DTSS_MINFILTER:
			*pValue = m_sampler_setting_values[Stage].filter_min.data();
			break;
		case D3DTSS_MIPFILTER:
			*pValue = m_sampler_setting_values[Stage].filter_mip.data();
			break;
		case D3DTSS_MIPMAPLODBIAS:
			*reinterpret_cast<float*>(pValue) = m_sampler_setting_values[Stage].mip_lod_bias.data();
			break;
		case D3DTSS_MAXMIPLEVEL:
			*pValue = m_sampler_setting_values[Stage].max_mip_level.data();
			break;
		case D3DTSS_MAXANISOTROPY:
			*pValue = m_sampler_setting_values[Stage].max_anisotropy.data();
			break;
		case D3DTSS_ADDRESSW:
			*pValue = m_sampler_setting_values[Stage].address_u.data();
			break;

		case D3DTSS_COLOROP:
			*pValue = m_per_texture.stages[Stage].color_op;
			break;
		case D3DTSS_COLORARG1:
			*pValue = m_per_texture.stages[Stage].color_arg1;
			break;
		case D3DTSS_COLORARG2:
			*pValue = m_per_texture.stages[Stage].color_arg2;
			break;
		case D3DTSS_ALPHAOP:
			*pValue = m_per_texture.stages[Stage].alpha_op;
			break;
		case D3DTSS_ALPHAARG1:
			*pValue = m_per_texture.stages[Stage].alpha_arg1;
			break;
		case D3DTSS_ALPHAARG2:
			*pValue = m_per_texture.stages[Stage].alpha_arg2;
			break;
		case D3DTSS_BUMPENVMAT00:
			*reinterpret_cast<float*>(pValue) = m_per_texture.stages[Stage].bump_env_mat00;
			break;
		case D3DTSS_BUMPENVMAT01:
			*reinterpret_cast<float*>(pValue) = m_per_texture.stages[Stage].bump_env_mat01;
			break;
		case D3DTSS_BUMPENVMAT10:
			*reinterpret_cast<float*>(pValue) = m_per_texture.stages[Stage].bump_env_mat10;
			break;
		case D3DTSS_BUMPENVMAT11:
			*reinterpret_cast<float*>(pValue) = m_per_texture.stages[Stage].bump_env_mat11;
			break;
		case D3DTSS_TEXCOORDINDEX:
			*pValue = m_per_texture.stages[Stage].tex_coord_index;
			break;
		// TODO: case D3DTSS_BORDERCOLOR:
		case D3DTSS_BUMPENVLSCALE:
			*reinterpret_cast<float*>(pValue) = m_per_texture.stages[Stage].bump_env_lscale;
			break;
		case D3DTSS_BUMPENVLOFFSET:
			*reinterpret_cast<float*>(pValue) = m_per_texture.stages[Stage].bump_env_loffset;
			break;
		case D3DTSS_TEXTURETRANSFORMFLAGS:
			*pValue = m_per_texture.stages[Stage].texture_transform_flags;
			break;
		case D3DTSS_COLORARG0:
			*pValue = m_per_texture.stages[Stage].color_arg0;
			break;
		case D3DTSS_ALPHAARG0:
			*pValue = m_per_texture.stages[Stage].alpha_arg0;
			break;
		case D3DTSS_RESULTARG:
			*pValue = m_per_texture.stages[Stage].result_arg;
			break;

		default:
			return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	switch (Type)
	{
		case D3DTSS_ADDRESSU:
			m_sampler_setting_values[Stage].address_u = static_cast<D3DTEXTUREADDRESS>(Value);
			break;

		case D3DTSS_ADDRESSV:
			m_sampler_setting_values[Stage].address_v = static_cast<D3DTEXTUREADDRESS>(Value);
			break;

		case D3DTSS_MAGFILTER:
			m_sampler_setting_values[Stage].filter_mag = static_cast<D3DTEXTUREFILTERTYPE>(Value);
			break;

		case D3DTSS_MINFILTER:
			m_sampler_setting_values[Stage].filter_min = static_cast<D3DTEXTUREFILTERTYPE>(Value);
			break;

		case D3DTSS_MIPFILTER:
			m_sampler_setting_values[Stage].filter_mip = static_cast<D3DTEXTUREFILTERTYPE>(Value);
			break;

		case D3DTSS_MIPMAPLODBIAS:
			m_sampler_setting_values[Stage].mip_lod_bias = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_MAXMIPLEVEL:
			m_sampler_setting_values[Stage].max_mip_level = Value;
			break;

		case D3DTSS_MAXANISOTROPY:
			m_sampler_setting_values[Stage].max_anisotropy = Value;
			break;

		case D3DTSS_ADDRESSW:
			m_sampler_setting_values[Stage].address_w = static_cast<D3DTEXTUREADDRESS>(Value);
			break;

		case D3DTSS_COLOROP:
			m_per_texture.stages[Stage].color_op = static_cast<D3DTEXTUREOP>(Value);

			switch (Value)
			{
				case D3DTOP_PREMODULATE:
				case D3DTOP_BUMPENVMAP:
				case D3DTOP_BUMPENVMAPLUMINANCE:
				case D3DTOP_DOTPRODUCT3:
					OutputDebugStringA("WARNING: Unsupported texture blending operation!\n");
					break;

				default:
					break;
			}
			break;

		case D3DTSS_COLORARG1:
			m_per_texture.stages[Stage].color_arg1 = Value;
			break;

		case D3DTSS_COLORARG2:
			m_per_texture.stages[Stage].color_arg2 = Value;
			break;

		case D3DTSS_ALPHAOP:
			m_per_texture.stages[Stage].alpha_op = static_cast<D3DTEXTUREOP>(Value);

			switch (Value)
			{
				case D3DTOP_PREMODULATE:
				case D3DTOP_BUMPENVMAP:
				case D3DTOP_BUMPENVMAPLUMINANCE:
				case D3DTOP_DOTPRODUCT3:
					OutputDebugStringA("WARNING: Unsupported texture blending operation!\n");
					break;

				default:
					break;
			}
			break;

		case D3DTSS_ALPHAARG1:
			m_per_texture.stages[Stage].alpha_arg1 = Value;
			break;

		case D3DTSS_ALPHAARG2:
			m_per_texture.stages[Stage].alpha_arg2 = Value;
			break;

		case D3DTSS_BUMPENVMAT00:
			m_per_texture.stages[Stage].bump_env_mat00 = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_BUMPENVMAT01:
			m_per_texture.stages[Stage].bump_env_mat01 = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_BUMPENVMAT10:
			m_per_texture.stages[Stage].bump_env_mat10 = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_BUMPENVMAT11:
			m_per_texture.stages[Stage].bump_env_mat11 = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_TEXCOORDINDEX:
			m_per_texture.stages[Stage].tex_coord_index = Value;
			break;

		case D3DTSS_BUMPENVLSCALE:
			m_per_texture.stages[Stage].bump_env_lscale = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_BUMPENVLOFFSET:
			m_per_texture.stages[Stage].bump_env_loffset = *reinterpret_cast<float*>(&Value);
			break;

		case D3DTSS_TEXTURETRANSFORMFLAGS:
			m_per_texture.stages[Stage].texture_transform_flags = static_cast<D3DTEXTURETRANSFORMFLAGS>(Value);
			break;

		case D3DTSS_COLORARG0:
			m_per_texture.stages[Stage].color_arg0 = Value;
			break;

		case D3DTSS_ALPHAARG0:
			m_per_texture.stages[Stage].alpha_arg0 = Value;
			break;

		case D3DTSS_RESULTARG:
			if (Value != D3DTA_CURRENT && Value != D3DTA_TEMP)
			{
				return D3DERR_INVALIDCALL;
			}

			m_per_texture.stages[Stage].result_arg = Value;
			break;

		default:
			return D3DERR_INVALIDCALL;
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::ValidateDevice(DWORD* pNumPasses)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetInfo(DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize)
{
	UNREFERENCED_PARAMETER(DevInfoID);
	UNREFERENCED_PARAMETER(pDevInfoStruct);
	UNREFERENCED_PARAMETER(DevInfoStructSize);

	return S_FALSE;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY* pEntries)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetCurrentTexturePalette(UINT PaletteNumber)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetCurrentTexturePalette(UINT* pPaletteNumber)
{
	NOT_IMPLEMENTED_RETURN;
}

void Direct3DDevice8::run_draw_prologues(const std::string& callback)
{
#ifndef _DEBUG
	shader_preprocess(m_shader_flags, false, m_draw_prologue_epilogue_preproc); // FIXME: assuming non-uber

	for (auto& fn : draw_prologues[callback])
	{
		fn(m_draw_prologue_epilogue_preproc, m_shader_flags);
	}
#endif
}

void Direct3DDevice8::run_draw_epilogues(const std::string& callback)
{
#ifndef _DEBUG
	shader_preprocess(m_shader_flags, false, m_draw_prologue_epilogue_preproc); // FIXME: assuming non-uber

	for (auto& fn : draw_epilogues[callback])
	{
		fn(m_draw_prologue_epilogue_preproc, m_shader_flags);
	}
#endif
}

bool Direct3DDevice8::set_primitive_type(D3DPRIMITIVETYPE primitive_type) const
{
	const auto topology = to_d3d11(primitive_type);

	if (topology != D3D11_PRIMITIVE_TOPOLOGY_UNDEFINED)
	{
		m_context->IASetPrimitiveTopology(topology);
		return true;
	}

	return false;
}

bool Direct3DDevice8::primitive_vertex_count(D3DPRIMITIVETYPE primitive_type, uint32_t& count)
{
	switch (primitive_type)
	{
		case D3DPT_TRIANGLELIST:
			count *= 3;
			break;

		case D3DPT_TRIANGLESTRIP:
		case D3DPT_TRIANGLEFAN:
			count += 2;
			break;

		case D3DPT_POINTLIST:
			break;

		case D3DPT_LINELIST:
			count *= 2;
			break;

		case D3DPT_LINESTRIP:
			++count;
			break;

		default:
			return false;
	}

	return true;
}

void Direct3DDevice8::oit_zwrite_force(DWORD& ZWRITEENABLE, DWORD& ZENABLE)
{
	GetRenderState(D3DRS_ZWRITEENABLE, &ZWRITEENABLE);
	GetRenderState(D3DRS_ZENABLE, &ZENABLE);

	if (m_shader_flags & ShaderFlags::rs_alpha && (m_oit_actually_enabled && oit_enabled))
	{
		// force zwrite on to enable writing 100% opaque
		// pixels to the real backbuffer and depth buffer.
		SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		SetRenderState(D3DRS_ZENABLE, TRUE); // this does nothing right now, but good practice!

		update_depth();
	}
}

void Direct3DDevice8::oit_zwrite_restore(DWORD ZWRITEENABLE, DWORD ZENABLE)
{
	SetRenderState(D3DRS_ZWRITEENABLE, ZWRITEENABLE);
	SetRenderState(D3DRS_ZENABLE, ZENABLE);

	update_depth();
}

// the other draw function (UP) gets routed through here
HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	// convert triangle fan to indexed triangle list before rendering since D3D11 can't render fans
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		ComPtr<Direct3DIndexBuffer8> last_index_buffer;
		UINT last_index_base = 0;
		GetIndices(&last_index_buffer, &last_index_base);

		uint32_t fan_vertex_count = PrimitiveCount;
		primitive_vertex_count(PrimitiveType, fan_vertex_count);

		const size_t tri_list_index_count = 3 * PrimitiveCount;
		const size_t tri_list_index_buffer_size = tri_list_index_count * 4;

		auto up_index_buffer = get_user_primitive_index_buffer(tri_list_index_buffer_size, D3DFMT_INDEX32);

		if (up_index_buffer == nullptr)
		{
			return D3DERR_INVALIDCALL;
		}

		BYTE* raw_output_indices_ptr = nullptr;
		up_index_buffer->Lock(0, static_cast<UINT>(tri_list_index_buffer_size), &raw_output_indices_ptr, D3DLOCK_DISCARD);

		auto tri_list_indices = std::span(reinterpret_cast<uint32_t*>(raw_output_indices_ptr), tri_list_index_count);

		{
			UINT n = 1;
			for (size_t i = 0; i < PrimitiveCount; ++i)
			{
				const size_t o = 3 * i;

				tri_list_indices[o + 0] = StartVertex;
				tri_list_indices[o + 1] = StartVertex + n++;
				tri_list_indices[o + 2] = StartVertex + n;
			}
		}

		up_index_buffer->Unlock();
		SetIndices(up_index_buffer.Get(), 0);

		const auto result = DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
		                                         0,
		                                         static_cast<UINT>(tri_list_index_count),
		                                         0,
		                                         PrimitiveCount);

		SetIndices(last_index_buffer.Get(), last_index_base);

		m_up_index_buffers.emplace_back(std::move(up_index_buffer));

		return result;
	}

	if (!set_primitive_type(PrimitiveType))
	{
		return D3DERR_INVALIDCALL;
	}

	draw_call_increment();
	if (!update())
	{
		return D3DERR_INVALIDCALL;
	}

	if (skip_draw())
	{
		//OutputDebugStringA("WARNING: SKIPPING DRAW CALL\n");
		return D3D_OK;
	}

	uint32_t count = PrimitiveCount;

	if (!primitive_vertex_count(PrimitiveType, count))
	{
		return D3DERR_INVALIDCALL;
	}

	DWORD ZWRITEENABLE, ZENABLE;
	oit_zwrite_force(ZWRITEENABLE, ZENABLE);

	run_draw_prologues(__FUNCTION__);
	m_context->Draw(count, StartVertex);
	run_draw_epilogues(__FUNCTION__);

	oit_zwrite_restore(ZWRITEENABLE, ZENABLE);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount)
{
	if (!m_current_index_buffer)
	{
		return D3DERR_INVALIDCALL;
	}

	// TODO: convert triangle fan to triangle list before rendering since D3D11 can't render fans? details below
	// we have to be able to reference the contents of the index buffer so that
	// it can be (temporarily) converted to a triangle list index buffer. but
	// with the vertex/index buffer rewrite, we no longer have a copy in system
	// memory like in the past. that will need to be reimplemented :/
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		// hopefully not too spammy, but this case doesn't seem common
		NOT_IMPLEMENTED;
		// let's just pretend everything is fine
		return D3D_OK;
	}

	if (!set_primitive_type(PrimitiveType))
	{
		return D3DERR_INVALIDCALL;
	}

	draw_call_increment();
	if (!update())
	{
		return D3DERR_INVALIDCALL;
	}

	if (skip_draw())
	{
		//OutputDebugStringA("WARNING: SKIPPING DRAW CALL\n");
		return D3D_OK;
	}

	auto count = PrimitiveCount;
	primitive_vertex_count(PrimitiveType, count);

	DWORD ZWRITEENABLE, ZENABLE;
	oit_zwrite_force(ZWRITEENABLE, ZENABLE);

	run_draw_prologues(__FUNCTION__);
	m_context->DrawIndexed(count, StartIndex, m_current_base_vertex_index);
	run_draw_epilogues(__FUNCTION__);

	oit_zwrite_restore(ZWRITEENABLE, ZENABLE);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	if (!pVertexStreamZeroData)
	{
		return D3DERR_INVALIDCALL;
	}

	// D3D11 can't render triangle fans natively, so we'll forward this draw call to
	// DrawIndexedPrimitiveUP and let it handle conversion to an indexed triangle list.
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		ComPtr<Direct3DIndexBuffer8> last_index_buffer;
		UINT last_index_base = 0;
		GetIndices(&last_index_buffer, &last_index_base);

		uint32_t vertex_count = 0;
		primitive_vertex_count(PrimitiveType, vertex_count);

		m_trifan_index_buffer.resize(vertex_count);

		for (uint32_t i = 0; i < vertex_count; ++i)
		{
			m_trifan_index_buffer[i] = i;
		}

		const auto result = DrawIndexedPrimitiveUP(PrimitiveType,
		                                           0,
		                                           vertex_count,
		                                           PrimitiveCount,
		                                           m_trifan_index_buffer.data(),
		                                           D3DFMT_INDEX32,
		                                           pVertexStreamZeroData,
		                                           VertexStreamZeroStride);

		SetIndices(last_index_buffer.Get(), last_index_base);

		return result;
	}

	if (!set_primitive_type(PrimitiveType))
	{
		return D3DERR_INVALIDCALL;
	}

	uint32_t count = PrimitiveCount;

	if (!primitive_vertex_count(PrimitiveType, count))
	{
		return D3DERR_INVALIDCALL;
	}

	draw_call_increment();
	if (!update())
	{
		return D3DERR_INVALIDCALL;
	}

	if (skip_draw())
	{
		//OutputDebugStringA("WARNING: SKIPPING DRAW CALL\n");
		return D3D_OK;
	}

	const auto size = count * VertexStreamZeroStride;

	auto up_vertex_buffer = get_user_primitive_vertex_buffer(size);

	if (up_vertex_buffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	BYTE* ptr;
	up_vertex_buffer->Lock(0, size, &ptr, D3DLOCK_DISCARD);

	memcpy(ptr, pVertexStreamZeroData, size);

	up_vertex_buffer->Unlock();

	SetStreamSource(0, up_vertex_buffer.Get(), VertexStreamZeroStride);

	run_draw_prologues(__FUNCTION__);
	const auto result = DrawPrimitive(PrimitiveType, 0, PrimitiveCount);
	run_draw_epilogues(__FUNCTION__);

	SetStreamSource(0, nullptr, 0);
	m_up_vertex_buffers.emplace_back(std::move(up_vertex_buffer));

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	if (!pVertexStreamZeroData)
	{
		return D3DERR_INVALIDCALL;
	}

	if (!pIndexData)
	{
		return D3DERR_INVALIDCALL;
	}

	if (IndexDataFormat != D3DFMT_INDEX16 && IndexDataFormat != D3DFMT_INDEX32)
	{
		return D3DERR_INVALIDCALL;
	}

	ComPtr<Direct3DIndexBuffer8> up_index_buffer;
	const size_t index_size = IndexDataFormat == D3DFMT_INDEX16 ? sizeof(uint16_t) : sizeof(uint32_t);

	// convert triangle fan to triangle list before rendering since D3D11 can't render fans
	if (PrimitiveType == D3DPT_TRIANGLEFAN)
	{
		const size_t tri_list_index_count = 3 * PrimitiveCount;
		const size_t tri_list_index_buffer_size = tri_list_index_count * index_size;

		up_index_buffer = get_user_primitive_index_buffer(tri_list_index_buffer_size, IndexDataFormat);

		if (up_index_buffer == nullptr)
		{
			return D3DERR_INVALIDCALL;
		}

		BYTE* raw_output_indices_ptr = nullptr;
		up_index_buffer->Lock(0, static_cast<UINT>(tri_list_index_buffer_size), &raw_output_indices_ptr, D3DLOCK_DISCARD);

		// copy/pasted code below to handle 16-bit and 32-bit indices because I was too lazy to make a template function
		if (IndexDataFormat == D3DFMT_INDEX16)
		{
			const auto* index_0 = static_cast<const uint16_t*>(pIndexData);
			const auto* input_indices = index_0 + 1;
			auto tri_list_indices = std::span(reinterpret_cast<uint16_t*>(raw_output_indices_ptr), tri_list_index_count);

			for (size_t i = 0; i < PrimitiveCount; ++i)
			{
				const size_t o = 3 * i;

				tri_list_indices[o + 0] = *index_0;
				tri_list_indices[o + 1] = *input_indices;

				++input_indices;
				tri_list_indices[o + 2] = *input_indices;
			}
		}
		else
		{
			const auto* index_0 = static_cast<const uint32_t*>(pIndexData);
			const auto* input_indices = index_0 + 1;
			auto tri_list_indices = std::span(reinterpret_cast<uint32_t*>(raw_output_indices_ptr), tri_list_index_count);

			for (size_t i = 0; i < PrimitiveCount; ++i)
			{
				const size_t o = 3 * i;

				tri_list_indices[o + 0] = *index_0;
				tri_list_indices[o + 1] = *input_indices;

				++input_indices;
				tri_list_indices[o + 2] = *input_indices;
			}
		}

		up_index_buffer->Unlock();

		PrimitiveType = D3DPT_TRIANGLELIST;
	}
	else
	{
		uint32_t vertex_count = PrimitiveCount;
		primitive_vertex_count(PrimitiveType, vertex_count);

		const size_t index_buffer_size = index_size * vertex_count;

		up_index_buffer = get_user_primitive_index_buffer(index_buffer_size, IndexDataFormat);

		BYTE* raw_output_indices_ptr = nullptr;
		up_index_buffer->Lock(0, static_cast<UINT>(index_buffer_size), &raw_output_indices_ptr, D3DLOCK_DISCARD);

		memcpy(raw_output_indices_ptr, pIndexData, index_buffer_size);

		up_index_buffer->Unlock();
	}

	if (!set_primitive_type(PrimitiveType))
	{
		return D3DERR_INVALIDCALL;
	}

	uint32_t count = PrimitiveCount;

	if (!primitive_vertex_count(PrimitiveType, count))
	{
		return D3DERR_INVALIDCALL;
	}

	draw_call_increment();
	if (!update())
	{
		return D3DERR_INVALIDCALL;
	}

	if (skip_draw())
	{
		//OutputDebugStringA("WARNING: SKIPPING DRAW CALL\n");
		return D3D_OK;
	}

	const auto size = count * VertexStreamZeroStride;

	ComPtr<Direct3DVertexBuffer8> up_vertex_buffer = get_user_primitive_vertex_buffer(size);

	if (up_vertex_buffer == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	BYTE* ptr;
	up_vertex_buffer->Lock(0, size, &ptr, D3DLOCK_DISCARD);

	memcpy(ptr, pVertexStreamZeroData, size);

	up_vertex_buffer->Unlock();

	SetStreamSource(0, up_vertex_buffer.Get(), VertexStreamZeroStride);
	SetIndices(up_index_buffer.Get(), MinVertexIndex);

	run_draw_prologues(__FUNCTION__);
	const auto result = DrawPrimitive(PrimitiveType, 0, PrimitiveCount);
	run_draw_epilogues(__FUNCTION__);

	SetIndices(nullptr, 0);
	SetStreamSource(0, nullptr, 0);
	m_up_vertex_buffers.emplace_back(std::move(up_vertex_buffer));
	m_up_index_buffers.emplace_back(std::move(up_index_buffer));

	return result;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, Direct3DVertexBuffer8* pDestBuffer, DWORD Flags)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreateVertexShader(const DWORD* pDeclaration, const DWORD* pFunction, DWORD* pHandle, DWORD Usage)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetVertexShader(DWORD Handle)
{
	HRESULT hr;

	if ((Handle & 0x80000000) == 0)
	{
		if ((Handle & D3DFVF_XYZRHW) && D3DFVF_XYZRHW != (Handle & (D3DFVF_XYZRHW | D3DFVF_XYZ | D3DFVF_NORMAL)))
		{
			return D3DERR_INVALIDCALL;
		}

		const auto fvf = fvf_sanitize(Handle);

		m_shader_flags &= ~ShaderFlags::fvf_mask;
		m_shader_flags |= fvf;
		m_fvf_flags = fvf;

		m_current_vertex_shader_handle = 0;
		hr = D3D_OK;
	}
	else
	{
		NOT_IMPLEMENTED_RETURN;
	}

	return hr;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShader(DWORD* pHandle)
{
	if (pHandle == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if (m_current_vertex_shader_handle == 0)
	{
		*pHandle = m_fvf_flags;
		return D3D_OK;
	}

	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeleteVertexShader(DWORD Handle)
{
#if 1
	NOT_IMPLEMENTED_RETURN;
#else
	if ((Handle & 0x80000000) == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	if (CurrentVertexShaderHandle == Handle)
	{
		SetVertexShader(0);
	}

	const DWORD HandleMagic = Handle << 1;
	VertexShaderInfo *const ShaderInfo = reinterpret_cast<VertexShaderInfo *>(HandleMagic);

	if (ShaderInfo->Shader != nullptr)
	{
		ShaderInfo->Shader->Release();
	}
	if (ShaderInfo->Declaration != nullptr)
	{
		ShaderInfo->Declaration->Release();
	}

	delete ShaderInfo;

	return D3D_OK;
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetVertexShaderConstant(DWORD Register, const void* pConstantData, DWORD ConstantCount)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderDeclaration(DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	UNREFERENCED_PARAMETER(Handle);
	UNREFERENCED_PARAMETER(pData);
	UNREFERENCED_PARAMETER(pSizeOfData);

	return D3DERR_INVALIDCALL;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetVertexShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData)
{
#if 1
	NOT_IMPLEMENTED_RETURN;
#else
	if ((Handle & 0x80000000) == 0)
	{
		return D3DERR_INVALIDCALL;
	}

	const DWORD HandleMagic = Handle << 1;
	IDirect3DVertexShader9 *VertexShaderInterface = reinterpret_cast<VertexShaderInfo *>(HandleMagic)->Shader;

	if (VertexShaderInterface == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	return VertexShaderInterface->GetFunction(pData, reinterpret_cast<UINT *>(pSizeOfData));
#endif
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetStreamSource(UINT StreamNumber, Direct3DVertexBuffer8* pStreamData, UINT Stride)
{
	const StreamPair pair = { pStreamData, pStreamData ? Stride : 0 };
	auto it = m_stream_sources.find(StreamNumber);

	if (it == m_stream_sources.end())
	{
		m_stream_sources[StreamNumber] = pair;
		safe_addref(pStreamData);
	}
	else
	{
		if (it->second == pair)
		{
			return D3D_OK;
		}

		safe_release(&it->second.buffer);

		it->second = pair;

		safe_addref(pStreamData);
	}

	if (pStreamData == nullptr)
	{
		m_context->IASetVertexBuffers(StreamNumber, 0, nullptr, nullptr, nullptr);
	}
	else
	{
		UINT zero = 0;
		ID3D11Buffer* buffer_resource = pStreamData->get_native_buffer();
		m_context->IASetVertexBuffers(StreamNumber, 1, &buffer_resource, &Stride, &zero);
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetStreamSource(UINT StreamNumber, Direct3DVertexBuffer8** ppStreamData, UINT* pStride)
{
	if (ppStreamData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	*ppStreamData = nullptr;

	if (pStride)
	{
		*pStride = 0;
	}

	auto it = m_stream_sources.find(StreamNumber);

	if (it != m_stream_sources.end() && it->second.buffer)
	{
		*ppStreamData = it->second.buffer;
		it->second.buffer->AddRef();

		if (pStride)
		{
			*pStride = it->second.stride;
		}
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetIndices(Direct3DIndexBuffer8* pIndexData, UINT BaseVertexIndex)
{
	if (BaseVertexIndex > 0x7FFFFFFF)
	{
		return D3DERR_INVALIDCALL;
	}

	if (pIndexData == nullptr)
	{
		if (pIndexData != m_current_index_buffer.Get())
		{
			m_current_index_buffer = pIndexData;
			m_context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
		}

		return D3D_OK;
	}

	m_current_index_buffer = pIndexData;
	m_current_base_vertex_index = static_cast<INT>(BaseVertexIndex);
	const auto dxgi = to_dxgi(m_current_index_buffer->get_d3d8_desc().Format);
	m_context->IASetIndexBuffer(m_current_index_buffer->get_native_buffer(), dxgi, 0);
	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetIndices(Direct3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex)
{
	if (ppIndexData == nullptr)
	{
		return D3DERR_INVALIDCALL;
	}

	if (m_current_index_buffer)
	{
		*ppIndexData = m_current_index_buffer.Get();
		(*ppIndexData)->AddRef();
	}
	else
	{
		*ppIndexData = nullptr;
	}

	if (pBaseVertexIndex != nullptr)
	{
		*pBaseVertexIndex = static_cast<UINT>(m_current_base_vertex_index);
	}

	return D3D_OK;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::CreatePixelShader(const DWORD* pFunction, DWORD* pHandle)
{
	return D3DERR_INVALIDCALL;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPixelShader(DWORD Handle)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShader(DWORD* pHandle)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeletePixelShader(DWORD Handle)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::SetPixelShaderConstant(DWORD Register, const void* pConstantData, DWORD ConstantCount)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::GetPixelShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData)
{
	return D3DERR_INVALIDCALL;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawRectPatch(UINT Handle, const float* pNumSegs, const D3DRECTPATCH_INFO* pRectPatchInfo)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DrawTriPatch(UINT Handle, const float* pNumSegs, const D3DTRIPATCH_INFO* pTriPatchInfo)
{
	NOT_IMPLEMENTED_RETURN;
}

HRESULT STDMETHODCALLTYPE Direct3DDevice8::DeletePatch(UINT Handle)
{
	NOT_IMPLEMENTED_RETURN;
}

void Direct3DDevice8::print_info_queue() const
{
#ifndef DEBUG
	if (!m_info_queue)
	{
		return;
	}

	UINT64 i = 0;

	do
	{
		SIZE_T size = 0;
		HRESULT hr = m_info_queue->GetMessageW(i, nullptr, &size);

		if (hr != S_FALSE)
		{
			break;
		}

		if (!size)
		{
			break;
		}

		auto pMessage = reinterpret_cast<D3D11_MESSAGE*>(new uint8_t[size]);

		hr = m_info_queue->GetMessageW(i, pMessage, &size);

		if (hr == S_OK && pMessage->pDescription)
		{
			OutputDebugStringA(pMessage->pDescription);
		}

		delete[] pMessage;

		++i;
	} while (true);

	m_info_queue->ClearStoredMessages();
#endif
}

bool Direct3DDevice8::update_input_layout()
{
	auto key = fvf_sanitize(m_shader_flags & ShaderFlags::fvf_mask);
	DWORD fvf = key;
	m_fvf_flags.clear();

	auto it = m_fvf_layouts.find(key);

	if (it != m_fvf_layouts.end())
	{
		m_context->IASetInputLayout(it->second.Get());
		return true;
	}

	if (!m_fvf_flags.data())
	{
		return true;
	}

	D3D11_INPUT_ELEMENT_DESC pos_element {};
	pos_element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	switch (fvf & D3DFVF_POSITION_MASK)
	{
		case D3DFVF_XYZ:
			pos_element.SemanticName = "POSITION";
			pos_element.Format       = DXGI_FORMAT_R32G32B32_FLOAT;
			break;

		case D3DFVF_XYZRHW:
			pos_element.SemanticName = "POSITION";
			pos_element.Format       = DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;

		default:
			throw std::runtime_error("unsupported D3DFVF_POSITION type");
	}

	D3D11_INPUT_ELEMENT_DESC elements[16] {};

	size_t i = 0;
	fvf &= ~D3DFVF_POSITION_MASK;
	elements[i++] = pos_element;

	if (fvf & D3DFVF_NORMAL)
	{
		fvf ^= D3DFVF_NORMAL;
		D3D11_INPUT_ELEMENT_DESC e {};

		e.SemanticName      = "NORMAL";
		e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		e.Format            = DXGI_FORMAT_R32G32B32_FLOAT;
		e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		elements[i++] = e;
	}

	if (fvf & D3DFVF_DIFFUSE)
	{
		fvf ^= D3DFVF_DIFFUSE;
		D3D11_INPUT_ELEMENT_DESC e {};

		e.SemanticName      = "COLOR";
		e.SemanticIndex     = 0;
		e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		e.Format            = DXGI_FORMAT_B8G8R8A8_UNORM;
		e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		elements[i++] = e;
	}

	if (fvf & D3DFVF_SPECULAR)
	{
		fvf ^= D3DFVF_SPECULAR;
		D3D11_INPUT_ELEMENT_DESC e {};

		e.SemanticName      = "COLOR";
		e.SemanticIndex     = 1;
		e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
		e.Format            = DXGI_FORMAT_B8G8R8A8_UNORM;
		e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

		elements[i++] = e;
	}

	auto tex = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;

	if (tex > 0)
	{
		fvf &= ~D3DFVF_TEXCOUNT_MASK;

		if (tex >= 1)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 0;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex >= 2)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 1;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex >= 3)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 2;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex >= 4)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 3;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex >= 5)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 4;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex >= 6)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 5;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex >= 7)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 6;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}

		if (tex == 8)
		{
			D3D11_INPUT_ELEMENT_DESC e {};

			e.SemanticName      = "TEXCOORD";
			e.SemanticIndex     = 7;
			e.InputSlotClass    = D3D11_INPUT_PER_VERTEX_DATA;
			e.Format            = DXGI_FORMAT_R32G32_FLOAT;
			e.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

			elements[i++] = e;
		}
	}

	if (i >= 16)
	{
		throw;
	}

	if (fvf != 0)
	{
		return false;
	}

	VertexShader vs = get_vertex_shader(m_shader_flags);

	ComPtr<ID3D11InputLayout> layout;

	HRESULT hr = m_device->CreateInputLayout(elements, static_cast<UINT>(i),
	                                         vs.blob->GetBufferPointer(), vs.blob->GetBufferSize(), &layout);

	if (FAILED(hr))
	{
		return false;
	}

	m_fvf_layouts[key] = layout;
	m_context->IASetInputLayout(layout.Get());

	const std::string str = std::format("Created input layout #{}\n", m_fvf_layouts.size());
	OutputDebugStringA(str.c_str());

	return true;
}

void Direct3DDevice8::commit_uber_shader_flags()
{
	if (!m_uber_shader_flags.dirty())
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped {};
	m_context->Map(m_uber_shader_cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	auto writer = CBufferWriter(reinterpret_cast<uint8_t*>(mapped.pData));
	m_uber_shader_flags.write(writer);
	m_context->Unmap(m_uber_shader_cbuffer.Get(), 0);
	m_uber_shader_flags.clear();
}

void Direct3DDevice8::commit_per_pixel()
{
	if (!m_per_pixel.dirty())
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped {};
	m_context->Map(m_per_pixel_cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	auto writer = CBufferWriter(reinterpret_cast<uint8_t*>(mapped.pData));
	m_per_pixel.write(writer);
	m_context->Unmap(m_per_pixel_cbuffer.Get(), 0);
	m_per_pixel.clear();
}

void Direct3DDevice8::commit_per_model()
{
	if (!m_per_model.dirty())
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped {};
	m_context->Map(m_per_model_cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	auto writer = CBufferWriter(reinterpret_cast<uint8_t*>(mapped.pData));

	m_per_model.write(writer);
	m_per_model.clear();

	m_context->Unmap(m_per_model_cbuffer.Get(), 0);
}

void Direct3DDevice8::commit_per_scene()
{
	m_per_scene.screen_dimensions = { m_viewport.Width, m_viewport.Height };

	if (!m_per_scene.dirty())
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped {};
	m_context->Map(m_per_scene_cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	auto writer = CBufferWriter(reinterpret_cast<uint8_t*>(mapped.pData));
	m_per_scene.write(writer);
	m_per_scene.clear();
	m_context->Unmap(m_per_scene_cbuffer.Get(), 0);
}

void Direct3DDevice8::commit_per_texture()
{
	if (!m_per_texture.dirty())
	{
		return;
	}

	D3D11_MAPPED_SUBRESOURCE mapped {};
	m_context->Map(m_per_texture_cbuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	auto writer = CBufferWriter(reinterpret_cast<uint8_t*>(mapped.pData));
	m_per_texture.write(writer);
	m_per_texture.clear();
	m_context->Unmap(m_per_texture_cbuffer.Get(), 0);
}

void Direct3DDevice8::update_sampler()
{
	for (auto& setting_it : m_sampler_setting_values)
	{
		auto& setting = setting_it.second;

		if (!setting.dirty())
		{
			continue;
		}

		setting.clear();

		const auto it = m_sampler_states.find(setting);

		if (it != m_sampler_states.end())
		{
			m_context->PSSetSamplers(setting_it.first, 1, it->second.GetAddressOf());
			return;
		}

		D3D11_SAMPLER_DESC sampler_desc {};

		sampler_desc.Filter         = to_d3d11(setting.filter_min, setting.filter_mag, setting.filter_mip);;
		sampler_desc.AddressU       = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(setting.address_u.data());
		sampler_desc.AddressV       = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(setting.address_v.data());
		sampler_desc.AddressW       = static_cast<D3D11_TEXTURE_ADDRESS_MODE>(setting.address_w.data());
		sampler_desc.MinLOD         = -D3D11_FLOAT32_MAX; // TODO: pull from render state values
		sampler_desc.MaxLOD         = D3D11_FLOAT32_MAX;  // TODO: pull from render state values
		sampler_desc.MipLODBias     = 0.0f;               // TODO: pull from render state values
		sampler_desc.MaxAnisotropy  = setting.max_anisotropy;
		sampler_desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampler_desc.BorderColor[0] = 1.0f;
		sampler_desc.BorderColor[1] = 1.0f;
		sampler_desc.BorderColor[2] = 1.0f;
		sampler_desc.BorderColor[3] = 1.0f;

		ComPtr<ID3D11SamplerState> sampler_state;
		HRESULT hr = m_device->CreateSamplerState(&sampler_desc, &sampler_state);

		if (FAILED(hr))
		{
			throw std::runtime_error("CreateSamplerState failed");
		}

		m_context->PSSetSamplers(setting_it.first, 1, sampler_state.GetAddressOf());
		m_sampler_states[setting] = sampler_state;
	}
}

void Direct3DDevice8::get_shaders(ShaderFlags::type flags, VertexShader* vs, PixelShader* ps)
{
	int result;

	do
	{
		try
		{
			*vs = get_vertex_shader(flags);
			*ps = get_pixel_shader(flags);
			break;
		}
		catch (std::exception& ex)
		{
			free_shaders();
			result = MessageBoxA(/*WindowHandle*/ nullptr, ex.what(), "Shader compilation error", MB_RETRYCANCEL | MB_ICONERROR);
		}
	} while (result == IDRETRY);
}

void Direct3DDevice8::update_shaders()
{
	if (m_oit_actually_enabled)
	{
		m_shader_flags |= ShaderFlags::rs_oit;
	}

	m_shader_flags &= ~ShaderFlags::stage_count_mask;
	m_shader_flags |= (static_cast<ShaderFlags::type>(count_texture_stages()) << ShaderFlags::stage_count_shift) & ShaderFlags::stage_count_mask;

	const ShaderFlags::type sanitized_flags = ShaderFlags::sanitize(m_shader_flags);

	m_uber_shader_flags.rs_lighting        = (sanitized_flags & ShaderFlags::rs_lighting) != 0;
	m_uber_shader_flags.rs_specular        = (sanitized_flags & ShaderFlags::rs_specular) != 0;
	m_uber_shader_flags.rs_alpha           = (sanitized_flags & ShaderFlags::rs_alpha) != 0;
	m_uber_shader_flags.rs_alpha_test      = (sanitized_flags & ShaderFlags::rs_alpha_test) != 0;
	m_uber_shader_flags.rs_fog             = (sanitized_flags & ShaderFlags::rs_fog) != 0;
	m_uber_shader_flags.rs_oit             = (sanitized_flags & ShaderFlags::rs_oit) != 0;
	m_uber_shader_flags.rs_alpha_test_mode = (sanitized_flags & ShaderFlags::rs_alpha_test_mode_mask) >> ShaderFlags::rs_alpha_test_mode_shift;
	m_uber_shader_flags.rs_fog_mode        = (sanitized_flags & ShaderFlags::rs_fog_mode_mask) >> ShaderFlags::rs_fog_mode_shift;

	commit_uber_shader_flags();

	if (ShaderFlags::sanitize(m_last_shader_flags) == sanitized_flags)
	{
		return;
	}

	VertexShader vs;
	PixelShader ps;

	get_shaders(m_shader_flags, &vs, &ps);

	if (vs != m_current_vs)
	{
		m_context->VSSetShader(vs.shader.Get(), nullptr, 0);
		m_current_vs = vs;
	}

	if (ps != m_current_ps)
	{
		m_context->PSSetShader(ps.shader.Get(), nullptr, 0);
		m_current_ps = ps;
	}

	m_last_shader_flags = m_shader_flags;
}

void Direct3DDevice8::update_blend()
{
	if (!m_blend_flags.dirty())
	{
		return;
	}

	m_blend_flags.clear();

	const auto it = m_blend_states.find(m_blend_flags.data());

	if (it != m_blend_states.end())
	{
		m_context->OMSetBlendState(it->second.Get(), nullptr, 0xFFFFFFFF);
		return;
	}

	D3D11_BLEND_DESC desc {};

	const auto flags = m_blend_flags.data();

	for (auto& rt : desc.RenderTarget)
	{
		rt.BlendEnable           = flags >> 15 & 1;
		rt.SrcBlend              = static_cast<D3D11_BLEND>(flags & 0xF);
		rt.DestBlend             = static_cast<D3D11_BLEND>((flags >> 4) & 0xF);
		rt.BlendOp               = static_cast<D3D11_BLEND_OP>((flags >> 8) & 0xF);
		rt.RenderTargetWriteMask = static_cast<D3D11_COLOR_WRITE_ENABLE>((flags >> BLEND_COLORMASK_SHIFT) & 0xF);
		rt.SrcBlendAlpha         = D3D11_BLEND_ONE;
		rt.DestBlendAlpha        = D3D11_BLEND_ZERO;
		rt.BlendOpAlpha          = D3D11_BLEND_OP_ADD;
	}

	ComPtr<ID3D11BlendState> blend_state;
	HRESULT hr = m_device->CreateBlendState(&desc, &blend_state);

	if (FAILED(hr))
	{
		throw std::runtime_error("CreateBlendState failed");
	}

	m_blend_states[flags] = blend_state;
	m_context->OMSetBlendState(blend_state.Get(), nullptr, 0xFFFFFFFF);
}

void Direct3DDevice8::update_depth()
{
	auto& stencilref = m_render_state_values[D3DRS_STENCILREF];

	if (!m_depth_stencil_flags.dirty() && !stencilref.dirty())
	{
		return;
	}

	m_depth_stencil_flags.clear();
	stencilref.clear();

	const auto it = m_depth_states.find(m_depth_stencil_flags);

	if (it != m_depth_states.end())
	{
		m_context->OMSetDepthStencilState(it->second.Get(), stencilref.data());
		return;
	}

	D3D11_DEPTH_STENCIL_DESC depth_desc {};

	const auto& flags = m_depth_stencil_flags.flags.data();

	depth_desc.DepthEnable    = !!(flags & DepthStencilFlags::depth_test_enabled);
	depth_desc.DepthWriteMask = (flags & DepthStencilFlags::depth_write_enabled) ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
	depth_desc.StencilEnable  = !!(flags & DepthStencilFlags::stencil_enabled);

	const auto& depth_flags = m_depth_stencil_flags.depth_flags.data();
	depth_desc.DepthFunc = static_cast<D3D11_COMPARISON_FUNC>(depth_flags & DepthFlags::comparison_mask);

	if (depth_desc.StencilEnable)
	{
		const auto& stencil_flags = m_depth_stencil_flags.stencil_flags.data();
		D3D11_DEPTH_STENCILOP_DESC stencil_desc;

		stencil_desc.StencilFailOp      = static_cast<D3D11_STENCIL_OP>((stencil_flags >> StencilFlags::fail_shift) & StencilFlags::op_mask);
		stencil_desc.StencilDepthFailOp = static_cast<D3D11_STENCIL_OP>((stencil_flags >> StencilFlags::zfail_shift) & StencilFlags::op_mask);
		stencil_desc.StencilPassOp      = static_cast<D3D11_STENCIL_OP>((stencil_flags >> StencilFlags::pass_shift) & StencilFlags::op_mask);
		stencil_desc.StencilFunc        = static_cast<D3D11_COMPARISON_FUNC>((stencil_flags >> StencilFlags::func_shift) & StencilFlags::op_mask);

		depth_desc.StencilReadMask  = (stencil_flags >> StencilFlags::read_shift) & StencilFlags::rw_mask;
		depth_desc.StencilWriteMask = (stencil_flags >> StencilFlags::write_shift) & StencilFlags::rw_mask;

		depth_desc.FrontFace = stencil_desc;
		depth_desc.BackFace  = stencil_desc;
	}
	else
	{
		depth_desc.FrontFace = {
			D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP,
			D3D11_COMPARISON_ALWAYS
		};

		depth_desc.BackFace = {
			D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP,
			D3D11_STENCIL_OP_KEEP,
			D3D11_COMPARISON_ALWAYS
		};
	}

	ComPtr<ID3D11DepthStencilState> depth_state;
	if (FAILED(m_device->CreateDepthStencilState(&depth_desc, &depth_state)))
	{
		throw std::runtime_error("Failed to create depth stencil!");
	}

	m_context->OMSetDepthStencilState(depth_state.Get(), stencilref.data());
	m_depth_states[m_depth_stencil_flags] = std::move(depth_state);
}

void Direct3DDevice8::update_rasterizers()
{
	if (!m_raster_flags.dirty())
	{
		return;
	}

	m_raster_flags.clear();
	const auto it = m_raster_states.find(m_raster_flags);

	if (it != m_raster_states.end())
	{
		m_context->RSSetState(it->second.Get());
		return;
	}

	D3D11_RASTERIZER_DESC raster {};

	raster.FillMode        = static_cast<D3D11_FILL_MODE>((m_raster_flags.data() >> 2) & 3);
	raster.CullMode        = static_cast<D3D11_CULL_MODE>(m_raster_flags.data() & 3);
	raster.DepthClipEnable = TRUE;

	ComPtr<ID3D11RasterizerState> raster_state;
	if (FAILED(m_device->CreateRasterizerState(&raster, &raster_state)))
	{
		throw std::runtime_error("failed to create rasterizer state");
	}

	m_context->RSSetState(raster_state.Get());
	m_raster_states.emplace(m_raster_flags, std::move(raster_state));
}

bool Direct3DDevice8::update()
{
	update_rasterizers();
	update_shaders();
	update_sampler();
	update_blend();
	update_depth();
	commit_per_scene();
	commit_per_texture();
	commit_per_model();
	commit_per_pixel();

	if (skip_draw())
	{
		return true;
	}

	return update_input_layout();
}

bool Direct3DDevice8::skip_draw() const
{
	return !m_current_ps.has_value() || !m_current_vs.has_value();
}

void Direct3DDevice8::free_shaders()
{
	m_thread_pool.wait();

	m_last_shader_flags = ShaderFlags::mask;

	m_current_vs = {};
	m_current_ps = {};

	m_vertex_shaders.clear();
	m_pixel_shaders.clear();
	m_uber_vertex_shaders.clear();
	m_uber_pixel_shaders.clear();

	for (auto& future : m_compiling_vertex_shaders | std::views::values)
	{
		future.wait();
	}

	for (auto& future : m_compiling_pixel_shaders | std::views::values)
	{
		future.wait();
	}

	m_compiling_vertex_shaders.clear();
	m_compiling_pixel_shaders.clear();

	m_shader_includer.clear_shader_source_cache();

	m_fvf_layouts.clear();

	for (auto& value : m_render_state_values)
	{
		value.mark();
	}

	m_depth_stencil_flags.mark();
	m_blend_flags.mark();

	for (auto& pair : m_sampler_setting_values)
	{
		pair.second.mark();
	}

	m_uber_shader_flags.mark();
	m_per_model.mark();
	m_per_pixel.mark();
	m_per_scene.mark();
	m_per_texture.mark();

	m_oit_composite_vs = {};
	m_oit_composite_ps = {};
}

void Direct3DDevice8::oit_load_shaders()
{
	D3D_SHADER_MACRO preproc[] = {
		{ "OIT_MAX_FRAGMENTS", m_oit_fragments_str.c_str() },
		{}
	};

	int message_box_result;

	do
	{
		try
		{
			ComPtr<ID3DBlob> errors;
			ComPtr<ID3DBlob> blob;

			std::filesystem::path shader_path = d3d8to11::config->get_shader_source_dir() / "composite.hlsl";

			if (d3d8to11::filesystem::should_extend_length(shader_path))
			{
				shader_path = d3d8to11::filesystem::as_extended_length(shader_path);
			}

			const std::string shader_path_string = shader_path.string(); // unfortunately a necessary evil :(
			const auto shader_source = m_shader_includer.get_shader_source(shader_path);

			// first, compile the vertex shader (vs_main)
			HRESULT hr = D3DCompile(shader_source.data(), shader_source.size(), shader_path_string.c_str(), &preproc[0], &m_shader_includer, "vs_main", "vs_5_0", 0, 0, &blob, &errors);

			if (FAILED(hr))
			{
				std::string str(static_cast<char*>(errors->GetBufferPointer()), 0, errors->GetBufferSize());
				throw std::runtime_error(str);
			}

			hr = m_device->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_oit_composite_vs.shader);

			if (FAILED(hr))
			{
				throw std::runtime_error("composite vertex shader creation failed");
			}

			m_oit_composite_vs.blob = std::exchange(blob, nullptr);

			// second, compile the pixel shader (ps_main)
			hr = D3DCompile(shader_source.data(), shader_source.size(), shader_path_string.c_str(), &preproc[0], &m_shader_includer, "ps_main", "ps_5_0", 0, 0, &blob, &errors);

			if (FAILED(hr))
			{
				std::string str(static_cast<char*>(errors->GetBufferPointer()), 0, errors->GetBufferSize());
				throw std::runtime_error(str);
			}

			hr = m_device->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, &m_oit_composite_ps.shader);

			if (FAILED(hr))
			{
				throw std::runtime_error("composite pixel shader creation failed");
			}

			m_oit_composite_ps.blob = std::exchange(blob, nullptr);
			break;
		}
		catch (std::exception& ex)
		{
			print_info_queue();
			free_shaders();
			message_box_result = MessageBoxA(nullptr, ex.what(), "Shader compilation error", MB_RETRYCANCEL | MB_ICONERROR);
		}
	} while (message_box_result == IDRETRY);
}

void Direct3DDevice8::oit_release()
{
	const std::array<ID3D11UnorderedAccessView*, 5> null {};

	m_context->OMSetRenderTargetsAndUnorderedAccessViews(1, m_render_target_view.GetAddressOf(), nullptr,
	                                                     1, static_cast<UINT>(null.size()), &null[0], nullptr);

	m_oit_frag_list_head      = nullptr;
	m_oit_frag_list_head_srv  = nullptr;
	m_oit_frag_list_head_uav  = nullptr;
	m_oit_frag_list_count     = nullptr;
	m_oit_frag_list_count_srv = nullptr;
	m_oit_frag_list_count_uav = nullptr;
	m_oit_frag_list_nodes     = nullptr;
	m_oit_frag_list_nodes_srv = nullptr;
	m_oit_frag_list_nodes_uav = nullptr;
}

void Direct3DDevice8::oit_write()
{
	// Unbinds the shader resource views for our fragment list and list head.
	// UAVs cannot be bound as standard resource views and UAVs simultaneously.
	const std::array<ID3D11ShaderResourceView*, 5> srvs {};
	m_context->PSSetShaderResources(0, static_cast<UINT>(srvs.size()), &srvs[0]);

	const std::array uavs = {
		m_oit_frag_list_head_uav.Get(),
		m_oit_frag_list_count_uav.Get(),
		m_oit_frag_list_nodes_uav.Get()
	};

	// This is used to set the hidden counter of m_oit_frag_list_nodes to 0.
	// It only works on m_oit_frag_list_nodes, but the number of elements here
	// must match the number of UAVs given.
	const uint32_t zero[3] = { 0, 0, 0 };

	// Binds our fragment list & list head UAVs for read/write operations.
	m_context->OMSetRenderTargetsAndUnorderedAccessViews(1, m_oit_actually_enabled ? m_oit_composite_view.GetAddressOf() : m_render_target_view.GetAddressOf(),
	                                                     m_current_depth_stencil->get_native_depth_stencil(), 1, static_cast<UINT>(uavs.size()), &uavs[0], &zero[0]);

	// Resets the list head indices to OIT_FRAGMENT_LIST_NULL.
	// 4 elements are required as this can be used to clear a texture
	// with 4 color channels, even though our list head only has one.
	const UINT clear_head[] = { UINT_MAX, UINT_MAX, UINT_MAX, UINT_MAX };
	m_context->ClearUnorderedAccessViewUint(m_oit_frag_list_head_uav.Get(), &clear_head[0]);

	const UINT clear_count[] = { 0, 0, 0, 0 };
	m_context->ClearUnorderedAccessViewUint(m_oit_frag_list_count_uav.Get(), &clear_count[0]);
}

void Direct3DDevice8::oit_read() const
{
	const std::array<ID3D11UnorderedAccessView*, 3> uavs {};

	// Unbinds our UAVs.
	m_context->OMSetRenderTargetsAndUnorderedAccessViews(1, m_render_target_view.GetAddressOf(), nullptr, 1, static_cast<UINT>(uavs.size()), &uavs[0], nullptr);

	const std::array srvs {
		m_oit_frag_list_head_srv.Get(),
		m_oit_frag_list_count_srv.Get(),
		m_oit_frag_list_nodes_srv.Get(),
		m_oit_composite_srv.Get(),
		m_current_depth_stencil->get_native_depth_srv()
	};

	// Binds the shader resource views of our UAV buffers as read-only.
	m_context->PSSetShaderResources(0, static_cast<UINT>(srvs.size()), &srvs[0]);
}

void Direct3DDevice8::oit_init()
{
	oit_frag_list_head_init();
	oit_frag_list_count_init();
	oit_frag_list_nodes_init();

	oit_write();
}

void Direct3DDevice8::oit_frag_list_head_init()
{
	D3D11_TEXTURE2D_DESC desc_2d = {};

	desc_2d.ArraySize          = 1;
	desc_2d.BindFlags          = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc_2d.Usage              = D3D11_USAGE_DEFAULT;
	desc_2d.Format             = DXGI_FORMAT_R32_UINT;
	desc_2d.Width              = static_cast<UINT>(m_viewport.Width);
	desc_2d.Height             = static_cast<UINT>(m_viewport.Height);
	desc_2d.MipLevels          = 1;
	desc_2d.SampleDesc.Count   = 1;
	desc_2d.SampleDesc.Quality = 0;

	if (FAILED(m_device->CreateTexture2D(&desc_2d, nullptr, &m_oit_frag_list_head)))
	{
		throw;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC desc_rv;

	desc_rv.Format                    = desc_2d.Format;
	desc_rv.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc_rv.Texture2D.MipLevels       = 1;
	desc_rv.Texture2D.MostDetailedMip = 0;

	if (FAILED(m_device->CreateShaderResourceView(m_oit_frag_list_head.Get(), &desc_rv, &m_oit_frag_list_head_srv)))
	{
		throw;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc_uav;

	desc_uav.Format              = desc_2d.Format;
	desc_uav.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
	desc_uav.Buffer.FirstElement = 0;
	desc_uav.Buffer.NumElements  = static_cast<UINT>(m_viewport.Width) * static_cast<UINT>(m_viewport.Height);
	desc_uav.Buffer.Flags        = 0;

	if (FAILED(m_device->CreateUnorderedAccessView(m_oit_frag_list_head.Get(), &desc_uav, &m_oit_frag_list_head_uav)))
	{
		throw;
	}
}

void Direct3DDevice8::oit_frag_list_count_init()
{
	D3D11_TEXTURE2D_DESC desc_2d = {};

	desc_2d.ArraySize          = 1;
	desc_2d.BindFlags          = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc_2d.Usage              = D3D11_USAGE_DEFAULT;
	desc_2d.Format             = DXGI_FORMAT_R32_UINT;
	desc_2d.Width              = static_cast<UINT>(m_viewport.Width);
	desc_2d.Height             = static_cast<UINT>(m_viewport.Height);
	desc_2d.MipLevels          = 1;
	desc_2d.SampleDesc.Count   = 1;
	desc_2d.SampleDesc.Quality = 0;

	if (FAILED(m_device->CreateTexture2D(&desc_2d, nullptr, &m_oit_frag_list_count)))
	{
		throw;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC desc_rv;

	desc_rv.Format                    = desc_2d.Format;
	desc_rv.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
	desc_rv.Texture2D.MipLevels       = 1;
	desc_rv.Texture2D.MostDetailedMip = 0;

	if (FAILED(m_device->CreateShaderResourceView(m_oit_frag_list_count.Get(), &desc_rv, &m_oit_frag_list_count_srv)))
	{
		throw;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc_uav;

	desc_uav.Format              = desc_2d.Format;
	desc_uav.ViewDimension       = D3D11_UAV_DIMENSION_TEXTURE2D;
	desc_uav.Buffer.FirstElement = 0;
	desc_uav.Buffer.NumElements  = static_cast<UINT>(m_viewport.Width) * static_cast<UINT>(m_viewport.Height);
	desc_uav.Buffer.Flags        = 0;

	if (FAILED(m_device->CreateUnorderedAccessView(m_oit_frag_list_count.Get(), &desc_uav, &m_oit_frag_list_count_uav)))
	{
		throw;
	}
}

void Direct3DDevice8::oit_frag_list_nodes_init()
{
	// see OITNode in the shader code
	constexpr UINT oit_node_size_bytes = 16;

	D3D11_BUFFER_DESC desc_buf = {};

	m_per_scene.oit_buffer_length = static_cast<UINT>(m_viewport.Width) * static_cast<UINT>(m_viewport.Height) * globals::max_fragments;

	desc_buf.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc_buf.BindFlags           = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc_buf.ByteWidth           = oit_node_size_bytes * static_cast<UINT>(m_viewport.Width) * static_cast<UINT>(m_viewport.Height) * globals::max_fragments;
	desc_buf.StructureByteStride = oit_node_size_bytes;

	if (FAILED(m_device->CreateBuffer(&desc_buf, nullptr, &m_oit_frag_list_nodes)))
	{
		throw;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC desc_rv = {};

	desc_rv.Format             = DXGI_FORMAT_UNKNOWN;
	desc_rv.ViewDimension      = D3D11_SRV_DIMENSION_BUFFER;
	desc_rv.Buffer.NumElements = static_cast<UINT>(m_viewport.Width) * static_cast<UINT>(m_viewport.Height) * globals::max_fragments;

	if (FAILED(m_device->CreateShaderResourceView(m_oit_frag_list_nodes.Get(), &desc_rv, &m_oit_frag_list_nodes_srv)))
	{
		throw;
	}

	D3D11_UNORDERED_ACCESS_VIEW_DESC desc_uav;

	desc_uav.Format              = DXGI_FORMAT_UNKNOWN;
	desc_uav.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
	desc_uav.Buffer.FirstElement = 0;
	desc_uav.Buffer.NumElements  = static_cast<UINT>(m_viewport.Width) * static_cast<UINT>(m_viewport.Height) * globals::max_fragments;
	desc_uav.Buffer.Flags        = D3D11_BUFFER_UAV_FLAG_COUNTER;

	if (FAILED(m_device->CreateUnorderedAccessView(m_oit_frag_list_nodes.Get(), &desc_uav, &m_oit_frag_list_nodes_uav)))
	{
		throw;
	}
}

ComPtr<Direct3DVertexBuffer8> Direct3DDevice8::get_user_primitive_vertex_buffer(size_t target_size)
{
	ComPtr<Direct3DVertexBuffer8> up_buffer;
	const size_t rounded = round_pow2(target_size);

	for (auto it = m_up_vertex_buffers.begin(); it != m_up_vertex_buffers.end(); ++it)
	{
		if ((*it)->get_d3d8_desc().Size >= rounded && (*it)->get_d3d8_desc().Size < 2 * rounded)
		{
			up_buffer = std::move(*it);
			m_up_vertex_buffers.erase(it);
			return up_buffer;
		}
	}

#if _DEBUG
	{
		const std::string str = std::format("creating new UP vertex buffer. count: {}; target size: {}; rounded size: {}\n",
		                                    m_up_vertex_buffers.size() + 1, target_size, rounded);

		OutputDebugStringA(str.c_str());
	}
#endif

	CreateVertexBuffer(static_cast<UINT>(rounded), D3DUSAGE_DYNAMIC, 0, D3DPOOL_DEFAULT, &up_buffer);
	return up_buffer;
}

ComPtr<Direct3DIndexBuffer8> Direct3DDevice8::get_user_primitive_index_buffer(size_t target_size, D3DFORMAT format)
{
	ComPtr<Direct3DIndexBuffer8> up_buffer;
	const size_t rounded = round_pow2(target_size);

	for (auto it = m_up_index_buffers.begin(); it != m_up_index_buffers.end(); ++it)
	{
		if ((*it)->get_d3d8_desc().Format != format)
		{
			continue;
		}

		if ((*it)->get_d3d8_desc().Size >= rounded && (*it)->get_d3d8_desc().Size < 2 * rounded)
		{
			up_buffer = std::move(*it);
			m_up_index_buffers.erase(it);
			return up_buffer;
		}
	}

#if _DEBUG
	{
		const std::string str = std::format("creating new UP index buffer. count: {}; target size: {}; rounded size: {}\n",
		                                    m_up_index_buffers.size() + 1, target_size, rounded);

		OutputDebugStringA(str.c_str());
	}
#endif

	CreateIndexBuffer(static_cast<UINT>(rounded), D3DUSAGE_DYNAMIC, format, D3DPOOL_DEFAULT, &up_buffer);
	return up_buffer;
}
