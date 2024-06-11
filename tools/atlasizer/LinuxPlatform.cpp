#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include <cstring>

#include <wordexp.h>
#include <unistd.h>

#include "LinuxPlatform.hpp"

struct RunCmdResult
{
	RunCmdResult(bool notfound, int return_code, const std::string &out)
		: notfound(notfound), return_code(return_code), out(out) {}

	bool notfound;
	int return_code;
	std::string out;
};

static RunCmdResult run_cmd(const std::string &cmd);

std::optional<std::vector<std::filesystem::path>> LinuxPlatform::file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const
{
	const std::string opt_save = open ? "" : " --save";
	const std::string opt_multiple = multiple ? " --multiple" : "";
	const std::string opt_extension = ext_filter.empty() ? "" : (" --file-filter=\"*." + ext_filter + "\"");
	const std::string opt_showdir = dir.empty() ? "" : (" --filename=\"" + (dir / "yeetskeetmcgeet").string() + "\"");

	const std::string cmd = "zenity --title=\"" + title + "\" --file-selection" + opt_multiple + opt_save + opt_extension + opt_showdir;
	const auto result = run_cmd(cmd);

	if (result.notfound)
	{
		std::cerr << "zenity is required for dialogs" << std::endl;
		return std::nullopt;
	}
	else if (result.return_code == 0)
		return split(result.out, '|');
	else
		return std::nullopt;
}

bool LinuxPlatform::ask(const std::string &title, const std::string &msg) const
{
	const std::string cmd = "zenity --question --default-cancel --text=\"" + msg + "\" --title=\"" + title + "\"";
	const auto result = run_cmd(cmd);

	if (result.notfound)
	{
		std::cerr << "zenity is required for dialogs" << std::endl;
		return false;
	}
	else
		return result.return_code == 0;
}

std::string LinuxPlatform::expand_env(const std::string &env) const
{
	wordexp_t w;
	wordexp(env.c_str(), &w, 0);
	std::filesystem::path result = *w.we_wordv;
	wordfree(&w);
	return result;
}

std::filesystem::path LinuxPlatform::get_exe_path() const
{
	char buf[1024];
	ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
	if (len == -1)
		win::bug("Couldn't open /proc/self/exe");

	buf[len] = 0;
	return buf;
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

RunCmdResult run_cmd(const std::string &cmd)
{
	//fprintf(stderr, "\"%s\"\n", cmd.c_str());

	FILE *proc;
	if ((proc = popen(cmd.c_str(), "r")) == NULL)
		return RunCmdResult(true, 0, "");

	char file[4096];
	memset(file, 0, sizeof(file));
	fgets(file, sizeof(file), proc);
	const auto len = strlen(file);
	file[sizeof(file) - 1] = 0;

	if (len > 0 && file[len - 1] == '\n')
		file[len - 1] = 0;

	const int ret = WEXITSTATUS(pclose(proc));
	return RunCmdResult(false, ret, file);
}

#endif
