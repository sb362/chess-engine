#pragma once

#include "types.hh"

#include "util/hashtable.hh"

namespace chess
{
	constexpr Value absolute_mate_value(Value value, Depth plies_to_root)
	{
		return value < 0
			 ? mated_in(depth_to_mate(value) + plies_to_root)
			 : mate_in (depth_to_mate(value) + plies_to_root);
	}

	constexpr Value relative_mate_value(Value value, Depth plies_to_root)
	{
		return value < 0
			 ? mated_in(depth_to_mate(value) - plies_to_root)
			 : mate_in (depth_to_mate(value) - plies_to_root);
	}

	/**
	* @brief Compact entry structure for use in a transposition table.
	* Contains the best move, best value, node type, the depth to which these were found,
	* as well as an 'epoch' value used to implement aging in the transposition table.
	*/
	struct Entry
	{
		constexpr Entry()
			: depth(0), move(Square::A1, Square::A1), value(0), bound(0), epoch(0)
		{
		}

		constexpr Entry(Depth depth, Move move, Value value, Bound bound, std::uint8_t epoch)
			: depth(depth), move(move), value(value), bound(util::underlying_value(bound)),
			  epoch(epoch)
		{
		}

		Depth depth;
		Move move;
		Value value;

		std::uint8_t bound : 2;
		std::uint8_t epoch : 6;
	};

	struct TranspositionTable : util::HashTable<Key, Entry>
	{
		/**
		* @brief Default size of the transposition table in bytes
		*/
		static constexpr unsigned DefaultSize = 8 * 1024 * 1024;

		void save(Key key, Depth depth, Depth plies_to_root, Value value, Bound bound, Move move);
	};

	/**
	* @brief Global transposition table
	*/
	extern TranspositionTable tt;
} // chess::tt
