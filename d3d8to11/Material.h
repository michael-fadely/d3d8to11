#pragma once
#include <CBufferWriter.h>

struct Material
{
	float4 diffuse  = {};   /* Diffuse color RGBA */
	float4 ambient  = {};   /* Ambient color RGB */
	float4 specular = {};   /* Specular 'shininess' */
	float4 emissive = {};   /* Emissive color RGB */
	float  power    = 0.0f; /* Sharpness if specular highlight */

	Material() = default;

	explicit Material(const D3DMATERIAL8& rhs);

	Material& operator=(const D3DMATERIAL8& rhs);

	bool operator==(const Material& rhs) const;
};

CBufferBase& operator<<(CBufferBase& writer, const Material& material);
