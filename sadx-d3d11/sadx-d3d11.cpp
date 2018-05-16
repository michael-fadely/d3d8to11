#include "stdafx.h"
#include "globals.h"

extern "C"
{
	EXPORT ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };
	EXPORT void __cdecl Init(const char* path, HelperFunctions& helpers)
	{
		globals::helper_functions = helpers;

		auto d3d9 = GetModuleHandle(L"d3d9.dll");
		if (d3d9)
		{
			FreeLibrary(d3d9);
		}

		auto d3d8 = GetModuleHandle(L"d3d8.dll");
		if (d3d8)
		{
			FreeLibrary(d3d8);
		}

		std::string inipath(path);
		inipath.append("\\config.ini");
		globals::max_fragments = GetPrivateProfileIntA("ppll", "maxFragCount", MAX_FRAGMENTS_DEFAULT, inipath.c_str());

		if (globals::max_fragments != MAX_FRAGMENTS_DEFAULT)
		{
			PrintDebug("Maximum fragments: %u\n", globals::max_fragments);
		}

		WriteJump(reinterpret_cast<void*>(0x007C235E), Direct3DCreate8);

		// HACK: I sure am glad SADX draws a triangle fan over the screen every frame with which it does literally nothing
		WriteData<5>(reinterpret_cast<void*>(0x0078B922), 0x90i8);
	}

	EXPORT void __cdecl OnFrame()
	{
		if (ControllerPointers[0])
		{
			if (ControllerPointers[0]->PressedButtons & Buttons_D)
			{
				Direct3D_Device->free_shaders();
				Direct3D_Device->oit_load_shaders();
			}

			if (ControllerPointers[0]->PressedButtons & Buttons_C)
			{
				Direct3D_Device->oit_enabled = !Direct3D_Device->oit_enabled;
			}
		}

		auto color = DebugFontColor;
		auto size = DebugFontSize;
		DebugFontSize = 12.0f * std::min(HorizontalStretch, VerticalStretch);

		SetDebugFontColor(Direct3D_Device->oit_enabled ? 0xFF00FF00 : 0xFFFF0000);
		DisplayDebugStringFormatted(NJM_LOCATION(1, 1), "OIT: %s", Direct3D_Device->oit_enabled ? "ON" : "OFF");

		SetDebugFontColor(color.color);
		DebugFontSize = size;
	}
}
