#include "evaluation.hh"

#include "fmt/color.h"

using namespace chess;
using namespace chess::eval;

constexpr unsigned TraceWidth = 36;

void trace_divider()
{
	fmt::print("{}\n", std::string(TraceWidth, '-'));
}

void trace_begin()
{
	std::string s;

	s += fmt::format("|{: <10}|", "");
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

template <PieceType T>
Value evaluate_mobility(const Position &position, const Colour us)
{
	static_assert(T != PieceType::Pawn && T != PieceType::King, "Unsupported piece type");

	Value value = 0;

	const Bitboard pinned = position.pinned();
	const Bitboard occ = position.occupied();
	const Bitboard their_pawns = position.occupied(~us);

	const Bitboard targets = ~position.occupied(us) & ~pawn_attacks(~us, their_pawns);

	Bitboard pieces = position.occupied(us, T) & ~pinned;
	while (pieces)
	{
		const Square sq = static_cast<Square>(util::lsb_64(pieces));
		pieces &= (pieces - 1);

		// Calculate mobility
		const Bitboard attacks = attacks_from<T>(sq, occ) & targets;
			value += (piece_mobility_weight_a(T) * util::popcount_64(attacks & targets))
					/ piece_mobility_weight_b(T);
	}

	return value;
}

/**
 * @brief Evaluation function
 * 
 * @param position 
 * @return Value Evaluation of @param position relative to the side to move
 */
Value eval::evaluate(const Position &position, const pawns::Entry *pawn_entry, const bool do_trace)
{
	// todo: rewrite me, this is a mess. Too many variables

	ASSERT(!position.checkers());

	const Colour us = position.side_to_move(), them = ~us;

	if (do_trace)
		trace_begin();

	Value our_total = 0, their_total = 0;

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

	// Our piece-specific bonuses and penalties, mobility
	const Value our_knight_mobility = evaluate_mobility<PieceType::Knight>(position, us);
	const Value our_bishop_mobility = evaluate_mobility<PieceType::Bishop>(position, us);
	const Value our_rook_mobility   = evaluate_mobility<PieceType::Rook>  (position, us);

	// Their piece-specific bonuses and penalties, mobility
	const Value their_knight_mobility = evaluate_mobility<PieceType::Knight>(position, them);
	const Value their_bishop_mobility = evaluate_mobility<PieceType::Bishop>(position, them);
	const Value their_rook_mobility   = evaluate_mobility<PieceType::Rook>  (position, them);

	const Value our_mobility   = our_knight_mobility + our_bishop_mobility + our_rook_mobility;
	const Value their_mobility = their_knight_mobility + their_bishop_mobility + their_rook_mobility;

	if (do_trace)
	{
		trace("  Knight", our_knight_mobility, their_knight_mobility);
		trace("  Bishop", our_bishop_mobility, their_bishop_mobility);
		trace("  Rook",   our_rook_mobility,   their_rook_mobility);
		trace("Mobility", our_mobility, 	   their_mobility);
		trace_divider();
	}

	our_total += our_mobility;
	their_total += their_mobility;

	// Pawn structure
	our_total += pawn_entry->eval(us);
	their_total += pawn_entry->eval(them);

	if (do_trace)
	{
		trace("Structure", pawn_entry->eval(us), pawn_entry->eval(them));
		trace_divider();
	}

	our_total += Tempo;

	if (do_trace)
	{
		trace("Tempo", Tempo, 0);
		trace_divider();
	}

	if (do_trace)
		trace_end(our_total, their_total);

	return our_total - their_total;
}
