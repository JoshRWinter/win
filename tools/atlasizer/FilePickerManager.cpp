#include <fstream>

#include "FilePickerManager.hpp"

FilePickerManager::FilePickerManager(const Platform &platform, const std::filesystem::path &savefile, const std::filesystem::path &default_dir)
	: platform(platform)
	, default_dir(default_dir)
	, defaults_file(savefile)
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

std::optional<std::vector<std::filesystem::path>> FilePickerManager::import_image()
{
	const auto result = platform.file_picker("Import .tga (Targa) file", true, true, "tga", get_image_dir());

	if (result.has_value() && !result.value().empty() && !result.value().at(0).empty())
	{
		const auto dir = result.value().at(0).parent_path();
		set_image_dir(dir);
	}

	return result;
}

std::optional<std::filesystem::path> FilePickerManager::import_layout()
{
	const auto &result = platform.file_picker("Import Atlas Layout (Text) file", true, true, "txt", get_import_export_dir());

	if (result.has_value() && !result.value().empty() && !result.value().at(0).empty())
	{
		const auto dir = result.value().at(0).parent_path();
		set_import_export_dir(dir);

		return result.value().at(0);
	}

	return std::nullopt;
}

std::optional<std::filesystem::path> FilePickerManager::export_layout()
{
	const auto &result = platform.file_picker("Export Atlas Layout (Text) file", false, false, "txt", get_import_export_dir());

	if (result.has_value() && !result.value().empty() && !result.value().at(0).empty())
	{
		const auto dir = result.value().at(0).parent_path();
		set_import_export_dir(dir);

		return result.value().at(0);
	}

	return std::nullopt;
}

void FilePickerManager::set_image_dir(const std::filesystem::path &dir)
{
	if (image_dir != dir)
	{
		image_dir = dir;
		save();
	}
}

std::filesystem::path FilePickerManager::get_image_dir()
{
	return image_dir.empty() ? default_dir : image_dir;
}

void FilePickerManager::set_import_export_dir(const std::filesystem::path &dir)
{
	if (import_export_dir != dir)
	{
		import_export_dir = dir;
		save();
	}
}

std::filesystem::path FilePickerManager::get_import_export_dir()
{
	return import_export_dir.empty() ? default_dir : import_export_dir;
}

void FilePickerManager::save() const
{
	std::ofstream out(defaults_file);

	out << image_dir.string() << std::endl;
	out << import_export_dir.string() << std::endl;
}
