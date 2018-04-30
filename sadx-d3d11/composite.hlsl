#include "include.hlsli"

/*
	This shader draws a full screen triangle onto
	which it composites the stored alpha fragments.
*/

struct VertexOutput
{
	float4 position : SV_POSITION;
};

VertexOutput vs_main(uint vertexId : SV_VertexID)
{
	VertexOutput output;

	float2 texcoord = float2((vertexId << 1) & 2, vertexId & 2);
	output.position = float4(texcoord * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
	return output;
}

float4 get_blend_factor(in uint mode, in float4 source, in float4 destination)
{
	switch (mode)
	{
		default: // error state
			return float4(1, 0, 0, 1);
		case D3DBLEND_ZERO:
			return float4(0, 0, 0, 0);
		case D3DBLEND_ONE:
			return float4(1, 1, 1, 1);
		case D3DBLEND_SRCCOLOR:
			return source;
		case D3DBLEND_INVSRCCOLOR:
			return 1.0f - source;
		case D3DBLEND_SRCALPHA:
			return source.aaaa;
		case D3DBLEND_INVSRCALPHA:
			return 1.0f - source.aaaa;
		case D3DBLEND_DESTALPHA:
			return destination.aaaa;
		case D3DBLEND_INVDESTALPHA:
			return 1.0f - destination.aaaa;
		case D3DBLEND_DESTCOLOR:
			return destination;
		case D3DBLEND_INVDESTCOLOR:
			return 1.0f - destination;
		case D3DBLEND_SRCALPHASAT:
			float f = min(source.a, 1 - destination.a);
			return float4(f, f, f, 1);
	}
}

float4 blend_colors(in uint srcBlend, in uint dstBlend, float4 texel, float4 pixel)
{
	float4 result;

	float4 src = get_blend_factor(srcBlend, texel, pixel);
	float4 dst = get_blend_factor(dstBlend, texel, pixel);
	return (texel * src) + (pixel * dst);
}

float4 ps_main(VertexOutput input) : SV_TARGET
{
	uint2 pos = uint2(input.position.xy);
	uint head = FragListHead[pos];

	if (head == FRAGMENT_LIST_NULL)
	{
		discard;
	}

	OitNode fragments[MAX_FRAGMENTS];

	int count = 0;
	uint index = head;

	// Counts the number of stored fragments for this index
	// until a null entry is found or we've reached the maximum
	// allowed sortable fragments.
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		fragments[count].color = FragListNodes[index].color;
		fragments[count].depth = FragListNodes[index].depth;

		++count;
		index = FragListNodes[index].next;

		if (index == FRAGMENT_LIST_NULL)
		{
			break;
		}
	}

#ifndef DISABLE_SORT
	// Performs an insertion sort to sort the fragments by depth.
	for (int k = 1; k < count; k++)
	{
		int j = k;
		OitNode t = fragments[k];

		while (fragments[j - 1].depth < t.depth)
		{
			fragments[j] = fragments[j - 1];
			j--;

			if (j <= 0)
			{
				break;
			}
		}

		if (j != k)
		{
			fragments[j] = t;
		}
	}
#endif

	float4 final = BackBuffer[input.position.xy];

	for (int l = 0; l < count; l++)
	{
		uint blend = fragments[l].flags;

		if (blend == 0)
		{
			return float4(1, 0, 0, 1);
		}

		uint srcBlend = blend >> 8;
		uint destBlend = blend & 0xFF;

		float4 color = unorm_to_float4(fragments[l].color);
		final = blend_colors(srcBlend, destBlend, color, final);
	}

	return float4(final.rgb, 1);
}
