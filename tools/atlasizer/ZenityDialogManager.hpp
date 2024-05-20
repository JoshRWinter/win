#pragma once

#include <win/Win.hpp>

#ifdef WINPLAT_LINUX

#include "DialogManager.hpp"

class ZenityDefaultDirectories
{
public:
	explicit ZenityDefaultDirectories(const std::filesystem::path &savefile);

	void set_image_dir(const std::filesystem::path &dir);
	void set_import_export_dir(const std::filesystem::path &dir);
	std::filesystem::path get_image_dir() const;
	std::filesystem::path get_import_export_dir() const;

private:
	void save() const;

	std::filesystem::path savefile;
	std::filesystem::path image_dir;
	std::filesystem::path import_export_dir;
};

class ZenityDialogManager : public DialogManager
{
public:
	explicit ZenityDialogManager(const std::filesystem::path &defaults_file, const std::filesystem::path &default_dir);

	std::optional<std::vector<std::filesystem::path>> import_image() override;
	std::optional<std::filesystem::path> import_layout() override;
	std::optional<std::filesystem::path> export_layout() override;
	void show_message(const std::string &title, const std::string &msg, MessageType type) override;
	bool yesno(const std::string &title, const std::string &msg) override;

private:
	static std::optional<std::vector<std::filesystem::path>> file_dialog(const std::string &title, bool open, bool multiple, const std::string &extension_filter, const std::filesystem::path &showdir);
	static std::optional<std::string> run_cmd(const std::string &cmd);
	static std::vector<std::filesystem::path> split(const std::string &s, char c);

	ZenityDefaultDirectories defaults;
	std::filesystem::path default_dir;
};

#endif
