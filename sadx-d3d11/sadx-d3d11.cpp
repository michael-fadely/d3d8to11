#include "stdafx.h"
#include "globals.h"

/*
	 object:
		[x] InitDirect3D8
		[x] Direct3D_SetupPresentParameters
		[x] Direct3D_CheckAdapters
		[x] CreateDirect3DDevice

	device:
		[ ] ApplyLSPalette
		[ ] CharSel_Qmark_Main
		[ ] ChunkTextureFlip
		[ ] CnkDisableSpecular
		[ ] CnkRestoreSpecular
		[ ] CopyConstantMaterial
		[ ] CorrectMaterial
		[ ] CreateDirect3DDevice
		[ ] Direct3D_ApplyFogTable
		[ ] Direct3D_BeginScene
		[ ] Direct3D_Clear
		[ ] Direct3D_ConfigureFog
		[ ] Direct3D_ConfigureLights
		[ ] Direct3D_CreateTextureWithExistingMipmap
		[ ] Direct3D_CreateTextureWithoutMipmap
		[ ] Direct3D_DiffuseSourceVertexColor
		[ ] Direct3D_DisableFog
		[ ] Direct3D_DrawFVF_H
		[ ] Direct3D_DrawQuad
		[ ] Direct3D_DrawSpriteTable
		[ ] Direct3D_DrawTriangleFan2D
		[ ] Direct3D_EnableAlpha
		[ ] Direct3D_EnableFog
		[ ] Direct3D_EnableHudAlpha
		[ ] Direct3D_EnableLighting
		[ ] Direct3D_EnableZWrite
		[ ] Direct3D_EndScene
		[ ] Direct3D_EndScene_Release
		[ ] Direct3D_EnvironmentMap
		[ ] Direct3D_ParseMaterial
		[ ] Direct3D_PerformLighting
		[ ] Direct3D_Present
		[ ] Direct3D_ProbablyAppliesPalette
		[ ] Direct3D_ResetTextureTransform
		[ ] Direct3D_ResetWorldTransform
		[ ] Direct3D_SetChunkModelRenderState
		[ ] Direct3D_SetCullMode
		[ ] Direct3D_SetDefaultRenderState
		[ ] Direct3D_SetDefaultTextureStageState
		[ ] Direct3D_SetNJSTexture
		[ ] Direct3D_SetNearFarPlanes
		[ ] Direct3D_SetNullTexture
		[ ] Direct3D_SetNullTextureStage
		[ ] Direct3D_SetProjectionMatrix
		[ ] Direct3D_SetTextureToStage
		[ ] Direct3D_SetViewportAndTransform
		[ ] Direct3D_SetWorldTransform
		[ ] Direct3D_SetZFunc
		[ ] Direct3D_SetupPresentParameters
		[ ] Direct3D_TextureFilterLinear
		[ ] Direct3D_TextureFilterPoint
		[ ] Direct3D_UnsetChunkModelRenderState
		[ ] DisplayDebugShape_
		[ ] DrawChaoHudThingB
		[ ] DrawChunkObject
		[ ] DrawRect_TextureVertexTriangleStrip
		[ ] DrawSomeTriangleFanThing
		[ ] InitDirect3D8
		[ ] MeshSetBuffer_CreateVertexBuffer
		[ ] ParseNjControl3D
		[ ] PolyBuff_DrawTriangleList
		[ ] PolyBuff_DrawTriangleStrip
		[ ] PolyBuff_Init
		[ ] PolyBuff_SetCurrent
		[ ] SetDisplayAdapterMaybe
		[ ] SetOceanAlphaModeAndFVF
		[ ] njAlphaMode
		[ ] njColorBlendingMode_
		[ ] njDrawLine2D_Direct3D
		[ ] njDrawLine3D_Direct3D
		[ ] njDrawModel_SADX
		[ ] njDrawPolygon
		[ ] njDrawSprite3D_DrawNow
		[ ] njDrawTriangle2D_List
		[ ] njDrawTriangle2D_Strip
		[ ] njReleaseTextureLow
		[ ] njSetCnkBlendMode
		[ ] njSetFogColor_
		[ ] njTextureShadingMode
		[ ] sub_5031E0
		[ ] sub_506E10
		[ ] sub_506E40
		[ ] sub_725A30
		[ ] sub_7270E0
		[ ] sub_7472E0
		[ ] sub_769320
		[ ] sub_76AD40
		[ ] sub_77EBA0
		[ ] sub_789690
		[ ] sub_7896F0
		[ ] sub_78A1F0
		[ ] sub_78A260
		[ ] sub_78A330
		[ ] sub_78A530
		[ ] sub_78B2F0
		[ ] sub_78BB50
		[ ] sub_78CB90
		[ ] sub_78E3D0
		[ ] sub_78F1B0
		[ ] sub_78FC90
		[ ] sub_795B90
		[ ] sub_795C90
		[ ] sub_795DB0
		[ ] sub_795EE0
		[ ] sub_796020
		[ ] sub_796180
		[ ] sub_7962E0
		[ ] sub_796440
 */

extern "C"
{
	EXPORT ModInfo SADXModInfo = { ModLoaderVer, nullptr, nullptr, 0, nullptr, 0, nullptr, 0, nullptr, 0 };
	EXPORT void __cdecl Init(const char *path, HelperFunctions& helpers)
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

		WriteJump(reinterpret_cast<void*>(0x007C235E), Direct3DCreate8);
	}

	EXPORT void __cdecl OnFrame()
	{
		if (ControllerPointers[0])
		{
			if (ControllerPointers[0]->PressedButtons & Buttons_Z)
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
		DisplayDebugStringFormatted(NJM_LOCATION(1,1), "OIT: %s", Direct3D_Device->oit_enabled ? "ON" : "OFF");

		SetDebugFontColor(color.color);
		DebugFontSize = size;
	}
}