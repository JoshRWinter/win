#pragma once

#include "Platform.hpp"

class FilePickerManager
{
public:
	explicit FilePickerManager(const Platform &platform, const std::filesystem::path &savefile, const std::filesystem::path &default_dir);

	std::optional<std::vector<std::filesystem::path>> import_image();
	std::optional<std::filesystem::path> import_layout();
	std::optional<std::filesystem::path> export_layout();

private:
	void set_image_dir(const std::filesystem::path &dir);
	std::filesystem::path get_image_dir();
	void set_import_export_dir(const std::filesystem::path &dir);
	std::filesystem::path get_import_export_dir();
	void save() const;

	const Platform &platform;
	std::filesystem::path default_dir;
	std::filesystem::path defaults_file;
	std::filesystem::path image_dir;
	std::filesystem::path import_export_dir;
};
