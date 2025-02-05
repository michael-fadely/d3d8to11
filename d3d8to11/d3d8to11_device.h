#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include <d3d11_1.h>
#include <wrl/client.h>

#include <dirty_t.h>

#include "alignment.h"
#include "cbuffers.h"
#include "DepthStencilFlags.h"
#include "SamplerSettings.h"
#include "Shader.h"
#include "ShaderFlags.h"
#include "ShaderIncluder.h"
#include "simple_math.h"
#include "ThreadPool.h"
#include "Unknown.h"

class Direct3DBaseTexture8;
class Direct3DIndexBuffer8;
class Direct3DTexture8;
class Direct3DVertexBuffer8;
class Direct3DSurface8;

using Direct3DSwapChain8 = void;
using Direct3DCubeTexture8 = void;
using Direct3DVolumeTexture8 = void;

using Microsoft::WRL::ComPtr;

class __declspec(uuid("7385E5DF-8FE8-41D5-86B6-D7B48547B6CF")) Direct3DDevice8;

// the destructor cannot be virtual because that would change the layout of the vtable
// ReSharper disable once CppPolymorphicClassWithNonVirtualPublicDestructor
class Direct3DDevice8 : public Unknown
{
public:
	Direct3DDevice8(const Direct3DDevice8&)     = delete;
	Direct3DDevice8(Direct3DDevice8&&) noexcept = delete;

	Direct3DDevice8& operator=(const Direct3DDevice8&)     = delete;
	Direct3DDevice8& operator=(Direct3DDevice8&&) noexcept = delete;

	Direct3DDevice8(Direct3D8* d3d, UINT adapter, D3DDEVTYPE device_type, HWND focus_window, DWORD behavior_flags, const D3DPRESENT_PARAMETERS8& parameters);
	~Direct3DDevice8() = default;

	[[nodiscard]] ID3D11Device* get_native_device() const
	{
		return m_device.Get();
	}

	[[nodiscard]] ID3D11DeviceContext* get_native_context() const
	{
		return m_context.Get();
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObj) override;
	virtual ULONG STDMETHODCALLTYPE AddRef() override;
	virtual ULONG STDMETHODCALLTYPE Release() override;

	virtual HRESULT STDMETHODCALLTYPE TestCooperativeLevel();
	virtual UINT STDMETHODCALLTYPE GetAvailableTextureMem();
	virtual HRESULT STDMETHODCALLTYPE ResourceManagerDiscardBytes(DWORD Bytes);
	virtual HRESULT STDMETHODCALLTYPE GetDirect3D(Direct3D8** ppD3D8);
	virtual HRESULT STDMETHODCALLTYPE GetDeviceCaps(D3DCAPS8* pCaps);
	virtual HRESULT STDMETHODCALLTYPE GetDisplayMode(D3DDISPLAYMODE* pMode);
	virtual HRESULT STDMETHODCALLTYPE GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS* pParameters);
	virtual HRESULT STDMETHODCALLTYPE SetCursorProperties(UINT XHotSpot, UINT YHotSpot, Direct3DSurface8* pCursorBitmap);
	virtual void STDMETHODCALLTYPE SetCursorPosition(UINT XScreenSpace, UINT YScreenSpace, DWORD Flags);
	virtual BOOL STDMETHODCALLTYPE ShowCursor(BOOL bShow);
	virtual HRESULT STDMETHODCALLTYPE CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS8* pPresentationParameters, Direct3DSwapChain8** ppSwapChain);
	virtual HRESULT STDMETHODCALLTYPE Reset(D3DPRESENT_PARAMETERS8* pPresentationParameters);
	virtual HRESULT STDMETHODCALLTYPE Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const RGNDATA* pDirtyRegion);
	virtual HRESULT STDMETHODCALLTYPE GetBackBuffer(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, Direct3DSurface8** ppBackBuffer);
	virtual HRESULT STDMETHODCALLTYPE GetRasterStatus(D3DRASTER_STATUS* pRasterStatus);
	virtual void STDMETHODCALLTYPE SetGammaRamp(DWORD Flags, const D3DGAMMARAMP* pRamp);
	virtual void STDMETHODCALLTYPE GetGammaRamp(D3DGAMMARAMP* pRamp);
	virtual HRESULT STDMETHODCALLTYPE CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DTexture8** ppTexture);
	virtual HRESULT STDMETHODCALLTYPE CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DVolumeTexture8** ppVolumeTexture);
	virtual HRESULT STDMETHODCALLTYPE CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DCubeTexture8** ppCubeTexture);
	virtual HRESULT STDMETHODCALLTYPE CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, Direct3DVertexBuffer8** ppVertexBuffer);
	virtual HRESULT STDMETHODCALLTYPE CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, Direct3DIndexBuffer8** ppIndexBuffer);
	virtual HRESULT STDMETHODCALLTYPE CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, BOOL Lockable, Direct3DSurface8** ppSurface);
	virtual HRESULT STDMETHODCALLTYPE CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, Direct3DSurface8** ppSurface);
	virtual HRESULT STDMETHODCALLTYPE CreateImageSurface(UINT Width, UINT Height, D3DFORMAT Format, Direct3DSurface8** ppSurface);
	virtual HRESULT STDMETHODCALLTYPE CopyRects(Direct3DSurface8* pSourceSurface, const RECT* pSourceRectsArray, UINT cRects, Direct3DSurface8* pDestinationSurface, const POINT* pDestPointsArray);
	virtual HRESULT STDMETHODCALLTYPE UpdateTexture(Direct3DBaseTexture8* pSourceTexture, Direct3DBaseTexture8* pDestinationTexture);
	virtual HRESULT STDMETHODCALLTYPE GetFrontBuffer(Direct3DSurface8* pDestSurface);
	virtual HRESULT STDMETHODCALLTYPE SetRenderTarget(Direct3DSurface8* pRenderTarget, Direct3DSurface8* pNewZStencil);
	virtual HRESULT STDMETHODCALLTYPE GetRenderTarget(Direct3DSurface8** ppRenderTarget);
	virtual HRESULT STDMETHODCALLTYPE GetDepthStencilSurface(Direct3DSurface8** ppZStencilSurface);
	virtual HRESULT STDMETHODCALLTYPE BeginScene();
	virtual HRESULT STDMETHODCALLTYPE EndScene();
	virtual HRESULT STDMETHODCALLTYPE Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	virtual HRESULT STDMETHODCALLTYPE SetTransform(D3DTRANSFORMSTATETYPE State, const matrix* pMatrix);
	virtual HRESULT STDMETHODCALLTYPE GetTransform(D3DTRANSFORMSTATETYPE State, matrix* pMatrix);
	virtual HRESULT STDMETHODCALLTYPE MultiplyTransform(D3DTRANSFORMSTATETYPE State, const matrix* pMatrix);
	virtual HRESULT STDMETHODCALLTYPE SetViewport(const D3DVIEWPORT8* pViewport);
	virtual HRESULT STDMETHODCALLTYPE GetViewport(D3DVIEWPORT8* pViewport);
	virtual HRESULT STDMETHODCALLTYPE SetMaterial(const D3DMATERIAL8* pMaterial);
	virtual HRESULT STDMETHODCALLTYPE GetMaterial(D3DMATERIAL8* pMaterial);
	virtual HRESULT STDMETHODCALLTYPE SetLight(DWORD Index, const D3DLIGHT8* pLight);
	virtual HRESULT STDMETHODCALLTYPE GetLight(DWORD Index, D3DLIGHT8* pLight);
	virtual HRESULT STDMETHODCALLTYPE LightEnable(DWORD Index, BOOL Enable);
	virtual HRESULT STDMETHODCALLTYPE GetLightEnable(DWORD Index, BOOL* pEnable);
	virtual HRESULT STDMETHODCALLTYPE SetClipPlane(DWORD Index, const float* pPlane);
	virtual HRESULT STDMETHODCALLTYPE GetClipPlane(DWORD Index, float* pPlane);
	virtual HRESULT STDMETHODCALLTYPE SetRenderState(D3DRENDERSTATETYPE State, DWORD Value);
	virtual HRESULT STDMETHODCALLTYPE GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue);
	virtual HRESULT STDMETHODCALLTYPE BeginStateBlock();
	virtual HRESULT STDMETHODCALLTYPE EndStateBlock(DWORD* pToken);
	virtual HRESULT STDMETHODCALLTYPE ApplyStateBlock(DWORD Token);
	virtual HRESULT STDMETHODCALLTYPE CaptureStateBlock(DWORD Token);
	virtual HRESULT STDMETHODCALLTYPE DeleteStateBlock(DWORD Token);
	virtual HRESULT STDMETHODCALLTYPE CreateStateBlock(D3DSTATEBLOCKTYPE Type, DWORD* pToken);
	virtual HRESULT STDMETHODCALLTYPE SetClipStatus(const D3DCLIPSTATUS8* pClipStatus);
	virtual HRESULT STDMETHODCALLTYPE GetClipStatus(D3DCLIPSTATUS8* pClipStatus);
	virtual HRESULT STDMETHODCALLTYPE GetTexture(DWORD Stage, Direct3DBaseTexture8** ppTexture);
	virtual HRESULT STDMETHODCALLTYPE SetTexture(DWORD Stage, Direct3DBaseTexture8* pTexture);
	virtual HRESULT STDMETHODCALLTYPE GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue);
	virtual HRESULT STDMETHODCALLTYPE SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value);
	virtual HRESULT STDMETHODCALLTYPE ValidateDevice(DWORD* pNumPasses);
	virtual HRESULT STDMETHODCALLTYPE GetInfo(DWORD DevInfoID, void* pDevInfoStruct, DWORD DevInfoStructSize);
	virtual HRESULT STDMETHODCALLTYPE SetPaletteEntries(UINT PaletteNumber, const PALETTEENTRY* pEntries);
	virtual HRESULT STDMETHODCALLTYPE GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries);
	virtual HRESULT STDMETHODCALLTYPE SetCurrentTexturePalette(UINT PaletteNumber);
	virtual HRESULT STDMETHODCALLTYPE GetCurrentTexturePalette(UINT* pPaletteNumber);
	virtual HRESULT STDMETHODCALLTYPE DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount);
	virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
	virtual HRESULT STDMETHODCALLTYPE DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	virtual HRESULT STDMETHODCALLTYPE DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, const void* pIndexData, D3DFORMAT IndexDataFormat, const void* pVertexStreamZeroData, UINT VertexStreamZeroStride);
	virtual HRESULT STDMETHODCALLTYPE ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, Direct3DVertexBuffer8* pDestBuffer, DWORD Flags);
	virtual HRESULT STDMETHODCALLTYPE CreateVertexShader(const DWORD* pDeclaration, const DWORD* pFunction, DWORD* pHandle, DWORD Usage);
	virtual HRESULT STDMETHODCALLTYPE SetVertexShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE GetVertexShader(DWORD* pHandle);
	virtual HRESULT STDMETHODCALLTYPE DeleteVertexShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE SetVertexShaderConstant(DWORD Register, const void* pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE GetVertexShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE GetVertexShaderDeclaration(DWORD Handle, void* pData, DWORD* pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE GetVertexShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE SetStreamSource(UINT StreamNumber, Direct3DVertexBuffer8* pStreamData, UINT Stride);
	virtual HRESULT STDMETHODCALLTYPE GetStreamSource(UINT StreamNumber, Direct3DVertexBuffer8** ppStreamData, UINT* pStride);
	virtual HRESULT STDMETHODCALLTYPE SetIndices(Direct3DIndexBuffer8* pIndexData, UINT BaseVertexIndex);
	virtual HRESULT STDMETHODCALLTYPE GetIndices(Direct3DIndexBuffer8** ppIndexData, UINT* pBaseVertexIndex);
	virtual HRESULT STDMETHODCALLTYPE CreatePixelShader(const DWORD* pFunction, DWORD* pHandle);
	virtual HRESULT STDMETHODCALLTYPE SetPixelShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE GetPixelShader(DWORD* pHandle);
	virtual HRESULT STDMETHODCALLTYPE DeletePixelShader(DWORD Handle);
	virtual HRESULT STDMETHODCALLTYPE SetPixelShaderConstant(DWORD Register, const void* pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE GetPixelShaderConstant(DWORD Register, void* pConstantData, DWORD ConstantCount);
	virtual HRESULT STDMETHODCALLTYPE GetPixelShaderFunction(DWORD Handle, void* pData, DWORD* pSizeOfData);
	virtual HRESULT STDMETHODCALLTYPE DrawRectPatch(UINT Handle, const float* pNumSegs, const D3DRECTPATCH_INFO* pRectPatchInfo);
	virtual HRESULT STDMETHODCALLTYPE DrawTriPatch(UINT Handle, const float* pNumSegs, const D3DTRIPATCH_INFO* pTriPatchInfo);
	virtual HRESULT STDMETHODCALLTYPE DeletePatch(UINT Handle);

	void run_draw_prologues(const std::string& callback);
	void run_draw_epilogues(const std::string& callback);

	void print_info_queue() const;

	[[nodiscard]] size_t count_texture_stages() const;
	void shader_preprocess(ShaderFlags::type flags, bool is_uber, std::vector<D3D_SHADER_MACRO>& result) const;
	[[nodiscard]] std::vector<D3D_SHADER_MACRO> shader_preprocess(ShaderFlags::type flags, bool is_uber) const;

	void draw_call_increment();

	[[nodiscard]] VertexShader compile_vertex_shader(ShaderFlags::type flags, bool is_uber);
	[[nodiscard]] PixelShader compile_pixel_shader(ShaderFlags::type flags, bool is_uber);
	void store_permutation_flags(ShaderFlags::type flags);
	[[nodiscard]] VertexShader get_vertex_shader(ShaderFlags::type flags);
	[[nodiscard]] PixelShader get_pixel_shader(ShaderFlags::type flags);
	void create_depth_stencil();
	void create_composite_texture(D3D11_TEXTURE2D_DESC& tex_desc);
	void create_render_target(D3D11_TEXTURE2D_DESC& tex_desc);
	void get_back_buffer();
	void create_native();
	bool set_primitive_type(D3DPRIMITIVETYPE primitive_type) const;
	static uint32_t primitive_vertex_count(D3DPRIMITIVETYPE type, UINT count);
	void oit_composite();
	void oit_start();
	void oit_zwrite_force(DWORD& ZWRITEENABLE, DWORD& ZENABLE);
	void oit_zwrite_restore(DWORD ZWRITEENABLE, DWORD ZENABLE);
	bool update_input_layout();
	void commit_uber_shader_flags();
	void commit_per_pixel();
	void commit_per_model();
	void commit_per_scene();
	void commit_per_texture();
	void update_sampler();
	void get_shaders(ShaderFlags::type flags, VertexShader* vs, PixelShader* ps);
	void update_shaders();
	void update_blend();
	void update_depth();
	void update_rasterizers();
	bool update();
	bool skip_draw() const;
	void free_shaders();
	void oit_load_shaders();
	void oit_release();
	void update_wv_inv_t();

	HRESULT make_cbuffer(ICBuffer& interface_, ComPtr<ID3D11Buffer>& cbuffer) const
	{
		D3D11_BUFFER_DESC desc {};

		const size_t cbuffer_size = interface_.cbuffer_size();

		desc.ByteWidth           = static_cast<decltype(desc.ByteWidth)>(align_up(cbuffer_size, 16)); // FIXME: magic number for buffer alignment
		desc.Usage               = D3D11_USAGE_DYNAMIC;
		desc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
		desc.StructureByteStride = static_cast<decltype(desc.StructureByteStride)>(cbuffer_size);

		return m_device->CreateBuffer(&desc, nullptr, &cbuffer);
	}

	using ShaderCallback = std::function<void(const std::vector<D3D_SHADER_MACRO>&, ShaderFlags::type)>;

	std::unordered_map<std::string, std::deque<ShaderCallback>> draw_prologues;
	std::unordered_map<std::string, std::deque<ShaderCallback>> draw_epilogues;

	bool oit_enabled = false;

private:
	struct StreamPair
	{
		Direct3DVertexBuffer8* buffer;
		UINT stride;

		bool operator==(const StreamPair& other) const
		{
			return buffer == other.buffer &&
			       stride == other.stride;
		}
	};

	void oit_write();
	void oit_read() const;
	void oit_init();
	void oit_frag_list_head_init();
	void oit_frag_list_count_init();
	void oit_frag_list_nodes_init();

	Direct3D8* const m_d3d;
	UINT m_adapter;
	HWND m_focus_window;
	D3DDEVTYPE m_device_type;
	DWORD m_behavior_flags;
	D3DPRESENT_PARAMETERS8 m_present_params {};

	std::unordered_map<size_t, std::string> m_digit_strings;
	const std::string m_oit_fragments_str;

	ThreadPool m_thread_pool;

	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11DeviceContext> m_context;
	ComPtr<ID3D11InfoQueue> m_info_queue;

	ComPtr<IDXGISwapChain> m_swap_chain;

	std::vector<uint32_t> m_trifan_index_buffer;

	std::fstream m_permutation_cache_file;
	std::unordered_set<ShaderFlags::type> m_permutation_flags;

	bool m_freeing_shaders = false;

	ShaderFlags::type m_shader_flags = ShaderFlags::none;
	ShaderFlags::type m_last_shader_flags = ShaderFlags::mask;

	ShaderIncluder m_shader_includer;

	VertexShader m_current_vs;
	PixelShader m_current_ps;

	std::unordered_map<ShaderFlags::type, VertexShader> m_vertex_shaders;
	std::unordered_map<ShaderFlags::type, PixelShader> m_pixel_shaders;

	std::unordered_map<ShaderFlags::type, std::future<VertexShader>> m_compiling_vertex_shaders;
	std::unordered_map<ShaderFlags::type, std::future<PixelShader>>  m_compiling_pixel_shaders;

	std::unordered_map<ShaderFlags::type, VertexShader> m_uber_vertex_shaders;
	std::unordered_map<ShaderFlags::type, PixelShader> m_uber_pixel_shaders;

	bool m_oit_actually_enabled = false;

	VertexShader m_oit_composite_vs;
	PixelShader m_oit_composite_ps;

	ComPtr<Direct3DTexture8> m_back_buffer;
	ComPtr<ID3D11RenderTargetView> m_back_buffer_view;

	ComPtr<Direct3DTexture8>         m_render_target_wrapper;
	ComPtr<ID3D11Texture2D>          m_render_target_texture;
	ComPtr<ID3D11RenderTargetView>   m_render_target_view;
	ComPtr<ID3D11ShaderResourceView> m_render_target_srv;

	ComPtr<Direct3DSurface8> m_current_render_target;
	ComPtr<Direct3DSurface8> m_current_depth_stencil;

	ComPtr<Direct3DTexture8> m_oit_composite_wrapper;
	ComPtr<ID3D11Texture2D> m_oit_composite_texture;
	ComPtr<ID3D11RenderTargetView> m_oit_composite_view;
	ComPtr<ID3D11ShaderResourceView> m_oit_composite_srv;

	ComPtr<ID3D11Texture2D>           m_oit_frag_list_head;
	ComPtr<ID3D11ShaderResourceView>  m_oit_frag_list_head_srv;
	ComPtr<ID3D11UnorderedAccessView> m_oit_frag_list_head_uav;

	ComPtr<ID3D11Texture2D>           m_oit_frag_list_count;
	ComPtr<ID3D11ShaderResourceView>  m_oit_frag_list_count_srv;
	ComPtr<ID3D11UnorderedAccessView> m_oit_frag_list_count_uav;

	ComPtr<ID3D11Buffer>              m_oit_frag_list_nodes;
	ComPtr<ID3D11ShaderResourceView>  m_oit_frag_list_nodes_srv;
	ComPtr<ID3D11UnorderedAccessView> m_oit_frag_list_nodes_uav;

	std::unordered_map<DWORD, Direct3DTexture8*> m_textures;
	std::unordered_map<DWORD, SamplerSettings> m_sampler_setting_values;
	std::array<dirty_t<DWORD>, 174> m_render_state_values;
	std::unordered_map<ShaderFlags::type, ComPtr<ID3D11InputLayout>> m_fvf_layouts;
	std::unordered_map<DWORD, StreamPair> m_stream_sources;

	dirty_t<uint32_t> m_raster_flags;
	std::unordered_map<uint32_t, ComPtr<ID3D11RasterizerState>> m_raster_states;

	std::unordered_map<SamplerSettings, ComPtr<ID3D11SamplerState>> m_sampler_states;

	dirty_t<uint32_t> m_blend_flags;
	std::unordered_map<uint32_t, ComPtr<ID3D11BlendState>> m_blend_states;

	ComPtr<Direct3DIndexBuffer8> m_current_index_buffer = nullptr;

	std::deque<ComPtr<Direct3DVertexBuffer8>> m_up_vertex_buffers;
	[[nodiscard]] ComPtr<Direct3DVertexBuffer8> get_user_primitive_vertex_buffer(size_t target_size);

	std::deque<ComPtr<Direct3DIndexBuffer8>> m_up_index_buffers;
	[[nodiscard]] ComPtr<Direct3DIndexBuffer8> get_user_primitive_index_buffer(size_t target_size, D3DFORMAT format);

	dirty_t<DWORD> m_fvf_flags;
	ComPtr<Direct3DTexture8> m_depth_stencil;

	DepthStencilFlags m_depth_stencil_flags {};
	std::unordered_map<DepthStencilFlags, ComPtr<ID3D11DepthStencilState>> m_depth_states;

	ComPtr<ID3D11Buffer> m_uber_shader_cbuffer;
	ComPtr<ID3D11Buffer> m_per_scene_cbuffer;
	ComPtr<ID3D11Buffer> m_per_model_cbuffer;
	ComPtr<ID3D11Buffer> m_per_pixel_cbuffer;
	ComPtr<ID3D11Buffer> m_per_texture_cbuffer;

	UberShaderFlagsBuffer m_uber_shader_flags {};
	PerSceneBuffer m_per_scene {};
	PerModelBuffer m_per_model {};
	PerPixelBuffer m_per_pixel {};
	TextureStages m_per_texture {};

	INT m_current_base_vertex_index = 0;
	//const BOOL ZBufferDiscarding = FALSE;
	DWORD m_current_vertex_shader_handle = 0;
	//DWORD CurrentPixelShaderHandle = 0;
	bool m_palette_flag = false;

	static constexpr size_t MAX_CLIP_PLANES = 6;
	float m_stored_clip_planes[MAX_CLIP_PLANES][4] = {};
	//DWORD ClipPlaneRenderState = 0;

	D3D11_VIEWPORT m_viewport {};
	D3DMATERIAL8 m_material {};

	std::vector<D3D_SHADER_MACRO> m_draw_prologue_epilogue_preproc;
};
