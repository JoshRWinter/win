#ifndef ROLLFILEPARSER_HPP
#define ROLLFILEPARSER_HPP

#include <vector>
#include <filesystem>

#include "../roll.hpp"

enum class RecipeInputLineClassification
{
	Input,
	SectionStart,
	SectionEnd
};

struct RecipeInputLine
{
	RecipeInputLine() : line_number(0) {}

	std::vector<std::string> tokens;
	std::string text;
	int line_number;
};

struct RecipeInputSection
{
	RecipeInputSection(const std::string &name, const std::vector<std::string> &options, const std::vector<RecipeInputLine> &lines, int line_number)
		: name(name)
		, options(options)
		, lines(lines)
		, line_number(line_number)
	{}

	RecipeInputSection() = default;

	std::string name;
	std::vector<std::string> options;
	std::vector<RecipeInputLine> lines;
	int line_number;
};

enum class RecipeRootInputType
{
	Line,
	Section
};

struct RecipeRootInput
{
	RecipeRootInput(const RecipeInputLine &line)
		: type(RecipeRootInputType::Line)
		, line(line)
	{}

	RecipeRootInput(const RecipeInputSection &section)
		: type(RecipeRootInputType::Section)
		, section(section)
	{}

	RecipeRootInputType type;

	RecipeInputLine line;
	RecipeInputSection section;
};

class RecipeParser
{
public:
	static std::vector<RecipeRootInput> parse(const std::filesystem::path&);

private:
	static RecipeInputLineClassification get_input_classification(const RecipeInputLine&);
	static void get_section_header(const RecipeInputLine&, std::string&, std::vector<std::string>&);
	static std::vector<std::string> cut_comments(const std::vector<std::string>&);
	static std::vector<std::string> tokenize(const std::string&);
	static bool next_token(std::string&, std::string&);
	static int find_token_start(const std::string&, bool&);
	static int find_token_end(bool, const std::string&, int);
	static std::string strip_escapes(const std::string&);
	static bool is_token_char(int);
};

#endif
