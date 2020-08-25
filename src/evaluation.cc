#include "evaluation.hh"

#include "movegen.hh"

using namespace chess;

Value eval::evaluate(const Position &position)
{
	Value value = Draw;

	// Material
	value += PawnValue   * position.count(Colour::White, PieceType::Pawn);
	value += KnightValue * position.count(Colour::White, PieceType::Knight);
	value += BishopValue * position.count(Colour::White, PieceType::Bishop);
	value += RookValue   * position.count(Colour::White, PieceType::Rook);
	value += QueenValue  * position.count(Colour::White, PieceType::Queen);

	value -= PawnValue   * position.count(Colour::Black, PieceType::Pawn);
	value -= KnightValue * position.count(Colour::Black, PieceType::Knight);
	value -= BishopValue * position.count(Colour::Black, PieceType::Bishop);
	value -= RookValue   * position.count(Colour::Black, PieceType::Rook);
	value -= QueenValue  * position.count(Colour::Black, PieceType::Queen);

	return position.side_to_move() == Colour::White ? value : -value;
}
