#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include "magic.hh"

using namespace chess;

int main(int argc, char *argv[])
{
	bitboards::init();
	magics::init();

	Catch::Session session;

	int status = session.applyCommandLine(argc, argv);
	if (status == 0)
		status = session.run();

	return status;
}
