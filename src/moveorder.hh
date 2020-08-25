#pragma once

#include "movegen.hh"
#include "heuristics.hh"

namespace chess::search
{
	extern void evaluate_move_list(const Position &position, MoveList &move_list, const Depth depth,
								   const Move &hash_move, const Heuristics &heuristics);
	
	extern void evaluate_move_list(const Position &position, MoveList &move_list);
}
