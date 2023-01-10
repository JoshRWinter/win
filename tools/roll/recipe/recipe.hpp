#pragma once

#include <fstream>
#include <memory>
#include <random>

#include "recipeparser.hpp"

class Recipe
{
public:
	Recipe(const std::filesystem::path&, const std::filesystem::path&);

	std::vector<RollItem> get_items(bool&) const;

private:
	void process_root_line(const RecipeInputLine&);
	void process_svg2tga_section(const RecipeInputSection&);
	void process_atlas_section(const RecipeInputSection&);
	std::filesystem::path get_real_file_path(const std::filesystem::path&);
	static std::vector<std::string> run_cmd(const std::string&, bool = true);
	static std::filesystem::path get_random_temp_file();

	std::filesystem::path recipe_file;
	std::filesystem::path roll_file;
	bool recreate;
	bool reconvert;
	std::vector<RollItem> items;
};
