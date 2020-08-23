#pragma once

#include "position.hh"

namespace chess
{
	/**
	 * @brief Move with value, used in move ordering
	 */
	struct MoveWithValue : Move
	{
		Value value = 0;

		constexpr bool operator<(const MoveWithValue &other) const
		{
			return value < other.value;
		}
	};

	/**
	 * @brief Legal move generator
	 */
	class MoveList
	{
	private:
		const Position &position;
		const Colour us;

		util::array_t<MoveWithValue, MaxMoves> moves;
		MoveWithValue *top, *cur;

	public:
		MoveList(const Position &position, const Colour us)
			: position(position), us(us), moves(), top(begin()), cur(begin())
		{
			generate();
		}

		MoveList(const Position &position)
			: MoveList(position, position.side_to_move())
		{
		}

	private:
		void generate();
	
	public:
		void clear() { top = begin(); cur = begin(); }

		void push_back(const Move &move) { ASSERT(size() < moves.size()); *top++ = {move}; }

		const MoveWithValue *begin() const { return moves.data(); }
		const MoveWithValue *end()   const { return top; }
		MoveWithValue *begin() { return moves.data(); }
		MoveWithValue *end()   { return top; }

		unsigned size() const { return end() - begin(); }
		
		bool find(const Move &move) const
		{
			return std::find(begin(), end(), move) != end();
		}

		MoveWithValue select();
	};

	extern unsigned approx_mobility(const Position &position, const Colour us);
}
