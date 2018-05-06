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

float4 get_blend_factor(uint mode, float4 source, float4 destination)
{
	switch (mode)
	{
		default: // error state
			return float4(1, 0, 0, 1);
		case BLEND_ZERO:
			return float4(0, 0, 0, 0);
		case BLEND_ONE:
			return float4(1, 1, 1, 1);
		case BLEND_SRCCOLOR:
			return source;
		case BLEND_INVSRCCOLOR:
			return 1.0f - source;
		case BLEND_SRCALPHA:
			return source.aaaa;
		case BLEND_INVSRCALPHA:
			return 1.0f - source.aaaa;
		case BLEND_DESTALPHA:
			return destination.aaaa;
		case BLEND_INVDESTALPHA:
			return 1.0f - destination.aaaa;
		case BLEND_DESTCOLOR:
			return destination;
		case BLEND_INVDESTCOLOR:
			return 1.0f - destination;
		case BLEND_SRCALPHASAT:
			float f = min(source.a, 1 - destination.a);
			return float4(f, f, f, 1);
	}
}

float4 blend_colors(uint srcBlend, uint dstBlend, float4 sourceColor, float4 destinationColor)
{
	float4 src = get_blend_factor(srcBlend, sourceColor, destinationColor);
	float4 dst = get_blend_factor(dstBlend, sourceColor, destinationColor);
	return (sourceColor * src) + (destinationColor * dst);
}

float4 ps_main(VertexOutput input) : SV_TARGET
{
	float4 backBufferColor = BackBuffer[input.position.xy];
	
	uint head = FragListHead[input.position.xy];

	if (head == FRAGMENT_LIST_NULL)
	{
		return backBufferColor;
	}

	OitNode fragments[MAX_FRAGMENTS];

	int count = 0;
	uint index = head;

	// Counts the number of stored fragments for this index
	// until a null entry is found or we've reached the maximum
	// allowed sortable fragments.
	for (int i = 0; i < MAX_FRAGMENTS; i++)
	{
		fragments[count] = FragListNodes[index];

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

		while (fragments[j - 1].depth <= t.depth)
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

	float4 final = backBufferColor;
	float opaqueDepth = DepthBuffer[input.position.xy].r;

	for (int l = 0; l < count; l++)
	{
		if (fragments[l].depth > opaqueDepth)
		{
			continue;
		}

		uint blend = fragments[l].flags;

		uint srcBlend = blend >> 8;
		uint destBlend = blend & 0xFF;

		float4 color = unorm_to_float4(fragments[l].color);
		final = blend_colors(srcBlend, destBlend, color, final);
	}

	return float4(final.rgb, 1);
}
