#include "perft.hh"
#include "uci.hh"

#include "util/cmdline.hh"
#include "util/compiler.hh" // Must be included after everything else to work

#include <iostream>

using namespace chess;
using namespace util;

int main(int argc, char *argv[])
{
	fmt::print("{} {}\n", uci::Name, uci::Version);

#if !defined(NDEBUG)
	fmt::print(
		"{}\n{}\n{}\n{}\n{}\n",
		os_info(), compiler_info(), build_time(), intrinsics_info(), attack_generation_info()
	);
#endif

	bitboards::init();
	magics::init();

	int status = 0;

	if (option_exists(argc, argv, "perft") || option_exists(argc, argv, "divide"))
		status = perft(argc, argv);
	else if (option_exists(argc, argv, "bench"))
	{
		// Benchmark
	}
	else
	{
		std::string line;
		while (std::getline(std::cin, line))
		{
			if (line == "uci")
			{
				uci::main(argc, argv);
				break;
			}
			else if (line == "quit")
				break;
		}
	}

	return status;
}
