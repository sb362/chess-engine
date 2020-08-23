
/*
 * Kogge-Stone implementation for sliding piece attacks
 * 	+ shifting bitboards w/o wrapping + assorted functions
 * 
 * Kogge-Stone functions are used in initialisation of
 *  fancy magic/PEXT/PEXT+PDEP bitboards, and are also used
 *  to determine attacks from multiple sliding pieces at once.
 * 
 * Note: this should only be included by bitboard.hh
 */

// Shift bitboard in any direction, preventing wrapping.
template <Direction D> constexpr Bitboard shift(const Bitboard &bb);

template <> constexpr Bitboard shift<North>(const Bitboard &bb)
{
	return bb << 8u;
}

template <> constexpr Bitboard shift<South>(const Bitboard &bb)
{
	return bb >> 8u;
}

template <> constexpr Bitboard shift<East>(const Bitboard &bb)
{
	return (bb & ~FileHBB) << 1u;
}

template <> constexpr Bitboard shift<West>(const Bitboard &bb)
{
	return (bb & ~FileABB) >> 1u;
}

template <> constexpr Bitboard shift<NorthEast>(const Bitboard &bb)
{
	return (bb & ~FileHBB) << 9u;
}

template <> constexpr Bitboard shift<SouthWest>(const Bitboard &bb)
{
	return (bb & ~FileABB) >> 9u;
}

template <> constexpr Bitboard shift<NorthWest>(const Bitboard &bb)
{
	return (bb & ~FileABB) << 7u;
}

template <> constexpr Bitboard shift<SouthEast>(const Bitboard &bb)
{
	return (bb & ~FileHBB) >> 7u;
}

template <class = void> constexpr Bitboard walk(const Bitboard &bb)
{
	return bb;
}

// Shifts a bitboard in several directions continuously.
// Example: walk<North, North, East>(A1) = B3
template <Direction Step, Direction... Next> constexpr Bitboard walk(const Bitboard &bb)
{
	return walk<Next...>(shift<Step>(bb));
}

template <class = void> constexpr Bitboard shift_ex(const Bitboard &)
{
	return 0;
}

// Shifts a bitboard in several directions at once
// Example: shift_ex<North, South, East>(E4) = E5 | E3 | F4
template <Direction Step, Direction... Next> constexpr Bitboard shift_ex(const Bitboard &bb)
{
	return shift<Step>(bb) | shift_ex<Next...>(bb);
}

//
// Implementations of Kogge-Stone flood fill + occluded fill.
// https://www.chessprogramming.org/Kogge-Stone_Algorithm
//

// Flood fill in a single direction.
template <Direction D> constexpr Bitboard fill(Bitboard gen);

// Occluded fill in a single direction.
template <Direction D> constexpr Bitboard fill(Bitboard gen, Bitboard pro);

template <> constexpr Bitboard fill<North>(Bitboard gen)
{
	gen |= (gen << 8u);
	gen |= (gen << 16u);
	gen |= (gen << 32u);

	return gen;
}

template <> constexpr Bitboard fill<South>(Bitboard gen)
{
	gen |= (gen >> 8u);
	gen |= (gen >> 16u);
	gen |= (gen >> 32u);

	return gen;
}

template <> constexpr Bitboard fill<East>(Bitboard gen)
{
	constexpr auto a = ~FileABB, b = a & (a << 1u), c = b & (b << 2u);

	gen |= a & (gen << 1u);
	gen |= b & (gen << 2u);
	gen |= c & (gen << 4u);

	return gen;
}

template <> constexpr Bitboard fill<West>(Bitboard gen)
{
	constexpr auto a = ~FileHBB, b = a & (a >> 1u), c = b & (b >> 2u);

	gen |= a & (gen >> 1u);
	gen |= b & (gen >> 2u);
	gen |= c & (gen >> 4u);

	return gen;
}

template <> constexpr Bitboard fill<NorthEast>(Bitboard gen)
{
	constexpr auto a = ~FileABB, b = a & (a << 9u), c = b & (b << 18u);

	gen |= a & (gen << 9u);
	gen |= b & (gen << 18u);
	gen |= c & (gen << 36u);

	return gen;
}

template <> constexpr Bitboard fill<SouthWest>(Bitboard gen)
{
	constexpr auto a = ~FileHBB, b = a & (a >> 9u), c = b & (b >> 18u);

	gen |= a & (gen >> 9u);
	gen |= b & (gen >> 18u);
	gen |= c & (gen >> 36u);

	return gen;
}

template <> constexpr Bitboard fill<NorthWest>(Bitboard gen)
{
	constexpr auto a = ~FileHBB, b = a & (a << 7u), c = b & (b << 14u);

	gen |= a & (gen << 7u);
	gen |= b & (gen << 14u);
	gen |= c & (gen << 28u);

	return gen;
}

template <> constexpr Bitboard fill<SouthEast>(Bitboard gen)
{
	constexpr auto a = ~FileABB, b = a & (a >> 7u), c = b & (b >> 14u);

	gen |= a & (gen >> 7u);
	gen |= b & (gen >> 14u);
	gen |= c & (gen >> 28u);

	return gen;
}

template <> constexpr Bitboard fill<North>(Bitboard gen, Bitboard pro)
{
	gen |= (gen << 8u) & pro;
	pro &= (pro << 8u);
	gen |= (gen << 16u) & pro;
	pro &= (pro << 16u);
	gen |= (gen << 32u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<South>(Bitboard gen, Bitboard pro)
{
	gen |= (gen >> 8u) & pro;
	pro &= (pro >> 8u);
	gen |= (gen >> 16u) & pro;
	pro &= (pro >> 16u);
	gen |= (gen >> 32u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<East>(Bitboard gen, Bitboard pro)
{
	pro &= ~FileABB;

	gen |= (gen << 1u) & pro;
	pro &= (pro << 1u);
	gen |= (gen << 2u) & pro;
	pro &= (pro << 2u);
	gen |= (gen << 4u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<West>(Bitboard gen, Bitboard pro)
{
	pro &= ~FileHBB;

	gen |= (gen >> 1u) & pro;
	pro &= (pro >> 1u);
	gen |= (gen >> 2u) & pro;
	pro &= (pro >> 2u);
	gen |= (gen >> 4u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<NorthEast>(Bitboard gen, Bitboard pro)
{
	pro &= ~FileABB;

	gen |= (gen << 9u) & pro;
	pro &= (pro << 9u);
	gen |= (gen << 18u) & pro;
	pro &= (pro << 18u);
	gen |= (gen << 36u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<SouthWest>(Bitboard gen, Bitboard pro)
{
	pro &= ~FileHBB;

	gen |= (gen >> 9u) & pro;
	pro &= (pro >> 9u);
	gen |= (gen >> 18u) & pro;
	pro &= (pro >> 18u);
	gen |= (gen >> 36u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<NorthWest>(Bitboard gen, Bitboard pro)
{
	pro &= ~FileHBB;

	gen |= (gen << 7u) & pro;
	pro &= (pro << 7u);
	gen |= (gen << 14u) & pro;
	pro &= (pro << 14u);
	gen |= (gen << 28u) & pro;

	return gen;
}

template <> constexpr Bitboard fill<SouthEast>(Bitboard gen, Bitboard pro)
{
	pro &= ~FileABB;

	gen |= (gen >> 7u) & pro;
	pro &= (pro >> 7u);
	gen |= (gen >> 14u) & pro;
	pro &= (pro >> 14u);
	gen |= (gen >> 28u) & pro;

	return gen;
}

template <class = void> constexpr Bitboard ray_attacks(const Bitboard)
{
	return 0;
}

// Determines direction-wise sliding piece attacks
template <Direction D, Direction... Next>
constexpr Bitboard ray_attacks(const Bitboard pieces)
{
	return shift<D>(fill<D>(pieces)) | ray_attacks<Next...>(pieces);
}

template <class = void> constexpr Bitboard ray_attacks(const Bitboard, const Bitboard)
{
	return 0;
}

// Determines direction-wise sliding piece attacks, with occupancy
template <Direction D, Direction... Next>
constexpr Bitboard ray_attacks(const Bitboard pieces, const Bitboard occ)
{
	return shift<D>(fill<D>(pieces, ~occ)) | ray_attacks<Next...>(pieces, occ);
}

// Determines attacks from several squares at once
template <PieceType> constexpr Bitboard attacks_from(Bitboard);

template <> constexpr Bitboard attacks_from<PieceType::Knight>(Bitboard pieces)
{
	const Bitboard l1 = (pieces >> 1) & 0x7f7f7f7f7f7f7f7f;
	const Bitboard l2 = (pieces >> 2) & 0x3f3f3f3f3f3f3f3f;
	const Bitboard r1 = (pieces << 1) & 0xfefefefefefefefe;
	const Bitboard r2 = (pieces << 2) & 0xfcfcfcfcfcfcfcfc;
	const Bitboard h1 = l1 | r1;
	const Bitboard h2 = l2 | r2;

	return (h1 << 16) | (h1 >> 16) | (h2 << 8) | (h2 >> 8);
}

template <> constexpr Bitboard attacks_from<PieceType::King>(Bitboard pieces)
{
	Bitboard attacks = shift_ex<West, East>(pieces);
	pieces |= attacks;
	return attacks | shift_ex<North, South>(pieces);
}

template <> constexpr Bitboard attacks_from<PieceType::Bishop>(Bitboard pieces)
{
	return ray_attacks<NorthEast, SouthEast, SouthWest, NorthWest>(pieces);
}

template <> constexpr Bitboard attacks_from<PieceType::Rook>(Bitboard pieces)
{
	return ray_attacks<North, East, South, West>(pieces);
}

template <> constexpr Bitboard attacks_from<PieceType::Queen>(Bitboard pieces)
{
	return attacks_from<PieceType::Bishop>(pieces) | attacks_from<PieceType::Rook>(pieces);
}

// Determines attacks from several squares at once, with occupancy
template <PieceType T> constexpr Bitboard attacks_from(Bitboard pieces, Bitboard)
{
	return attacks_from<T>(pieces);
}

template <> constexpr Bitboard attacks_from<PieceType::Bishop>(Bitboard pieces, Bitboard occ)
{
	return ray_attacks<NorthEast, SouthEast, SouthWest, NorthWest>(pieces, occ);
}

template <> constexpr Bitboard attacks_from<PieceType::Rook>(Bitboard pieces, Bitboard occ)
{
	return ray_attacks<North, East, South, West>(pieces, occ);
}

template <> constexpr Bitboard attacks_from<PieceType::Queen>(Bitboard pieces, Bitboard occ)
{
	return attacks_from<PieceType::Bishop>(pieces, occ) | attacks_from<PieceType::Rook>(pieces, occ);
}
