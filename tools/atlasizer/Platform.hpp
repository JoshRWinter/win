#pragma once

#include <vector>
#include <filesystem>
#include <optional>

#include <win/Win.hpp>

class Platform
{
	WIN_NO_COPY_MOVE(Platform);
public:
	Platform() = default;

	virtual std::optional<std::vector<std::filesystem::path>> file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const = 0;
	virtual void info_message(const std::string &title, const std::string &msg) const = 0;
	virtual bool ask(const std::string &title, const std::string &msg) const = 0;
	virtual std::string expand_env(const std::string &env) const = 0;
};
