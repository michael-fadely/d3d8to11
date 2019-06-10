// Maximum number of fragments to be sorted per pixel
#ifndef MAX_FRAGMENTS
#define MAX_FRAGMENTS 16
#endif

#define M_PI 3.14159265358979323846

#define CMP_NEVER        1
#define CMP_LESS         2
#define CMP_EQUAL        3
#define CMP_LESSEQUAL    4
#define CMP_GREATER      5
#define CMP_NOTEQUAL     6
#define CMP_GREATEREQUAL 7
#define CMP_ALWAYS       8

// D3DBLEND enum
#define BLEND_ZERO            1
#define BLEND_ONE             2
#define BLEND_SRCCOLOR        3
#define BLEND_INVSRCCOLOR     4
#define BLEND_SRCALPHA        5
#define BLEND_INVSRCALPHA     6
#define BLEND_DESTALPHA       7
#define BLEND_INVDESTALPHA    8
#define BLEND_DESTCOLOR       9
#define BLEND_INVDESTCOLOR    10
#define BLEND_SRCALPHASAT     11

// D3DMCS enum (color source)
#define CS_MATERIAL 0 // Color from material is used
#define CS_COLOR1   1 // Diffuse vertex color is used
#define CS_COLOR2   2 // Specular vertex color is used

// From FixedFuncEMU.fx
// Copyright (c) 2005 Microsoft Corporation. All rights reserved.
#define FOGMODE_NONE   0
#define FOGMODE_EXP    1
#define FOGMODE_EXP2   2
#define FOGMODE_LINEAR 3
#define E              2.71828

#define LIGHT_POINT       1
#define LIGHT_SPOT        2
#define LIGHT_DIRECTIONAL 3

/*
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
globallycoherent RWTexture2D<uint>           FragListCount : register(u2);
globallycoherent RWStructuredBuffer<OitNode> FragListNodes : register(u3);

#else

// Read-only mode.

Texture2D<uint>           FragListHead  : register(t0);
Texture2D<uint>           FragListCount : register(t1);
StructuredBuffer<OitNode> FragListNodes : register(t2);
Texture2D                 BackBuffer    : register(t3);
Texture2D                 DepthBuffer   : register(t4);

#endif
*/

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
