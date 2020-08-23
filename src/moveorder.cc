#include "moveorder.hh"
#include "evaluation.hh"

using namespace chess;

constexpr Value HashMoveOffset        = 30000;
constexpr Value PromotionsOffset      = 20000;
constexpr Value CapturesOffset        = 20000;
constexpr Value KillerMovesOffset     = 20000;
constexpr Value QuietsOffset          = 10000;

void search::evaluate_move_list(const Position &position, MoveList &move_list, const Depth depth,
								const Move &hash_move, const Heuristics &heuristics)
{
	for (MoveWithValue &move : move_list)
	{
		
	}
}
