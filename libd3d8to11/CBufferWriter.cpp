#include "CBufferWriter.h"

#include <stdexcept>

CBufferWriter::CBufferWriter(uint8_t* ptr)
	: m_ptr(ptr)
{
}

bool CBufferBase::align(size_t size)
{
	if (!m_alignment)
	{
		return false;
	}

	const auto delta = VECTOR_SIZE - m_alignment;

	if (delta >= size)
	{
		return false;
	}

	m_offset += delta;
	m_alignment = 0;
	return true;
}

void CBufferBase::add(size_t size)
{
	m_offset += size;
	m_alignment = (m_alignment + size) % VECTOR_SIZE;
}

void CBufferBase::reset()
{
	m_offset    = 0;
	m_alignment = 0;
}

void CBufferWriter::write(const void* data, size_t size)
{
	if (size % 4)
	{
		throw std::runtime_error("data size must be 4-byte aligned.");
	}

	align(size);
	memcpy(&m_ptr[offset()], data, size);
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
