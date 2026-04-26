#pragma once

#include <fstream>

#include "recipeparser.hpp"

class Recipe
{
    struct ShadowDef
    {
        std::string name;
        std::vector<float> weights;
    };

public:
    Recipe(const std::filesystem::path &, const std::filesystem::path &);

    std::vector<RollItem> get_items(bool &) const;

private:
    void process_root_line(const RecipeInputLine &section);
    void process_svg2tga_section(const RecipeInputSection &section);
    void process_atlas_section(const RecipeInputSection &section);
    void process_shadowdef_section(const RecipeInputSection &section);
    std::filesystem::path get_real_file_path(const std::filesystem::path &);
    static std::vector<std::string> run_cmd(const std::string &, bool = true);
    static std::filesystem::path get_random_temp_file();

    std::filesystem::path recipe_file;
    std::filesystem::path roll_file;
    std::filesystem::file_time_type recipe_file_lastwrite;
    std::filesystem::file_time_type roll_file_lastwrite;
    bool recreate;
    std::vector<RollItem> items;
    std::vector<ShadowDef> shadowdefs;
};
