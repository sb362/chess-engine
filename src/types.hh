#pragma once

#include <cstdint>
#include <string_view>

#include "fmt/format.h"

#include "util/array.hh"
#include "util/enum.hh"
#include "util/maths.hh"
#include "util/random.hh"
#include "util/tuple.hh"

/*
 * Definitions for squares, files, ranks, directions + parsing/to_string
 */

#define ENABLE_ARITHMETIC_OPERATORS(A)                                                             \
constexpr A operator+(const A a, const int b) { return static_cast<A>(static_cast<int>(a) + b); }  \
constexpr A operator-(const A a, const int b) { return static_cast<A>(static_cast<int>(a) - b); }  \
constexpr A operator*(const A a, const int b) { return static_cast<A>(static_cast<int>(a) * b); }  \
constexpr A operator/(const A a, const int b) { return static_cast<A>(static_cast<int>(a) / b); }  \
constexpr A operator%(const A a, const int b) { return static_cast<A>(static_cast<int>(a) % b); }  \
constexpr A &operator+=(A &a, const int b) { return a = a + b; }                                   \
constexpr A &operator-=(A &a, const int b) { return a = a - b; }                                   \
constexpr A &operator++(A &a) { return a += 1; }                                                   \
constexpr A &operator--(A &a) { return a -= 1; }

namespace chess
{
	constexpr unsigned Ranks = 8;
	constexpr unsigned Files = 8;
	constexpr unsigned Squares = 64;

	enum class File : std::uint8_t
	{
		A, B, C, D, E, F, G, H
	};

	enum class Rank : std::uint8_t
	{
		One, Two, Three, Four, Five, Six, Seven, Eight
	};

	enum class Square : std::uint8_t
	{
		A1, B1, C1, D1, E1, F1, G1, H1,
		A2, B2, C2, D2, E2, F2, G2, H2,
		A3, B3, C3, D3, E3, F3, G3, H3,
		A4, B4, C4, D4, E4, F4, G4, H4,
		A5, B5, C5, D5, E5, F5, G5, H5,
		A6, B6, C6, D6, E6, F6, G6, H6,
		A7, B7, C7, D7, E7, F7, G7, H7,
		A8, B8, C8, D8, E8, F8, G8, H8,
		Invalid
	};

	using Direction = std::int8_t;

	constexpr Direction North		= 8;
	constexpr Direction South		= -North;
	constexpr Direction East		= 1;
	constexpr Direction West		= -East;
	constexpr Direction NorthEast	= North + East;
	constexpr Direction NorthWest	= North + West;
	constexpr Direction SouthEast	= South + East;
	constexpr Direction SouthWest	= South + West;

	ENABLE_ARITHMETIC_OPERATORS(File)
	ENABLE_ARITHMETIC_OPERATORS(Rank)
	ENABLE_ARITHMETIC_OPERATORS(Square)

	constexpr bool is_valid(const File file)
	{
		return file <= File::H;
	}

	constexpr bool is_valid(const Rank rank)
	{
		return rank <= Rank::Eight;
	}

	constexpr bool is_valid(const Square sq)
	{
		return sq <= Square::H8;
	}

	constexpr File file_of(const Square sq)
	{
		return static_cast<File>(sq % Files);
	}

	constexpr Rank rank_of(const Square sq)
	{
		return static_cast<Rank>(sq / Ranks);
	}

	constexpr Square make_square(const File file, const Rank rank)
	{
		return static_cast<Square>(file + util::underlying_value(rank * Ranks));
	}

	constexpr File parse_file(const char c)
	{
		return static_cast<File>(c - 'a');
	}

	constexpr Rank parse_rank(const char c)
	{
		return static_cast<Rank>(c - '1');
	}

	constexpr Square parse_square(std::string_view s)
	{
		return make_square(parse_file(s[0]), parse_rank(s[1]));
	}

	constexpr std::uint8_t rank_distance(const Square a, const Square b)
	{
		return std::uint8_t(util::abs(int(rank_of(a)) - int(rank_of(b))));
	}

	constexpr std::uint8_t file_distance(const Square a, const Square b)
	{
		return std::uint8_t(util::abs(int(file_of(a)) - int(file_of(b))));
	}

	constexpr std::uint8_t distance(const Square a, const Square b)
	{
		return util::max(rank_distance(a, b), file_distance(a, b));
	}

	constexpr char to_char(const File file, bool upper_case = false)
	{
		return char((upper_case ? 'A' : 'a') + util::underlying_value(file));
	}

	constexpr char to_char(const Rank rank, bool = false)
	{
		return char('1' + util::underlying_value(rank));
	}
} // chess

#undef ENABLE_ARITHMETIC_OPERATORS

template <> struct fmt::formatter<chess::File>
{
	template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	constexpr auto format(const chess::File &file, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", to_char(file));
	}
};

template <> struct fmt::formatter<chess::Rank>
{
	template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	constexpr auto format(const chess::Rank &rank, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", to_char(rank));
	}
};

template <> struct fmt::formatter<chess::Square>
{
	template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	constexpr auto format(const chess::Square &sq, FormatContext &ctx)
	{
		return is_valid(sq)	? format_to(ctx.out(), "{}{}", file_of(sq), rank_of(sq))
							: format_to(ctx.out(), "-");
	}
};

/*
 * Definitions for piece types, colours, pieces + related functions
 */

namespace chess
{
	constexpr unsigned Colours = 2;
	constexpr unsigned PieceTypes = 6;
	constexpr unsigned Pieces = 12;

	enum class Colour : std::uint8_t
	{
		White, Black
	};

	enum class PieceType : std::uint8_t
	{
		Pawn, Knight, Bishop, Rook, Queen, King, Invalid
	};

	enum class Piece : std::uint8_t
	{
		WhitePawn,		BlackPawn,
		WhiteKnight,	BlackKnight,
		WhiteBishop,	BlackBishop,
		WhiteRook,		BlackRook,
		WhiteQueen, 	BlackQueen,
		WhiteKing,		BlackKing,
		Invalid
	};

	// Flip Colour (white -> black, black -> white)
	constexpr Colour operator~(const Colour colour)
	{
		return static_cast<Colour>(
			util::underlying_value(colour) ^ util::underlying_value(Colour::Black)
		);
	}

	// Get the Colour of a Piece
	constexpr Colour colour_of(const Piece piece)
	{
		return static_cast<Colour>(util::underlying_value(piece) & 1u);
	}

	// Get the PieceType of a Piece
	constexpr PieceType type_of(const Piece piece)
	{
		return static_cast<PieceType>(util::underlying_value(piece) >> 1u);
	}

	constexpr bool is_valid(const PieceType type)
	{
		return type <= PieceType::King;
	}

	// Construct a Piece from Colour and PieceType
	constexpr Piece make_piece(const Colour colour, const PieceType piece_type)
	{
		return static_cast<Piece>(
			util::underlying_value(colour) | (util::underlying_value(piece_type) << 1u)
		);
	}

	constexpr bool is_valid(const Piece piece)
	{
		return piece < Piece::Invalid;
	}

	constexpr std::string_view PieceTypeChars = "pnbrqk";
	constexpr std::string_view PieceTypeCharsUpper = "PNBRQK";
	constexpr std::string_view PieceChars = "PpNnBbRrQqKk-";

	constexpr char to_char(const Colour colour)
	{
		return colour == chess::Colour::White ? 'w' : 'b';
	}

	constexpr char to_char(const PieceType type, bool upper_case = false)
	{
		return upper_case ? PieceTypeCharsUpper[util::underlying_value(type)]
						  : PieceTypeChars     [util::underlying_value(type)];
	}

	constexpr char to_char(const Piece piece)
	{
		return PieceChars[util::underlying_value(piece)];
	}
} // chess

template <> struct fmt::formatter<chess::Colour>
{
	template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	constexpr auto format(const chess::Colour &colour, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", to_char(colour));
	}
};

template <> struct fmt::formatter<chess::PieceType>
{
	template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	constexpr auto format(const chess::PieceType &piece_type, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", to_char(piece_type));
	}
};

template <> struct fmt::formatter<chess::Piece>
{
	template <typename ParseContext> constexpr auto parse(ParseContext &ctx)
	{
		return ctx.begin();
	}

	template <typename FormatContext>
	constexpr auto format(const chess::Piece &piece, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", chess::to_char(piece));
	}
};

/*
 * Definitions for castling rights
 */

namespace chess
{
	enum class Castling : std::uint8_t
	{
		None     = 0,

		WhiteOO  = 1,
		WhiteOOO = 2,
		BlackOO  = 4,
		BlackOOO = 8,

		White    = WhiteOO  | WhiteOOO,
		Black    = BlackOO  | BlackOOO,
		OO       = WhiteOO  | BlackOO,
		OOO      = WhiteOOO | BlackOOO,

		Any      = White | Black,
	};

	constexpr Castling operator&(const Castling a, const Castling b)
	{
		return static_cast<Castling>(util::underlying_value(a) & util::underlying_value(b));
	}

	constexpr Castling operator|(const Castling a, const Castling b)
	{
		return static_cast<Castling>(util::underlying_value(a) | util::underlying_value(b));
	}

	constexpr Castling operator^(const Castling a, const Castling b)
	{
		return static_cast<Castling>(util::underlying_value(a) ^ util::underlying_value(b));
	}

	constexpr Castling &operator&=(Castling &a, const Castling b)
	{
		return a = a & b;
	}

	constexpr Castling &operator|=(Castling &a, const Castling b)
	{
		return a = a | b;
	}

	constexpr Castling &operator^=(Castling &a, const Castling b)
	{
		return a = a ^ b;
	}

	constexpr Castling operator~(const Castling a)
	{
		return static_cast<Castling>(~util::underlying_value(a)) & Castling::Any;
	}

	constexpr Square castling_king_dest(const Castling rights)
	{
		const File file = bool(rights & Castling::OO)    ? File::G   : File::C;
		const Rank rank = bool(rights & Castling::White) ? Rank::One : Rank::Eight;

		return make_square(file, rank);
	}

	constexpr Square castling_rook_dest(const Castling rights)
	{
		const File file = bool(rights & Castling::OO)    ? File::F   : File::D;
		const Rank rank = bool(rights & Castling::White) ? Rank::One : Rank::Eight;

		return make_square(file, rank);
	}

	constexpr Castling make_castling_rights(const Colour us, const bool oo)
	{
		return (oo					? Castling::OO	  : Castling::OOO)
			 & (us == Colour::White ? Castling::White : Castling::Black);
	}
} // chess

/*
 * Definitions for Zobrist hashing, required for transposition tables
 */

namespace chess
{
	/**
	 * @brief Zobrist hash type
	 * 
	 */
	using Key = std::uint64_t;

	/**
	 * @brief Implementation of Zobrist hashing
	 * See: https://www.chessprogramming.org/Zobrist_Hashing
	 */
	struct Zobrist
	{
		Key side;
		util::array_t<Key, 16> castling;
		util::array_t<Key, Files> en_passant;
		util::array_t<Key, Pieces, Squares> piece_square;
		util::array_t<Key, Pieces, 8> hand;

		constexpr Zobrist()
			: side(), castling(), en_passant(), piece_square(), hand()
		{
			util::PRNG prng {736209358, 11200023, 904492875, 3429570234895};

			side = prng.rand();

			for (Key &key : castling)
				key = prng.rand();

			castling[util::underlying_value(Castling::None)] = 0;
			
			for (Key &key : en_passant)
				key = prng.rand();

			for (auto &key_array : piece_square)
				for (Key &key : key_array)
					key = prng.rand();

			for (auto &key_array : hand)
			{
				for (Key &key : key_array)
					key = prng.rand();
			
				key_array[0] = 0;
			}
		}
	};

	static constexpr Zobrist zobrist {};
} // chess

/*
 * Move representation and associated functions
 */

namespace chess
{
	/**
	 * @brief Maximum number of legal moves in a standard chess position
	 */
#if defined(CRAZYHOUSE)
	constexpr unsigned MaxMoves = /*???*/256;
#else
	constexpr unsigned MaxMoves = /*218*/128;
#endif

	/**
	* @brief Compact 16-bit move representation
	*/
	struct Move
	{
		/**
		 * @brief 16-bit unsigned integer used to store move information, including
		 * the source square, destination square, and promotion/drop type if any.
		 *
		 * Internally, the first 6 bits represent the source square.
		 * The next 6 bits store the destination square, plus an
		 * additional bit to indicate validity (corresponding to Square::Invalid).
		 * The remaining 3 bits represent a piece type, either for promotions or drops.
		 *
		 * For invalid moves:        !is_valid(to())
		 * For non-promotions/drops: from() != to() && !is_valid(promotion())
		 * For promotions:           from() != to() && is_valid(promotion())
		 * For drops:                from() == to() && is_valid(promotion())
		 *
		 * drop() is an alias of promotion().
		 *
		 */
		std::uint16_t data;

		/**
		 * @brief Default constructor, makes an invalid move
		 * 
		 */
		constexpr Move()
			: Move(Square::A1, Square::Invalid, PieceType::Invalid)
		{
		}

		/**
		 * @brief Constructor for non-promotions/drops
		 * 
		 */
		constexpr Move(const Square from, const Square to)
			: Move(from, to, PieceType::Invalid)
		{
		}

		/**
		 * @brief Constructor for promotions
		 * 
		 */
		constexpr Move(const Square from, const Square to, const PieceType promotion)
			: data(0)
		{
			data |= (util::underlying_value(from) & 0x3fu);            // 6 bits
			data |= (util::underlying_value(to) & 0x7fu) << 6u;        // 7 bits
			data |= (util::underlying_value(promotion) & 0x7u) << 13u; // 3 bits
		}

		/**
		 * @brief Constructor for drops
		 * 
		 */
		constexpr Move(const Square to, const PieceType promotion)
			: Move(to, to, promotion)
		{
		}

		constexpr Square from() const
		{
			return static_cast<Square>(data & 0x3fu);
		}

		constexpr Square to() const
		{
			return static_cast<Square>((data >> 6u) & 0x7fu);
		}

		constexpr PieceType promotion() const
		{
			return static_cast<PieceType>((data >> 13u) & 0x7u);
		}

		constexpr PieceType drop() const
		{
			return promotion();
		}

		constexpr bool operator==(const Move move) const { return data == move.data; }
		constexpr bool operator!=(const Move move) const { return !operator==(move); }

		constexpr bool is_valid() const
		{
			return ::chess::is_valid(to());
		}

		constexpr bool is_promotion() const
		{
			return ::chess::is_valid(promotion()) && from() != to();
		}

#if defined(CRAZYHOUSE)
		constexpr bool is_drop() const
		{
			return ::chess::is_valid(promotion()) && from() == to();
		}
#endif
	};
} // chess

/*
 * Definitions shared by search/evaluation
 */

namespace chess
{
	using Nodes = std::uint64_t;
	using Depth = std::uint8_t;
	using Value = std::int16_t;

	template <std::size_t S> using Values = util::Tuple<Value, S>;

	struct ValuePair : Values<2>
	{
		constexpr Value &mg() { return operator[](0); }
		constexpr Value &eg() { return operator[](1); }
		constexpr const Value &mg() const { return operator[](0); }
		constexpr const Value &eg() const { return operator[](1); }
	};

	/**
	 * @brief Sequence of moves, used to store variations
	 * 
	 */
	using MoveSequence = std::vector<Move>;

	/**
	 * @brief Value bounds in alpha-beta framework
	 * https://www.chessprogramming.org/Node_Types
	 */
	enum class Bound : std::uint8_t
	{
		Upper = 0, // Fail-low  / all-node / beta is an upper bound on the score / no node improved alpha
		Exact = 1, // PV-node   / alpha and beta were improved (alpha < score < beta)
		Lower = 2  // Fail-high / cut-node / alpha is a lower bound on the score / beta cutoff
	};

	/**
	 * @brief Search depth limit
	 */
	constexpr Depth MaxDepth = 64;

	constexpr Value Draw = 0;
	constexpr Value Mate = 32000 + MaxDepth;
	constexpr Value Mated = -Mate;

	constexpr Value Infinite = 32767;

	constexpr Value mate_in(const Depth plies)
	{
		return Mate - plies;
	}

	constexpr Value mated_in(const Depth plies)
	{
		return -mate_in(plies);
	}

	constexpr Depth depth_to_mate(const Value value)
	{
		return Mate - util::abs(value);
	}

	constexpr bool is_mate(const Value value)
	{
		return util::abs(value) >= Mate - MaxDepth;
	}
} // chess

/*
 * Some useful FENs
 */
namespace chess::fens
{
	constexpr const char *Startpos = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -";
	constexpr const char *Kiwipete = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";

	constexpr const char *StartposCH = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR[-] w KQkq -";
	constexpr const char *KiwipeteCH = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R[-] w KQkq -";
} // chess::fens

