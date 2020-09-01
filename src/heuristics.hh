#pragma once

/*
 * Assorted search statistics/heuristics used for move ordering and pruning
 * https://www.chessprogramming.org/History_Heuristic
 * https://www.chessprogramming.org/Killer_Heuristic
 */

#include "types.hh"

#include "util/array.hh"

namespace chess::search
{

struct Killers : util::array_t<Move, 2>
{
	void update(const Move move)
	{
		if ((*this)[0] != move)
			(*this)[1] = (*this)[0];
		
		(*this)[0] = move;
	}

	void clear() { fill({}); }

	bool is_killer(const Move move) const
	{
		return move == (*this)[0] || move == (*this)[1];
	}
};

struct KillerHeuristic : util::array_t<Killers, MaxDepth>
{
	void update(const Depth depth, const Move move)
	{
		(*this)[depth].update(move);
	}

	void clear()
	{
		for (Killers &killers : *this)
			killers.clear();
	}
};

constexpr Value MaxHistoryValue = 2000;

// History heuristic derived from http://rebel13.nl/rebel13/blog/lmr%20advanced.html
struct HistoryHeuristic : util::array_t<Value, Pieces, Squares>
{
	void update(const Value value, const Piece piece, const Square to)
	{
		(*this)[util::underlying_value(piece)][util::underlying_value(to)] += value;

		if (util::abs(probe(piece, to)) >= 2000)
			(*this)[util::underlying_value(piece)][util::underlying_value(to)] /= 2;
	}

	void clear()
	{
		for (util::array_t<Value, Squares> &piece_history : *this)
			piece_history.fill(0);
	}

	Value probe(const Piece piece, const Square to) const
	{
		return (*this)[util::underlying_value(piece)][util::underlying_value(to)];
	}
};

struct Heuristics
{
	KillerHeuristic killer;
	HistoryHeuristic history;

	void clear()
	{
		killer.clear();
		history.clear();
	}
};

} // chess::search
