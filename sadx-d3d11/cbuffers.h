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
	dirty_t<matrix> view_matrix;
	dirty_t<matrix> projection_matrix;
	dirty_t<float2> screen_dimensions;
	dirty_t<float3> view_position;

	void write(CBufferBase& cbuf) const override;

	[[nodiscard]] bool dirty() const override;
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

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerModelBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<matrix>                         world_matrix;
	dirty_t<matrix>                         wv_matrix_inv_t;
	dirty_t<matrix>                         texture_matrix;
	std::array<dirty_t<Light>, LIGHT_COUNT> lights;
	dirty_t<Material>                       material;
	MaterialSources                         material_sources;
	dirty_t<float4>                         ambient;
	dirty_t<bool>                           color_vertex = dirty_t<bool>(true);

	void write(CBufferBase& cbuf) const override;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class PerPixelBuffer : public ICBuffer, dirty_impl
{
public:
	dirty_t<uint>   fog_mode;
	dirty_t<float>  fog_start;
	dirty_t<float>  fog_end;
	dirty_t<float>  fog_density;
	dirty_t<float4> fog_color;
	dirty_t<bool>   alpha_reject;
	dirty_t<uint>   alpha_reject_mode;
	dirty_t<float>  alpha_reject_threshold;
	dirty_t<float4> texture_factor;

	void write(CBufferBase& cbuf) const override;

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};


struct TextureStage : dirty_impl
{
	dirty_t<bool>                     bound;
	dirty_t<D3DTEXTUREOP>             color_op;
	dirty_t<uint>                     color_arg1; // D3DTA
	dirty_t<uint>                     color_arg2; // D3DTA
	dirty_t<D3DTEXTUREOP>             alpha_op;
	dirty_t<uint>                     alpha_arg1; // D3DTA
	dirty_t<uint>                     alpha_arg2; // D3DTA
	dirty_t<float>                    bump_env_mat00;
	dirty_t<float>                    bump_env_mat01;
	dirty_t<float>                    bump_env_mat10;
	dirty_t<float>                    bump_env_mat11;
	dirty_t<uint>                     tex_coord_index; // D3DTSS_TCI
	dirty_t<float>                    bump_env_lscale;
	dirty_t<float>                    bump_env_loffset;
	dirty_t<D3DTEXTURETRANSFORMFLAGS> texture_transform_flags;
	dirty_t<uint>                     color_arg0; // D3DTA
	dirty_t<uint>                     alpha_arg0; // D3DTA

	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};

class TextureStages : public ICBuffer, dirty_impl
{
public:
	std::array<TextureStage, TEXTURE_STAGE_COUNT> stages {};
	void write(CBufferBase& cbuf) const override;
	[[nodiscard]] bool dirty() const override;
	void clear() override;
	void mark() override;
};
