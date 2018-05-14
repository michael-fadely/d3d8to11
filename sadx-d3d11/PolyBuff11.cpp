#include "stdafx.h"
#include <deque>
#include <stack>
#include <Trampoline.h>
#include "PolyBuff11.h"
#include "int_multiple.h"

#pragma pack(push, 1)
struct PolyBuff_RenderArgs
{
	uint32_t StartVertex;
	uint32_t PrimitiveCount;
	uint32_t CullMode;
	Direct3DVertexBuffer8* buffer;
};

struct PolyBuffEx;

struct PolyBuff
{
	void*                pStreamData; // will be unused
	uint32_t             TotalSize; // ditto
	uint32_t             CurrentSize; // ditto
	uint32_t             Stride; // ditto
	uint32_t             FVF;
	PolyBuff_RenderArgs* RenderArgs;
	uint32_t             LockCount;
	const char*          name;
	PolyBuffEx*          ex;
};
#pragma pack(pop)

struct PolyBuffEx
{
	PolyBuff const* parent;
	std::deque<Direct3DVertexBuffer8*> free_buffers;
	std::deque<Direct3DVertexBuffer8*> used_buffers;
	std::stack<Direct3DVertexBuffer8*> live_buffers;

	PolyBuffEx(PolyBuff* parent_)
		: parent(parent_)
	{
	}

	~PolyBuffEx()
	{
		for (auto& ptr : free_buffers)
		{
			ptr->Release();
		}

		for (auto& ptr : used_buffers)
		{
			ptr->Release();
		}
	}

	void discard()
	{
		if (!used_buffers.empty())
		{
			free_buffers.insert(free_buffers.end(), used_buffers.begin(), used_buffers.end());
			used_buffers.clear();
		}
	}

	Direct3DVertexBuffer8* get(uint32_t target_size)
	{
		for (auto it = free_buffers.begin(); it != free_buffers.end(); ++it)
		{
			if ((*it)->desc8.Size >= target_size)
			{
				auto result = *it;
				free_buffers.erase(it);
				used_buffers.push_back(result);
				return result;
			}
		}

		PrintDebug("%s is allocating bytes (%u + 1): %u\n", parent->name, free_buffers.size() + used_buffers.size(), target_size);

		Direct3DVertexBuffer8* result;
		auto hr = Direct3D_Device->CreateVertexBuffer(int_multiple(target_size, 16), D3DUSAGE_DYNAMIC, parent->FVF, D3DPOOL_MANAGED, &result);

		if (FAILED(hr))
		{
			return nullptr;
		}

		used_buffers.push_back(result);
		return result;
	}

	void lock(Direct3DVertexBuffer8* b)
	{
		live_buffers.push(b);
	}

	HRESULT unlock()
	{
		auto top = live_buffers.top();
		live_buffers.pop();
		return top->Unlock();
	}
};

void __fastcall     PolyBuff_Init(PolyBuff*              _this, uint32_t count, uint32_t stride, uint32_t FVF, const char* name);
void __fastcall     PolyBuff_Free(PolyBuff*              _this);
void __fastcall     PolyBuff_Discard(PolyBuff*           _this);
void __fastcall     PolyBuff_SetCurrent(PolyBuff*        _this);
uint8_t* __fastcall PolyBuff_LockTriangleStrip(PolyBuff* _this, uint32_t primitives, uint32_t cullmode);
uint8_t* __fastcall PolyBuff_LockTriangleList(PolyBuff*  _this, uint32_t primitives, uint32_t cullmode);
HRESULT __fastcall  PolyBuff_Unlock(PolyBuff*            _this);
void __fastcall     PolyBuff_DrawTriangleStrip(PolyBuff* _this);
void __fastcall     PolyBuff_DrawTriangleList(PolyBuff*  _this);

Trampoline PolyBuff_Init_t(0x00794540, 0x00794546, PolyBuff_Init);
Trampoline PolyBuff_Free_t(0x007945C0, 0x007945C6, PolyBuff_Free);
Trampoline PolyBuff_Discard_t(0x00794800, 0x00794806, PolyBuff_Discard);
Trampoline PolyBuff_SetCurrent_t(0x00794600, 0x00794605, PolyBuff_SetCurrent);
Trampoline PolyBuff_LockTriangleStrip_t(0x00794630, 0x00794636, PolyBuff_LockTriangleStrip);
Trampoline PolyBuff_LockTriangleList_t(0x007946C0, 0x007946C6, PolyBuff_LockTriangleList);
Trampoline PolyBuff_Unlock_t(0x00794750, 0x00794755, PolyBuff_Unlock);
Trampoline PolyBuff_DrawTriangleStrip_t(0x00794760, 0x00794767, PolyBuff_DrawTriangleStrip);
Trampoline PolyBuff_DrawTriangleList_t(0x007947B0, 0x007947B7, PolyBuff_DrawTriangleList);

#define TARGET(NAME) reinterpret_cast<decltype((NAME))*>((NAME) ## _t.Target())  // NOLINT(misc-macro-parentheses)

void __fastcall PolyBuff_Init(PolyBuff* _this, uint32_t count, uint32_t stride, uint32_t FVF, const char* name)
{
	PrintDebug("PolyBuff_Init %s: count: %d - stride: %d\n", name, count, stride);

	int _count = count;

	if (!_this)
	{
		return;
	}

	*_this = {};

	_this->Stride      = stride;
	_this->TotalSize   = stride * count;
	_this->CurrentSize = stride * count;
	_this->FVF         = FVF;
	_this->name        = name;

#if 0
	Direct3D_Device->CreateVertexBuffer(
		int_multiple(512 * stride, 16),
		D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY,
		FVF,
		D3DPOOL_MANAGED,
		&_this->pStreamData);
#else
	_this->ex = new PolyBuffEx(_this);
#endif

	_this->RenderArgs = new PolyBuff_RenderArgs[count]{};
	_this->LockCount = 0;

	if (_count)
	{
		int i = 0;
		do
		{
			_this->RenderArgs[i].StartVertex = 0;
			_this->RenderArgs[i].PrimitiveCount = 0;
			++i;
			--_count;
		} while (_count);
	}
}

void __fastcall PolyBuff_Free(PolyBuff* _this)
{
	if (!_this)
	{
		return;
	}

#if 0
	if (_this->pStreamData)
	{
		_this->pStreamData->Release();
	}
#else
	delete _this->ex;
#endif

	delete[] _this->RenderArgs;
	*_this = {};
}

void __fastcall PolyBuff_Discard(PolyBuff *_this)
{
	if (!_this)
	{
		return;
	}

	if (_this->ex)
	{
		_this->ex->discard();
	}
}

void __fastcall PolyBuff_SetCurrent(PolyBuff* _this)
{
	Direct3D_Device->SetVertexShader(_this->FVF);
}

uint8_t* __fastcall PolyBuff_LockTriangleStrip(PolyBuff* _this, uint32_t primitives, uint32_t cullmode)
{
	primitives = std::max(3u, primitives);
	uint32_t current = _this->CurrentSize;
	uint32_t stride  = _this->Stride;
	uint32_t size = primitives * stride;

	_this->CurrentSize = current - size;

	PolyBuff_RenderArgs* args = &_this->RenderArgs[_this->LockCount];

	args->PrimitiveCount = primitives - 2;
	args->StartVertex    = 0;
	args->CullMode       = cullmode;

	++_this->LockCount;

	auto buffer = _this->ex->get(size);
	args->buffer = buffer;

	BYTE* result = nullptr;

	buffer->Lock(0, size, &result, D3DLOCK_DISCARD);
	_this->ex->lock(buffer);

	return result;
}

uint8_t* __fastcall PolyBuff_LockTriangleList(PolyBuff* _this, uint32_t primitives, uint32_t cullmode)
{
	primitives = std::max(3u, primitives);
	uint32_t current = _this->CurrentSize;
	uint32_t stride  = _this->Stride;
	uint32_t size    = (1 + primitives) * stride;

	_this->CurrentSize = current - size;

	PolyBuff_RenderArgs* args = &_this->RenderArgs[_this->LockCount];

	args->PrimitiveCount = primitives / 3;
	args->StartVertex    = 0;
	args->CullMode       = cullmode;

	++_this->LockCount;

	auto buffer = _this->ex->get(size);
	args->buffer = buffer;

	BYTE* result = nullptr;

	buffer->Lock(0, size, &result, D3DLOCK_DISCARD);
	_this->ex->lock(buffer);

	return result;
}

HRESULT __fastcall PolyBuff_Unlock(PolyBuff* _this)
{
	return _this->ex->unlock();
}

void __fastcall PolyBuff_DrawTriangleStrip(PolyBuff* _this)
{
	for (size_t i = 0; i < _this->LockCount; i++)
	{
		const auto& args = _this->RenderArgs[i];
		Direct3D_Device->SetStreamSource(0, args.buffer, _this->Stride);
		Direct3D_Device->SetRenderState(D3DRS_CULLMODE, args.CullMode);
		Direct3D_Device->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, args.PrimitiveCount);
	}

	_this->LockCount = 0;
	_this->ex->discard();
}

void __fastcall PolyBuff_DrawTriangleList(PolyBuff* _this)
{
	for (size_t i = 0; i < _this->LockCount; i++)
	{
		const auto& args = _this->RenderArgs[i];
		Direct3D_Device->SetStreamSource(0, args.buffer, _this->Stride);
		Direct3D_Device->SetRenderState(D3DRS_CULLMODE, args.CullMode);
		Direct3D_Device->DrawPrimitive(D3DPT_TRIANGLELIST, 0, args.PrimitiveCount);
	}

	_this->LockCount = 0;
	_this->ex->discard();
}
