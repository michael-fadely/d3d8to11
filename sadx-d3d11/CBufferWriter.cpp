#include "stdafx.h"
#include "CBufferWriter.h"
#include <gsl/span>

static constexpr auto VECTOR_ALIGN = (sizeof(float) * 4);

CBufferWriter::CBufferWriter(uint8_t* ptr_)
	: ptr(ptr_)
{
}

void CBufferWriter::write(const void* data, size_t size)
{
	if (size % 4)
	{
		throw;
	}

	if (write_align)
	{
		auto delta = VECTOR_ALIGN - write_align;
		if (delta < size)
		{
			write_pos += delta;
			write_align = 0;
		}
	}

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
CBufferWriter& CBufferWriter::operator<<(const int& data)
{
	write(&data, sizeof(int));
	return *this;
}

template <>
CBufferWriter& CBufferWriter::operator<<(const float& data)
{
	write(&data, sizeof(float));
	return *this;
}

template <>
CBufferWriter& CBufferWriter::operator<<(gsl::span<const float> data)
{
	write(&data[0], data.size_bytes());
	return *this;
}