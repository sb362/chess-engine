#pragma once

/*
 * Fancy magic/PEXT/PEXT+PDEP bitboards implementation for fast sliding piece attack generation
 */

#include "bitboard.hh"

namespace chess
{

// Stores square-specific info required in fancy magic/PEXT/PEXT+PDEP bitboards
struct MagicInfo
{
#if defined(USE_FANCY)
		Bitboard *attacks, mask, magic;
		std::uint8_t shift;
#elif defined(USE_PEXT)
		Bitboard *attacks, mask;
#elif defined(USE_PDEP)
		std::uint16_t *attacks;
		Bitboard mask, postmask;
#endif
};

template <PieceType T> constexpr std::size_t AttackTableSize = 0;
template <> constexpr std::size_t AttackTableSize<PieceType::Bishop> = 5248;
template <> constexpr std::size_t AttackTableSize<PieceType::Rook>   = 102400;

// Holds magic info for each square + attack database
template <PieceType T> class MagicTable
{
public:
	MagicTable() : _info(), _attacks() {}

	MagicInfo &info(const Square sq)
	{
		ASSERT(is_valid(sq));

		return _info[util::underlying_value(sq)];
	}

	const MagicInfo &info(const Square sq) const
	{
		ASSERT(is_valid(sq));

		return _info[util::underlying_value(sq)];
	}

	std::size_t index(const Square sq, const Bitboard occ) const
	{
		const auto &m = info(sq);

#if defined(USE_FANCY)
		return ((occ & m.mask) * m.magic) >> m.shift;
#elif defined(USE_PEXT)
		return pext_64(occ, m.mask);
#elif defined(USE_PDEP)
		return pext_64(occ, m.mask);
#endif
		return 0;
	}

	Bitboard attacks(const Square sq, const Bitboard occ) const
	{
#if defined(CHESS_CONFIG_BITBOARDS_PDEP)
		return pdep(info(sq).attacks[index(sq, occ)], info(sq).postmask);
#else
		return info(sq).attacks[index(sq, occ)];
#endif
	}

	void init();

private:
	util::array_t<MagicInfo, Squares> _info;

#if defined(USE_PDEP)
	util::array_t<std::uint16_t, AttackTableSize<T>> _attacks;
#else
	util::array_t<Bitboard,      AttackTableSize<T>> _attacks;
#endif
};

namespace magics
{
	/**
	 * @brief Initialises all magic/PEXT/PDEP bitboard lookup tables
	 * 
	 */
	extern void init();

#if !defined(USE_KOGGE_STONE)
	extern MagicTable<PieceType::Bishop> bishop_magics;
	extern MagicTable<PieceType::Rook> rook_magics;
#endif
}

#if !defined(USE_KOGGE_STONE)
template <> inline Bitboard attacks_from<PieceType::Bishop>(const Square sq, const Bitboard occ)
{
	return magics::bishop_magics.attacks(sq, occ);
}

template <> inline Bitboard attacks_from<PieceType::Rook>(const Square sq, const Bitboard occ)
{
	return magics::rook_magics.attacks(sq, occ);
}

template <> inline Bitboard attacks_from<PieceType::Queen>(const Square sq, const Bitboard occ)
{
	return attacks_from<PieceType::Bishop>(sq, occ) | attacks_from<PieceType::Rook>(sq, occ);
}
#endif

}
