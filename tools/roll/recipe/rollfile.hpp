#ifndef ROLLFILE_HPP
#define ROLLFILE_HPP

#include <fstream>
#include <memory>

#include "rollfileparser.hpp"

class Rollfile
{
public:
	Rollfile(const std::string&, bool);

	std::vector<RollItem> get_items() const;
	bool update() const;

private:
	void process_root_item(const RollfileInputLine&);
	std::string get_file_path(const std::string&);
	bool is_newer(const std::string&);

	std::string rollfile;
	bool needs_update;
	std::vector<RollItem> items;
};

#endif
