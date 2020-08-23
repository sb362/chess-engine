#include "tt.hh"

namespace chess
{
	TranspositionTable tt {TranspositionTable::DefaultSize};

	void TranspositionTable::save(Key key, Depth depth, Depth plies_to_root,
								  Value value, Bound bound, Move move)
	{
		// "Fix" mate values, as mate values need to be stored relative to the current position.
		// This solves the issue of retrieving a mate in x when plies to root > x.
		if (is_mate(value))
			value = relative_mate_value(value, plies_to_root);

		write(key, {depth, move, value, bound, current_epoch()}, util::always_replace);
	}
}
