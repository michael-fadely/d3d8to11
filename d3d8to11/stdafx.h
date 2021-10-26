#pragma once

#define EXPORT __declspec(dllexport)

#define WIN32_LEAN_AND_MEAN

// STL
#include <array>
#include <cassert>
#include <cstdint>
#include <deque>
#include <exception>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <shared_mutex>
#include <mutex>

// Windows
#include <Windows.h>
#include <VersionHelpers.h>
#include <WTypes.h>

// DirectX 11
#include <d3d11_1.h>
#include "d3d8types.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>

// GSL
#include <gsl/span>

// DirectXTK
#include <wrl/client.h>
#include <SimpleMath.h>

// Local
#include "alignment.h"
#include "cbuffers.h"
#include "CBufferWriter.h"
#include "ShaderFlags.h"
#include "d3d8to11.hpp"
#include "d3d8to11_base.h"
#include "d3d8to11_device.h"
#include "d3d8to11_index_buffer.h"
#include "d3d8to11_resource.h"
#include "d3d8to11_surface.h"
#include "d3d8to11_texture.h"
#include "d3d8to11_vertex_buffer.h"
#include "d3d8types.hpp"
#include "defs.h"
#include <dirty_t.h>
#include "hash_combine.h"
#include "Light.h"
#include "Material.h"
#include "Shader.h"
#include "simple_math.h"
#include "SimpleMath.h"
#include "typedefs.h"
#include "Unknown.h"
#include "dynarray.h"
