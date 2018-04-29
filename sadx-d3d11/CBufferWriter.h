#pragma once

#include <gsl/span>

class CBufferWriter
{
	uint8_t* ptr = nullptr;

	size_t write_pos   = 0;
	size_t write_align = 0;

public:
	CBufferWriter(uint8_t* ptr_);
	bool start_new(size_t size = 4 * sizeof(float));

	template <typename T>
	CBufferWriter& operator<<(const T& data) = delete;

	template <typename T>
	CBufferWriter& operator<<(const gsl::span<T>& data) = delete;

	template <typename T>
	CBufferWriter& operator<<(const gsl::span<const T>& data) = delete;

	template <typename T, size_t size>
	CBufferWriter& operator<<(const std::array<T, size>& array)
	{
		return *this << gsl::span<const T>(array);
	}

	template <typename T, size_t size>
	CBufferWriter& operator<<(const T(&array)[size])
	{
		return *this << gsl::span<const T>(array);
	}

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
CBufferWriter& CBufferWriter::operator<<(const DirectX::SimpleMath::Matrix& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const gsl::span<float>& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const gsl::span<const float>& data);
