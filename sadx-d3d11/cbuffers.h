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
	dirty_t<matrix>   viewMatrix;
	dirty_t<matrix>   projectionMatrix;
	dirty_t<float2>   screenDimensions;
	dirty_t<float3>   viewPosition;

	void write(CBufferBase& cbuf) const override;

	bool dirty() const override;
	void clear() override;
	void mark() override;
};

class MaterialSources : dirty_impl
{
public:
	dirty_t<uint> diffuse;
	dirty_t<uint> specular;
	dirty_t<uint> ambient;
	dirty_t<uint> emissive;

	bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerModelBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<matrix>                         worldMatrix;
	dirty_t<matrix>                         wvMatrixInvT;
	dirty_t<matrix>                         textureMatrix;
	std::array<dirty_t<Light>, LIGHT_COUNT> lights;
	dirty_t<Material>                       material;
	MaterialSources                         materialSources;
	dirty_t<float4>                         ambient;
	dirty_t<bool>                           colorVertex = dirty_t<bool>(true);

	void write(CBufferBase& cbuf) const override;

	bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerPixelBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<uint>   fogMode;
	dirty_t<float>  fogStart;
	dirty_t<float>  fogEnd;
	dirty_t<float>  fogDensity;
	dirty_t<float4> fogColor;
	dirty_t<bool>   alphaReject;
	dirty_t<uint>   alphaRejectMode;
	dirty_t<float>  alphaRejectThreshold;
	dirty_t<float4> textureFactor;

	void write(CBufferBase& cbuf) const override;

	bool dirty() const override;
	void clear() override;
	void mark() override;

	void set_color(uint color);
};


struct TextureStage : public dirty_impl
{
	dirty_t<bool>                     bound;
	dirty_t<D3DTEXTUREOP>             colorOp;
	dirty_t<uint>                     colorArg1; // D3DTA
	dirty_t<uint>                     colorArg2; // D3DTA
	dirty_t<D3DTEXTUREOP>             alphaOp;
	dirty_t<uint>                     alphaArg1; // D3DTA
	dirty_t<uint>                     alphaArg2; // D3DTA
	dirty_t<float>                    bumpEnvMat00;
	dirty_t<float>                    bumpEnvMat01;
	dirty_t<float>                    bumpEnvMat10;
	dirty_t<float>                    bumpEnvMat11;
	dirty_t<uint>                     texCoordIndex; // D3DTSS_TCI
	dirty_t<float>                    bumpEnvLScale;
	dirty_t<float>                    bumpEnvLOffset;
	dirty_t<D3DTEXTURETRANSFORMFLAGS> textureTransformFlags;
	dirty_t<uint>                     colorArg0; // D3DTA
	dirty_t<uint>                     alphaArg0; // D3DTA

	bool dirty() const override;
	void clear() override;
	void mark() override;
};

class TextureStages : public ICBuffer, dirty_impl
{
public:
	std::array<TextureStage, TEXTURE_STAGE_COUNT> stages {};
	void write(CBufferBase& cbuf) const override;
	bool dirty() const override;
	void clear() override;
	void mark() override;
};
