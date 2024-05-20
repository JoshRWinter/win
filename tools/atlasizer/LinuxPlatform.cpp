#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <cstring>

#include <wordexp.h>

#include "LinuxPlatform.hpp"

std::optional<std::vector<std::filesystem::path>> LinuxPlatform::file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const
{
	const std::string opt_save = open ? "" : " --save";
	const std::string opt_multiple = multiple ? " --multiple" : "";
	const std::string opt_extension = ext_filter.empty() ? "" : (" --file-filter=\"*." + ext_filter + "\"");
	const std::string opt_showdir = dir.empty() ? "" : (" --filename=\"" + (dir / "yeetskeetmcgeet").string() + "\"");

	const std::string cmd = "zenity --title=\"" + title + "\" --file-selection" + opt_multiple + opt_save + opt_extension + opt_showdir;
	const auto result = run_cmd(cmd);

	if (result.has_value())
		return split(result.value(), '|');
	else
		return std::nullopt;
}

void LinuxPlatform::info_message(const std::string &title, const std::string &msg) const
{
}

bool LinuxPlatform::ask(const std::string &title, const std::string &msg) const
{
	return false;
}

std::string LinuxPlatform::expand_env(const std::string &env) const
{
	wordexp_t w;
	wordexp(env.c_str(), &w, 0);
	std::filesystem::path result = *w.we_wordv;
	wordfree(&w);
	return result;
}

std::optional<std::string> LinuxPlatform::run_cmd(const std::string &cmd)
{
	fprintf(stderr, "\"%s\"\n", cmd.c_str());

	FILE *zenity;
	if ((zenity = popen(cmd.c_str(), "r")) == NULL)
	{
		std::cerr << "zenity is required for dialogs" << std::endl;
		return std::nullopt;
	}

	char file[4096];
	memset(file, 0, sizeof(file));
	fgets(file, sizeof(file), zenity);
	const auto len = strlen(file);
	file[sizeof(file) - 1] = 0;

	if (len > 0 && file[len - 1] == '\n')
		file[len - 1] = 0;

	if (pclose(zenity) == 0)
		return file;
	else
		return std::nullopt;
}

std::vector<std::filesystem::path> LinuxPlatform::split(const std::string &s, const char c)
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
