#ifndef ROLLFILESECTIONPROCESSOR_HPP
#define ROLLFILESECTIONPROCESSOR_HPP

#include <string>
#include <vector>

#include "../roll.hpp"

struct RollInputLine
{
	std::string text;
	std::string raw_text;
	int line_number;
};

class RollFileSectionProcessor
{
public:
	virtual ~RollFileSectionProcessor();

	virtual bool update() const = 0;
	virtual std::vector<RollItem> get_items() const = 0;

protected:
	static bool run_process(const std::string&, std::string&);
	static void parse_input_line(const RollInputLine&, std::string&, std::vector<std::string>&);
	static std::string trim(const std::string&);
};

#endif
