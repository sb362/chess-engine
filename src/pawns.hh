#pragma once

#include "position.hh"

#include "util/array.hh"
#include "util/hashtable.hh"

namespace chess::pawns
{
	//constexpr Value Hole			= -50;
	constexpr Value Doubled			= -15;
	constexpr Value Tripled			= -30;
	constexpr Value Blocked			= -10;
	constexpr Value Isolated		= -20;
	constexpr Value Backwards		= -50;
	//constexpr Value OverlyAdvanced	= -50;

	constexpr Value Connected		= 10;

	constexpr Value Passed			= 50;

	constexpr Value pawn_square_value(const Colour us, const Square sq)
	{
		constexpr util::array_t<Value, Squares> PawnSquareTable
		{
			 0,  0,  0,  0,  0,  0,  0,  0,
			50, 50, 50, 50, 50, 50, 50, 50,
			10, 10, 20, 30, 30, 20, 10, 10,
			 5,  5, 10, 25, 25, 10,  5,  5,
			 0,  0,  0, 20, 20,  0,  0,  0,
			 5, -5,-10,  0,  0,-10, -5,  5,
			 5, 10, 10,-20,-20, 10, 10,  5,
			 0,  0,  0,  0,  0,  0,  0,  0
		};

		return PawnSquareTable[util::underlying_value(sq) ^ (56 * (us == Colour::White))];
	}

	struct Entry
	{
		Entry() = default;
		Entry(const Entry &) = default;

		Entry(const Position &position);

		Bitboard passed;
		Value white_eval, black_eval;

		constexpr Value eval(const Colour side) const
		{
			return side == Colour::White ? white_eval : black_eval;
		}
	};

	constexpr std::size_t CacheSize = 262144;

	struct Cache : util::FixedSizeHashTable<Key, Entry, CacheSize>
	{
		const Entry *probe_or_assign(const Position &position);
	};
} // chess::pawns
