#pragma once

#include <array>
#include <deque>
#include <unordered_map>
#include <wrl/client.h>

#include "Unknown.h"
#include "dirty_t.h"
#include "simple_math.h"
#include "Shader.h"
#include "cbuffers.h"

class Direct3DBaseTexture8;
class Direct3DIndexBuffer8;
class Direct3DTexture8;
class Direct3DVertexBuffer8;
class Direct3DSurface8;

using Direct3DSwapChain8 = void;
using Direct3DCubeTexture8 = void;
using Direct3DVolumeTexture8 = void;

using Microsoft::WRL::ComPtr;

struct ShaderFlags
{
	enum T : uint32_t
	{
		none,
		fvf_rhw     = 0b000000001,
		fvf_normal  = 0b000000010,
		fvf_diffuse = 0b000000100,
		fvf_tex1    = 0b000001000,
		tci_envmap  = 0b000010000,
		rs_lighting = 0b000100000,
		rs_specular = 0b001000000,
		rs_alpha    = 0b010000000,
		rs_fog      = 0b100000000,

		fvf_mask = 0b000001111,
		mask     = 0b111111111,

		count
	};

#ifdef PER_PIXEL
	// TODO
#else
	static constexpr uint32_t vs_mask = fvf_mask | tci_envmap | rs_lighting | rs_specular;
	static constexpr uint32_t ps_mask = fvf_tex1 | rs_alpha | rs_fog;
#endif

	static uint32_t sanitize(uint32_t flags);
};

// TODO: align to 4 bits just 'cause
struct SamplerFlags
{
	enum T
	{
		none,
		u_wrap,
		u_mirror,
		u_clamp,
		u_border,
		u_mirror_once,
		v_wrap        = u_wrap << 3,
		v_mirror      = u_mirror << 3,
		v_clamp       = u_clamp << 3,
		v_border      = u_border << 3,
		v_mirror_once = u_mirror_once << 3,
		w_wrap        = v_wrap << 3,
		w_mirror      = v_mirror << 3,
		w_clamp       = v_clamp << 3,
		w_border      = v_border << 3,
		w_mirror_once = v_mirror_once << 3
	};

	static constexpr auto u_mask = 0b000000111;
	static constexpr auto v_mask = 0b000111000;
	static constexpr auto w_mask = 0b111000000;
};

struct StreamPair
{
	Direct3DVertexBuffer8* buffer;
	UINT stride;
};

struct OitNode
{
	float depth; // fragment depth
	uint  color; // 32-bit packed fragment color
	uint  flags; // source blend, destination blend, blend operation
	uint  next;  // index of the next entry, or FRAGMENT_LIST_NULL
};

class __declspec(uuid("7385E5DF-8FE8-41D5-86B6-D7B48547B6CF")) Direct3DDevice8;

class Direct3DDevice8 : public Unknown
{
public:
	Direct3DDevice8(const Direct3DDevice8&) = delete;
	Direct3DDevice8& operator=(const Direct3DDevice8&) = delete;

	Direct3DDevice8(Direct3D8* d3d, const D3DPRESENT_PARAMETERS8& parameters);
	~Direct3DDevice8() = default;

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

	void print_info_queue() const;

	static std::vector<D3D_SHADER_MACRO> shader_preprocess(uint32_t flags);
	VertexShader get_vs(uint32_t flags);
	PixelShader get_ps(uint32_t flags);
	void create_depth_stencil();
	void create_render_target();
	void create_native();
	bool set_primitive_type(D3DPRIMITIVETYPE PrimitiveType) const;
	static bool primitive_vertex_count(D3DPRIMITIVETYPE PrimitiveType, uint32_t& count);
	bool update_input_layout();
	void commit_per_pixel();
	void commit_per_model();
	void commit_per_scene();
	void update_sampler();
	void compile_shaders(uint32_t flags, VertexShader& vs, PixelShader& ps);
	void update_shaders();
	void update_blend();
	bool update();
	void free_shaders();
	void update_wv_inv_t();

	void oit_load_shaders();
	void oit_release();
	void oit_write();
	void oit_read();
	void oit_init();
	void FragListHead_Init();
	void FragListNodes_Init();

	uint32_t shader_flags = ShaderFlags::none;
	std::unordered_map<uint32_t, VertexShader> vertex_shaders;
	std::unordered_map<uint32_t, PixelShader> pixel_shaders;

	D3DPRESENT_PARAMETERS8 present_params {};
	ComPtr<IDXGISwapChain> swap_chain;

	ComPtr<ID3D11Device> device;
	ComPtr<ID3D11DeviceContext> context;
	ComPtr<ID3D11InfoQueue> info_queue;
	ComPtr<ID3D11RasterizerState> raster_state;

	bool oit_enabled = true;

protected:
	bool oit_enabled_ = true;
	Direct3D8* const d3d;

	VertexShader composite_vs;
	PixelShader composite_ps;

	ComPtr<ID3D11Texture2D>           FragListHeadB;
	ComPtr<ID3D11ShaderResourceView>  FragListHeadSRV;
	ComPtr<ID3D11UnorderedAccessView> FragListHeadUAV;
	ComPtr<ID3D11Buffer>              FragListNodesB;
	ComPtr<ID3D11ShaderResourceView>  FragListNodesSRV;
	ComPtr<ID3D11UnorderedAccessView> FragListNodesUAV;

	std::unordered_map<DWORD, Direct3DTexture8*> texture_stages;
	std::unordered_map<DWORD, std::unordered_map<DWORD, dirty_t<DWORD>>> texture_state_values;
	std::array<dirty_t<DWORD>, 174> render_state_values;
	std::unordered_map<DWORD, ComPtr<ID3D11InputLayout>> fvf_layouts;
	std::unordered_map<DWORD, StreamPair> stream_sources;

	dirty_t<uint32_t> sampler_flags;
	std::unordered_map<SamplerFlags::T, ComPtr<ID3D11SamplerState>> sampler_states;

	dirty_t<uint32_t> blend_flags;
	std::unordered_map<uint32_t, ComPtr<ID3D11BlendState>> blend_states;

	Direct3DIndexBuffer8* index_buffer = nullptr;

	void up_get(size_t target_size);

	ComPtr<Direct3DVertexBuffer8> up_buffer;
	std::deque<ComPtr<Direct3DVertexBuffer8>> up_buffers;

	dirty_t<DWORD> FVF;
	ComPtr<ID3D11Texture2D> depth_texture;
	ComPtr<ID3D11DepthStencilState> depth_state_rw, depth_state_ro;
	ComPtr<ID3D11DepthStencilView> depth_view;
	ComPtr<ID3D11ShaderResourceView> depth_srv;
	ComPtr<ID3D11RenderTargetView> render_target;

	ComPtr<ID3D11Texture2D> composite_texture;
	ComPtr<ID3D11RenderTargetView> composite_view;
	ComPtr<ID3D11ShaderResourceView> composite_srv;

	ComPtr<ID3D11Buffer> per_scene_cbuf, per_model_cbuf, per_pixel_cbuf;
	PerSceneBuffer per_scene {};
	PerModelBuffer per_model {};
	PerPixelBuffer per_pixel {};

	INT CurrentBaseVertexIndex = 0;
	//const BOOL ZBufferDiscarding = FALSE;
	DWORD CurrentVertexShaderHandle = 0;
	//DWORD CurrentPixelShaderHandle = 0;
	bool palette_flag = false;

	static constexpr size_t MAX_CLIP_PLANES = 6;
	float StoredClipPlanes[MAX_CLIP_PLANES][4] = {};
	//DWORD ClipPlaneRenderState = 0;

	D3D11_VIEWPORT viewport {};
	D3DMATERIAL8 material {};
};
