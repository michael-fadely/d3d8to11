#pragma once

#include "simple_math.h"
#include "dirty_t.h"
#include "CBufferWriter.h"
#include "Light.h"
#include "Material.h"
#include "defs.h"

class PerSceneBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<matrix> viewMatrix;
	dirty_t<matrix> projectionMatrix;
	dirty_t<float2> screenDimensions;
	dirty_t<float3> viewPosition;

	void write(CBufferBase& cbuf) const override;

	bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerModelBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<matrix>                         worldMatrix;
	dirty_t<matrix>                         textureMatrix;
	std::array<dirty_t<Light>, LIGHT_COUNT> lights;
	dirty_t<Material>                       material;

	void write(CBufferBase& cbuf) const override;

	bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerPixelBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<uint>   srcBlend;
	dirty_t<uint>   destBlend;
	dirty_t<uint>   fogMode;
	dirty_t<float>  fogStart;
	dirty_t<float>  fogEnd;
	dirty_t<float>  fogDensity;
	dirty_t<float4> fogColor;

	void write(CBufferBase& cbuf) const override;

	bool dirty() const override;
	void clear() override;
	void mark() override;

	void set_color(uint color);
};
