#include "pch.h"
#include "simple_math.h"

float4 to_color4(uint color)
{
	float4 color4;

	color4.x = static_cast<float>((color >> 16) & 0xFF) / 255.0f;
	color4.y = static_cast<float>((color >> 8) & 0xFF)  / 255.0f;
	color4.z = static_cast<float>(color & 0xFF)         / 255.0f;
	color4.w = static_cast<float>((color >> 24) & 0xFF) / 255.0f;

	return color4;
}
