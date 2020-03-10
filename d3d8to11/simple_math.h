#pragma once

#include <array>
#include <SimpleMath.h>
#include "typedefs.h"

using float2   = DirectX::SimpleMath::Vector2;
using float3   = DirectX::SimpleMath::Vector3;
using float4   = DirectX::SimpleMath::Vector4;
using matrix   = DirectX::SimpleMath::Matrix;
using float4x4 = DirectX::SimpleMath::Matrix;
using int2     = std::array<int, 2>;
using int3     = std::array<int, 3>;
using int4     = std::array<int, 4>;
using uint2    = std::array<uint, 2>;
using uint3    = std::array<uint, 3>;
using uint4    = std::array<uint, 4>;

/**
 * \brief Converts an RGB or ARGB integer color to float4.
 * \param color The 32-bit integer color to convert to a float4
 * \return The float4 representation of \p color
 */
float4 to_color4(uint color);
