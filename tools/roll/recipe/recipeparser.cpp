#include <fstream>
#include <cctype>

#include "recipeparser.hpp"

std::vector<RecipeRootInput> RecipeParser::parse(const std::filesystem::path &file)
{
	std::ifstream stream(file);
	if (!stream)
		throw std::runtime_error("Couldn't open \"" + file.string() + "\" for reading");

	std::vector<RecipeRootInput> inputs;

	bool in_section = false;
	std::string section_name;
	std::vector<std::string> section_options;
	std::vector<RecipeInputLine> section_lines;

	int line_number = 0;
	while (!stream.eof())
	{
		++line_number;


		std::string line;
		std::getline(stream, line);

		RecipeInputLine inputline;
		inputline.line_number = line_number;
		inputline.text = line;

		try
		{
			inputline.tokens = cut_comments(tokenize(line));
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(std::to_string(line_number) + ": " + e.what());
		}

		const auto classification = get_input_classification(inputline);

		if (classification == RecipeInputLineClassification::SectionStart)
		{
			if (in_section)
				throw std::runtime_error(std::to_string(line_number) + ": Section start marker was not expected");

			in_section = true;
			section_options.clear();
			section_lines.clear();
			get_section_header(inputline, section_name, section_options);
		}
		else if (classification == RecipeInputLineClassification::SectionEnd)
		{
			if (!in_section)
				throw std::runtime_error(std::to_string(line_number) + ": Section end marker was not expected");

			in_section = false;
			inputs.emplace_back(RecipeInputSection(section_name, section_options, section_lines, line_number));
		}

		else if (in_section)
			section_lines.push_back(inputline);
		else if (inputline.tokens.size() > 0)
			inputs.emplace_back(inputline);
	}

	if (in_section)
		throw std::runtime_error("Missing section end marker");

	return inputs;
}

RecipeInputLineClassification RecipeParser::get_input_classification(const RecipeInputLine &input)
{
	if (input.tokens.size() == 0)
		return RecipeInputLineClassification::Input;

	if (input.tokens.at(input.tokens.size() - 1) == "{")
		return RecipeInputLineClassification::SectionStart;

	if (input.tokens.at(input.tokens.size() - 1).find("{") == input.tokens.at(input.tokens.size() - 1).length() - 1)
		return RecipeInputLineClassification::SectionStart;

	if (input.tokens.size() == 1 && input.tokens.at(0) == "}")
		return RecipeInputLineClassification::SectionEnd;

	return RecipeInputLineClassification::Input;
}

void RecipeParser::get_section_header(const RecipeInputLine &input, std::string &section_name, std::vector<std::string> &section_options)
{
	section_name = "";
	section_options.clear();

	if (input.tokens.size() == 1)
	{
		if (input.tokens.at(0) == "{")
			return;
		else
		{
			section_name = input.tokens.at(0);
			section_name.erase(section_name.end() - 1);
			return;
		}
	}

	if (input.tokens.size() > 1)
		section_name = input.tokens.at(0);

	for (int i = 1; i < input.tokens.size() - 1; ++i)
		section_options.push_back(input.tokens.at(i));

	if (input.tokens.at(input.tokens.size() - 1) != "{")
	{
		std::string tail = input.tokens.at(input.tokens.size() - 1);
		tail.erase(tail.end() - 1);
		section_options.push_back(tail);
	}
}

std::vector<std::string> RecipeParser::cut_comments(const std::vector<std::string> &tokens)
{
	std::vector<std::string> new_tokens;
	for (const std::string &token : tokens)
	{
		if (token == "#" || (token.length() > 0 && token.at(0) == '#'))
			return new_tokens;

		new_tokens.emplace_back(token);
	}

	return new_tokens;
}

std::vector<std::string> RecipeParser::tokenize(const std::string &line)
{
	std::vector<std::string> tokens;
	std::string s = line;

	std::string token;
	while (next_token(s, token))
		tokens.push_back(token);

	return tokens;
}

bool RecipeParser::next_token(std::string &line, std::string &token)
{
	bool quoted_token = false;
	const int token_start = find_token_start(line, quoted_token);
	const int token_end = find_token_end(quoted_token, line, token_start);

	if (token_start < 0)
	{
		token = "";
		line = "";

		return false;
	}

	const std::string t = line.substr(token_start, (token_end + 1) - token_start);
	line = line.substr(token_end + 1 + (quoted_token ? 1 : 0));

	token = strip_escapes(t);

	return true;
}

int RecipeParser::find_token_start(const std::string &s, bool &quoted)
{
	for (int i = 0; i < s.length(); ++i)
	{
		const char c = s.at(i);

		if (is_token_char(c))
		{
			if (c == '"')
			{
				quoted = true;
				return i + 1;
			}
			else
				return i;
		}
	}

	return -1;
}

int RecipeParser::find_token_end(bool quoted, const std::string &s, int token_start)
{
	bool escape = false;

	for (int i = token_start; i < s.length(); ++i)
	{
		const char c = s.at(i);

		const bool is_quote = c == '"';
		const bool is_sep = !is_token_char(c);
		const bool is_escape = c == '\\';

		if (escape)
		{
			if (!is_quote)
				throw std::runtime_error(std::string("Unrecognized escape sequence \"\\") + c + "\"");

			escape = false;
			continue;
		}

		if (is_escape)
		{
			escape = true;
			continue;
		}

		if (is_quote && !quoted)
			return i - 1;
		else if (is_quote && quoted)
			return i - 1;
		else if (is_sep && !quoted)
			return i - 1;
	}

	if (quoted)
		throw std::runtime_error("Missing closing quote");

	return s.length() - 1;
}

std::string RecipeParser::strip_escapes(const std::string &s)
{
	std::string stripped = s;

	for (auto it = stripped.begin(); it != stripped.end();)
	{
		const char c = *it;

		if (c == '\\')
		{
			stripped.erase(it);
			continue;
		}

		++it;
	}

	return stripped;
}

bool RecipeParser::is_token_char(int c)
{
	return std::isprint(c) && !std::isspace(c);
}
