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
		throw std::runtime_error("data size must be 4-byte aligned.");
	}

	align(size);
	memcpy(&ptr[offset()], data, size);
	add(size);
}

size_t ICBuffer::cbuffer_size() const
{
	CBufferDummy cbuff;
	write(cbuff);
	return cbuff.offset();
}

CBufferBase& CBufferBase::operator<<(const CBufferAlign& align_of)
{
	align(align_of.size);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const uint32_t& data)
{
	write(data);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const float& data)
{
	write(data);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Matrix& data)
{
	write(data);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector2& data)
{
	write(data);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector3& data)
{
	write(data);
	return *this;
}

template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector4& data)
{
	write(data);
	return *this;
}
