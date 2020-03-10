#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>

class ini_section
{
public:
	std::map<std::string, std::string> pairs;

	[[nodiscard]] bool contains(const std::string& key) const;

	template <typename T>
	[[nodiscard]] T get(const std::string& key) const = delete;

	template <typename T>
	[[nodiscard]] T get_or(const std::string& key, const T& default_value) const
	{
		if (!pairs.contains(key))
		{
			return default_value;
		}

		return get<T>(key);
	}

	template <typename T>
	void set(const std::string& key, const T& value) = delete;

	void set(const std::string& key, float value);

	void set(const std::string& key, double value);

	void set(const std::string& key, int16_t value);

	void set(const std::string& key, uint16_t value);

	void set(const std::string& key, int32_t value);

	void set(const std::string& key, uint32_t value);

	void set(const std::string& key, int64_t value);

	void set(const std::string& key, uint64_t value);

	void set(const std::string& key, bool value);

	void set(const std::string& key, const std::string& value);

	bool erase(const std::string& key);
};

template <>
[[nodiscard]] float ini_section::get<float>(const std::string& key) const;

template <>
[[nodiscard]] double ini_section::get<double>(const std::string& key) const;

template <>
[[nodiscard]] int16_t ini_section::get<int16_t>(const std::string& key) const;

template <>
[[nodiscard]] uint16_t ini_section::get<uint16_t>(const std::string& key) const;

template <>
[[nodiscard]] int32_t ini_section::get<int32_t>(const std::string& key) const;

template <>
[[nodiscard]] uint32_t ini_section::get<uint32_t>(const std::string& key) const;

template <>
[[nodiscard]] int64_t ini_section::get<int64_t>(const std::string& key) const;

template <>
[[nodiscard]] uint64_t ini_section::get<uint64_t>(const std::string& key) const;

template <>
[[nodiscard]] bool ini_section::get<bool>(const std::string& key) const;

template <>
[[nodiscard]] std::string ini_section::get<std::string>(const std::string& key) const;

class ini_file
{
public:
	std::map<std::string, std::shared_ptr<ini_section>> sections;

	ini_file() = default;

	void read(std::fstream& stream);
	void write(std::fstream& stream);

	[[nodiscard]] bool contains_section(const std::string& section_name) const;

	[[nodiscard]] std::shared_ptr<ini_section> get_section(const std::string& section_name) const;
	void set_section(const std::string& section_name, std::shared_ptr<ini_section> section_ptr);
};
