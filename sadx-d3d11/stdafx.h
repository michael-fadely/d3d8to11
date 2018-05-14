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

// Windows
#include <Windows.h>
#include <VersionHelpers.h>
#include <WTypes.h>

// DirectX 11
#include <d3d11_1.h>
#include <d3d8types.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// GSL
#include <gsl/span>

// DirectXTK
#include <wrl/client.h>
#include <SimpleMath.h>

// Mod Loader
#include <SADXModLoader.h>
#include <MemAccess.h>
#include <Trampoline.h>

// Local
#include "cbuffers.h"
#include "CBufferWriter.h"
#include "d3d8to9.hpp"
#include "d3d8to9_base.h"
#include "d3d8to9_device.h"
#include "d3d8to9_index_buffer.h"
#include "d3d8to9_resource.h"
#include "d3d8to9_surface.h"
#include "d3d8to9_texture.h"
#include "d3d8to9_vertex_buffer.h"
#include "d3d8types.hpp"
#include "defs.h"
#include "dirty_t.h"
#include "globals.h"
#include "hash_combine.h"
#include "int_multiple.h"
#include "Light.h"
#include "Material.h"
#include "PolyBuff11.h"
#include "Shader.h"
#include "simple_math.h"
#include "SimpleMath.h"
#include "typedefs.h"
#include "Unknown.h"

DataPointer(Direct3DDevice8*, Direct3D_Device, 0x03D128B0);
