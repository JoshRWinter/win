#include <win/Win.hpp>

#ifdef WINPLAT_WINDOWS

#include <iostream>

#include <Commdlg.h>

#include "WindowsPlatform.hpp"

std::optional<std::vector<std::filesystem::path>> WindowsPlatform::file_picker(const std::string &title, bool open, bool multiple, const std::string &ext_filter, const std::filesystem::path &dir) const
{
	std::string filter_description;
	if (ext_filter == "tga")
		filter_description = std::string("TARGA images") + (char)0 + "*.tga" + (char)0 +  "All files" + (char)0 + "*" + (char)0 + (char)0;
	else if (ext_filter == "txt")
		filter_description = std::string("Text files") + (char)0 + "*.txt" + (char)0 + "All files" + (char)0 + "*" + (char)0 + (char)0;
	else
		win::bug("ext not supported");

	char filebuf[4096] = "";

	const std::string &initialdir = dir.string();

	OPENFILENAMEA ofn;
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.hInstance = NULL;
	ofn.lpstrFilter = filter_description.length() > 0 ? (filter_description).c_str() : NULL;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 0;
	ofn.lpstrFile = filebuf;
	ofn.nMaxFile = sizeof(filebuf);
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = initialdir.c_str();
	ofn.lpstrTitle = open ? "Open file" : "Save file";
	ofn.Flags = OFN_DONTADDTORECENT | OFN_ENABLESIZING | OFN_EXPLORER | OFN_NOCHANGEDIR;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;
	ofn.lpstrDefExt = !open ? ext_filter.c_str() : NULL;
	ofn.lCustData = NULL;
	ofn.lpfnHook = NULL;
	ofn.lpTemplateName = NULL;

	if (multiple)
		ofn.Flags |= OFN_ALLOWMULTISELECT;

	if (open)
		ofn.Flags |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	else
		ofn.Flags |= OFN_OVERWRITEPROMPT;

	if (open)
	{
		if (!GetOpenFileNameA(&ofn))
			return std::nullopt;
	}
	else
	{
		if (!GetSaveFileNameA(&ofn))
			return std::nullopt;
	}

	filebuf[sizeof(filebuf) - 1] = 0;

	const auto splits = split(filebuf, 0);

	if (splits.size() < 1)
		return std::nullopt; // this shouldn't happen i think
	else if (splits.size() < 2) // only one item was selected
	{
		std::vector<std::filesystem::path> v;
		v.push_back(splits.at(0));
		return v;
	}

	// multiple items were selected
	std::vector<std::filesystem::path> results;
	const auto &parent = splits.at(0);
	for (int i = 1; i < splits.size(); ++i)
		results.emplace_back(parent / splits.at(i));

	return results;
}

bool WindowsPlatform::ask(const std::string &title, const std::string &msg) const
{
	return MessageBoxA(NULL, msg.c_str(), title.c_str(), MB_YESNO | MB_DEFBUTTON2) == IDYES;
}

std::string WindowsPlatform::expand_env(const std::string &env) const
{
	char buf[500];
	ExpandEnvironmentStringsA(env.c_str(), buf, sizeof(buf) - 2);
	buf[sizeof(buf) - 1] = 0;
	buf[sizeof(buf) - 2] = 0;

	return buf;
}

std::filesystem::path WindowsPlatform::get_exe_path() const
{
	return "";
}

std::vector<std::filesystem::path> WindowsPlatform::split(const char *buf, const char c)
{
	std::vector<std::filesystem::path> results;

	// puzzle out the length
	int length = 0;

	{
		bool zero = false;
		int i = 0;
		while (true)
		{
			if (buf[i] == 0)
			{
				if (zero)
				{
					length = i + 1;
					break;
				}

				zero = true;
			}
			else
				zero = false;

			++i;
		}
	}

	size_t start = 0;
	for (size_t i = 0; i < length; ++i)
	{
		if (buf[i] == c)
		{

			if (i - start > 0)
				results.emplace_back(std::string(buf + start, 0, i - start));

			start = i + 1;
		}
	}

	if (length - start > 0)
		results.emplace_back(std::string(buf + start, 0, length - start));

	return results;
}

#endif
