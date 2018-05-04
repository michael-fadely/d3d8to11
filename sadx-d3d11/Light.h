#pragma once

#include "d3d8types.hpp"
#include "simple_math.h"

class CBufferBase;

struct Light
{
	bool   Enabled      = false; /* Light enabled state */
	int    Type         = 0;     /* Type of light source */
	float4 Diffuse      = {};    /* Diffuse color of light */
	float4 Specular     = {};    /* Specular color of light */
	float4 Ambient      = {};    /* Ambient color of light */
	float3 Position     = {};    /* Position in world space */
	float3 Direction    = {};    /* Direction in world space */
	float  Range        = 0.0f;  /* Cutoff range */
	float  Falloff      = 0.0f;  /* Falloff */
	float  Attenuation0 = 0.0f;  /* Constant attenuation */
	float  Attenuation1 = 0.0f;  /* Linear attenuation */
	float  Attenuation2 = 0.0f;  /* Quadratic attenuation */
	float  Theta        = 0.0f;  /* Inner angle of spotlight cone */
	float  Phi          = 0.0f;  /* Outer angle of spotlight cone */

	         Light() = default;
	explicit Light(const D3DLIGHT8& rhs);

	Light& operator=(const D3DLIGHT8& rhs);

	bool operator==(const Light& rhs) const;
	bool operator!=(const Light& rhs) const;
};

CBufferBase& operator<<(CBufferBase& writer, const Light& l);
