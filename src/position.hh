#pragma once

#include "magic.hh"

namespace chess
{
	/**
	 * @brief Type used to represent ply count
	 */
	using Counter = std::uint8_t;

	/**
	 * @brief Position representation
	 */
	class Position
	{
	private:
		util::array_t<Bitboard, Colours> _colours;
		util::array_t<Bitboard, PieceTypes> _types;

		Key _key;

		Counter _rule50;
		Square _en_passant;
		Castling _castling;

		Colour _side;
		Bitboard _checkers, _pinned, _blockers;

#if defined(CRAZYHOUSE)
		bool _crazyhouse;
		util::array_t<Counter, Pieces> _reserve;
		Bitboard _promoted_pawns;
#endif

		Counter _plies;

	public:
		Position();

		Position(const Position &position) = default;
		Position(Position &&position) = default;

		Position(const std::string &fen);

		Position &operator=(const Position &position) = default;
		Position &operator=(Position &&position) = default;

		////////////////////////////////////////////////////////////////////////////////////////////

		void clear();

		bool is_ok() const;

		int set_fen(const std::string &fen);

		std::string fen() const;
		std::string castling_fen() const;

		std::string to_string() const;

		////////////////////////////////////////////////////////////////////////////////////////////

		Colour colour_of_piece_on(const Square sq) const;
		PieceType type_of_piece_on(const Square sq) const;
		Piece piece_on(const Square sq) const;
		bool is_empty(const Square sq) const;

		Bitboard occupied() const;
		Bitboard occupied(const Colour colour) const;
		Bitboard occupied(const PieceType type) const;
		Bitboard occupied(const PieceType type, const PieceType type2) const;

		Bitboard occupied(const Colour colour, const PieceType type) const;
		Bitboard occupied(const Colour colour, const PieceType type, const PieceType type2) const;
		Bitboard occupied(const Piece piece) const;

		Square king_square(const Colour colour) const;

		unsigned count(const Piece piece) const;
		unsigned count(const Colour colour, const PieceType type) const;

		////////////////////////////////////////////////////////////////////////////////////////////

		Colour side_to_move() const;
		void swap_side_to_move();
		void set_side_to_move(const Colour new_side);

		Counter plies_to_root() const;
		Counter fullmoves() const;
		void set_plies_to_root(Counter new_plies);
		void reset_plies_to_root();
		void increment_plies_to_root();
		void decrement_plies_to_root();

		Counter rule50_counter() const;
		bool is_draw_by_rule50() const;
		void set_rule50_counter(Counter new_rule50);
		void reset_rule50_counter();
		void increment_rule50_counter();

		Castling castling_rights() const;
		void set_castling_rights(const Castling rights);
		void reset_castling_rights(const Castling rights);
		void add_castling_rights(const Castling rights);
		constexpr Castling castling_rights(const Square sq) const;
		bool castling_blocked(const Castling rights) const;
		bool castling_attacked(const Castling rights) const;
		bool can_castle(const Castling rights) const;
		constexpr Square castling_rook_square(const Castling rights) const;

#if defined(CRAZYHOUSE)
		Counter hand_count(const Piece piece) const;
		Counter hand_count(const Colour colour) const;
		void set_hand_count(const Piece piece, const Counter count = 1);
		void add_to_hand(const Piece piece, const Counter count = 1);
		void remove_from_hand(const Piece piece, const Counter count = 1);
		void mark_promoted_pawn(const Square sq);
		void unmark_promoted_pawn(const Square sq);
		bool is_promoted_pawn(const Square sq) const;
		bool is_crazyhouse() const;
		void set_crazyhouse(bool enabled);
#endif

		Square en_passant_square() const;
		bool has_en_passant() const;
		void set_en_passant(const Square sq);
		void reset_en_passant();

		Key key() const;

		////////////////////////////////////////////////////////////////////////////////////////////

		Bitboard checkers() const;
		Bitboard pinned() const;
		Bitboard blockers() const;
		void pinned_to(const Square sq, /*OUT*/ Bitboard &pinned, /*OUT*/ Bitboard &blockers) const;

		template <PieceType T, PieceType... Types>
		Bitboard attackers_to(const Square sq, const Bitboard occ) const;

		template <class = void>
		Bitboard attackers_to(const Square, const Bitboard) const { return 0; }

		Bitboard attackers_to(const Square sq, const Bitboard occ) const;

	private:
		template <PieceType T>
		Bitboard _attackers_to(const Square sq, const Bitboard occ) const;

	public:

		////////////////////////////////////////////////////////////////////////////////////////////

		void set_piece(const Square sq, const Piece piece, bool promoted_pawn = false);
		void remove_piece(const Square sq);
		void move_piece(const Square from, const Square to);

	private:

		void remove_piece(const Square sq, const Piece piece);
		void move_piece(const Square from, const Square to, const Piece piece);

	public:

		////////////////////////////////////////////////////////////////////////////////////////////

		Piece moved_piece(const Move move) const;
		Piece captured_piece(const Move move) const;

		bool is_capture(const Move move) const;
		bool is_castling(const Move move) const;

		bool gives_check(const Move move) const;

		bool is_pseudolegal(const Move move) const;
		bool is_legal(const Move move) const;

		void do_move(const Move move);

	private:
		Bitboard least_valuable_piece(Bitboard pieces, Colour us, PieceType &piece_type) const;

	public:

		Value see(const Move move) const;

	private:
		void update();

	};

	inline Position::Position()
		: _colours(), _types(), _key(), _rule50(),
		  _en_passant(Square::Invalid), _castling(),
		  _side(), _checkers(), _pinned(), _blockers(),
#if defined(CRAZYHOUSE)
		  _crazyhouse(), _reserve(), _promoted_pawns(),
#endif
		  _plies()
	{
	}

	inline Position::Position(const std::string &fen)
		: Position()
	{
		set_fen(fen);
	}

	inline Colour Position::colour_of_piece_on(const Square sq) const
	{
		ASSERT(is_valid(sq));

		return (occupied(Colour::Black) & sq) ? Colour::Black : Colour::White;
	}

	inline PieceType Position::type_of_piece_on(const Square sq) const
	{
		ASSERT(is_valid(sq));

		using PT = PieceType;
		for (PT type : {PT::Pawn, PT::Knight, PT::Bishop, PT::Rook, PT::Queen, PT::King})
			if (occupied(type) & sq)
				return type;

		return PT::Invalid;
	}

	inline Piece Position::piece_on(const Square sq) const
	{
		ASSERT(!is_empty(sq));

		return make_piece(colour_of_piece_on(sq), type_of_piece_on(sq));
	}

	inline bool Position::is_empty(const Square sq) const
	{
		return (occupied() & sq) == 0;
	}

	inline Bitboard Position::occupied() const
	{
		return occupied(Colour::White) | occupied(Colour::Black);
	}

	inline Bitboard Position::occupied(const Colour colour) const
	{
		ASSERT(colour == Colour::White || colour == Colour::Black);

		return _colours[util::underlying_value(colour)];
	}

	inline Bitboard Position::occupied(const PieceType type) const
	{
		ASSERT(is_valid(type));

		return _types[util::underlying_value(type)];
	}

	inline Bitboard Position::occupied(const PieceType type, const PieceType type2) const
	{
		return occupied(type) | occupied(type2);
	}

	inline Bitboard Position::occupied(const Colour colour, const PieceType type) const
	{
		return occupied(colour) & occupied(type);
	}

	inline Bitboard Position::occupied(const Colour colour, const PieceType type,
									   const PieceType type2) const
	{
		return occupied(colour) & occupied(type, type2);
	}

	inline Bitboard Position::occupied(const Piece piece) const
	{
		return occupied(colour_of(piece), type_of(piece));
	}

	inline Square Position::king_square(const Colour colour) const
	{
		return static_cast<Square>(util::lsb_64(occupied(colour, PieceType::King)));
	}

	inline unsigned Position::count(const Piece piece) const
	{
		return util::popcount_64(occupied(piece));
	}

	inline unsigned Position::count(const Colour colour, const PieceType type) const
	{
		return util::popcount_64(occupied(colour, type));
	}

	inline Colour Position::side_to_move() const
	{
		return _side;
	}

	inline void Position::swap_side_to_move()
	{
		_side = ~_side;
		_key ^= zobrist.side;
	}

	inline void Position::set_side_to_move(const Colour new_side)
	{
		_key ^= zobrist.side * (_side == Colour::Black);
		_side = new_side;
		_key ^= zobrist.side * (_side == Colour::Black);
	}

	inline Counter Position::plies_to_root() const
	{
		return _plies;
	}

	inline Counter Position::fullmoves() const
	{
		return (plies_to_root() - (side_to_move() == Colour::Black)) / 2;
	}

	inline void Position::set_plies_to_root(Counter new_plies)
	{
		_plies = new_plies;
	}

	inline void Position::reset_plies_to_root()
	{
		set_plies_to_root(0);
	}

	inline void Position::increment_plies_to_root()
	{
		set_plies_to_root(plies_to_root() + 1);
	}

	inline void Position::decrement_plies_to_root()
	{
		set_plies_to_root(plies_to_root() - 1);
	}

	inline Counter Position::rule50_counter() const
	{
		return _rule50;
	}

	inline bool Position::is_draw_by_rule50() const
	{
		return rule50_counter() >= 100;
	}

	inline void Position::set_rule50_counter(Counter new_rule50)
	{
		_rule50 = new_rule50;
	}

	inline void Position::reset_rule50_counter()
	{
		set_rule50_counter(0);
	}

	inline void Position::increment_rule50_counter()
	{
		set_rule50_counter(rule50_counter() + 1);
	}

	inline Castling Position::castling_rights() const
	{
		return _castling;
	}

	inline void Position::set_castling_rights(const Castling rights)
	{
		_key ^= zobrist.castling[util::underlying_value(_castling)];
		_castling = rights;
		_key ^= zobrist.castling[util::underlying_value(_castling)];
	}

	inline void Position::reset_castling_rights(const Castling rights)
	{
		_key ^= zobrist.castling[util::underlying_value(_castling)];
		_castling &= ~rights;
		_key ^= zobrist.castling[util::underlying_value(_castling)];
	}

	inline void Position::add_castling_rights(const Castling rights)
	{
		_key ^= zobrist.castling[util::underlying_value(_castling)];
		_castling |= rights;
		_key ^= zobrist.castling[util::underlying_value(_castling)];
	}

	constexpr Castling Position::castling_rights(const Square sq) const
	{
		ASSERT(is_valid(sq));
	
		constexpr util::array_t<Castling, Squares> BySquare
		{
			// Rank 1
			Castling::WhiteOOO, Castling::None, Castling::None, Castling::None,
			Castling::White,    Castling::None, Castling::None, Castling::WhiteOO,

			// Ranks 2-7
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,
			Castling::None,     Castling::None, Castling::None, Castling::None,

			// Rank 8
			Castling::BlackOOO, Castling::None, Castling::None, Castling::None,
			Castling::Black,    Castling::None, Castling::None, Castling::BlackOO
		};

		return BySquare[util::underlying_value(sq)];
	}

	inline bool Position::can_castle(const Castling rights) const
	{
		return bool(castling_rights() & rights)
			&& !castling_blocked(rights) && !castling_attacked(rights);
	}

	constexpr Square Position::castling_rook_square(const Castling rights) const
	{
		ASSERT(only_one(util::underlying_value(rights)));

		constexpr util::array_t<Square, 16> SourceSquares
		{
			Square::Invalid, Square::H1,      Square::A1,      Square::Invalid,
			Square::H8,      Square::Invalid, Square::Invalid, Square::Invalid,
			Square::A8,      Square::Invalid, Square::Invalid, Square::Invalid,
			Square::Invalid, Square::Invalid, Square::Invalid, Square::Invalid
		};

		return SourceSquares[util::underlying_value(rights)];
	}

#if defined(CRAZYHOUSE)
	inline Counter Position::hand_count(const Piece piece) const
	{
		ASSERT(is_valid(piece));

		return _reserve[util::underlying_value(piece)];
	}
	
	inline Counter Position::hand_count(const Colour colour) const
	{
		ASSERT(colour == Colour::White || colour == Colour::Black);

		Counter count = 0;

		using PT = PieceType;
		for (PT type : {PT::Pawn, PT::Knight, PT::Bishop, PT::Rook, PT::Queen})
			count += hand_count(make_piece(colour, type));

		return count;
	}

	inline void Position::set_hand_count(const Piece piece, const Counter count)
	{
		ASSERT(is_valid(piece));

		const unsigned i = util::underlying_value(piece);

		_key ^= zobrist.hand[i][_reserve[i]];
		_reserve[i] = count;
		_key ^= zobrist.hand[i][_reserve[i]];
	}

	inline void Position::add_to_hand(const Piece piece, const Counter count)
	{
		set_hand_count(piece, hand_count(piece) + count);
	}

	inline void Position::remove_from_hand(const Piece piece, const Counter count)
	{
		ASSERT(hand_count(piece) >= count);

		set_hand_count(piece, hand_count(piece) - count);
	}

	inline void Position::mark_promoted_pawn(const Square sq)
	{
		_promoted_pawns |= sq;
	}

	inline void Position::unmark_promoted_pawn(const Square sq)
	{
		_promoted_pawns &= ~square_bb(sq);
	}

	inline bool Position::is_promoted_pawn(const Square sq) const
	{
		return _promoted_pawns & sq;
	}

	inline bool Position::is_crazyhouse() const
	{
		return _crazyhouse;
	}

	inline void Position::set_crazyhouse(bool enabled)
	{
		_crazyhouse = enabled;
	}
#endif

	inline Square Position::en_passant_square() const
	{
		return _en_passant;
	}

	inline bool Position::has_en_passant() const
	{
		return is_valid(en_passant_square());
	}

	inline void Position::set_en_passant(const Square sq)
	{
		if (is_valid(_en_passant))
			_key ^= zobrist.en_passant[util::underlying_value(file_of(_en_passant))];

		_en_passant = sq;

		if (is_valid(_en_passant))
			_key ^= zobrist.en_passant[util::underlying_value(file_of(_en_passant))];
	}

	inline void Position::reset_en_passant()
	{
		if (is_valid(_en_passant))
			_key ^= zobrist.en_passant[util::underlying_value(file_of(_en_passant))];

		_en_passant = Square::Invalid;
	}

	inline Key Position::key() const
	{
		return _key;
	}

	inline Bitboard Position::checkers() const
	{
		return _checkers;
	}

	inline Bitboard Position::pinned() const
	{
		return _pinned;
	}

	inline Bitboard Position::blockers() const
	{
		return _blockers;
	}

	template <PieceType T, PieceType ...Types>
	inline Bitboard Position::attackers_to(const Square sq, const Bitboard occ) const
	{
		return _attackers_to<T>(sq, occ) | attackers_to<Types...>(sq, occ);
	}

	inline Bitboard Position::attackers_to(const Square sq, const Bitboard occ) const
	{
		using PT = PieceType;
		return attackers_to<PT::Pawn, PT::Knight, PT::Bishop, PT::Rook, PT::King>(sq, occ);
	}
	
	template <PieceType T>
	inline Bitboard Position::_attackers_to(const Square sq, const Bitboard occ) const
	{
		return attacks_from<T>(sq, occ) & occupied(T);
	}

	template <>
	inline Bitboard Position::_attackers_to<PieceType::Bishop>(const Square sq,
															   const Bitboard occ) const
	{
		return occupied(PieceType::Bishop, PieceType::Queen)
			 & attacks_from<PieceType::Bishop>(sq, occ);
	}

	template <>
	inline Bitboard Position::_attackers_to<PieceType::Rook>(const Square sq,
															 const Bitboard occ) const
	{
		return occupied(PieceType::Rook, PieceType::Queen)
			 & attacks_from<PieceType::Rook>(sq, occ);
	}

	template <>
	inline Bitboard Position::_attackers_to<PieceType::Pawn>(const Square sq,
															 const Bitboard) const
	{
		return (pawn_attacks(Colour::White, sq) & occupied(Colour::Black, PieceType::Pawn))
			 | (pawn_attacks(Colour::Black, sq) & occupied(Colour::White, PieceType::Pawn));
	}

	inline Piece Position::moved_piece(const Move move) const
	{
		return piece_on(move.from());
	}

	inline Piece Position::captured_piece(const Move move) const
	{
		// Does not consider en passant!
		return piece_on(move.to());
	}

	inline bool Position::is_capture(const Move move) const
	{
		return !is_empty(move.to());
	}

	inline bool Position::is_castling(const Move move) const
	{
		return file_distance(move.from(), move.to()) == 2
			&& (move.from() & occupied(PieceType::King));
	}

	inline void Position::update()
	{
		const Colour us = side_to_move();
		_checkers = attackers_to(king_square(us), occupied()) & occupied(~us);
		
		_pinned = _blockers = 0;
		pinned_to(king_square(us), _pinned, _blockers);
		pinned_to(king_square(~us), _pinned, _blockers);
	}
} // chess
