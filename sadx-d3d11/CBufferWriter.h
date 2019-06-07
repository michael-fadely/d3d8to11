#pragma once

#include <gsl/span>

constexpr auto VECTOR_SIZE = sizeof(float) * 4;

class CBufferBase;

struct CBufferAlign
{
	size_t size;
	explicit CBufferAlign(size_t size_ = VECTOR_SIZE) : size(size_) {}
};

__forceinline CBufferAlign cbuff_align(size_t size = VECTOR_SIZE)
{
	return CBufferAlign(size);
}

class ICBuffer
{
public:
	virtual ~ICBuffer() = default;
	virtual void write(CBufferBase& cbuf) const = 0;

	size_t cbuffer_size() const;

	template <typename T>
	static size_t cbuffer_size()
	{
		T t;
		return t.cbuffer_size();
	}
};

class CBufferBase
{
protected:
	size_t offset_ = 0;
	size_t alignment_ = 0;

public:
	virtual ~CBufferBase() = default;
	bool align(size_t size = VECTOR_SIZE);
	void add(size_t size);
	void reset();

	size_t offset() const { return offset_; }
	size_t alignment() const { return alignment_; }

	template <typename T>
	CBufferBase& operator<<(const T& data) = delete;

	template <typename T>
	CBufferBase& operator<<(const gsl::span<T>& data) = delete;

	template <typename T>
	CBufferBase& operator<<(const gsl::span<const T>& data) = delete;

	CBufferBase& operator<<(const CBufferAlign& align_of);

	template <size_t size>
	CBufferBase& operator<<(const std::array<float, size>& array)
	{
		write(array.data(), size * sizeof(float));
		return *this;
	}

	template <size_t size>
	CBufferBase& operator<<(const float(&array)[size])
	{
		write(&array[0], size * sizeof(float));
		return *this;
	}

	template <typename T>
	CBufferBase& operator<<(const dirty_t<T>& value)
	{
		return *this << value.data();
	}

	virtual void write(const void* data, size_t size) = 0;
};

class CBufferDummy : public CBufferBase
{
public:
	void write(const void* data, size_t size) override
	{
		align(size);
		add(size);
	}
};

template <>
CBufferBase& CBufferBase::operator<<(const int32_t& data);
template <>
CBufferBase& CBufferBase::operator<<(const uint32_t& data);
template <>
CBufferBase& CBufferBase::operator<<(const float& data);
template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Matrix& data);
template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector2& data);
template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector3& data);
template <>
CBufferBase& CBufferBase::operator<<(const DirectX::SimpleMath::Vector4& data);

template <>
__forceinline CBufferBase& CBufferBase::operator<<(const DWORD& data)
{
	return *this << static_cast<uint32_t>(data);
}

template <>
__forceinline CBufferBase& CBufferBase::operator<<(const bool& data)
{
	return *this << (data ? 1 : 0);
}

class CBufferWriter : public CBufferBase
{
	uint8_t* ptr = nullptr;

public:
	CBufferWriter(uint8_t* ptr_);

private:
	void write(const void* data, size_t size) override;
};
