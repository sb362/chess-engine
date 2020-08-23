#include "movegen.hh"

using namespace chess;

template <PieceType T>
void append_moves(MoveList &move_list, const Position &position,
				  const Colour us, Bitboard targets)
{
	// Use append_king_moves / append_pawn_moves instead
	static_assert(T != PieceType::Pawn && T != PieceType::King, "Unsupported piece type");

	const Square ksq = position.king_square(us);
	const Bitboard pinned = position.pinned();
	const Bitboard occ = position.occupied();

	// Loop through all pieces of given type
	Bitboard pieces = position.occupied(us, T);
	while (pieces)
	{
		const Square from = static_cast<Square>(util::lsb_64(pieces));

		// Loop through all squares in attack set
		Bitboard attacks = attacks_from<T>(from, occ) & targets;
		while (attacks)
		{
			const Square to = static_cast<Square>(util::lsb_64(attacks));

			// Append move if the piece is not pinned to the king
			if (!(pinned & from) || aligned(ksq, from, to))
				move_list.push_back({from, to});

			attacks &= (attacks - 1);
		}

		pieces &= (pieces - 1);
	}
}

void append_king_moves(MoveList &move_list, const Position &position,
					   const Colour us, Bitboard targets)
{
	const Square ksq = position.king_square(us);
	const Bitboard enemy = position.occupied(~us);
	const Bitboard occ = position.occupied();

	// Loop through all squares in the attack set
	Bitboard attacks = attacks_from<PieceType::King>(ksq) & targets;
	while (attacks)
	{
		const Square to = static_cast<Square>(util::lsb_64(attacks));

		if ((position.attackers_to(to, occ ^ ksq) & enemy) == 0)
			move_list.push_back({ksq, to});

		attacks &= (attacks - 1);
	}
}

#if defined(CRAZYHOUSE)
template <PieceType T>
void append_drops(MoveList &move_list, const Position &position,
				  const Colour us, Bitboard targets)
{
	const Bitboard occ = position.occupied();
	const Piece piece = make_piece(us, T);

	// Ensure squares we are dropping onto are empty
	targets &= ~occ;

	if constexpr (T == PieceType::Pawn)
		targets &= ~(Rank1BB | Rank8BB);

	// Loop through all squares in target set
	while (targets)
	{
		const Square to = static_cast<Square>(util::lsb_64(targets));

		if (position.hand_count(piece))
			move_list.push_back({to, T});

		targets &= (targets - 1);
	}
}
#endif

void append_promotions(MoveList &move_list, const Square from, const Square to)
{
	move_list.push_back({from, to, PieceType::Queen});
	move_list.push_back({from, to, PieceType::Rook});
	move_list.push_back({from, to, PieceType::Bishop});
	move_list.push_back({from, to, PieceType::Knight});
}

template <Colour Us>
void append_pawn_moves(MoveList &move_list, const Position &position, Bitboard targets)
{
	constexpr Rank Rank3 = Us == Colour::White ? Rank::Three : Rank::Six;
	constexpr Rank Rank7 = Us == Colour::White ? Rank::Seven : Rank::Two;
	constexpr Direction Up = pawn_push(Us);
	constexpr Direction UpWest = Up + West, UpEast = Up + East;

	const Square ksq = position.king_square(Us);
	const Bitboard pinned = position.pinned();
	const Bitboard pawns = position.occupied(Us, PieceType::Pawn);
	const Bitboard occ = position.occupied(), empty = ~occ, enemy = position.occupied(~Us);

	// En passant
	if (position.has_en_passant())
	{
		const Square en_passant = position.en_passant_square();
		const Square target_sq = en_passant + pawn_push(~Us);
		if (targets & target_sq)
		{
			// 'candidates' is the bitboard of pawns that could perform en passant (at most two)
			Bitboard candidates = pawn_attacks(~Us, en_passant)
								& position.occupied(Us, PieceType::Pawn);
			while (candidates)
			{
				const Square from = static_cast<Square>(util::lsb_64(candidates));

				// Check if performing en passant puts us in check.
				// Only sliding pieces can put us in check here.
				const Bitboard nocc = (occ ^ from ^ target_sq) | en_passant;
				if (!(position.attackers_to<PieceType::Bishop, PieceType::Rook>(ksq, nocc) & enemy))
					move_list.push_back({from, en_passant});
		
				candidates &= (candidates - 1);
			}
		}
	}

	const Bitboard pawns_on_7 = pawns & Rank7, pawns_not_on_7 = pawns & ~pawns_on_7;
	Bitboard bb;

	// Promotions, w/o captures
	bb = shift<Up>(pawns_on_7) & empty & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb));
		const Square from = to - Up;

			if (!(pinned & from))
			append_promotions(move_list, from, to);

		bb &= (bb - 1);
	}

	// Captures, w/ promotion, 1/2
	bb = shift<UpWest>(pawns_on_7) & enemy & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb));
		const Square from = to - UpWest;

		if ((!(pinned & from) || aligned(ksq, from, to)))
			append_promotions(move_list, from, to);

		bb &= (bb - 1);
	}

	// Captures, w/ promotion, 2/2
	bb = shift<UpEast>(pawns_on_7) & enemy & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb));
		const Square from = to - UpEast;

		if ((!(pinned & from) || aligned(ksq, from, to)))
			append_promotions(move_list, from, to);

		bb &= (bb - 1);
	}

	// Pawn push, w/o promotion
	const Bitboard single_push = shift<Up>(pawns_not_on_7) & empty;
	bb = single_push & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb));
		const Square from = to - Up;

		if ((!(pinned & from) || aligned(ksq, from, to)))
			move_list.push_back({from, to});

		bb &= (bb - 1);
	}

	// Double pawn push
	bb = shift<Up>(single_push & Rank3) & empty & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb)); 
		const Square from = to - Up * 2;

		if ((!(pinned & from) || aligned(ksq, from, to)))
			move_list.push_back({from, to});

		bb &= (bb - 1);
	}

	// Captures, w/o promotion, 1/2
	bb = shift<UpWest>(pawns_not_on_7) & enemy & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb));
		const Square from = to - UpWest;

		if ((!(pinned & from) || aligned(ksq, from, to)))
			move_list.push_back({from, to});

		bb &= (bb - 1);
	}

	// Captures, w/o promotion, 2/2
	bb = shift<UpEast>(pawns_not_on_7) & enemy & targets;
	while (bb)
	{
		const Square to = static_cast<Square>(util::lsb_64(bb));
		const Square from = to - UpEast;

		if ((!(pinned & from) || aligned(ksq, from, to)))
			move_list.push_back({from, to});

		bb &= (bb - 1);
	}
}

void MoveList::generate()
{
	const Square ksq = position.king_square(us);
	const Bitboard checkers = position.checkers();

	ASSERT(!checkers || us == position.side_to_move());

	Bitboard targets = ~position.occupied(us);

	// King moves
	append_king_moves(*this, position, us, targets);

	// Check
	if (checkers)
	{
		// If in double check, only king moves will be legal.
		if (more_than_one(checkers))
			return;

		// Apply a filter to target set such that the moves
		// will block the check or capture the checking piece.
		const Square checker = static_cast<Square>(util::lsb_64(checkers));
		targets &= line_between(ksq, checker) | checkers;
	}
	else
	{
		if (Castling rights = make_castling_rights(us, true); position.can_castle(rights))
			push_back({ksq, castling_king_dest(rights)});

		if (Castling rights = make_castling_rights(us, false); position.can_castle(rights))
			push_back({ksq, castling_king_dest(rights)});
	}

	// Crazyhouse drops
#if defined(CRAZYHOUSE)
	if (position.is_crazyhouse())
	{
		append_drops<PieceType::Queen> (*this, position, us, targets);
		append_drops<PieceType::Rook>  (*this, position, us, targets);
		append_drops<PieceType::Bishop>(*this, position, us, targets);
		append_drops<PieceType::Knight>(*this, position, us, targets);
		append_drops<PieceType::Pawn>  (*this, position, us, targets);
	}
#endif

	// Regular moves
	append_moves<PieceType::Queen> (*this, position, us, targets);
	append_moves<PieceType::Rook>  (*this, position, us, targets);
	append_moves<PieceType::Bishop>(*this, position, us, targets);
	append_moves<PieceType::Knight>(*this, position, us, targets);

	// Pawn moves
	us == Colour::White ? append_pawn_moves<Colour::White>(*this, position, targets)
						: append_pawn_moves<Colour::Black>(*this, position, targets);
}

MoveWithValue MoveList::select()
{
	std::iter_swap(cur, std::max_element(cur, end()));
	return *cur++;
}


template <PieceType T>
unsigned approx_mobility(const Position &position, const Colour us, Bitboard targets)
{
	static_assert(T != PieceType::Pawn && T != PieceType::King, "Unsupported piece type");

	unsigned count = 0;

	const Bitboard pinned = position.pinned();
	const Bitboard occ = position.occupied();

	Bitboard pieces = position.occupied(us, T) & ~pinned;
	while (pieces)
	{
		const Square from = static_cast<Square>(util::lsb_64(pieces));

		Bitboard attacks = attacks_from<T>(from, occ) & targets;
		count += util::popcount_64(attacks);
		pieces &= (pieces - 1);
	}

	return count;
}

template <Colour Us>
unsigned approx_mobility(const Position &position)
{
	ASSERT(!position.checkers());

	unsigned count = 0;

	const Square ksq = position.king_square(Us);
	const Bitboard pinned = position.pinned();
	const Bitboard enemy = position.occupied(~Us);
	const Bitboard occ = position.occupied();

	Bitboard targets = ~position.occupied(Us);

	// King moves
	Bitboard attacks = attacks_from<PieceType::King>(ksq) & targets;
	while (attacks)
	{
		const Square to = static_cast<Square>(util::lsb_64(attacks));
		count += (position.attackers_to(to, occ ^ ksq) & enemy) == 0;
		attacks &= (attacks - 1);
	}

#if defined(CRAZYHOUSE)
	if (position.is_crazyhouse())
	{
		for (PieceType type : {PieceType::Knight, PieceType::Bishop, PieceType::Rook, PieceType::Queen})
		{
			const Bitboard drop_targets = targets & ~occ;
			const Piece piece = make_piece(Us, type);
			
			count += util::popcount_64(drop_targets) * position.hand_count(piece) >= 1;
		}

		count +=  util::popcount_64(targets & ~occ & ~(Rank1BB | Rank8BB))
				* (position.hand_count(make_piece(Us, PieceType::Pawn)) >= 1);
	}
#endif

	constexpr Rank Rank3 = Us == Colour::White ? Rank::Three : Rank::Six;
	constexpr Rank Rank7 = Us == Colour::White ? Rank::Seven : Rank::Two;
	constexpr Direction Up = pawn_push(Us);
	constexpr Direction UpWest = Up + West, UpEast = Up + East;

	count += approx_mobility<PieceType::Knight>(position, Us, targets);
	count += approx_mobility<PieceType::Bishop>(position, Us, targets);
	count += approx_mobility<PieceType::Rook>  (position, Us, targets);
	count += approx_mobility<PieceType::Queen> (position, Us, targets);

	const Bitboard pawns = position.occupied(Us, PieceType::Pawn) & ~pinned;
	const Bitboard pawns_on_7 = pawns & Rank7, pawns_not_on_7 = pawns & ~pawns_on_7;

	const Bitboard single_push = shift<Up>(pawns_not_on_7) & ~occ;
	count += util::popcount_64(single_push & targets);
	count += util::popcount_64(shift<Up>(single_push & Rank3) & ~occ & targets);

	count += util::popcount_64(shift<Up    >(pawns_on_7) & ~occ & targets) * 4;
	count += util::popcount_64(shift<UpWest>(pawns_on_7) & enemy & targets) * 4;
	count += util::popcount_64(shift<UpEast>(pawns_on_7) & enemy & targets) * 4;

	count += util::popcount_64(shift<UpWest>(pawns_not_on_7) & enemy & targets);
	count += util::popcount_64(shift<UpEast>(pawns_not_on_7) & enemy & targets);

	return count;
}

unsigned chess::approx_mobility(const Position &position, const Colour us)
{
	return us == Colour::White ? ::approx_mobility<Colour::White>(position)
							   : ::approx_mobility<Colour::Black>(position);
}
