#pragma once

#include "position.hh"
#include "pawns.hh"

namespace chess::eval
{
	//
	// Material values
	//

	constexpr Value PawnValue   = 100;
	constexpr Value KnightValue = 300;
	constexpr Value BishopValue = 325;
	constexpr Value RookValue   = 550;
	constexpr Value QueenValue  = 1000;

	constexpr Value piece_value(const PieceType piece_type)
	{
		constexpr util::array_t<Value, PieceTypes> values
		{
			PawnValue, KnightValue, BishopValue, RookValue, QueenValue, 20000
		};

		return values[util::underlying_value(piece_type)];
	}

	//
	// Mobility
	//

	constexpr Value piece_mobility_weight_a(const PieceType piece_type)
	{
		constexpr util::array_t<Value, PieceTypes> weights {1, 2, 2, 2, 0, 0};
		return weights[util::underlying_value(piece_type)];
	}

	constexpr Value piece_mobility_weight_b(const PieceType piece_type)
	{
		constexpr util::array_t<Value, PieceTypes> weights {1, 1, 1, 1, 1, 1};
		return weights[util::underlying_value(piece_type)];
	}

	//
	// Misc. bonuses and penalties
	//

	constexpr Value Tempo = 29;

	extern Value evaluate(const Position &position, const pawns::Entry *pawn_entry, const bool do_trace = false);

	inline Value evaluate(const Position &position, pawns::Cache *pawn_cache, const bool do_trace = false)
	{
		return evaluate(position, pawn_cache->probe_or_assign(position), do_trace);
	}

	inline Value evaluate(const Position &position, const bool do_trace = false)
	{
		pawns::Entry pawn_entry {position};
		return evaluate(position, &pawn_entry, do_trace);
	}
}
