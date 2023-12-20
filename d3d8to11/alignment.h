#pragma once

template <typename T>
T constexpr align_up(T value, size_t alignment)
{
	if (!value || alignment < 2)
	{
		return value;
	}

	value += alignment - 1;
	value -= value % alignment;
	return value;
}

template <typename T>
T constexpr align_down(T value, size_t alignment)
{
	if (!value || alignment < 2)
	{
		return value;
	}

	value -= value % alignment;
	return value;
}

template <typename T>
constexpr T round_pow2(T value)
{
	static_assert(sizeof(T) <= sizeof(uint64_t), "Integer size is too large!");

	--value;

	if constexpr (sizeof(T) >= 1)
	{
		value |= value >> 1;
		value |= value >> 2;
		value |= value >> 4;
	}

	if constexpr (sizeof(T) >= 2)
	{
		value |= value >> 8;
	}

	if constexpr (sizeof(T) >= 4)
	{
		value |= value >> 16;
	}

	if constexpr (sizeof(T) >= 8)
	{
		value |= value >> 32;
	}

	++value;

	return value;
}
