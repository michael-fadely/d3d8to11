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

	// TODO: blend with actual backbuffer
	float4 final = BackBuffer[input.position.xy];

	for (int l = 0; l < count; l++)
	{
		float4 color = unorm_to_float4(fragments[l].color);
		final = (color.a * color) + ((1.0 - color.a) * final);
	}

	return float4(final.rgb, 1);
}
