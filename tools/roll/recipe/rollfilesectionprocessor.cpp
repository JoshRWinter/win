#include <stdexcept>
#include <string.h>

#include "rollfilesectionprocessor.hpp"

#ifdef __linux__
#include <stdio.h>
#endif

RollFileSectionProcessor::~RollFileSectionProcessor()
{
}

bool RollFileSectionProcessor::run_process(const std::string &cmd, std::string &output)
{
	output = "";
#ifdef __linux__
	FILE *f = popen(cmd.c_str(), "r");
	if (f == NULL)
	{
		output = "Couldn't run \"" + cmd + "\"";
		return false;;
	}

    char buf[1024];
	memset(buf, 0, sizeof(buf));
	fgets(buf, sizeof(buf), f);
	buf[sizeof(buf) - 1] = 0;

	if (pclose(f) == 0)
		return true;

	output = buf;
	return false;

#endif
}

void RollFileSectionProcessor::parse_input_line(const RollInputLine &line, std::string &file, std::vector<std::string> &options)
{
	options.clear();

	std::string leftovers = line.text;

	// get the filename
	if (line.text.at(0) == '"')
	{
		const auto close_quote = line.text.substr(1).find('"');
		if (close_quote == std::string::npos)
			throw std::runtime_error(std::to_string(line.line_number) + ": Missing closing quote");

		file = line.text.substr(1, close_quote);

		leftovers = leftovers.substr(close_quote + 2);
	}
    else
	{
		const auto space = line.text.find(' ');
		if (space == std::string::npos)
		{
	    	leftovers = "";
			file = line.text;
		}
		else
		{
			file = line.text.substr(0, space);
			leftovers = leftovers.substr(space + 1);
		}
	}

	while (true)
	{
		leftovers = trim(leftovers);

		const auto space = leftovers.find(' ');
		if (space == std::string::npos)
		{
			if (leftovers.length() > 0)
				options.push_back(leftovers);

			break;
		}

		options.push_back(leftovers.substr(0, space));
		leftovers = leftovers.substr(space + 1);
	}
}

std::string RollFileSectionProcessor::trim(const std::string &s)
{
	std::string str = s;

	// trim beginning
	for(int i = 0; i < str.size(); ++i)
	{
		if(isspace(str.at(i)))
		{
			str.erase(str.begin() + i);
			--i;
		}
		else
			break;
	}

	// trim end
	for(int i = str.size() - 1; i >= 0; --i)
	{
		if(isspace(str.at(i)))
			str.erase(str.begin() + i);
		else
			break;
	}

	return str;
}
