#pragma once

#include <gsl/span>

class CBufferWriter
{
	uint8_t* ptr = nullptr;

	size_t write_pos   = 0;
	size_t write_align = 0;

public:
	CBufferWriter(uint8_t* ptr_);

	template <typename T>
	CBufferWriter& operator<<(const T& data) = delete;

	template <typename T>
	CBufferWriter& operator<<(gsl::span<const T> data) = delete;

private:
	void write(const void* data, size_t size);
};

template <>
CBufferWriter& CBufferWriter::operator<<(const bool& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const int& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const float& data);
template <>
CBufferWriter& CBufferWriter::operator<<(gsl::span<const float> data);
