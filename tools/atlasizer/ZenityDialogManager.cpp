#include <cstdio>
#include <cstring>

#include "ZenityDialogManager.hpp"

std::optional<std::vector<std::filesystem::path>> ZenityDialogManager::import_image()
{
	return open_zenity("Import .tga (Targa) file", true, true, "tga", std::filesystem::path());
}

std::optional<std::filesystem::path> ZenityDialogManager::import_layout()
{
	const auto &result = open_zenity("Import Atlas Layout (Text) file", true, true, "txt", std::filesystem::path());

	if (!result.has_value())
		return std::nullopt;

	return result.value().empty() ? std::nullopt : std::optional<std::filesystem::path>(result.value().at(0));
}

std::optional<std::filesystem::path> ZenityDialogManager::export_layout()
{
	const auto &result = open_zenity("Export Atlas Layout (Text) file", false, false, "txt", "atlas.txt");

	if (!result.has_value())
		return std::nullopt;

	return result.value().empty() ? std::nullopt : std::optional<std::filesystem::path>(result.value().at(0));
}

void ZenityDialogManager::show_message(const std::string &title, const std::string &msg, DialogManager::MessageType type)
{
}

std::optional<std::vector<std::filesystem::path>> ZenityDialogManager::open_zenity(const std::string &title, bool open, bool multiple, const std::string &extension_filter, const std::filesystem::path &default_filename)
{
	std::string cmd;

	if (open)
		cmd = R"(zenity --title=")" + title + R"(" --file-selection )" + (multiple ? "--multiple" : "") + R"( --file-filter="*.)" + extension_filter + "\"";
	else
		cmd = R"(zenity --title="Save atlas layout" --file-selection --save --filename=")" + default_filename.string() + "\"";

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
		 return split(file, '|');
	else
		return std::nullopt; // this means no file was chosen
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
