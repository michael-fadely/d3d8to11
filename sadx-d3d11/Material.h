#pragma once
#include "CBufferWriter.h"

struct Material
{
	float4 Diffuse  = {};   /* Diffuse color RGBA */
	float4 Ambient  = {};   /* Ambient color RGB */
	float4 Specular = {};   /* Specular 'shininess' */
	float4 Emissive = {};   /* Emissive color RGB */
	float  Power    = 0.0f; /* Sharpness if specular highlight */

	Material() = default;

	explicit Material(const D3DMATERIAL8& rhs);

	Material& operator=(const D3DMATERIAL8& rhs);

	bool operator==(const Material& rhs) const;
};

CBufferBase& operator<<(CBufferBase& writer, const Material& material);
