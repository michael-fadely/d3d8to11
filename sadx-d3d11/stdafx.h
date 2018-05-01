#pragma once

#define EXPORT __declspec(dllexport)

// Windows
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <atlbase.h>


// Direct3D
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

// DirectXTK
#include <SimpleMath.h>
#include <wrl/client.h>

// Mod Loader
#include <SADXModLoader.h>
#include <Trampoline.h>

// STL
#include <string>
#include <sstream>
#include <stdexcept>
#include <vector>
#include <unordered_map>

// Local
#include "typedefs.h"
#include "d3d8to9.hpp"

DataPointer(Direct3DDevice8*, Direct3D_Device, 0x03D128B0);
