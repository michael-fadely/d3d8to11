#include "ini_file.h"

#include <algorithm>
#include <cctype>
#include <locale>
#include <ranges>
#include <string_view>
#include <utility>

namespace
{
// TODO: handle locale differences

void trim_left(std::string& str)
{
	str.erase(str.begin(), std::ranges::find_if(str, [](int ch)
	{
		return !std::isspace(ch);
	}));
}

void trim_right(std::string& str)
{
	str.erase(std::ranges::find_if(std::ranges::reverse_view(str), [](int ch)
	{
		return !std::isspace(ch);
	}).base(), str.end());
}

void trim(std::string& str)
{
	trim_left(str);
	trim_right(str);
}

bool equals_case_insensitive(const std::string_view& a, const std::string_view& b)
{
	auto fn = [](unsigned char ca, unsigned char cb) { return std::tolower(ca) == std::tolower(cb); };
	return std::ranges::equal(a, b, fn);
}
}  // namespace

bool ini_section::contains(const std::string& key) const
{
	return m_pairs.contains(key);
}

template <>
float ini_section::get<float>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return std::stof(value);
}

template <>
double ini_section::get<double>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return std::stod(value);
}

template <>
int16_t ini_section::get<int16_t>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return static_cast<int16_t>(std::stol(value));
}

template <>
uint16_t ini_section::get<uint16_t>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return static_cast<int16_t>(std::stoul(value));
}

template <>
int32_t ini_section::get<int32_t>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return static_cast<int32_t>(std::stol(value));
}

template <>
uint32_t ini_section::get<uint32_t>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return static_cast<uint32_t>(std::stoul(value));
}

template <>
int64_t ini_section::get<int64_t>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return std::stoll(value);
}

template <>
uint64_t ini_section::get<uint64_t>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return std::stoull(value);
}

template <>
bool ini_section::get<bool>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return equals_case_insensitive(value, "true");
}

template <>
std::string ini_section::get<std::string>(const std::string& key) const
{
	const std::string& value = m_pairs.at(key);
	return value;
}

void ini_section::set(const std::string& key, float value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, double value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, int16_t value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, uint16_t value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, int32_t value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, uint32_t value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, int64_t value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, uint64_t value)
{
	m_pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, bool value)
{
	m_pairs[key] = value ? "true" : "false";
}

void ini_section::set(const std::string& key, const std::string& value)
{
	m_pairs[key] = value;
}

bool ini_section::erase(const std::string& key)
{
	const auto it = m_pairs.find(key);

	if (it == m_pairs.end())
	{
		return false;
	}

	m_pairs.erase(it);
	return true;
}

decltype(ini_section::m_pairs)::size_type ini_section::size() const
{
	return m_pairs.size();
}

bool ini_section::empty() const
{
	return m_pairs.empty();
}

ini_file::ini_file(const ini_file& other)
{
	for (auto& [section_name, section_ptr] : other.m_sections)
	{
		auto new_section_ptr = std::make_shared<ini_section>(*section_ptr);
		set_section(section_name, std::move(new_section_ptr));
	}
}

ini_file& ini_file::operator=(const ini_file& rhs)
{
	if (this != &rhs)
	{
		for (auto& [section_name, section_ptr] : rhs.m_sections)
		{
			auto new_section_ptr = std::make_shared<ini_section>(*section_ptr);
			set_section(section_name, std::move(new_section_ptr));
		}
	}

	return *this;
}

void ini_file::read(std::fstream& stream)
{
	static const std::string assignment_delim = "=";
	std::string line;

	std::shared_ptr<ini_section> section_ptr;

	while (std::getline(stream, line))
	{
		trim(line);

		if (line.empty())
		{
			continue;
		}

		// skip comment
		if (line.starts_with(';'))
		{
			continue;
		}

		if (line.starts_with('[') && line.ends_with(']'))
		{
			const std::string section_name = std::string(line.begin() + 1, line.end() - 1);
			section_ptr = std::make_shared<ini_section>();

			set_section(section_name, section_ptr);
			continue;
		}

		if (!section_ptr)
		{
			continue;
		}

		const auto assignment = line.find(assignment_delim);

		if (assignment == std::string::npos)
		{
			continue;
		}

		std::string key = line.substr(0, assignment);
		trim(key);

		std::string value = line.substr(assignment + assignment_delim.length());
		trim(value);

		section_ptr->set(key, value);
	}
}

void ini_file::write(std::fstream& stream)
{
	auto write_section = [](std::fstream& s, const decltype(m_sections)::value_type& section_pair)
	{
		s << "[" << section_pair.first << "]" << std::endl;

		for (auto& [key, value] : section_pair.second->get_pairs())
		{
			s << key << " = " << value << std::endl;
		}
	};

	// we do this to separate each [section] by a blank line efficiently

	auto it = m_sections.begin();
	write_section(stream, *it);
	++it;

	while (it != m_sections.end())
	{
		stream << std::endl;
		write_section(stream, *it);
		++it;
	}
}

bool ini_file::contains_section(const std::string& section_name) const
{
	return m_sections.contains(section_name);
}

std::shared_ptr<ini_section> ini_file::get_section(const std::string& section_name) const
{
	const auto it = m_sections.find(section_name);

	if (it == m_sections.cend())
	{
		return nullptr;
	}

	return it->second;
}

void ini_file::set_section(const std::string& section_name, std::shared_ptr<ini_section> section_ptr)
{
	if (!section_ptr)
	{
		m_sections.erase(section_name);
	}
	else
	{
		m_sections[section_name] = std::move(section_ptr);
	}
}

decltype(ini_file::m_sections)::size_type ini_file::size() const
{
	return m_sections.size();
}

bool ini_file::empty() const
{
	return m_sections.empty();
}

void ini_file::clear()
{
	m_sections.clear();
}
