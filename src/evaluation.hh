#pragma once

#include "position.hh"

namespace chess::eval
{
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

	constexpr Value OurMobilityA   = 1, OurMobilityB   = 15;
	constexpr Value TheirMobilityA = 1, TheirMobilityB = 20;

	extern Value evaluate(const Position &position);
}
