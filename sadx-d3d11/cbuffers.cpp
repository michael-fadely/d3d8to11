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

void PerModelBuffer::write(CBufferBase& cbuf) const
{
	cbuf << worldMatrix << textureMatrix;

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

	return worldMatrix.dirty() || textureMatrix.dirty() || material.dirty();
}

void PerModelBuffer::clear()
{
	for (auto& light : lights)
	{
		light.clear();
	}

	worldMatrix.clear();
	textureMatrix.clear();
	material.clear();
}

void PerModelBuffer::mark()
{
	for (auto& light : lights)
	{
		light.mark();
	}

	worldMatrix.mark();
	textureMatrix.mark();
	material.mark();
}

void PerPixelBuffer::write(CBufferBase& cbuf) const
{
	cbuf << srcBlend << destBlend << fogMode
		<< fogStart << fogEnd << fogDensity << fogColor;
}

bool PerPixelBuffer::dirty() const
{
	return srcBlend.dirty() ||
		   destBlend.dirty() ||
		   fogMode.dirty() ||
		   fogStart.dirty() ||
		   fogEnd.dirty() ||
		   fogDensity.dirty() ||
		   fogColor.dirty();
}

void PerPixelBuffer::clear()
{
	srcBlend.clear();
	destBlend.clear();
	fogMode.clear();
	fogStart.clear();
	fogEnd.clear();
	fogDensity.clear();
	fogColor.clear();
}

void PerPixelBuffer::mark()
{
	srcBlend.mark();
	destBlend.mark();
	fogMode.mark();
	fogStart.mark();
	fogEnd.mark();
	fogDensity.mark();
	fogColor.mark();
}

void PerPixelBuffer::set_color(uint color)
{
	float4 fcolor;

	fcolor.x = ((color >> 16) & 0xFF) / 255.0f;
	fcolor.y = ((color >> 8) & 0xFF) / 255.0f;
	fcolor.z = (color & 0xFF) / 255.0f;
	fcolor.w = ((color >> 24) & 0xFF) / 255.0f;

	fogColor = fcolor;
}
