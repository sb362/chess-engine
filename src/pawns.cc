#include "pawns.hh"

using namespace chess;
using namespace chess::pawns;

Entry::Entry(const Position &position)
{
	
}

const Entry *Cache::probe_or_assign(const Position &position)
{
	const Key key = position.pawn_key();

	if (const Entry *entry = probe(key); entry)
		return entry;
	
	assign(key, {position});
	return probe(key);
}
