#include "evaluation.hh"

#include "movegen.hh"

using namespace chess;

Value eval::evaluate(const Position &position)
{
	const Colour us = position.side_to_move();

	Value value = Draw;

	// Material
	value += PawnValue   * position.count(us, PieceType::Pawn);
	value += KnightValue * position.count(us, PieceType::Knight);
	value += BishopValue * position.count(us, PieceType::Bishop);
	value += RookValue   * position.count(us, PieceType::Rook);
	value += QueenValue  * position.count(us, PieceType::Queen);
	value -= PawnValue   * position.count(~us, PieceType::Pawn);
	value -= KnightValue * position.count(~us, PieceType::Knight);
	value -= BishopValue * position.count(~us, PieceType::Bishop);
	value -= RookValue   * position.count(~us, PieceType::Rook);
	value -= QueenValue  * position.count(~us, PieceType::Queen);

	return value;
}
