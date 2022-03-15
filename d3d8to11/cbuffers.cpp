#include "stdafx.h"
#include "cbuffers.h"

void UberShaderFlagsBuffer::write(CBufferBase& cbuff) const
{
	cbuff << rs_lighting
	      << rs_specular
	      << rs_alpha
	      << rs_fog
	      << rs_oit;
}

bool UberShaderFlagsBuffer::dirty() const
{
	return rs_lighting.dirty() ||
	       rs_specular.dirty() ||
	       rs_alpha.dirty() ||
	       rs_fog.dirty() ||
	       rs_oit.dirty();
}

void UberShaderFlagsBuffer::clear()
{
	rs_lighting.clear();
	rs_specular.clear();
	rs_alpha.clear();
	rs_fog.clear();
	rs_oit.clear();
}

void UberShaderFlagsBuffer::mark()
{
	rs_lighting.mark();
	rs_specular.mark();
	rs_alpha.mark();
	rs_fog.mark();
	rs_oit.mark();
}

void PerSceneBuffer::write(CBufferBase& cbuff) const
{
	cbuff << this->view_matrix
	      << this->projection_matrix
	      << this->screen_dimensions
	      << this->view_position
	      << this->oit_buffer_length;
}

bool PerSceneBuffer::dirty() const
{
	return view_matrix.dirty() ||
	       projection_matrix.dirty() ||
	       screen_dimensions.dirty() ||
	       view_position.dirty() ||
	       oit_buffer_length.dirty();
}

void PerSceneBuffer::clear()
{
	view_matrix.clear();
	projection_matrix.clear();
	screen_dimensions.clear();
	view_position.clear();
	oit_buffer_length.clear();
}

void PerSceneBuffer::mark()
{
	view_matrix.mark();
	projection_matrix.mark();
	screen_dimensions.mark();
	view_position.mark();
	oit_buffer_length.mark();
}

bool MaterialSources::dirty() const
{
	return diffuse.dirty() ||
	       specular.dirty() ||
	       ambient.dirty() ||
	       emissive.dirty();
}

void MaterialSources::clear()
{
	diffuse.clear();
	specular.clear();
	ambient.clear();
	emissive.clear();
}

void MaterialSources::mark()
{
	diffuse.mark();
	specular.mark();
	ambient.mark();
	emissive.mark();
}

inline CBufferBase& operator<<(CBufferBase& buffer, const MaterialSources& data)
{
	return buffer << CBufferAlign()
	       << data.diffuse.data()
	       << data.specular.data()
	       << data.ambient.data()
	       << data.emissive.data();
}

void PerModelBuffer::write(CBufferBase& cbuff) const
{
	cbuff << draw_call << world_matrix << wv_matrix_inv_t
		<< material_sources << ambient << color_vertex;

	for (const auto& light : lights)
	{
		cbuff << CBufferAlign() << light;
	}

	cbuff << CBufferAlign() << material;
}

bool PerModelBuffer::dirty() const
{
	for (const auto& light : lights)
	{
		if (light.dirty())
		{
			return true;
		}
	}

	return draw_call.dirty() ||
	       world_matrix.dirty() ||
	       wv_matrix_inv_t.dirty() ||
	       material.dirty() ||
	       material_sources.dirty() ||
	       ambient.dirty() ||
	       color_vertex.dirty();
}

void PerModelBuffer::clear()
{
	for (auto& light : lights)
	{
		light.clear();
	}

	draw_call.clear();
	world_matrix.clear();
	wv_matrix_inv_t.clear();
	material.clear();
	material_sources.clear();
	ambient.clear();
	color_vertex.clear();
}

void PerModelBuffer::mark()
{
	for (auto& light : lights)
	{
		light.mark();
	}

	draw_call.mark();
	world_matrix.mark();
	wv_matrix_inv_t.mark();
	material.mark();
	material_sources.mark();
	ambient.mark();
	color_vertex.mark();
}

void PerPixelBuffer::write(CBufferBase& cbuff) const
{
	cbuff << src_blend << dst_blend << blend_op
	      << fog_mode << fog_start << fog_end << fog_density << fog_color
	      << alpha_reject << alpha_reject_mode << alpha_reject_threshold << texture_factor;
}

bool PerPixelBuffer::dirty() const
{
	return src_blend.dirty() ||
	       dst_blend.dirty() ||
	       blend_op.dirty() ||
	       fog_mode.dirty() ||
	       fog_start.dirty() ||
	       fog_end.dirty() ||
	       fog_density.dirty() ||
	       fog_color.dirty() ||
	       alpha_reject.dirty() ||
	       alpha_reject_mode.dirty() ||
	       alpha_reject_threshold.dirty() ||
	       texture_factor.dirty();
}

void PerPixelBuffer::clear()
{
	src_blend.clear();
	dst_blend.clear();
	blend_op.clear();
	fog_mode.clear();
	fog_start.clear();
	fog_end.clear();
	fog_density.clear();
	fog_color.clear();
	alpha_reject.clear();
	alpha_reject_mode.clear();
	alpha_reject_threshold.clear();
	texture_factor.clear();
}

void PerPixelBuffer::mark()
{
	src_blend.mark();
	dst_blend.mark();
	blend_op.mark();
	fog_mode.mark();
	fog_start.mark();
	fog_end.mark();
	fog_density.mark();
	fog_color.mark();
	alpha_reject.mark();
	alpha_reject_mode.mark();
	alpha_reject_threshold.mark();
	texture_factor.mark();
}

bool TextureStage::dirty() const
{
	return bound.dirty() ||
	       transform.dirty() ||
	       color_op.dirty() ||
	       color_arg1.dirty() ||
	       color_arg2.dirty() ||
	       alpha_op.dirty() ||
	       alpha_arg1.dirty() ||
	       alpha_arg2.dirty() ||
	       bump_env_mat00.dirty() ||
	       bump_env_mat01.dirty() ||
	       bump_env_mat10.dirty() ||
	       bump_env_mat11.dirty() ||
	       tex_coord_index.dirty() ||
	       bump_env_lscale.dirty() ||
	       bump_env_loffset.dirty() ||
	       texture_transform_flags.dirty() ||
	       color_arg0.dirty() ||
	       alpha_arg0.dirty() ||
	       result_arg.dirty();
}

void TextureStage::clear()
{
	bound.clear();
	transform.clear();
	color_op.clear();
	color_arg1.clear();
	color_arg2.clear();
	alpha_op.clear();
	alpha_arg1.clear();
	alpha_arg2.clear();
	bump_env_mat00.clear();
	bump_env_mat01.clear();
	bump_env_mat10.clear();
	bump_env_mat11.clear();
	tex_coord_index.clear();
	bump_env_lscale.clear();
	bump_env_loffset.clear();
	texture_transform_flags.clear();
	color_arg0.clear();
	alpha_arg0.clear();
	result_arg.clear();
}

void TextureStage::mark()
{
	bound.mark();
	transform.mark();
	color_op.mark();
	color_arg1.mark();
	color_arg2.mark();
	alpha_op.mark();
	alpha_arg1.mark();
	alpha_arg2.mark();
	bump_env_mat00.mark();
	bump_env_mat01.mark();
	bump_env_mat10.mark();
	bump_env_mat11.mark();
	tex_coord_index.mark();
	bump_env_lscale.mark();
	bump_env_loffset.mark();
	texture_transform_flags.mark();
	color_arg0.mark();
	alpha_arg0.mark();
	result_arg.mark();
}

void TextureStages::write(CBufferBase& cbuff) const
{
	for (auto& it : stages)
	{
		cbuff
			<< it.bound
			<< it.transform
			<< static_cast<uint>(it.color_op.data())
			<< it.color_arg1
			<< it.color_arg2
			<< static_cast<uint>(it.alpha_op.data())
			<< it.alpha_arg1
			<< it.alpha_arg2
			<< it.bump_env_mat00
			<< it.bump_env_mat01
			<< it.bump_env_mat10
			<< it.bump_env_mat11
			<< it.tex_coord_index
			<< it.bump_env_lscale
			<< it.bump_env_loffset
			<< static_cast<uint>(it.texture_transform_flags.data())
			<< it.color_arg0
			<< it.alpha_arg0
			<< it.result_arg
			<< CBufferAlign();
	}
}

bool TextureStages::dirty() const
{
	for (auto& it : stages)
	{
		if (it.dirty())
		{
			return true;
		}
	}

	return false;
}

void TextureStages::clear()
{
	for (auto& it : stages)
	{
		it.clear();
	}
}

void TextureStages::mark()
{
	for (auto& it : stages)
	{
		it.mark();
	}
}
