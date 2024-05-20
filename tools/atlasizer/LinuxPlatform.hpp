#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include "Platform.hpp"

class LinuxPlatform : public Platform
{
public:
	std::optional<std::vector<std::filesystem::path>> file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const override;
	void info_message(const std::string &title, const std::string &msg) const override;
	bool ask(const std::string &title, const std::string &msg) const override;
	std::string expand_env(const std::string &env) const override;

private:
	static std::optional<std::string> run_cmd(const std::string &cmd);
	static std::vector<std::filesystem::path> split(const std::string &s, char c);
};

#endif
