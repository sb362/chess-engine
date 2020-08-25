#include "moveorder.hh"
#include "evaluation.hh"

using namespace chess;

constexpr Value HashMoveOffset     = 30000;

constexpr Value PromotionsOffset   = 20000;

constexpr Value GoodCapturesOffset = 20000;
constexpr Value EvenCapturesOffset = 20000;
constexpr Value BadCapturesOffset  = 20000;

constexpr Value KillerMovesOffset  = 20000;
constexpr Value QuietsOffset       = 10000;

void search::evaluate_move_list(const Position &position, MoveList &move_list, const Depth depth,
								const Move &hash_move, const Heuristics &heuristics)
{
	for (MoveWithValue &move : move_list)
	{
		if (move == hash_move)
		{
			move.value = HashMoveOffset;
			continue;
		}

		const bool is_capture = position.is_capture(move);
		const Piece moved_piece = position.moved_piece(move);

		if (move.is_promotion())
		{
			move.value = PromotionsOffset + eval::piece_value(move.promotion());
			move.value += is_capture ? eval::piece_value(type_of(position.captured_piece(move)))
									 : 0;
		}
		else if (is_capture)
		{
			const Value see = position.see(move);
			if (see > 0)
				move.value = GoodCapturesOffset + see;
			else if (see < 0)
				move.value = BadCapturesOffset + see;
			else /*if (see == 0)*/
				move.value = EvenCapturesOffset;

			continue;
		}
		else /*if (is_quiet)*/
		{
			if (heuristics.killer[depth].is_killer(move))
				move.value = KillerMovesOffset;
			else
				move.value = QuietsOffset + heuristics.history.probe(moved_piece, move.to());
		}
	}
}
