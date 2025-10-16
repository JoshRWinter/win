#include <iostream>
#include <random>

#include "recipe.hpp"
#include "recipeparser.hpp"

#if defined __linux__
#include <unistd.h>
#elif defined _WIN32
#define NOMINMAX
#include <windows.h>
#endif

Recipe::Recipe(const std::filesystem::path &recipe_file, const std::filesystem::path &roll_file)
	: recreate(false)
	, reconvert(false)
	, recipe_file(recipe_file)
	, roll_file(roll_file)
{
	if (!std::filesystem::exists(roll_file))
	{
		recreate = true;
	}
	else if (std::filesystem::last_write_time(recipe_file) > std::filesystem::last_write_time(roll_file))
	{
		reconvert = true;
		recreate = true;
	}

	const std::vector<RecipeRootInput> inputs = RecipeParser::parse(recipe_file);

	for (const RecipeRootInput &input : inputs)
	{
		if (input.type == RecipeRootInputType::Line)
		{
			process_root_line(input.line);
		}
		else
		{
			if (input.section.name == "svg2tga")
				process_svg2tga_section(input.section);
			else if (input.section.name == "atlas")
				process_atlas_section(input.section);
			else if (input.section.name.empty())
				throw std::runtime_error(std::to_string(input.section.line_number) + ": Missing section name");
			else
				throw std::runtime_error(std::to_string(input.section.line_number) + ": Unrecognized section \"" + input.section.name + "\"");
		}
	}
}

std::vector<RollItem> Recipe::get_items(bool &out_recreate) const
{
	out_recreate = recreate;
	return items;
}

void Recipe::process_root_line(const RecipeInputLine &line)
{
	if (line.tokens.empty())
		return;

	const std::filesystem::path recorded_file = line.tokens.at(0);
	const std::filesystem::path real_file = get_real_file_path(line.tokens.at(0));

	if (!std::filesystem::exists(real_file))
		throw std::runtime_error(std::to_string(line.line_number) + ": \"" + real_file.string() + "\" doesn't exist");

	if (line.tokens.size() > 2)
		throw std::runtime_error(std::to_string(line.line_number) + ": Unrecognized extra options");

	bool compress = false;
	if (line.tokens.size() > 1)
	{
		if (line.tokens.at(1) != "z")
			throw std::runtime_error(std::to_string(line.line_number) + ": Unrecognized option \"" + line.tokens.at(1) + "\"");

		compress = true;
	}

	if (recreate || std::filesystem::last_write_time(real_file) > std::filesystem::last_write_time(roll_file))
		recreate = true;

	items.emplace_back(real_file.string(), recorded_file.string(), compress);
}

void Recipe::process_svg2tga_section(const RecipeInputSection &section)
{
	if (section.options.size() > 0)
		throw std::runtime_error(std::to_string(section.line_number) + ": Unrecognized svg2tga section options");

	std::string converted_png = "";
	for (const RecipeInputLine &line : section.lines)
	{
		if (line.tokens.size() == 0)
			continue;

		if (line.tokens.size() < 3)
			throw std::runtime_error(std::to_string(line.line_number) + ": svg2tga: need at least 3 arguments - filename width height");

		bool exclude = false;
		int token_index = 0;

		const std::filesystem::path recorded_file = line.tokens.at(token_index);
		const std::filesystem::path real_file = get_real_file_path(recorded_file);
		++token_index;

		const std::string width_string = line.tokens.at(token_index);
		++token_index;
		const std::string height_string = line.tokens.at(token_index);
		++token_index;

		if (line.tokens.size() > 3)
		{
			if (line.tokens.at(token_index) != "exclude")
				throw std::runtime_error(std::to_string(line.line_number) + ": svg2tga: unrecognized parameter \"" + line.tokens.at(token_index) + "\"");

			++token_index;
			exclude = true;
		}

		int test;
		if (sscanf(width_string.c_str(), "%d", &test) != 1)
			throw std::runtime_error(std::to_string(line.line_number) + ": Bad width \"" + width_string + "\"");
		if (sscanf(height_string.c_str(), "%d", &test) != 1)
			throw std::runtime_error(std::to_string(line.line_number) + ": Bad height \"" + height_string + "\"");

		if (!std::filesystem::exists(real_file))
			throw std::runtime_error(std::to_string(line.line_number) + ": No file \"" + real_file.string() + "\"");

		if (!real_file.has_extension() || (real_file.extension() != ".svg" && real_file.extension() != ".SVG"))
			throw std::runtime_error(std::to_string(line.line_number) + ": Expected .svg file \"" + real_file.string() + "\"");

		if (converted_png.empty()) converted_png = get_random_temp_file().string() + ".png";
		const std::filesystem::path converted_tga = (real_file.parent_path() / real_file.stem()).string() + ".tga";
		const std::filesystem::path recorded_converted_tga = (recorded_file.parent_path() / recorded_file.stem()).string() + ".tga";
		const bool converted_tga_exists = std::filesystem::exists(converted_tga);

		const bool conversion_needed =
			reconvert ||
			!converted_tga_exists ||
			std::filesystem::last_write_time(real_file) > std::filesystem::last_write_time(converted_tga)
		;

		if (conversion_needed)
		{
			run_cmd("rsvg-convert --width " + width_string + " --height " + height_string + " " + real_file.string() + " > " + converted_png);
			run_cmd("convert -auto-orient " + converted_png + " " + converted_tga.string());
		}

		std::filesystem::remove(converted_png);

		if (recreate || (!exclude && std::filesystem::last_write_time(converted_tga) > std::filesystem::last_write_time(roll_file)))
			recreate = true;

		if (!exclude)
			items.emplace_back(converted_tga.string(), recorded_converted_tga.string(), true);
	}
}

void Recipe::process_atlas_section(const RecipeInputSection &section)
{
	if (section.options.size() > 0)
		throw std::runtime_error(std::to_string(section.line_number) + ": Unrecognized atlas section options");

	for (const RecipeInputLine &line : section.lines)
	{
		if (line.tokens.size() < 1)
			continue;

		if (line.tokens.size() != 2)
			throw std::runtime_error(std::to_string(line.line_number) + ": Need 2 args: LayoutFile and AtlasFile");

		const std::filesystem::path layout_file = line.tokens.at(0);
		const std::filesystem::path real_layout_file = get_real_file_path(layout_file);

		const std::filesystem::path recorded_atlas_file = line.tokens.at(1);
		const std::filesystem::path real_atlas_file = get_real_file_path(recorded_atlas_file);

		const bool real_atlas_file_exists = std::filesystem::exists(real_atlas_file);

		if (!std::filesystem::exists(real_layout_file))
			throw std::runtime_error(std::to_string(line.line_number) + ": Layout file \"" + real_layout_file.string() + "\" doesn't exist");

		bool conversion_needed =
			reconvert ||
			!real_atlas_file_exists ||
			std::filesystem::last_write_time(real_layout_file) > std::filesystem::last_write_time(real_atlas_file)
		;

		const std::vector<std::string> atlas_items = run_cmd("atlasizer --list " + real_layout_file.string(), false);
		for (const std::string &item : atlas_items)
		{
			const std::filesystem::path file = std::filesystem::relative(std::filesystem::canonical(real_layout_file).parent_path() / item);
			if (!std::filesystem::exists(file))
				throw std::runtime_error(std::to_string(line.line_number) + ": Atlas item \"" + file.string() + "\" doesn't exist");

			if (real_atlas_file_exists && std::filesystem::last_write_time(file) > std::filesystem::last_write_time(real_atlas_file))
				conversion_needed = true;
		}

		if (conversion_needed)
		{
			recreate = true;
			run_cmd("atlasizer " + real_layout_file.string() + " " + real_atlas_file.string());
		}

		items.emplace_back(real_atlas_file.string(), recorded_atlas_file.string(), true);
	}
}

std::filesystem::path Recipe::get_real_file_path(const std::filesystem::path &file)
{
	return std::filesystem::relative(std::filesystem::canonical(recipe_file).parent_path() / file);
}

std::vector<std::string> Recipe::run_cmd(const std::string &cmd, bool echo)
{
	std::vector<std::string> output;

	if (echo)
		std::cout << "$ " << cmd << std::endl;

#if defined __linux__
	FILE *file = popen(cmd.c_str(), "r");
#elif defined _WIN32
	FILE *file = _popen(cmd.c_str(), "r");
#endif
	if (file == NULL)
		throw std::runtime_error("Couldn't run command");
	char buf[500];

	while (fgets(buf, sizeof(buf), file) != NULL)
	{
		std::string line = buf;
		if (line.at(line.size() - 1) == '\n')
			line.erase(line.end() - 1);
		if (line.at(line.size() - 1) == '\r')
			line.erase(line.end() - 1);

		output.push_back(line);
	}

	int exit_code;
#if defined __linux__
	if ((exit_code = pclose(file)) != 0)
#elif defined _WIN32
	if ((exit_code = _pclose(file)) != 0)
#endif
		throw std::runtime_error("Command failed with exit code " + std::to_string(exit_code));

	return output;
}

std::filesystem::path Recipe::get_random_temp_file()
{
	std::random_device rd;

	char r[11];

	for (int i = 0; i < 10; ++i)
		r[i] = (char)std::uniform_int_distribution<int>('a', 'z')(rd);

	r[10] = 0;

	return std::filesystem::temp_directory_path() / r;
}
