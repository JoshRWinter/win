#include <fstream>

#include "rollfileparser.hpp"

std::vector<RollfileRootInput> RollfileParser::parse(std::ifstream &file)
{
	std::vector<RollfileRootInput> inputs;

	bool in_section = false;
	std::string section_name;
	std::vector<std::string> section_options;
	std::vector<RollfileInputLine> section_inputs;

	int line_number = 0;
    while (!file.eof())
	{
		++line_number;


		std::string line;
		std::getline(file, line);

		RollfileInputLine inputline;
		inputline.line_number = line_number;
		inputline.text = line;

		try
		{
			inputline.tokens = tokenize(line);
		}
		catch (const std::exception &e)
		{
			throw std::runtime_error(std::to_string(line_number) + ": " + e.what());
		}

		const auto classification = get_input_classification(inputline);

		if (classification == RollfileInputLineClassification::SectionStart)
		{
	    	if (in_section)
				throw std::runtime_error(std::to_string(line_number) + ": Section start marker was not expected");

			in_section = true;
	    	section_options.clear();
			get_section_header(inputline, section_name, section_options);
		}
		else if (classification == RollfileInputLineClassification::SectionEnd)
		{
			if (!in_section)
				throw std::runtime_error(std::to_string(line_number) + ": Section end marker was not expected");

			in_section = false;
			inputs.emplace_back(RollfileInputSection(section_name, section_options, section_inputs));
		}

		else if (in_section)
			section_inputs.push_back(inputline);
		else if (inputline.tokens.size() > 0)
			inputs.emplace_back(inputline);
	}

	if (in_section)
		throw std::runtime_error("Missing section end marker");

	return inputs;
}

RollfileInputLineClassification RollfileParser::get_input_classification(const RollfileInputLine &input)
{
    if (input.tokens.size() == 0)
		return RollfileInputLineClassification::Input;

	if (input.tokens.at(input.tokens.size() - 1) == "{")
		return RollfileInputLineClassification::SectionStart;

	if (input.tokens.at(input.tokens.size() - 1).find("{") == input.tokens.at(input.tokens.size() - 1).length() - 1)
		return RollfileInputLineClassification::SectionStart;

	if (input.tokens.size() == 1 && input.tokens.at(0) == "}")
		return RollfileInputLineClassification::SectionEnd;

	return RollfileInputLineClassification::Input;
}

void RollfileParser::get_section_header(const RollfileInputLine &input, std::string &section_name, std::vector<std::string> &section_options)
{
	section_name = "";
	section_options.clear();

	if (input.tokens.size() > 0)
		section_name = input.tokens.at(0);

	for (auto it = input.tokens.begin() + 1; it != input.tokens.end() - 1; ++it)
		section_options.push_back(*it);

	const std::string &last_token = input.tokens.at(input.tokens.size() - 1);
	if (last_token != "{")
	{
		std::string copy = last_token;
		copy.erase(copy.end() - 1);
		section_options.push_back(copy);
	}
}

std::vector<std::string> RollfileParser::tokenize(const std::string &line)
{
	std::vector<std::string> tokens;
	std::string s = line;

	std::string token;
	while (next_token(s, token))
		tokens.push_back(token);

	return tokens;
}

bool RollfileParser::next_token(std::string &line, std::string &token)
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

int RollfileParser::find_token_start(const std::string &s, bool &quoted)
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

int RollfileParser::find_token_end(bool quoted, const std::string &s, int token_start)
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
		    	throw std::runtime_error("Unrecognized escape sequence");

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

std::string RollfileParser::strip_escapes(const std::string &s)
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

bool RollfileParser::is_token_char(int c)
{
	return std::isprint(c) && !std::isspace(c);
}
