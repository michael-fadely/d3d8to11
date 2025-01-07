#pragma once

#include <cstdint>
#include <fstream>
#include <map>
#include <memory>
#include <string>

class ini_section
{
	std::map<std::string, std::string> m_pairs;

public:
	[[nodiscard]] bool contains(const std::string& key) const;

	template <typename T>
	[[nodiscard]] T get(const std::string& key) const = delete;

	template <typename T>
	[[nodiscard]] T get_or(const std::string& key, const T& default_value) const
	{
		if (!m_pairs.contains(key))
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

	/**
	 * Erases (removes) a key/value pair from this section.
	 * @param key The key to erase.
	 * @return \c true if the key was present and successfully erased.
	 */
	bool erase(const std::string& key);

	/**
	 * Get the number of key/value pairs in this section.
	 * @return The number of key/value pairs in this section.
	 */
	[[nodiscard]] decltype(m_pairs)::size_type size() const;

	/**
	 * Check if this section is empty, i.e. contains no key/value pairs.
	 * @return \c true if there are no key/value pairs in this section.
	 */
	[[nodiscard]] bool empty() const;

	// FIXME: honestly I'm just too lazy to fix the ini_file code that needs m_pairs access
	[[nodiscard]] const auto& get_pairs() const
	{
		return m_pairs;
	}
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
	// FIXME: this shared_ptr stuff is gross
	std::map<std::string, std::shared_ptr<ini_section>> m_sections;

public:
	ini_file() = default;
	~ini_file() = default;

	ini_file(ini_file&&) noexcept = default;
	ini_file(const ini_file& other);

	ini_file& operator=(ini_file&&) noexcept = default;
	ini_file& operator=(const ini_file& rhs);

	void read(std::fstream& stream);
	void write(std::fstream& stream);

	[[nodiscard]] bool contains_section(const std::string& section_name) const;

	/**
	 * Gets a section by name.
	 * @param section_name The name of the section to retrieve.
	 * @return A pointer to the section associated with the given name, or \c nullptr if not found.
	 */
	[[nodiscard]] std::shared_ptr<ini_section> get_section(const std::string& section_name) const;

	/**
	 * Sets or erases a section's contents.
	 * @param section_name The name of the section to set.
	 * @param section_ptr A pointer to the section contents, or \c nullptr to erase the section.
	 * \sa ini_section
	 */
	void set_section(const std::string& section_name, std::shared_ptr<ini_section> section_ptr);

	/**
	 * Get the number of sections in this file.
	 * @return The number of sections in this file.
	 * \sa ini_section
	 */
	[[nodiscard]] decltype(m_sections)::size_type size() const;

	/**
	 * Check if this file is empty, i.e. contains no sections.
	 * @return \c true if there are no sections in the file.
	 */
	[[nodiscard]] bool empty() const;

	void clear();
};
