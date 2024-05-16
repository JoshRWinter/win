#pragma once

#include "DialogManager.hpp"

class ZenityDialogManager : public DialogManager
{
public:
	std::optional<std::vector<std::filesystem::path>> import_image() override;
	std::optional<std::filesystem::path> import_layout() override;
	std::optional<std::filesystem::path> export_layout() override;
	void show_message(const std::string &title, const std::string &msg, MessageType type) override;

private:
	std::optional<std::vector<std::filesystem::path>> open_zenity(const std::string &title, bool open, bool multiple, const std::string &extension_filter, const std::filesystem::path &default_filename);
	static std::vector<std::filesystem::path> split(const std::string &s, char c);
};
