#pragma once

#include "movegen.hh"

namespace chess
{
	extern Nodes perft(const Position &position, const Depth depth);
	extern Nodes divide(const Position &position, const Depth depth);

	extern int perft(int argc, char *argv[]);
} // chess
