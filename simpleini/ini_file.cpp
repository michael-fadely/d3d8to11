#include "pch.h"
#include "ini_file.h"

#include <utility>
#include <cctype>
#include <algorithm>
#include <locale>

inline void trim_left(std::string& str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch)
	{
		return !std::isspace(ch);
	}));
}

inline void trim_right(std::string& str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch)
	{
		return !std::isspace(ch);
	}).base(), str.end());
}

inline void trim(std::string& str)
{
	trim_left(str);
	trim_right(str);
}

bool ini_section::contains(const std::string& key) const
{
	return pairs.contains(key);
}

template <>
float ini_section::get<float>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return std::stof(value);
}

template <>
double ini_section::get<double>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return std::stod(value);
}

template <>
int16_t ini_section::get<int16_t>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return static_cast<int16_t>(std::stol(value));
}

template <>
uint16_t ini_section::get<uint16_t>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return static_cast<int16_t>(std::stoul(value));
}

template <>
int32_t ini_section::get<int32_t>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return static_cast<int32_t>(std::stol(value));
}

template <>
uint32_t ini_section::get<uint32_t>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return static_cast<uint32_t>(std::stoul(value));
}

template <>
int64_t ini_section::get<int64_t>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return std::stoll(value);
}

template <>
uint64_t ini_section::get<uint64_t>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return std::stoull(value);
}

template <>
bool ini_section::get<bool>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return value == "true";
}

template <>
std::string ini_section::get<std::string>(const std::string& key) const
{
	const std::string& value = pairs.at(key);
	return value;
}

void ini_section::set(const std::string& key, float value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, double value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, int16_t value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, uint16_t value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, int32_t value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, uint32_t value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, int64_t value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, uint64_t value)
{
	pairs[key] = std::to_string(value);
}

void ini_section::set(const std::string& key, bool value)
{
	pairs[key] = value ? "true" : "false";
}

void ini_section::set(const std::string& key, const std::string& value)
{
	pairs[key] = value;
}

bool ini_section::erase(const std::string& key)
{
	const auto it = pairs.find(key);

	if (it == pairs.end())
	{
		return false;
	}

	pairs.erase(it);
	return true;
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

		auto assignment = line.find(assignment_delim);

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
	auto write_section = [&](decltype(sections)::value_type& section_pair)
	{
		stream << "[" << section_pair.first << "]" << std::endl;

		for (auto& value_pair : section_pair.second->pairs)
		{
			stream << value_pair.first << " = " << value_pair.second << std::endl;
		}
	};

	// we do this to separate each [section] by a blank line efficiently

	auto it = sections.begin();
	write_section(*it);
	++it;

	while (it != sections.end())
	{
		stream << std::endl;
		write_section(*it);
		++it;
	}
}

bool ini_file::contains_section(const std::string& section_name) const
{
	return sections.contains(section_name);
}

std::shared_ptr<ini_section> ini_file::get_section(const std::string& section_name) const
{
	const auto it = sections.find(section_name);

	if (it == sections.cend())
	{
		return nullptr;
	}

	return it->second;
}

void ini_file::set_section(const std::string& section_name, std::shared_ptr<ini_section> section_ptr)
{
	if (!section_ptr)
	{
		sections.erase(section_name);
	}
	else
	{
		sections[section_name] = std::move(section_ptr);
	}
}
