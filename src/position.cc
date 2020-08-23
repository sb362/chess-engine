#include "position.hh"

#include <sstream>

using namespace chess;

/**
 * @brief Clear this position, removing all the pieces and resetting all state variables
 */
void Position::clear()
{
	for (Bitboard &bb : _colours)
		bb = 0;

	for (Bitboard &bb : _types)
		bb = 0;

	_side = Colour::White;
	_plies = _rule50 = 0;
	_castling = Castling::None;
	_en_passant = Square::Invalid;
	_checkers = _pinned = _blockers = 0;
	_key = 0;

#if defined(CRAZYHOUSE)
	_crazyhouse = false;
	_promoted_pawns = 0;

	for (Counter &count : _reserve)
		count = 0;
#endif
}

/**
 * @brief Test if a position is valid
 * 
 * @return true 
 * @return false 
 */
bool Position::is_ok() const
{
	return true;
}

/**
 * @brief 
 * 
 * @param fen 
 * @return int 
 */
int Position::set_fen(const std::string &fen)
{
	clear();

	std::istringstream ss {fen};
	std::string token;

	// Parse piece placement
	// Check each character until we encounter a space or an error
	ss >> token;

	File file = File::A;
	Rank rank = Rank::Eight;

	bool is_in_hand = false;
	bool lichess_style = false;
	bool next_piece_is_promoted = false;

	for (char c : token)
	{
		if (std::isdigit(c))
		{
			const int skip = c - '0';

			file += skip;

			// Check bounds
			//if (!is_valid(file))
			//	return 11; // Out of bounds

			continue;
		}
		else if (c == '/')
		{
			file = File::A;
			--rank;

			// Lichess crazyhouse FENs contain a 9th rank for storing pieces in hand
			if (!is_valid(rank))
			{
				is_in_hand = true;
				lichess_style = true;
			}

			continue;
		}
		else if (const std::size_t pos = PieceChars.find(c); pos != std::string::npos)
		{
			const Square sq = make_square(file, rank);
			const Piece piece = static_cast<Piece>(pos);

			if (!is_valid(piece))
				continue;

#if defined(CRAZYHOUSE)
			if (is_in_hand)
				add_to_hand(piece);
			else
#endif
			{
				set_piece(sq, piece, next_piece_is_promoted);
				next_piece_is_promoted = false;
			}
		}
		else if (c == ' ')
			break;
		else if (c == '[')
		{
			is_in_hand = true;
			continue;
		}
		else if (c == ']')
			continue;
		else if (c == '~')
		{
			next_piece_is_promoted = true;
			continue;
		}
		else
			return 19; // Unknown/unexpected character

		++file;
	}

#if defined(CRAZYHOUSE)
	set_crazyhouse(is_in_hand);
#endif

	// Parse side to move
	ss >> token;
	if (token == "w" || token == "b")
		set_side_to_move(token == "w" ? Colour::White : Colour::Black);
	else
		return 39; // Unknown/unexpected character

	// Update check and pin info now that we have piece placement + side to move
	update();

	// Parse castling
	ss >> token;
	if (token != "-")
	{
		for (char c : token)
		{
			const Colour us = std::isupper(c) ? Colour::White : Colour::Black;
			const Piece rook = make_piece(us, PieceType::Rook);
			const Square ksq = king_square(us);
			Square rsq;

			if (c == 'K')
				rsq = static_cast<Square>(util::msb_64(occupied(rook) & Rank1BB));
			else if (c == 'Q')
				rsq = static_cast<Square>(util::lsb_64(occupied(rook) & Rank1BB));
			else if (c == 'k')
				rsq = static_cast<Square>(util::msb_64(occupied(rook) & Rank8BB));
			else if (c == 'q')
				rsq = static_cast<Square>(util::lsb_64(occupied(rook) & Rank8BB));
			else if (c = std::tolower(c); 'a' <= c && c <= 'h')
				return 41; // FRC/chess960 not supported
			else if (c == ' ')
				break;
			else
				return 49; // Unexpected/unknown character

			add_castling_rights(make_castling_rights(us, rsq > ksq));
		}
	}

	// Parse en passant
	ss >> token;

	if (token != "-")
	{
		if (const Square sq = parse_square(token); is_valid(sq))
			set_en_passant(sq);
		else
			return 59; // Unexpected/unknown character
	}

	// Parse fifty moves rule counter (default to zero)
	if (int counter; ss >> counter)
		set_rule50_counter(counter);

	// Parse fullmoves counter (default to one)
	if (int counter; ss >> counter)
		set_plies_to_root((counter - 1) * 2 + (side_to_move() == Colour::Black));

	return 0;
}

/**
 * @brief Generates the FEN string describing this position
 * 
 * @return std::string Forsyth-Edwards notation
 */
std::string Position::fen() const
{
	std::string s;

	int empty = 0;
	for (Rank rank = Rank::Eight; is_valid(rank); --rank)
	{
		for (File file = File::A; is_valid(file); ++file)
		{
			const Square sq = make_square(file, rank);

			if (is_empty(sq))
				++empty;
			else
			{
				if (empty)
				{
					s += ('0' + empty);
					empty = 0;
				}

#if defined(CRAZYHOUSE)
				if (is_crazyhouse() && is_promoted_pawn(sq))
					s += '~';
#endif

				s += to_char(piece_on(sq));
			}
		}

		if (empty)
		{
			s += ('0' + empty);
			empty = 0;
		}

		if (rank > Rank::One)
			s += '/';
	}

#if defined(CRAZYHOUSE)
	if (is_crazyhouse())
	{
		s += '/';

		using PT = PieceType;
		for (Colour colour : {Colour::White, Colour::Black})
		{
			for (PT type : {PT::Pawn, PT::Knight, PT::Bishop, PT::Rook, PT::Queen})
			{
				const Piece piece = make_piece(colour, type);
				s += std::string(hand_count(piece), to_char(piece));
			}
		}
	}
#endif

	return fmt::format("{} {} {} {} {} {}", s, side_to_move(), castling_fen(),
					   en_passant_square(), rule50_counter(), 1 + fullmoves());
}

/**
 * @brief Generates the castling part of a FEN string (e.g. "KQkq")
 * 
 * @return std::string 
 */
std::string Position::castling_fen() const
{
	if (!bool(castling_rights() & Castling::Any))
		return "-";
	
	std::string s;

	if (bool(castling_rights() & Castling::WhiteOO))  s += 'K';
	if (bool(castling_rights() & Castling::WhiteOOO)) s += 'Q';
	if (bool(castling_rights() & Castling::BlackOO))  s += 'k';
	if (bool(castling_rights() & Castling::BlackOOO)) s += 'q';

	return s;
}

/**
 * @brief Generates a human-readable string representing this position, ideal for terminal output
 * 
 * @return std::string 
 */
std::string Position::to_string() const
{
	std::string s = "/---------------\\\n";

	for (Rank rank = Rank::Eight; is_valid(rank); --rank)
	{
		for (File file = File::A; is_valid(file); ++file)
		{
			const Square sq = make_square(file, rank);

			s += '|';
			
			if (is_empty(sq))
				s += '-';
			else
				s += to_char(piece_on(sq));
		}

		s += "|\n";
	}

	s += "\\---------------/\n";
	s += fmt::format("Side to move:      {}\n", side_to_move() == Colour::White ? "white" : "black");
	s += fmt::format("Castling rights:   {}\n", castling_fen());
	s += fmt::format("En passant square: {}\n", en_passant_square());
	s += fmt::format("Fullmoves:         {}\n", fullmoves() + 1);
	s += fmt::format("Half-move clock:   {}\n", rule50_counter());

#if defined(CRAZYHOUSE)
	if (is_crazyhouse())
	{
		s += "Hand:             ";

		using PT = PieceType;
		for (Colour colour : {Colour::White, Colour::Black})
		{
			for (PT type : {PT::Pawn, PT::Knight, PT::Bishop, PT::Rook, PT::Queen})
			{
				Counter count;
				if (const Piece piece = make_piece(colour, type); (count = hand_count(piece)))
				{
					s += ' ';
					if (count > 1)
						s += fmt::format("{}{}", count, to_char(piece));
					else
						s += to_char(piece);
				}
			}
		}

		if ((hand_count(Colour::White) + hand_count(Colour::Black)) == 0)
			s += " (empty)";
		
		s += '\n';
	}
#endif

	s += fmt::format("Zobrist hash:      {:016x}\n", key());

	return s;
}

/**
 * @brief Determines all pieces pinned to the piece placed on @param sq
 * Note that this function does not set @param pinned and @param blockers to zero before populating.
 * 
 * @param sq 
 * @param pinned Pieces of the same colour as the piece on @param sq that are blocking an attack
 * @param blockers Pieces not of the same colour as the piece on @param sq that are blocking an attack
 */
void Position::pinned_to(const Square sq, Bitboard &pinned, Bitboard &blockers) const
{
	ASSERT(is_valid(sq));
	ASSERT(!is_empty(sq));

	const Colour us = colour_of_piece_on(sq);
	const Bitboard friendly = occupied(us), enemy = occupied(~us);
	const Bitboard occ = occupied();

	// Possible pinners
	Bitboard candidates = attackers_to<PieceType::Bishop, PieceType::Rook>(sq, 0) & enemy;

	while (candidates)
	{
		const Square csq = static_cast<Square>(util::lsb_64(candidates));

		// If there is only one piece between sq and csq, then that
		// piece is pinned if it is the same colour as the piece on sq.
		const Bitboard maybe_pinned = line_between(sq, csq) & occ;
		if (only_one(maybe_pinned))
		{
			pinned |= maybe_pinned & friendly;
			blockers |= maybe_pinned & enemy;
		}

		// Pop LSB
		candidates &= (candidates - 1);
	}
}

/**
 * @brief Determines if castling is obstructed (i.e. pieces lie between the king and rook)
 * 
 * @param rights 
 * @return true 
 * @return false 
 */
bool Position::castling_blocked(const Castling rights) const
{
	ASSERT(only_one(util::underlying_value(rights)));

	const Colour us  = bool(rights & Castling::White) ? Colour::White : Colour::Black;
	const Square rsq = castling_rook_square(rights);
	const Square ksq = king_square(us);
	const Square rto = castling_rook_dest(rights);
	const Square kto = castling_king_dest(rights);

	return occupied() & castling_path(ksq, kto, rsq, rto);
}

/**
 * @brief Determines if castling would put the king in check
 * 
 * @param rights 
 * @return true 
 * @return false 
 */
bool Position::castling_attacked(const Castling rights) const
{
	ASSERT(only_one(util::underlying_value(rights)));

	const Colour us  = bool(rights & Castling::White) ? Colour::White : Colour::Black;
	const Square kto = castling_king_dest(rights);
	const Square ksq = king_square(us);

	const Direction d = kto > ksq ? West : East;
	for (Square sq = kto; sq != ksq; sq += d)
		if (attackers_to(sq, occupied()) & occupied(~us))
			return true;

	return false;
}

/**
 * @brief Place a given piece @param piece on given square @param sq
 * 
 * @param sq Square
 * @param piece Piece
 */
void Position::set_piece(const Square sq, const Piece piece, bool promoted_pawn)
{
	ASSERT(is_valid(sq));
	ASSERT(is_valid(piece));

#if defined(CRAZYHOUSE)
	if (promoted_pawn)
		_promoted_pawns |= sq;
#else
	(void)promoted_pawn;
#endif

	_types  [util::underlying_value(type_of(piece))]   |= sq;
	_colours[util::underlying_value(colour_of(piece))] |= sq;

	_key ^= zobrist.piece_square[util::underlying_value(piece)][util::underlying_value(sq)];
}

/**
 * @brief Remove the piece that exists on square @param sq
 * 
 * @param sq Square
 */
void Position::remove_piece(const Square sq)
{
	remove_piece(sq, piece_on(sq));
}

/**
 * @brief Teleport a piece from square @param from to square @param to.
 * Not to be confused with do_move().
 * 
 * @param from Square
 * @param to Square
 */
void Position::move_piece(const Square from, const Square to)
{
	move_piece(from, to, piece_on(from));
}

void Position::remove_piece(const Square sq, const Piece piece)
{
	ASSERT(is_valid(sq));
	ASSERT(is_valid(piece));

#if defined(CRAZYHOUSE)
	_promoted_pawns &= ~square_bb(sq);
#endif
	_types  [util::underlying_value(type_of(piece))]   ^= sq;
	_colours[util::underlying_value(colour_of(piece))] ^= sq;

	_key ^= zobrist.piece_square[util::underlying_value(piece)][util::underlying_value(sq)];
}

void Position::move_piece(const Square from, const Square to, const Piece piece)
{
	ASSERT(is_valid(from));
	ASSERT(is_valid(to));
	ASSERT(is_valid(piece));

	const Bitboard mask = squares_bb(from, to);

#if defined(CRAZYHOUSE)
	if (is_promoted_pawn(from))
		_promoted_pawns ^= mask;
#endif

	_types  [util::underlying_value(type_of(piece))]   ^= mask;
	_colours[util::underlying_value(colour_of(piece))] ^= mask;

	_key ^= zobrist.piece_square[util::underlying_value(piece)][util::underlying_value(from)];
	_key ^= zobrist.piece_square[util::underlying_value(piece)][util::underlying_value(to)];
}

/**
 * @brief Test if a move would put the opponent's king in check.
 * The move is assumed to be pseudo-legal.
 * 
 * @param move Move
 * @return true 
 * @return false 
 */
bool Position::gives_check(const Move) const
{
	return false; // todo
}

/**
 * @brief Test if a move is 'pseudo-legal' (i.e. may put our king in check, but otherwise is legal)
 * 
 * @param move Move
 * @return true 
 * @return false 
 */
bool Position::is_pseudolegal(const Move) const
{
	return true; // todo
}

/**
 * @brief Test if a move is legal (i.e. would not put our king in check).
 * The move is assumed to be pseudo-legal.
 * 
 * @param move Move
 * @return true 
 * @return false 
 */
bool Position::is_legal(const Move) const
{
	return true; // todo
}

/**
 * @brief Apply the given move to this position.
 * The move is assumed to be both pseudo-legal and legal.
 * It will probably throw an assertion failed exception if the move is illegal.
 * 
 * @param move Move
 */
void Position::do_move(const Move move)
{
	ASSERT(move.is_valid());

	const Square from = move.from(), to = move.to();
	const Colour us = side_to_move();

	// Update counters, swap side to move
	increment_plies_to_root();
	increment_rule50_counter();
	swap_side_to_move();

	const Square en_passant = en_passant_square();
	reset_en_passant();

	// Capture
	if (!is_empty(to))
	{
		const PieceType captured_piece_type = type_of_piece_on(to);
#if defined(CRAZYHOUSE)
		if (is_promoted_pawn(to))
			add_to_hand(make_piece(us, PieceType::Pawn));
		else
			add_to_hand(make_piece(us, captured_piece_type));
#endif

		remove_piece(to, make_piece(~us, captured_piece_type));
		reset_rule50_counter();
	}

	// Drops
#if defined(CRAZYHOUSE)
	if (move.is_drop())
	{
		ASSERT(is_empty(to));

		const Piece drop = make_piece(us, move.drop());

		set_piece(to, drop);
		remove_from_hand(drop);
	} else
#endif
	if (from & occupied(PieceType::Pawn))
	{
		// Pawn moves reset fifty moves rule counter
		reset_rule50_counter();

		// Promotions
		if (move.is_promotion())
		{
			remove_piece(from, make_piece(us, PieceType::Pawn));
			set_piece(to, make_piece(us, move.promotion()));

#if defined(CRAZYHOUSE)
			mark_promoted_pawn(to);
#endif
		}
		else
		{
			// En passant capture
			if (en_passant == to)
			{
				remove_piece(to - pawn_push(us), make_piece(~us, PieceType::Pawn));
#if defined(CRAZYHOUSE)
				add_to_hand(make_piece(us, PieceType::Pawn));
#endif
			}
			else if (rank_distance(from, to) == 2)
			{
				const Square new_en_passant = to - pawn_push(us);
				if (pawn_attacks(us, new_en_passant) & occupied(~us, PieceType::Pawn))
					set_en_passant(new_en_passant);
			}

			move_piece(from, to, make_piece(us, PieceType::Pawn));
		}
	}
	else if (is_castling(move))
	{
		const Castling rights = make_castling_rights(us, to > from);
		const Square rsq = castling_rook_square(rights);
		const Square rto = castling_rook_dest(rights);

		move_piece(rsq, rto, make_piece(us, PieceType::Rook));
		move_piece(from, to, make_piece(us, PieceType::King));
	}
	else
		move_piece(from, to);

	// Update castling rights
	if (castling_rights(from) != Castling::None)
		reset_castling_rights(castling_rights(from));

	if (castling_rights(to) != Castling::None)
		reset_castling_rights(castling_rights(to));

	// Update check and pin info
	update();

	ASSERT(is_ok());
}

Bitboard Position::least_valuable_piece(Bitboard, Colour, PieceType &) const
{
	return 0;
}

Value Position::see(const Move) const
{
	return 0;
}
