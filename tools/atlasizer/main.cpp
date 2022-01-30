#include <iostream>
#include <string.h>
#include "layoutexporter.hpp"

#include "atlasizer.hpp"

static const char *helptext =
		"atlasizer layout_file output_file";

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		gui();
		return 0;
	}
	else if (argc == 2 || argc > 3)
	{
		std::cout << helptext << std::endl;
		return 0;
	}

	try
	{
		if (!strcmp(argv[1], "--list"))
		{
			int padding;
			const auto descriptors = LayoutExporter::import(argv[2], padding, false);
			for (const auto &descriptor : descriptors)
				std::cout << descriptor.filename << std::endl;

			return 0;
		}

		compileatlas(argv[1], argv[2]);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
