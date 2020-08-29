#include "evaluation.hh"

#include "fmt/color.h"

using namespace chess;

constexpr unsigned TraceWidth = 44;

void trace_divider()
{
	fmt::print("{}\n", std::string(TraceWidth, '-'));
}

void trace_begin()
{
	std::string s;

	s += fmt::format("\n|{: <10}|", "");
	s += fmt::format(fmt::emphasis::bold, "{: ^5}", "Us") + '|';
	s += fmt::format(fmt::emphasis::bold, "{: ^5}", "Them") + '|';
	s += fmt::format(fmt::emphasis::bold, "{: ^5}", "Δ") + '|';
	s += fmt::format(fmt::emphasis::bold, "{: ^5}", "Σ") + '|';

	trace_divider();
	fmt::print("{}\n", s);
	trace_divider();
}

void trace(std::string_view term, Value ours, Value theirs)
{
	std::string s = "|";

	s += fmt::format(fmt::emphasis::bold, "{: <10}", term) + '|';
	s += fmt::format("{: >5}", ours) + '|';
	s += fmt::format("{: >5}", theirs) + '|';
	s += fmt::format("{: >5}", ours - theirs) + '|';
	s += fmt::format("{: >5}", ours + theirs) + '|';

	fmt::print("{}\n", s);
}

void trace_end(const Value white_total, const Value their_total)
{
	trace("Total", white_total, their_total);
	trace_divider();
}

/**
 * @brief Evaluation function
 * 
 * @param position 
 * @return Value Evaluation of @param position relative to the side to move
 */
Value eval::evaluate(const Position &position, const pawns::Entry *pawn_entry, const bool do_trace)
{
	ASSERT(!position.checkers());

	const Colour us = position.side_to_move(), them = ~us;

	Value our_total = 0, their_total = 0;
	if (do_trace) trace_begin();

	// Our material
	const Value our_pawn_material   = position.count(us, PieceType::Pawn)   * PawnValue;
	const Value our_knight_material = position.count(us, PieceType::Knight) * KnightValue;
	const Value our_bishop_material = position.count(us, PieceType::Bishop) * BishopValue;
	const Value our_rook_material   = position.count(us, PieceType::Rook)   * RookValue;
	const Value our_queen_material  = position.count(us, PieceType::Queen)  * QueenValue;

	// Their material
	const Value their_pawn_material   = position.count(them, PieceType::Pawn)   * PawnValue;
	const Value their_knight_material = position.count(them, PieceType::Knight) * KnightValue;
	const Value their_bishop_material = position.count(them, PieceType::Bishop) * BishopValue;
	const Value their_rook_material   = position.count(them, PieceType::Rook)   * RookValue;
	const Value their_queen_material  = position.count(them, PieceType::Queen)  * QueenValue;

	const Value our_material   = our_pawn_material + our_knight_material + our_bishop_material
							   + our_rook_material + our_queen_material;
	const Value their_material = their_pawn_material + their_knight_material + their_bishop_material
							   + their_rook_material + their_queen_material;

	our_total += our_material;
	their_total += their_material;

	if (do_trace)
	{
		trace("  Pawn",   our_pawn_material,   their_pawn_material);
		trace("  Knight", our_knight_material, their_knight_material);
		trace("  Bishop", our_bishop_material, their_bishop_material);
		trace("  Rook",   our_rook_material,   their_rook_material);
		trace("  Queen",  our_queen_material,  their_queen_material);
		trace("Material", our_material, 	   their_material);
		trace_divider();
	}

	// Pawn structure
	our_total += pawn_entry->eval(us);
	our_total -= pawn_entry->eval(them);

	if (do_trace)
	{
		trace("Structure", pawn_entry->eval(us), pawn_entry->eval(them));
		trace_divider();
	}

	if (do_trace) trace_end(our_total, their_total);
	return our_total - their_total;
}
