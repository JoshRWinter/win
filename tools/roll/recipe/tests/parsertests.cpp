#include <iostream>
#include <filesystem>
#include <fstream>

#include "../recipeparser.hpp"
#include "test.hpp"

int main()
{
	return run_tests();
}

std::vector<std::string> cmds;
static std::string write(const std::string &t)
{
	for (const std::string &cmd : cmds)
		if (cmd == t)
			std::cerr << "Warning: duplicate test - \"" << cmd << "\"" << std::endl;

	cmds.push_back(t);

	const std::string tmpfile = std::filesystem::temp_directory_path() / "recipe-parser-tests.txt";
	std::ofstream out(tmpfile);
	if (!out)
		throw std::runtime_error("couldn't write to tmp directory");

	out.write(t.c_str(), t.size());

	return tmpfile;
}

TEST()
{
	auto inputs = RecipeParser::parse(write(""));

	is_true(inputs.size() == 0);
}

TEST()
{
	auto inputs = RecipeParser::parse(write("\n"));

	is_true(inputs.size() == 0);
}

TEST()
{
	auto inputs = RecipeParser::parse(write("  \n  "));

	is_true(inputs.size() == 0);
}

TEST()
{
	auto inputs = RecipeParser::parse(write("word"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("word", inputs.at(0).line.text);
	are_equal(1, inputs.at(0).line.tokens.size());
	are_equal("word", inputs.at(0).line.tokens.at(0));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("\"word\""));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("\"word\"", inputs.at(0).line.text);
	are_equal(1, inputs.at(0).line.tokens.size());
	are_equal("word", inputs.at(0).line.tokens.at(0));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("\"wo\\\"rd\""));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("\"wo\\\"rd\"", inputs.at(0).line.text);
	are_equal(1, inputs.at(0).line.tokens.size());
	are_equal("wo\"rd", inputs.at(0).line.tokens.at(0));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("word hello"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("word hello", inputs.at(0).line.text);
	are_equal(2, inputs.at(0).line.tokens.size());
	are_equal("word", inputs.at(0).line.tokens.at(0));
	are_equal("hello", inputs.at(0).line.tokens.at(1));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("\"word\" hello"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("\"word\" hello", inputs.at(0).line.text);
	are_equal(2, inputs.at(0).line.tokens.size());
	are_equal("word", inputs.at(0).line.tokens.at(0));
	are_equal("hello", inputs.at(0).line.tokens.at(1));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("    \" word  \"    hello     # comment"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("    \" word  \"    hello     # comment", inputs.at(0).line.text);
	are_equal(2, inputs.at(0).line.tokens.size());
	are_equal(" word  ", inputs.at(0).line.tokens.at(0));
	are_equal("hello", inputs.at(0).line.tokens.at(1));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("    \" word  \"    hello     #comment"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Line, (int)inputs.at(0).type);
	are_equal("    \" word  \"    hello     #comment", inputs.at(0).line.text);
	are_equal(2, inputs.at(0).line.tokens.size());
	are_equal(" word  ", inputs.at(0).line.tokens.at(0));
	are_equal("hello", inputs.at(0).line.tokens.at(1));
}

TEST()
{
	auto inputs = RecipeParser::parse(write("sectionname{\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("sectionname", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool a{\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(1, inputs.at(0).section.options.size());
	are_equal("a", inputs.at(0).section.options.at(0));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool a {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(1, inputs.at(0).section.options.size());
	are_equal("a", inputs.at(0).section.options.at(0));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool a b{\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(2, inputs.at(0).section.options.size());
	are_equal("a", inputs.at(0).section.options.at(0));
	are_equal("b", inputs.at(0).section.options.at(1));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool a b  {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(2, inputs.at(0).section.options.size());
	are_equal("a", inputs.at(0).section.options.at(0));
	are_equal("b", inputs.at(0).section.options.at(1));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool \"a b\"  {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(1, inputs.at(0).section.options.size());
	are_equal("a b", inputs.at(0).section.options.at(0));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("cool \"a\\\" b\"  {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool", inputs.at(0).section.name);
	are_equal(1, inputs.at(0).section.options.size());
	are_equal("a\" b", inputs.at(0).section.options.at(0));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("\"cool section\" a b  {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool section", inputs.at(0).section.name);
	are_equal(2, inputs.at(0).section.options.size());
	are_equal("a", inputs.at(0).section.options.at(0));
	are_equal("b", inputs.at(0).section.options.at(1));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("\"cool\\\" section\" a b  {\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("cool\" section", inputs.at(0).section.name);
	are_equal(2, inputs.at(0).section.options.size());
	are_equal("a", inputs.at(0).section.options.at(0));
	are_equal("b", inputs.at(0).section.options.at(1));
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("{\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write("  {   \n  }"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(0, inputs.at(0).section.lines.size());
}

TEST()
{
	auto inputs = RecipeParser::parse(write(" section {\n\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("section", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(1, inputs.at(0).section.lines.size());
	are_equal(0, inputs.at(0).section.lines.at(0).tokens.size());
	are_equal("", inputs.at(0).section.lines.at(0).text);
}

TEST()
{
	auto inputs = RecipeParser::parse(write(" section {\n hello\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("section", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(1, inputs.at(0).section.lines.size());
	are_equal(1, inputs.at(0).section.lines.at(0).tokens.size());
	are_equal(" hello", inputs.at(0).section.lines.at(0).text);
	are_equal("hello", inputs.at(0).section.lines.at(0).tokens.at(0));
}

TEST()
{
	auto inputs = RecipeParser::parse(write(" section {\n hello\n\"test\"\n}"));

	are_equal(1, inputs.size());
	are_equal((int)RecipeRootInputType::Section, (int)inputs.at(0).type);
	are_equal("section", inputs.at(0).section.name);
	are_equal(0, inputs.at(0).section.options.size());
	are_equal(2, inputs.at(0).section.lines.size());
	are_equal(1, inputs.at(0).section.lines.at(0).tokens.size());
	are_equal(" hello", inputs.at(0).section.lines.at(0).text);
	are_equal("hello", inputs.at(0).section.lines.at(0).tokens.at(0));
	are_equal(1, inputs.at(0).section.lines.at(1).tokens.size());
	are_equal("\"test\"", inputs.at(0).section.lines.at(1).text);
	are_equal("test", inputs.at(0).section.lines.at(1).tokens.at(0));
}

TEST()
{
	try
	{
		auto inputs = RecipeParser::parse(write("\"hello\\\""));
	}
	catch (const std::exception &e)
	{
		is_true(std::string(e.what()).find("Missing closing quote") != std::string::npos);
		return;
	}

	throw std::runtime_error("no exception");
}

TEST()
{
	try
	{
		auto inputs = RecipeParser::parse(write("\"hell\\o\""));
	}
	catch (const std::exception &e)
	{
		is_true(std::string(e.what()).find("Unrecognized escape sequence") != std::string::npos);
		return;
	}

	throw std::runtime_error("no exception");
}
