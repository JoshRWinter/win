#include <iostream>
#include <filesystem>

#include "rollfile.hpp"
#include "rollfileparser.hpp"

Rollfile::Rollfile(const std::string &file, bool force_update)
	: needs_update (force_update)
	, rollfile(file)
{
	std::ifstream rollfile(file);
    const std::vector<RollfileRootInput> inputs = RollfileParser::parse(rollfile);

	for (const RollfileRootInput &input : inputs)
	{
		if (input.type == RollfileRootInputType::Line)
		{
	    	process_root_item(input.line);
		}
	}
}

std::vector<RollItem> Rollfile::get_items() const
{
	return items;
}

bool Rollfile::update() const
{
	return needs_update;
}

void Rollfile::process_root_item(const RollfileInputLine &line)
{
    try
    {
	    const std::string filepath = get_file_path(line.tokens.at(0));
		if (!std::filesystem::exists(filepath))
			throw std::runtime_error("\"" + filepath + "\" doesn't exist");

	    if (line.tokens.size () > 2)
		    throw std::runtime_error("Unrecognized extra options");

	    bool compress = false;
	    if (line.tokens.size() > 1)
	    {
		    if (line.tokens.at(1) != "z")
			    throw std::runtime_error("Unrecognized option \"" + line.tokens.at(1) + "\"");
		    else
			    compress = true;
	    }

	    items.emplace_back(filepath, compress);
    }
    catch (const std::exception &e)
    {
	    throw std::runtime_error(std::to_string(line.line_number) + ": " + e.what());
    }
}

std::string Rollfile::get_file_path(const std::string &file)
{
	return std::filesystem::absolute(std::filesystem::relative(std::filesystem::absolute(std::filesystem::path(rollfile)).parent_path() / file).string());
}
