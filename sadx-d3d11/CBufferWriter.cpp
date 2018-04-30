#include "stdafx.h"
#include "CBufferWriter.h"
#include <gsl/span>

static constexpr auto VECTOR_ALIGN = (sizeof(float) * 4);

CBufferWriter::CBufferWriter(uint8_t* ptr_)
	: ptr(ptr_)
{
}

bool CBufferWriter::start_new(size_t size)
{
	if (!write_align)
	{
		return false;
	}

	const auto delta = VECTOR_ALIGN - write_align;

	if (delta >= size)
	{
		return false;
	}

	write_pos += delta;
	write_align = 0;
	return true;
}

void CBufferWriter::write(const void* data, size_t size)
{
	if (size % 4)
	{
		throw;
	}

	start_new(size);

	memcpy(&ptr[write_pos], data, size);
	write_pos += size;
	write_align = (write_align + size) % VECTOR_ALIGN;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const bool& data)
{
	return *this << (data ? 1 : 0);
}

template <>
CBufferWriter& CBufferWriter::operator<<(const int32_t& data)
{
	write(&data, sizeof(int32_t));
	return *this;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const uint32_t& data)
{
	write(&data, sizeof(uint32_t));
	return *this;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const float& data)
{
	write(&data, sizeof(float));
	return *this;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const DirectX::SimpleMath::Matrix& data)
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
CBufferWriter& CBufferWriter::operator<<(const DirectX::SimpleMath::Vector2& data)
{
	const float array[] = { data.x, data.y };
	return *this << array;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const DirectX::SimpleMath::Vector3& data)
{
	const float array[] = { data.x, data.y, data.z };
	return *this << array;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const DirectX::SimpleMath::Vector4& data)
{
	const float array[] = { data.x, data.y, data.z, data.w };
	return *this << array;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const gsl::span<float>& data)
{
	write(&data[0], data.size_bytes());
	return *this;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const gsl::span<const float>& data)
{
	write(&data[0], data.size_bytes());
	return *this;
}
