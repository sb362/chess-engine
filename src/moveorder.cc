#include "moveorder.hh"
#include "evaluation.hh"

using namespace chess;
using namespace eval;

constexpr Value HashMoveOffset    = 30000;

constexpr Value PromotionsOffset  = 20000;

constexpr Value CapturesOffset    = 20000;

constexpr Value KillerMovesOffset = 20000;
constexpr Value QuietsOffset      = 10000;

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
			move.value = PromotionsOffset + piece_value(move.promotion());
			move.value += is_capture ? piece_value(type_of(position.captured_piece(move)))
									 : 0;
		}
		else if (is_capture)
		{
			// todo: static exchange evaluation
			//const Value see = position.see(move);
			//move.value = CapturesOffset + see;
			//continue;
			const Piece captured_piece = position.captured_piece(move);
			const Value delta = piece_value(type_of(captured_piece)) - piece_value(type_of(moved_piece));

			move.value = CapturesOffset + delta;
		}
		else
		{
			if (heuristics.killer[depth].is_killer(move))
				move.value = KillerMovesOffset;
			else
				move.value = QuietsOffset + heuristics.history.probe(moved_piece, move.to());
			
			// todo: pawn sac bonus?
		}
	}
}

void search::evaluate_move_list(const Position &position, MoveList &move_list)
{
	for (MoveWithValue &move : move_list)
	{
		const bool is_capture = position.is_capture(move);
		const Piece moved_piece = position.moved_piece(move);

		if (move.is_promotion())
		{
			move.value = PromotionsOffset + piece_value(move.promotion());
			move.value += is_capture ? piece_value(type_of(position.captured_piece(move)))
									 : 0;
		}
		else if (is_capture)
		{
			const Piece captured_piece = position.captured_piece(move);
			const Value delta = piece_value(type_of(captured_piece)) - piece_value(type_of(moved_piece));

			move.value = CapturesOffset + delta;
		}
	}
}
