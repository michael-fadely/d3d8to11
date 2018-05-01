// Maximum number of fragments to be sorted per pixel
#ifndef MAX_FRAGMENTS
#define MAX_FRAGMENTS 16
#endif

#define D3DBLEND_ZERO            1
#define D3DBLEND_ONE             2
#define D3DBLEND_SRCCOLOR        3
#define D3DBLEND_INVSRCCOLOR     4
#define D3DBLEND_SRCALPHA        5
#define D3DBLEND_INVSRCALPHA     6
#define D3DBLEND_DESTALPHA       7
#define D3DBLEND_INVDESTALPHA    8
#define D3DBLEND_DESTCOLOR       9
#define D3DBLEND_INVDESTCOLOR    10
#define D3DBLEND_SRCALPHASAT     11
#define D3DBLEND_BOTHSRCALPHA    12
#define D3DBLEND_BOTHINVSRCALPHA 13

// Magic number to consider a null-entry.
static const uint FRAGMENT_LIST_NULL = 0xFFFFFFFF;

// Fragment list node.
struct OitNode
{
	float depth; // fragment depth
	uint  color; // 32-bit packed fragment color
	uint  flags; // source blend, destination blend, blend operation
	uint  next;  // index of the next entry, or FRAGMENT_LIST_NULL
};

// TODO: per-pixel link count
// TODO: test append buffer again, increase max fragments

#ifdef NODE_WRITE

// Read/write mode.

globallycoherent RWTexture2D<uint>           FragListHead  : register(u1);
globallycoherent RWStructuredBuffer<OitNode> FragListNodes : register(u2);

#else

// Read-only mode.

Texture2D<uint>           FragListHead  : register(t0);
StructuredBuffer<OitNode> FragListNodes : register(t1);
Texture2D                 BackBuffer    : register(t2);
Texture2D                 DepthBuffer   : register(t3);

#endif

// from D3DX_DXGIFormatConvert.inl

uint float_to_uint(float _V, float _Scale)
{
	return (uint)floor(_V * _Scale + 0.5f);
}

float4 unorm_to_float4(uint packedInput)
{
	precise float4 unpackedOutput;
	unpackedOutput.x = (float)(packedInput & 0x000000ff) / 255;
	unpackedOutput.y = (float)(((packedInput >> 8) & 0x000000ff)) / 255;
	unpackedOutput.z = (float)(((packedInput >> 16) & 0x000000ff)) / 255;
	unpackedOutput.w = (float)(packedInput >> 24) / 255;
	return unpackedOutput;
}

uint float4_to_unorm(precise float4 unpackedInput)
{
	uint packedOutput;
	packedOutput = ((float_to_uint(saturate(unpackedInput.x), 255)) |
		(float_to_uint(saturate(unpackedInput.y), 255) << 8) |
		(float_to_uint(saturate(unpackedInput.z), 255) << 16) |
		(float_to_uint(saturate(unpackedInput.w), 255) << 24));
	return packedOutput;
}
