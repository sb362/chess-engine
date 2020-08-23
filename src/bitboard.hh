#pragma once

/*
 * Bitboard definition + operator overloads
 *  + related constants/functions, including some attack generation
 */

//
// Toggle fancy magic/PEXT/PDEP bitboards w/ these macros
//  Note: USE_PEXT/USE_PDEP requires USE_BMI2 (see util/bits.hh)
//

// #define USE_KOGGE_STONE
#define USE_FANCY
// #define USE_PEXT
// #define USE_PDEP

#include <cstdint>

#include "fmt/format.h"

#include "types.hh"

#include "util/array.hh"
#include "util/assert.hh"
#include "util/bits.hh"
#include "util/enum.hh"

namespace chess
{

using Bitboard = std::uint64_t;

constexpr Bitboard OneBB  = 1;
constexpr Bitboard ZeroBB = 0;
constexpr Bitboard AllBB  = ~ZeroBB;

constexpr Bitboard Rank1BB = ~(AllBB << Ranks);
constexpr Bitboard Rank8BB = Rank1BB << (Ranks * 7u);

constexpr Bitboard FileABB = Bitboard(0x101010101010101);
constexpr Bitboard FileHBB = FileABB << 7u;

constexpr Bitboard rank_bb(const Rank r) { return Rank1BB << (Files * util::underlying_value(r)); }
constexpr Bitboard rank_bb(const Square sq) { return rank_bb(rank_of(sq)); }

constexpr Bitboard file_bb(const File f) { return FileABB << util::underlying_value(f); }
constexpr Bitboard file_bb(const Square sq) { return file_bb(file_of(sq)); }

constexpr Bitboard square_bb(const Square sq) { return OneBB << util::underlying_value(sq); }

#define ENABLE_BITBOARD_OPERATORS(Type, Convert)                                                   \
constexpr Bitboard operator&(const Bitboard bb, const Type x) { return bb & Convert(x); }          \
constexpr Bitboard operator|(const Bitboard bb, const Type x) { return bb | Convert(x); }          \
constexpr Bitboard operator^(const Bitboard bb, const Type x) { return bb ^ Convert(x); }          \
                                                                                                   \
constexpr Bitboard &operator&=(Bitboard &bb, const Type x) { return bb &= Convert(x); }            \
constexpr Bitboard &operator|=(Bitboard &bb, const Type x) { return bb |= Convert(x); }            \
constexpr Bitboard &operator^=(Bitboard &bb, const Type x) { return bb ^= Convert(x); }            \
                                                                                                   \
constexpr Bitboard operator&(const Type x, const Bitboard bb) { return bb & x; }                   \
constexpr Bitboard operator|(const Type x, const Bitboard bb) { return bb | x; }                   \
constexpr Bitboard operator^(const Type x, const Bitboard bb) { return bb ^ x; }

ENABLE_BITBOARD_OPERATORS(Square, square_bb)
ENABLE_BITBOARD_OPERATORS(File, file_bb)
ENABLE_BITBOARD_OPERATORS(Rank, rank_bb)

#undef ENABLE_BITBOARD_OPERATORS

constexpr Bitboard squares_bb(const Square a, const Square b)
{
	return square_bb(a) | b;
}

/**
 * @brief 
 * 
 * @param bb Bitboard
 * @return true if @param bb contains more than one set bit
 * @return false otherwise
 */
constexpr bool more_than_one(const Bitboard bb)
{
	// Pop LSB, if bb is still nonzero then
	// there must be more than one set bit.
	return bb & (bb - 1);
}

/**
 * @brief 
 * 
 * @param bb Bitboard
 * @return true if @param bb is has exactly one non-zero bit
 * @return false otherwise
 */
constexpr bool only_one(const Bitboard bb)
{
	return bb && !more_than_one(bb);
}

// Kogge-Stone implementation + other functions
#include "kogge-impl.hh"

namespace bitboards
{
	/**
	 * @brief Initialises all bitboard lookup tables
	 * 
	 */
	extern void init();

	extern util::array_t<Bitboard, Squares, Squares> line_connecting;
	extern util::array_t<Bitboard, Squares> knight_attacks;
}

/**
 * @brief Calculate attacks from a piece on a square
 * 
 * @tparam T PieceType
 * @param sq Square
 * @return Bitboard of all attacks from a piece given type placed on the given square
 */
template <PieceType T> Bitboard attacks_from(const Square sq);

template <>
inline Bitboard attacks_from<PieceType::Knight>(const Square sq)
{
	ASSERT(is_valid(sq));

	return bitboards::knight_attacks[util::underlying_value(sq)];
}

template <>
inline Bitboard attacks_from<PieceType::King>(const Square sq)
{
	ASSERT(is_valid(sq));

	return attacks_from<PieceType::King>(square_bb(sq));
}

template <>
inline Bitboard attacks_from<PieceType::Bishop>(const Square sq)
{
	ASSERT(is_valid(sq));

	return attacks_from<PieceType::Bishop>(square_bb(sq));
}

template <>
inline Bitboard attacks_from<PieceType::Rook>(const Square sq)
{
	ASSERT(is_valid(sq));

	return attacks_from<PieceType::Rook>(square_bb(sq));
}

template <>
inline Bitboard attacks_from<PieceType::Queen>(const Square sq)
{
	return attacks_from<PieceType::Bishop>(sq) | attacks_from<PieceType::Rook>(sq);
}

/**
 * @brief Calculate attacks from a piece on a square, with occupancy
 * 
 * @tparam T PieceType
 * @param sq Square
 * @param occ Bitboard of all pieces
 * @return Bitboard of all attacks from a piece given type placed on the given square
 */
template <PieceType T> Bitboard attacks_from(const Square sq, const Bitboard occ)
{
	return attacks_from<T>(sq);
}

#if defined(USE_KOGGE_STONE)
template <>
constexpr Bitboard attacks_from<PieceType::Bishop>(const Square sq, const Bitboard occ)
{
	ASSERT(is_valid(sq));

	return attacks_from<PieceType::Bishop>(square_bb(sq), occ);
}

template <>
constexpr Bitboard attacks_from<PieceType::Rook>(const Square sq, const Bitboard occ)
{
	ASSERT(is_valid(sq));

	return attacks_from<PieceType::Rook>(square_bb(sq), occ);
}

template <>
constexpr Bitboard attacks_from<PieceType::Queen>(const Square sq, const Bitboard occ)
{
	return attacks_from<PieceType::Bishop>(sq, occ) | attacks_from<PieceType::Rook>(sq, occ);
}
#else
// See magic.hh and magic.cc for fancy magic/PEXT/PEXT+PDEP bitboards.
#endif

/**
 * @brief Calculates bitboard of all pawn attacks from @param pawns from a given colour @param us
 * 
 * @param us Colour
 * @param pawns Bitboard
 * @return Bitboard 
 */
constexpr Bitboard pawn_attacks(const Colour us, const Bitboard pawns)
{
	return us == Colour::White	? shift_ex<NorthWest, NorthEast>(pawns)
								: shift_ex<SouthWest, SouthEast>(pawns);
}

/**
 * @brief Calculates bitboard of all pawn attacks from @param sq from a given colour @param us
 * 
 * @param us Colour
 * @param sq Square
 * @return Bitboard 
 */
constexpr Bitboard pawn_attacks(const Colour us, const Square sq)
{
	return pawn_attacks(us, square_bb(sq));
}

/**
 * @brief Returns the direction in which pawns move for the given side @param us
 * 
 * @param us Colour
 * @return Direction 
 */
constexpr Direction pawn_push(const Colour us)
{
	return us == Colour::White ? North : South;
}

/**
 * @brief Calculates the line on a bitboard that intersects squares @param a and @param b
 * 
 * @param a Square
 * @param b Square
 * @return Bitboard 
 */
inline Bitboard line_connecting(const Square a, const Square b)
{
	const auto i = util::underlying_value(a), j = util::underlying_value(b);
	return bitboards::line_connecting[i][j];
}

/**
 * @brief Calculates the line on a bitboard that is between squares @param a and @param b
 * 
 * @param a Square
 * @param b Square
 * @return Bitboard 
 */
inline Bitboard line_between(const Square a, const Square b)
{
	const auto i = util::underlying_value(a), j = util::underlying_value(b);
	const Bitboard bb = line_connecting(a, b) & ((AllBB << i) ^ (AllBB << j));

	return bb & (bb - 1);
}

/**
 * @brief Tests if squares @param a, @param b, and @param c lie on the same line
 * 
 * @param a Square
 * @param b Square
 * @param c Square
 * @return Bitboard 
 */
inline Bitboard aligned(const Square a, const Square b, const Square c)
{
	return line_connecting(a, b) & c;
}

/**
 * @brief Turns a bitboard into a human-readable string, ideal for debugging.
 * 
 * @param bb Bitboard
 * @return std::string 
 */
extern std::string to_string(const Bitboard bb);

/**
 * @brief 
 * 
 * @param ksq Square
 * @param kto Square
 * @param rsq Square
 * @param rto Square
 * @return Bitboard 
 */
inline Bitboard castling_path(const Square ksq, const Square kto,
							  const Square rsq, const Square rto)
{
	return (line_between(ksq, kto) | line_between(rsq, rto) | rto | kto) & ~squares_bb(ksq, rsq);
}

}
