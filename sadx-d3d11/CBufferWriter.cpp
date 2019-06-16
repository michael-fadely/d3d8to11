#include "stdafx.h"
#include "CBufferWriter.h"

CBufferWriter::CBufferWriter(uint8_t* ptr_)
	: ptr(ptr_)
{
}

bool CBufferBase::align(size_t size)
{
	if (!alignment_)
	{
		return false;
	}

	const auto delta = VECTOR_SIZE - alignment_;

	if (delta >= size)
	{
		return false;
	}

	offset_ += delta;
	alignment_ = 0;
	return true;
}

void CBufferBase::add(size_t size)
{
	offset_ += size;
	alignment_ = (alignment_ + size) % VECTOR_SIZE;
}

void CBufferBase::reset()
{
	offset_    = 0;
	alignment_ = 0;
}

void CBufferWriter::write(const void* data, size_t size)
{
	if (size % 4)
	{
		throw;
	}

	align(size);
	memcpy(&ptr[offset()], data, size);
	add(size);
}

size_t ICBuffer::cbuffer_size() const
{
	CBufferDummy cbuf;
	write(cbuf);
	return cbuf.offset();
}

CBufferBase& CBufferBase::operator<<(const CBufferAlign& align_of)
{
	align(align_of.size);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const int32_t& data)
{
	write(&data, sizeof(int32_t));
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const uint32_t& data)
{
	write(&data, sizeof(uint32_t));
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const float& data)
{
	write(&data, sizeof(float));
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Matrix& data)
{
	const float array[] = {
		data._11, data._12, data._13, data._14,
		data._21, data._22, data._23, data._24,
		data._31, data._32, data._33, data._34,
		data._41, data._42, data._43, data._44,
	};

	return *this << array;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector2& data)
{
	const float array[] = { data.x, data.y };
	return *this << array;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector3& data)
{
	const float array[] = { data.x, data.y, data.z };
	return *this << array;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector4& data)
{
	const float array[] = { data.x, data.y, data.z, data.w };
	return *this << array;
}
