#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include "Platform.hpp"

class WindowsPlatform : public Platform
{
public:
	std::optional<std::vector<std::filesystem::path>> file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const override;
	bool ask(const std::string &title, const std::string &msg) const override;
	std::string expand_env(const std::string &env) const override;
	std::filesystem::path get_exe_path() const override;

private:
	static std::vector<std::filesystem::path> split(const char *buf, char c);
};

#endif

