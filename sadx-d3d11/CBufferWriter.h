#pragma once

#include <gsl/span>
#include "simple_math.h"

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
	__forceinline CBufferWriter& operator<<(const std::array<T, size>& array)
	{
		return *this << gsl::span<const T>(array);
	}

	template <typename T, size_t size>
	__forceinline CBufferWriter& operator<<(const T(&array)[size])
	{
		return *this << gsl::span<const T>(array);
	}

	template <typename T>
	__forceinline CBufferWriter& operator<<(const dirty_t<T>& value)
	{
		return *this << value.data();
	}

private:
	void write(const void* data, size_t size);
};

template <>
CBufferWriter& CBufferWriter::operator<<(const int32_t& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const uint32_t& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const float& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const Matrix& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const Vector2& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const Vector3& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const Vector4& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const gsl::span<float>& data);
template <>
CBufferWriter& CBufferWriter::operator<<(const gsl::span<const float>& data);

template <>
__forceinline CBufferWriter& CBufferWriter::operator<<(const DWORD& data)
{
	return *this << static_cast<uint32_t>(data);
}

template <>
__forceinline CBufferWriter& CBufferWriter::operator<<(const bool& data)
{
	return *this << (data ? 1 : 0);
}
