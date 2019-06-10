#include "stdafx.h"
#include "cbuffers.h"

void PerSceneBuffer::write(CBufferBase& cbuf) const
{
	cbuf << this->viewMatrix << this->projectionMatrix << this->screenDimensions << this->viewPosition;
}

bool PerSceneBuffer::dirty() const
{
	return viewMatrix.dirty() || projectionMatrix.dirty() ||
	       screenDimensions.dirty() || viewPosition.dirty();
}

void PerSceneBuffer::clear()
{
	viewMatrix.clear();
	projectionMatrix.clear();
	screenDimensions.clear();
	viewPosition.clear();
}

void PerSceneBuffer::mark()
{
	viewMatrix.mark();
	projectionMatrix.mark();
	screenDimensions.mark();
	viewPosition.mark();
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

void PerModelBuffer::write(CBufferBase& cbuf) const
{
	cbuf << worldMatrix << wvMatrixInvT << textureMatrix
		<< materialSources << ambient << colorVertex;

	for (const auto& light : lights)
	{
		cbuf << CBufferAlign() << light;
	}

	cbuf << CBufferAlign() << material;
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

	return worldMatrix.dirty() || wvMatrixInvT.dirty() || textureMatrix.dirty() ||
	       material.dirty() || materialSources.dirty() || ambient.dirty() || colorVertex.dirty();
}

void PerModelBuffer::clear()
{
	for (auto& light : lights)
	{
		light.clear();
	}

	worldMatrix.clear();
	wvMatrixInvT.clear();
	textureMatrix.clear();
	material.clear();
	materialSources.clear();
	ambient.clear();
	colorVertex.clear();
}

void PerModelBuffer::mark()
{
	for (auto& light : lights)
	{
		light.mark();
	}

	worldMatrix.mark();
	wvMatrixInvT.mark();
	textureMatrix.mark();
	material.mark();
	materialSources.mark();
	ambient.mark();
	colorVertex.mark();
}

void PerPixelBuffer::write(CBufferBase& cbuf) const
{
	cbuf << fogMode << fogStart << fogEnd << fogDensity << fogColor
		<< alphaReject << alphaRejectMode << alphaRejectThreshold;
}

bool PerPixelBuffer::dirty() const
{
	return fogMode.dirty() ||
	       fogStart.dirty() ||
	       fogEnd.dirty() ||
	       fogDensity.dirty() ||
	       fogColor.dirty() ||
	       alphaReject.dirty() ||
	       alphaRejectMode.dirty() ||
	       alphaRejectThreshold.dirty();
}

void PerPixelBuffer::clear()
{
	fogMode.clear();
	fogStart.clear();
	fogEnd.clear();
	fogDensity.clear();
	fogColor.clear();
	alphaReject.clear();
	alphaRejectMode.clear();
	alphaRejectThreshold.clear();
}

void PerPixelBuffer::mark()
{
	fogMode.mark();
	fogStart.mark();
	fogEnd.mark();
	fogDensity.mark();
	fogColor.mark();
	alphaReject.mark();
	alphaRejectMode.mark();
	alphaRejectThreshold.mark();
}

void PerPixelBuffer::set_color(uint color)
{
	fogColor = to_color4(color);
}

bool TextureStage::dirty() const
{
	return colorOp.dirty() ||
	       colorArg1.dirty() ||
	       colorArg2.dirty() ||
	       alphaOp.dirty() ||
	       alphaArg1.dirty() ||
	       alphaArg2.dirty() ||
	       bumpEnvMat00.dirty() ||
	       bumpEnvMat01.dirty() ||
	       bumpEnvMat10.dirty() ||
	       bumpEnvMat11.dirty() ||
	       texCoordIndex.dirty() ||
	       bumpEnvLScale.dirty() ||
	       bumpEnvLOffset.dirty() ||
	       textureTransformFlags.dirty() ||
	       colorArg0.dirty() ||
	       alphaArg0.dirty();
}

void TextureStage::clear()
{
	colorOp.clear();
	colorArg1.clear();
	colorArg2.clear();
	alphaOp.clear();
	alphaArg1.clear();
	alphaArg2.clear();
	bumpEnvMat00.clear();
	bumpEnvMat01.clear();
	bumpEnvMat10.clear();
	bumpEnvMat11.clear();
	texCoordIndex.clear();
	bumpEnvLScale.clear();
	bumpEnvLOffset.clear();
	textureTransformFlags.clear();
	colorArg0.clear();
	alphaArg0.clear();
}

void TextureStage::mark()
{
	colorOp.mark();
	colorArg1.mark();
	colorArg2.mark();
	alphaOp.mark();
	alphaArg1.mark();
	alphaArg2.mark();
	bumpEnvMat00.mark();
	bumpEnvMat01.mark();
	bumpEnvMat10.mark();
	bumpEnvMat11.mark();
	texCoordIndex.mark();
	bumpEnvLScale.mark();
	bumpEnvLOffset.mark();
	textureTransformFlags.mark();
	colorArg0.mark();
	alphaArg0.mark();
}

void TextureStages::write(CBufferBase& cbuf) const
{
	for (auto& it : stages)
	{
		cbuf
			<< static_cast<uint>(it.colorOp.data())
			<< it.colorArg1
			<< it.colorArg2
			<< static_cast<uint>(it.alphaOp.data())
			<< it.alphaArg1
			<< it.alphaArg2
			<< it.bumpEnvMat00
			<< it.bumpEnvMat01
			<< it.bumpEnvMat10
			<< it.bumpEnvMat11
			<< it.texCoordIndex
			<< it.bumpEnvLScale
			<< it.bumpEnvLOffset
			<< static_cast<uint>(it.textureTransformFlags.data())
			<< it.colorArg0
			<< it.alphaArg0
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
