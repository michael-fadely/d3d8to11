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
