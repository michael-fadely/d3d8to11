#pragma once

#include "d3d8types.hpp"
#include "simple_math.h"

class CBufferBase;

struct Light
{
	bool   enabled      = false; /* Light enabled state */
	int    type         = 0;     /* Type of light source */
	float4 diffuse      = {};    /* Diffuse color of light */
	float4 specular     = {};    /* Specular color of light */
	float4 ambient      = {};    /* Ambient color of light */
	float3 position     = {};    /* Position in world space */
	float3 direction    = {};    /* Direction in world space */
	float  range        = 0.0f;  /* Cutoff range */
	float  falloff      = 0.0f;  /* Falloff */
	float  attenuation0 = 0.0f;  /* Constant attenuation */
	float  attenuation1 = 0.0f;  /* Linear attenuation */
	float  attenuation2 = 0.0f;  /* Quadratic attenuation */
	float  theta        = 0.0f;  /* Inner angle of spotlight cone */
	float  phi          = 0.0f;  /* Outer angle of spotlight cone */

	Light() = default;

	void copy(const D3DLIGHT8& rhs);

	bool operator==(const Light& rhs) const;
	bool operator!=(const Light& rhs) const;
};

CBufferBase& operator<<(CBufferBase& writer, const Light& l);
