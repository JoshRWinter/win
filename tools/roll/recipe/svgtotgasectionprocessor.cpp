#include <stdexcept>
#include <iostream>

#include "svgtotgasectionprocessor.hpp"

SvgToTgaSectionProcessor::SvgToTgaSectionProcessor(std::vector<RollInputLine> inputs, bool force_update)
	: force_update(force_update)
	, needs_update(false)
{
	for (const RollInputLine &input : inputs)
	{
		std::string file;
		std::vector<std::string> options;

		parse_input_line(input, file, options);

		if (options.size() != 2)
			throw std::runtime_error(std::to_string(input.line_number) + ": svg_to_tga inputs must have 2 options (width, height)");

		int width;
		int height;

		if (sscanf(options.at(0).c_str(), "%d", &width) != 1)
			throw std::runtime_error(std::to_string(input.line_number) + ": svg_to_tga: Invalid width \"" + options.at(0) + "\"");
		if (sscanf(options.at(1).c_str(), "%d", &height) != 1)
			throw std::runtime_error(std::to_string(input.line_number) + ": svg_to_tga: Invalid height \"" + options.at(1) + "\"");

		std::string rsvg_convert = "farticus fart";
		std::string output;
		std::cout << "$ rsvg_convert" << std::endl;

		if (!run_process(rsvg_convert, output))
			throw std::runtime_error(std::to_string(input.line_number) + ": svg_to_tga: Couldn't run command. " + output);

		items.emplace_back(file, true);
	}
}

SvgToTgaSectionProcessor::~SvgToTgaSectionProcessor()
{
}

bool SvgToTgaSectionProcessor::update() const
{
	return needs_update || force_update;
}

std::vector<RollItem> SvgToTgaSectionProcessor::get_items() const
{
	return items;
}
