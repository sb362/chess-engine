#include "pawns.hh"

using namespace chess;
using namespace chess::pawns;

template <Colour Us>
Value populate(const Bitboard our_pawns, const Bitboard their_pawns, Entry &entry)
{
	constexpr Direction Up = pawn_push(Us);

	const Bitboard pawns = our_pawns | their_pawns;

	Value value = 0;

	Bitboard bb = our_pawns;
	while (bb)
	{
		const Square sq = static_cast<Square>(util::lsb_64(bb));	
		bb &= (bb - 1);

		const Square stop = sq + Up;

		const bool blocked = their_pawns & stop;

		const Bitboard west_file = shift<West>(file_bb(sq));
		const Bitboard east_file = shift<East>(file_bb(sq));

		const Bitboard front_span = shift<Up>(fill<Up>(square_bb(sq)));
		const Bitboard front_attack_span = fill<Up>(pawn_attacks(Us, sq));

		const Bitboard rear_span = shift<-Up>(fill<-Up>(square_bb(sq))); 
		const Bitboard rear_attack_span = fill<-Up>(pawn_attacks(~Us, sq));

		const bool doubled = front_span & our_pawns;
		const bool tripled = more_than_one(front_span & our_pawns);

		const Bitboard adjacent_pawns = pawns & (west_file | east_file);
		const Bitboard support = our_pawns & rank_bb(sq - Up);

		const bool isolated = !(adjacent_pawns & our_pawns);

		const Bitboard sentries = shift<Up>(front_attack_span) & their_pawns;

		const bool passed = !(front_span & (our_pawns | their_pawns)) && !sentries;

		const bool backwards = !(adjacent_pawns & (rear_span | rear_attack_span))
							 && (blocked || (pawn_attacks(Us, stop) & their_pawns));

		if (doubled)
			value += tripled ? Tripled : Doubled;
		
		if (isolated)
			value += Isolated;

		if (blocked)
			value += Blocked;

		if (support)
			value += Connected;

		if (backwards)
			value += Backwards;

		if (passed)
		{
			entry.passed |= sq;
			value += Passed;
		}

		value += pawn_square_value(Us, sq);
	}

	return value;
}

Entry::Entry(const Position &position)
	: Entry()
{
	const Bitboard white_pawns = position.occupied(Piece::WhitePawn);
	const Bitboard black_pawns = position.occupied(Piece::BlackPawn);

	white_eval = populate<Colour::White>(white_pawns, black_pawns, *this);
	black_eval = populate<Colour::Black>(black_pawns, white_pawns, *this);
}

const Entry *Cache::probe_or_assign(const Position &position)
{
	const Key key = position.pawn_key();

	if (const Entry *entry = probe(key); entry)
		return entry;
	
	assign(key, {position});
	return probe(key);
}
