#pragma once

template <typename T>
T constexpr align_up(T value, int alignment)
{
	if (!value || alignment < 2)
	{
		return value;
	}

	value += alignment - (value % alignment);
	return value;
}

template <typename T>
T constexpr align_down(T value, int alignment)
{
	if (!value || alignment < 2)
	{
		return value;
	}

	value -= value % alignment;
	return value;
}

template <typename T>
T round_pow2(T value)
{
	--value;
	value |= value >> 1;
	value |= value >> 2;
	value |= value >> 4;
	value |= value >> 8;
	value |= value >> 16;
	++value;
	return value;
}
