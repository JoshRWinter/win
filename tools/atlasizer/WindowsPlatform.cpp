#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <iostream>

#include "WindowsPlatform.hpp"

std::optional<std::vector<std::filesystem::path>> WindowsPlatform::file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const
{
	return std::nullopt;
}

bool WindowsPlatform::ask(const std::string &title, const std::string &msg) const
{
	return false;
}

std::string WindowsPlatform::expand_env(const std::string &env) const
{
	return "";
}

std::filesystem::path WindowsPlatform::get_exe_path() const
{
	return "";
}

std::vector<std::filesystem::path> WindowsPlatform::split(const std::string &s, const char c)
{
	std::vector<std::filesystem::path> results;

	size_t start = 0;
	const auto len = s.length();
	for (size_t i = 0; i < len; ++i)
	{
		if (s[i] == c)
		{
			results.emplace_back(std::string(s, start, i - start));
			start = i + 1;
		}
	}

	results.emplace_back(std::string(s, start, len - start));

	return results;
}

#endif
