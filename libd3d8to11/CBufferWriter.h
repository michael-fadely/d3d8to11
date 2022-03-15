#pragma once

#include <d3d11.h>

#include <SimpleMath.h>
#include <gsl/span>
#include <cstdint>
#include "dirty_t.h"

constexpr auto VECTOR_SIZE = sizeof(float) * 4;

class CBufferBase;

struct CBufferAlign
{
	size_t size;

	explicit CBufferAlign(size_t size_ = VECTOR_SIZE)
		: size(size_)
	{
	}
};

class ICBuffer
{
public:
	virtual ~ICBuffer() = default;
	virtual void write(CBufferBase& cbuf) const = 0;

	[[nodiscard]] size_t cbuffer_size() const;

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

	[[nodiscard]] size_t offset() const { return offset_; }
	[[nodiscard]] size_t alignment() const { return alignment_; }

	template <typename T>
	CBufferBase& operator<<(const T& data) = delete;

	template <typename T>
	CBufferBase& operator<<(const gsl::span<T>& data) = delete;

	template <typename T>
	CBufferBase& operator<<(const gsl::span<const T>& data) = delete;

	CBufferBase& operator<<(const CBufferAlign& align_of);

	template <size_t length>
	CBufferBase& operator<<(const std::array<float, length>& array)
	{
		constexpr size_t size_in_bytes = length * sizeof(float);

		if constexpr (length == 1)
		{
			write(array[0]);
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Vector2))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Vector2*>(array.data()));
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Vector3))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Vector3*>(array.data()));
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Vector4))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Vector4*>(array.data()));
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Matrix))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Matrix*>(array.data()));
		}
		else
		{
			write(array.data(), size_in_bytes);
		}

		return *this;
	}

	template <size_t length>
	CBufferBase& operator<<(const float (&array)[length])
	{
		constexpr size_t size_in_bytes = length * sizeof(float);

		if constexpr (length == 1)
		{
			write(array[0]);
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Vector2))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Vector2*>(&array[0]));
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Vector3))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Vector3*>(&array[0]));
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Vector4))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Vector4*>(&array[0]));
		}
		else if constexpr (size_in_bytes == sizeof(DirectX::SimpleMath::Matrix))
		{
			write(*reinterpret_cast<const DirectX::SimpleMath::Matrix*>(&array[0]));
		}
		else
		{
			write(&array[0], size_in_bytes);
		}

		return *this;
	}

	template <typename T, dirty_mode set_mode>
	CBufferBase& operator<<(const dirty_t<T, set_mode>& value)
	{
		return *this << value.data();
	}

	virtual void write(const void* data, size_t size) = 0;
	virtual void write(const uint32_t& data) = 0;
	virtual void write(const float& data) = 0;
	virtual void write(const DirectX::SimpleMath::Matrix& data) = 0;
	virtual void write(const DirectX::SimpleMath::Vector2& data) = 0;
	virtual void write(const DirectX::SimpleMath::Vector3& data) = 0;
	virtual void write(const DirectX::SimpleMath::Vector4& data) = 0;

	inline void write(const int32_t& data)
	{
		write(static_cast<uint32_t>(data));
	}

	inline void write(bool data)
	{
		write(static_cast<uint32_t>(data ? 1 : 0));
	}
};

class CBufferDummy : public CBufferBase
{
private:
	inline void write(const void* data, size_t size) override
	{
		align(size);
		add(size);
	}

	inline void write(const uint32_t& data) override
	{
		write_t(data);
	}

	inline void write(const float& data) override
	{
		write_t(data);
	}

	inline void write(const DirectX::SimpleMath::Matrix& data) override
	{
		write_t(data);
	}

	inline void write(const DirectX::SimpleMath::Vector2& data) override
	{
		write_t(data);
	}

	inline void write(const DirectX::SimpleMath::Vector3& data) override
	{
		write_t(data);
	}

	inline void write(const DirectX::SimpleMath::Vector4& data) override
	{
		write_t(data);
	}

	template <typename T>
	void write_t(const T& data)
	{
		write(&data, sizeof(T));
	}
};

template <>
inline CBufferBase& CBufferBase::operator<<(const int32_t& data)
{
	write(data);
	return *this;
}

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
inline CBufferBase& CBufferBase::operator<<(const DWORD& data)
{
	return *this << static_cast<uint32_t>(data);
}

template <>
inline CBufferBase& CBufferBase::operator<<(const bool& data)
{
	return *this << static_cast<uint32_t>(data ? 1 : 0);
}

class CBufferWriter : public CBufferBase
{
	uint8_t* ptr = nullptr;

public:
	explicit CBufferWriter(uint8_t* ptr_);

	void write(const void* data, size_t size) override;

	void write(const uint32_t& data) override
	{
		write_t(data);
	}

	void write(const float& data) override
	{
		write_t(data);
	}

	void write(const DirectX::SimpleMath::Matrix& data) override
	{
		write_t(data);
	}

	void write(const DirectX::SimpleMath::Vector2& data) override
	{
		write_t(data);
	}

	void write(const DirectX::SimpleMath::Vector3& data) override
	{
		write_t(data);
	}

	void write(const DirectX::SimpleMath::Vector4& data) override
	{
		write_t(data);
	}

	template <typename T>
	void write_t(const T& data)
	{
		align(sizeof(T));
		*reinterpret_cast<T*>(&ptr[offset()]) = data;
		add(sizeof(T));
	}
};
