#include <cstdio>
#include <cstring>

#include "ZenityDialogManager.hpp"

std::optional<std::vector<std::filesystem::path>> ZenityDialogManager::import_image()
{
	return file_dialog("Import .tga (Targa) file", true, true, "tga", std::filesystem::path());
}

std::optional<std::filesystem::path> ZenityDialogManager::import_layout()
{
	const auto &result = file_dialog("Import Atlas Layout (Text) file", true, true, "txt", std::filesystem::path());

	if (!result.has_value())
		return std::nullopt;

	return result.value().empty() ? std::nullopt : std::optional<std::filesystem::path>(result.value().at(0));
}

std::optional<std::filesystem::path> ZenityDialogManager::export_layout()
{
	const auto &result = file_dialog("Export Atlas Layout (Text) file", false, false, "txt", "atlas.txt");

	if (!result.has_value())
		return std::nullopt;

	return result.value().empty() ? std::nullopt : std::optional<std::filesystem::path>(result.value().at(0));
}

void ZenityDialogManager::show_message(const std::string &title, const std::string &msg, DialogManager::MessageType type)
{
}

bool ZenityDialogManager::yesno(const std::string &title, const std::string &msg)
{
	return false;
}

std::optional<std::vector<std::filesystem::path>> ZenityDialogManager::file_dialog(const std::string &title, bool open, bool multiple, const std::string &extension_filter, const std::filesystem::path &default_filename)
{
	const std::string opt_save = open ? "" : " --save";
	const std::string opt_multiple = multiple ? " --multiple" : "";
	const std::string opt_extension = extension_filter.empty() ? "" : (" --file-filter=\"*." + extension_filter + "\"");
	const std::string opt_filename = default_filename.empty() ? "" : (" --filename=\"" + default_filename.string() + "\"");

	const std::string cmd = "zenity --title=\"" + title + "\" --file-selection" + opt_multiple + opt_save + opt_extension + opt_filename;
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
