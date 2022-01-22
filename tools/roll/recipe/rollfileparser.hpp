#ifndef ROLLFILEPARSER_HPP
#define ROLLFILEPARSER_HPP

#include <vector>

#include "../roll.hpp"

enum class RollfileInputLineClassification
{
	Input,
	SectionStart,
	SectionEnd
};

struct RollfileInputLine
{
	RollfileInputLine() : line_number(0) {}

	std::vector<std::string> tokens;
	std::string text;
	int line_number;
};

struct RollfileInputSection
{
	RollfileInputSection(const std::string &name, const std::vector<std::string> &options, const std::vector<RollfileInputLine> &inputs)
		: name(name)
		, options(options)
		, inputs(inputs)
	{}

	RollfileInputSection() = default;

	std::string name;
	std::vector<std::string> options;
	std::vector<RollfileInputLine> inputs;
};

enum class RollfileRootInputType
{
	Line,
	Section
};

struct RollfileRootInput
{
	RollfileRootInput(const RollfileInputLine &line)
		: type(RollfileRootInputType::Line)
		, line(line)
	{}

	RollfileRootInput(const RollfileInputSection &section)
		: type(RollfileRootInputType::Section)
		, section(section)
	{}

	RollfileRootInputType type;

	RollfileInputLine line;
	RollfileInputSection section;
};

class RollfileParser
{
public:
	static std::vector<RollfileRootInput> parse(std::ifstream&);

private:
	static RollfileInputLineClassification get_input_classification(const RollfileInputLine&);
	static void get_section_header(const RollfileInputLine&, std::string&, std::vector<std::string>&);
	static std::vector<std::string> tokenize(const std::string&);
	static bool next_token(std::string&, std::string&);
	static int find_token_start(const std::string&, bool&);
	static int find_token_end(bool, const std::string&, int);
	static std::string strip_escapes(const std::string&);
	static bool is_token_char(int);
};

#endif
