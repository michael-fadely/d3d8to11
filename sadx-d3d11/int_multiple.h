#pragma once

float constexpr cceil(float f)
{
	const auto i = static_cast<int>(f);

	if (i < f)
	{
		return static_cast<float>(i + 1);
	}

	return f;
}

float constexpr cfloor(float f)
{
	const auto i = static_cast<int>(f);

	if (i > f)
	{
		return static_cast<float>(i - 1);
	}

	return f;
}

template <typename T>
T constexpr int_multiple(T value, int multiple)
{
	auto m = static_cast<float>(multiple);
	return static_cast<T>(cceil(value / m) * m);
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
