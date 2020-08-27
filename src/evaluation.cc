#include "evaluation.hh"

#include "movegen.hh"

using namespace chess;

/**
 * @brief Evaluation function
 * 
 * @param position 
 * @return Value Evaluation of @param position relative to the side to move
 */
Value eval::evaluate(const Position &position)
{
	const Colour us = position.side_to_move(), them = ~us;

	Value value = Draw;

	// Our material
	value += PawnValue   * position.count(us, PieceType::Pawn);
	value += KnightValue * position.count(us, PieceType::Knight);
	value += BishopValue * position.count(us, PieceType::Bishop);
	value += RookValue   * position.count(us, PieceType::Rook);
	value += QueenValue  * position.count(us, PieceType::Queen);

	// Their material
	value -= PawnValue   * position.count(them, PieceType::Pawn);
	value -= KnightValue * position.count(them, PieceType::Knight);
	value -= BishopValue * position.count(them, PieceType::Bishop);
	value -= RookValue   * position.count(them, PieceType::Rook);
	value -= QueenValue  * position.count(them, PieceType::Queen);

	//if (!position.checkers())
	//{
		// Our mobility
	//	value += (OurMobilityA * approx_mobility(position, us)) / OurMobilityB;

		// Their mobility
	//	value -= (TheirMobilityA * approx_mobility(position, them)) / TheirMobilityB;
	//}

	return value;
}
