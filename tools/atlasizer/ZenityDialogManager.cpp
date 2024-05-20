#include <win/Win.hpp>
#ifdef WINPLAT_LINUX

#include <cstdio>
#include <fstream>
#include <cstring>

#include <wordexp.h>

#include "ZenityDialogManager.hpp"

ZenityDefaultDirectories::ZenityDefaultDirectories(const std::filesystem::path &savefile)
	: savefile(savefile)
{
	std::ifstream in(savefile);
	if (in)
	{
		std::string s, s2;
		std::getline(in, s);
		std::getline(in, s2);

		if (!s.empty())
			image_dir = s;
		if (!s2.empty())
			import_export_dir = s2;
	}
}

void ZenityDefaultDirectories::set_image_dir(const std::filesystem::path &dir)
{
	if (image_dir != dir)
	{
		image_dir = dir;
		save();
	}
}

void ZenityDefaultDirectories::set_import_export_dir(const std::filesystem::path &dir)
{
	if (import_export_dir != dir)
	{
		import_export_dir = dir;
		save();
	}
}

std::filesystem::path ZenityDefaultDirectories::get_image_dir() const
{
	return image_dir;
}

std::filesystem::path ZenityDefaultDirectories::get_import_export_dir() const
{
	return import_export_dir;
}

void ZenityDefaultDirectories::save() const
{
	std::ofstream out(savefile);

	out << image_dir.string() << std::endl;
	out << import_export_dir.string() << std::endl;
}

ZenityDialogManager::ZenityDialogManager(const std::filesystem::path &defaults_file, const std::filesystem::path &default_dir)
	: defaults(defaults_file)
	, default_dir(default_dir)
{}

std::optional<std::vector<std::filesystem::path>> ZenityDialogManager::import_image()
{
	const std::filesystem::path showdir = defaults.get_image_dir().empty() ? default_dir : defaults.get_image_dir();
	const auto result = file_dialog("Import .tga (Targa) file", true, true, "tga", showdir);

	if (result.has_value() && !result.value().empty() && !result.value().at(0).empty())
	{
		const auto dir = result.value().at(0).parent_path();
		defaults.set_image_dir(dir);
	}

	return result;
}

std::optional<std::filesystem::path> ZenityDialogManager::import_layout()
{
	const std::filesystem::path showdir = defaults.get_import_export_dir().empty() ? default_dir : defaults.get_import_export_dir();
	const auto &result = file_dialog("Import Atlas Layout (Text) file", true, true, "txt", showdir);

	if (result.has_value() && !result.value().empty() && !result.value().at(0).empty())
	{
		const auto dir = result.value().at(0).parent_path();
		defaults.set_import_export_dir(dir);

		return result.value().at(0);
	}

	return std::nullopt;
}

std::optional<std::filesystem::path> ZenityDialogManager::export_layout()
{
	const std::filesystem::path showdir = defaults.get_import_export_dir().empty() ? default_dir : defaults.get_import_export_dir();
	const auto &result = file_dialog("Export Atlas Layout (Text) file", false, false, "txt", showdir);

	if (result.has_value() && !result.value().empty() && !result.value().at(0).empty())
	{
		const auto dir = result.value().at(0).parent_path();
		defaults.set_import_export_dir(dir);

		return result.value().at(0);
	}

	return std::nullopt;
}

void ZenityDialogManager::show_message(const std::string &title, const std::string &msg, DialogManager::MessageType type)
{
}

bool ZenityDialogManager::yesno(const std::string &title, const std::string &msg)
{
	return false;
}

std::optional<std::vector<std::filesystem::path>> ZenityDialogManager::file_dialog(const std::string &title, bool open, bool multiple, const std::string &extension_filter, const std::filesystem::path &showdir)
{
	const std::string opt_save = open ? "" : " --save";
	const std::string opt_multiple = multiple ? " --multiple" : "";
	const std::string opt_extension = extension_filter.empty() ? "" : (" --file-filter=\"*." + extension_filter + "\"");
	const std::string opt_showdir = showdir.empty() ? "" : (" --filename=\"" + (showdir / "yeetskeetmcgeet").string() + "\"");

	const std::string cmd = "zenity --title=\"" + title + "\" --file-selection" + opt_multiple + opt_save + opt_extension + opt_showdir;
	const auto result = run_cmd(cmd);

	if (result.has_value())
		return split(result.value(), '|');
	else
		return std::nullopt;
}

std::optional<std::string> ZenityDialogManager::run_cmd(const std::string &cmd)
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

std::vector<std::filesystem::path> ZenityDialogManager::split(const std::string &s, const char c)
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
