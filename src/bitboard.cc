#include "bitboard.hh"

namespace chess::bitboards
{

util::array_t<Bitboard, Squares, Squares> line_connecting;
util::array_t<Bitboard, Squares> knight_attacks;

void init()
{
	for (Square a = Square::A1; a <= Square::H8; ++a)
	{
		const auto i = util::underlying_value(a);
		const auto bb = square_bb(a);

		knight_attacks[i] = attacks_from<PieceType::Knight>(bb);

		for (auto b = Square::A1; b <= Square::H8; ++b)
		{
			const auto j = util::underlying_value(b);
			const auto ab = squares_bb(a, b);

			if (attacks_from<PieceType::Bishop>(a) & b)
				line_connecting[i][j] =
					(attacks_from<PieceType::Bishop>(a) & attacks_from<PieceType::Bishop>(b)) | ab;
			else if (attacks_from<PieceType::Rook>(a) & b)
				line_connecting[i][j] =
					(attacks_from<PieceType::Rook>(a) & attacks_from<PieceType::Rook>(b)) | ab;
		}
	}
}

}

std::string chess::to_string(const Bitboard bb)
{
	std::string s = "/---------------\\\n";

	for (auto rank = Rank::Eight; is_valid(rank); --rank)
	{
		for (auto file = File::A; is_valid(file); ++file)
		{
			s += bb & make_square(file, rank) ? "|1" : "|0";
		}
		s += "|\n";
	}

	s += "\\---------------/\n";

	return s;
}
