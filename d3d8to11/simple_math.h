#pragma once

#include <SimpleMath.h>
#include "typedefs.h"

using float2   = DirectX::SimpleMath::Vector2;
using float3   = DirectX::SimpleMath::Vector3;
using float4   = DirectX::SimpleMath::Vector4;
using matrix   = DirectX::SimpleMath::Matrix;
using float4x4 = DirectX::SimpleMath::Matrix;

/**
 * \brief Converts an RGB or ARGB integer color to float4.
 * \param color The 32-bit integer color to convert to a float4
 * \return The float4 representation of \p color
 */
float4 to_color4(uint color);
