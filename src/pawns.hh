#pragma once

#include "position.hh"

#include "util/hashtable.hh"

namespace chess::pawns
{
	struct Entry
	{
		Entry() = default;
		Entry(const Entry &) = default;

		Entry(const Position &position);

		Bitboard isolated, doubled, blocked, backwards, passed;
		Bitboard white_pawn_chain, black_pawn_chain;
		Bitboard holes;

		Value white_eval, black_eval;

		constexpr Value eval(const Colour side) const
		{
			return side == Colour::White ? white_eval : black_eval;
		}
	};

	constexpr std::size_t CacheSize = 65536;

	struct Cache : util::FixedSizeHashTable<Key, Entry, CacheSize>
	{
		const Entry *probe_or_assign(const Position &position);
	};
} // chess::pawns
