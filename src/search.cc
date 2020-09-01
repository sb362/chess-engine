#include "evaluation.hh"
#include "moveorder.hh"
#include "search.hh"
#include "tt.hh"

using namespace chess;
using namespace chess::search;

Thread::Thread(std::size_t id)
	: threading::Thread(id), root_position(), limits(),
	  id_depth(), sel_depth(), nodes(), qnodes(),
	  pawn_cache(std::make_unique<pawns::Cache>()),
	  heuristics(), root_pv(), root_value(-Infinite)
{
}

/**
 * @brief Main search routine, using alpha-beta pruning
 * 
 * @param alpha 
 * @param beta 
 * @param depth 
 * @param plies_to_root 
 * @param pv 
 * @return Value 
 */
Value Thread::search(const Position &position, Value alpha, Value beta, const Depth depth,
					 const Depth plies_to_root, MoveSequence &pv)
{
	const Nodes total_nodes_searched = nodes_searched() + qnodes_searched();

	// Check for stop signal or if we've reached the node limit
	if (should_stop() || (limits.nodes && total_nodes_searched >= limits.nodes))
	{
		// If we are in check, the position is probably dangerous. Return draw value instead.
		return position.checkers() ? Draw : eval::evaluate(position, pawn_cache.get());
	}

	// Zobrist key for this node
	const Key key = position.key();

	// Check for draw by fifty moves / threefold repetition
	// The value we return here is Draw ± 1, which solves an issue with threefold blindness.
	if (position.is_draw_by_rule50() || std::count(key_history.begin(), key_history.end(), key) >= 3)
		return (total_nodes_searched & 3) - 1;

	// Update selective depth
	sel_depth = util::max(sel_depth, plies_to_root);

	// Move found in transposition table or PV move from last iteration
	Move hash_move;

	// At root node, make sure we try the best move from the previous iteration first
	if (plies_to_root == 0)
	{
		if (!root_pv.empty())
			hash_move = root_pv[0];
	}
	// Probe transposition table at non-root nodes
	else
	{
		const Entry *entry = tt.probe(key);

		if (entry)
		{
			// If the entry contains results from a greater depth
			// we can check for a cutoff. If not, we can still use
			// the move stored for ordering later.
			if (entry->depth >= depth)
			{
				const Bound bound = static_cast<Bound>(entry->bound);
				Value value = entry->value;

				// Fix mate values, as mate values are stored relative
				// to the current position. This solves the issue of
				// retrieving a mate in x when plies to root > x.
				if (is_mate(value))
					value = absolute_mate_value(value, plies_to_root);

				if (bound == Bound::Exact)
					return value;
				else if (bound == Bound::Upper && value <= alpha)
					return alpha;
				else if (bound == Bound::Lower && value >= beta)
					return beta;
			}

			hash_move = entry->move;
		}
	}

	// Horizon node, start quiescence search
	if (depth == 0)
		return qsearch(position, alpha, beta, plies_to_root, pv);
	
	// Generate all legal moves
	MoveList move_list {position};

	// Check for checkmate or stalemate
	if (move_list.size() == 0)
		return position.checkers() ? mated_in(plies_to_root) : Draw; 

	// Move ordering
	evaluate_move_list(position, move_list, depth, hash_move, heuristics);

	Bound bound = Bound::Upper;
	Value value;
	Move best_move;
	MoveSequence child_pv;

	// Search each move
	for (unsigned move_number = 0; move_number < move_list.size(); ++move_number)
	{
		const Move move = move_list.select();

		const Piece moved_piece = position.moved_piece(move);
		const bool is_capture = position.is_capture(move);
		const bool is_promotion = move.is_promotion();

		// Make a copy, apply the move, store new key in key stack, and increment nodes counter
		Position next_position {position};
		next_position.do_move(move);
		key_history.push_back(next_position.key());
		nodes.fetch_add(1, std::memory_order_relaxed);

		const bool gives_check = next_position.checkers();

		// Reductions and extensions
		Depth r = 1;

		// Late move reductions
		// http://rebel13.nl/rebel13/blog/lmr%20advanced.html
		bool did_lmr = false;
		if (depth >= LMRDepthLimit && move_number > LMRMoveNumber
			&& !gives_check && !is_capture && !is_promotion)
		{
			++r;

			if (plies_to_root > 0)
			{
				r += move_number > LMRMoveNumber2;

				// Reduce further if the move has a bad history
				r += heuristics.history.probe(moved_piece, move.to()) < 0;
			}

			did_lmr = true;

			r = util::clamp(r, 1, depth);
		}

		// Search this move
		child_pv.clear();
		value = -search(next_position, -beta, -alpha, depth - r, plies_to_root + 1, child_pv);

		// Redo search with full depth if we reduced this node but we improved alpha
		if (did_lmr && value > alpha)
		{
			child_pv.clear();
			value = -search(next_position, -beta, -alpha, depth - 1, plies_to_root + 1, child_pv);
		}

		// Undo
		key_history.pop_back();

		// Check if we have a new best value
		if (value > alpha)
		{
			alpha = value;
			best_move = move;
			bound = Bound::Exact;

			// Update our principal variation by pushing the best move
			// for this node and copying the child PV onto the end
			pv.clear();
			pv.push_back(best_move);
			pv.insert(pv.end(), child_pv.begin(), child_pv.end());

			// Update history heuristic
			if (plies_to_root <= 8 && !is_capture && !is_promotion)
			{
				// Update history heuristic
				heuristics.history.update(depth * depth, moved_piece, move.to());
			}

			// Check for beta cutoff
			if (alpha >= beta)
			{
				bound = Bound::Lower;

				// Update heuristics for quiet moves
				if (!is_capture && !is_promotion)
					heuristics.killer.update(depth, move);

				// Save to transposition table
				tt.save(key, depth, plies_to_root, beta, bound, best_move);

				// Fail-hard beta-cutoff
				return beta;
			}
		}
		else
		{
			// Update history heuristic
			if (plies_to_root <= 8 && !is_capture && !is_promotion)
			{
				// Update history heuristic
				heuristics.history.update(-depth, moved_piece, move.to());
			}
		}
	}

	// Save to transposition table
	tt.save(key, depth, plies_to_root, alpha, bound, best_move);

	return alpha;
}

/**
 * @brief Quiescence search, needed to stabilise evaluation
 * 
 * @tparam IsPV true if this node is a PV node, false otherwise
 * @param alpha 
 * @param beta 
 * @param plies_to_root 
 * @param pv 
 * @return Value 
 */
Value Thread::qsearch(const Position &position, Value alpha, Value beta,
					  const Depth plies_to_root, MoveSequence &pv)
{
	const Nodes total_nodes_searched = nodes_searched() + qnodes_searched();

	// Check time (main thread only)
	if (is_main_thread() && total_nodes_searched % CheckTimeEvery == 0)
		static_cast<MainThread *>(this)->check_time_fast();

	// Check for stop signal or if we've reached the node limit
	if (should_stop() || (limits.nodes && total_nodes_searched >= limits.nodes))
	{
		// If we are in check, the position is probably dangerous. Return draw value instead.
		return position.checkers() ? Draw : eval::evaluate(position, pawn_cache.get());
	}

	// Check for draw by fifty moves / threefold repetition
	if (   position.is_draw_by_rule50()
		|| std::count(key_history.begin(), key_history.end(), position.key()) >= 3)
		return Draw;

	// Update selective depth
	sel_depth = util::max(sel_depth, plies_to_root);

	// Generate all legal moves
	MoveList move_list {position};

	// Check for checkmate or stalemate
	if (move_list.size() == 0)
		return position.checkers() ? mated_in(plies_to_root) : Draw; 

	// "stand pat" evaluation
	if (!position.checkers())
	{
		const Value stand_pat = eval::evaluate(position, pawn_cache.get());

		if (stand_pat >= beta)
			return beta;

		alpha = util::max(alpha, stand_pat);
	}

	// Move ordering
	evaluate_move_list(position, move_list);

	Bound bound = Bound::Upper;
	Value value;
	Move best_move;
	MoveSequence child_pv;

	// Search each move
	for (unsigned move_number = 0; move_number < move_list.size(); ++move_number)
	{
		const Move move = move_list.select();

		const bool is_capture = position.is_capture(move);
		const bool is_promotion = move.is_promotion();
#if defined(CRAZYHOUSE)
		const bool is_drop = move.is_drop();
#endif

		// Search captures, promotions, drops only if not in check
		if (!position.checkers())
		{
#if defined(CRAZYHOUSE)
			if (!is_capture && !is_promotion && !is_drop)
				continue;
#else
			if (!is_capture && !is_promotion)
				continue;
#endif
		}

		// Make a copy, apply the move, store new key in key stack, and increment nodes counter
		Position next_position {position};
		next_position.do_move(move);
		key_history.push_back(next_position.key());
		qnodes.fetch_add(1, std::memory_order_relaxed);

		child_pv.clear();
		value = -qsearch(next_position, -beta, -alpha, plies_to_root + 1, child_pv);

		// Undo
		key_history.pop_back();

		// Check if we have a new best value
		if (value > alpha)
		{
			alpha = value;
			best_move = move;
			bound = Bound::Exact;

			// Update our principal variation by pushing the best move
			// for this node and copying the child PV onto the end
			pv.clear();
			pv.push_back(best_move);
			pv.insert(pv.end(), child_pv.begin(), child_pv.end());

			// Check for beta cutoff
			if (alpha >= beta)
			{
				bound = Bound::Lower;

				// Fail-hard beta-cutoff
				return beta;
			}
		}
	}

	return alpha;
}

/**
 * @brief Iterative deepening loop.
 * Calls search() repeatedly with increasing depth until times up or stop_thinking() is called.
 */
void Thread::think()
{
	clear();

	Value alpha = -Infinite, beta = Infinite;
	Value value = -Infinite;
	MoveSequence pv;

	// Iterative deepening loop
	for (id_depth = 1; (!limits.depth || id_depth <= limits.depth) || limits.infinite; ++id_depth)
	{
		sel_depth = 0;

		if (id_depth > 1)
		{
			alpha = util::max(value - AspirationWindowHalfWidth, -Infinite);
			beta  = util::min(value + AspirationWindowHalfWidth,  Infinite);
		}

		// Aspiration loop
		while (!should_stop())
		{

			pv.clear();
			value = search(root_position, alpha, beta, id_depth, 0, pv);

			// Fail-low
			if (value <= alpha)
				alpha = util::max(value - AspirationWindowHalfWidth, -Infinite);
			// Fail-high
			else if (value >= beta)
				beta = util::min(value + AspirationWindowHalfWidth, Infinite);
			else
				break;
		}

		// If search was stopped prematurely, don't update the root PV / value / depth.
		if (!should_stop())
		{
			root_pv = pv;
			root_value = value;

			uci::message(
				"info depth {:d} seldepth {:d} thread {} score {} pv {}",
				id_depth, sel_depth, id(), uci::format_value(root_value),
				uci::format_variation(root_pv)
			);

#if !defined(NDEBUG)
			uci::message(
				"info depth {:d} thread {} qt {} pawnhitrate {}",
				id_depth, id(), (100 * qnodes) / (nodes + qnodes),
				pawn_cache->hit_rate()
			);
#endif

			if (is_main_thread())
			{
				MainThread *main_thread = static_cast<MainThread *>(this);

				main_thread->post_statistics();
				main_thread->check_time_slow();
			}
		}
		else
		{
			--id_depth;
			break;
		}
	}
}

/**
 * @brief Main thread. The search starts here.
 */
void MainThread::think()
{
	// Check for checkmate/stalemate before doing anything else
	MoveList root_moves {root_position};
	if (root_moves.size() == 0)
	{
		const bool checkmate = root_position.checkers();
		uci::message("info depth 0 score {}", uci::format_value(checkmate ? Mated : Draw));
		uci::message("bestmove {}", uci::format_move({}));
		return;
	}
	// If we are playing against an opponent and there is only one legal move,
	// skip searching and report that move as the best.
	else if (root_moves.size() == 1 && !limits.tc.is_nonzero())
	{
		uci::message("info depth 0 score {}", uci::format_value(Draw));
		uci::message("bestmove {}", uci::format_move(*root_moves.begin()));
		return;
	}

	// todo: this isn't actually implemented yet, an always-replace strategy is used
	// Increment transposition table epoch, so old entries are immediately overwritten
	tt.increment_epoch();

	// Set search start time
	times_up = false;
	t0 = t1 = high_resolution_clock::now();

	// Start helper threads
	for (auto &thread : helpers)
		thread->start_thinking(limits);

	// Enter iterative deepening loop
	Thread::think();

	// Stop helper threads if time's up
	if (times_up)
	{
		for (auto &thread : helpers)
			thread->stop_thinking();
	}
	// If search is infinite, wait until GUI sends stop command
	else if (limits.infinite)
	{
		while (!should_stop()) {};

		for (auto &thread : helpers)
			thread->stop_thinking();
	}

	// Wait for helper threads to finish
	for (auto &thread : helpers)
		thread->wait_until_idle();

	// Determine best thread
	Thread *best_thread = this;

	for (auto &thread : helpers)
		if (thread->depth_reached() > best_thread->depth_reached())
			best_thread = thread.get();

	MoveSequence pv = best_thread->principal_variation();
	Value value = best_thread->best_value();
	Depth depth = best_thread->depth_reached();

	if (pv.empty())
		pv.emplace_back(); // Send null move

	uci::message(
		"info depth {:d} thread {} score {} pv {}",
		depth, best_thread->id(), uci::format_value(value), uci::format_variation(pv)
	);

	if (pv.size() >= 2)
		uci::message("bestmove {} ponder {}", uci::format_move(pv[0]), uci::format_move(pv[1]));
	else
		uci::message("bestmove {}", uci::format_move(pv[0]));
}

/**
 * @brief Called inside search() and qsearch() every so often. Calls stop_thinking() if time's up.
 */
void MainThread::check_time_fast()
{
	const Colour us = root_position.side_to_move();
	const milliseconds elapsed = total_search_time();
	const milliseconds our_time = limits.tc.time(us);

	// Check movetime
	if (limits.tc.movetime != limits.tc.movetime.zero() &&
		elapsed >= (limits.tc.movetime - Overhead))
	{
		times_up = true;
		stop_thinking();
	}

	// Check wtime/btime
	if (our_time != our_time.zero() && elapsed > (our_time - Overhead) / 10)
	{
		times_up = true;
		stop_thinking();
	}
}

/**
 * @brief Called after each search iteration. Calls stop_thinking() if time's up.
 */
void MainThread::check_time_slow()
{
	check_time_fast();

	t1 = high_resolution_clock::now();
}

void MainThread::post_statistics()
{
	const milliseconds time = total_search_time();
	const Nodes total_nodes = total_nodes_searched();
	const int nps = (1000 * total_nodes) / (time.count() + 1);

	uci::message(
		"info nodes {} time {} nps {} hashfull {} hitrate {}",
		total_nodes, time.count(), nps, tt.hashfull_approx(), tt.hit_rate()
	);
}

/**
 * @brief Sets the position about to be searched
 * 
 * @param root_position 
 */
void Thread::initialise(const Position &new_root_position, const KeyHistory &new_key_history)
{
	root_position = new_root_position;
	key_history = new_key_history;

	clear();
}

/**
 * @brief Wakes up this thread and starts searching with the given limits @param search_limits
 * 
 * @param search_limits 
 */
void Thread::start_thinking(const Limits &search_limits)
{
	limits = search_limits;
	start_thinking();
}

/**
 * @brief Reset all counters/statistics/results
 */
void Thread::clear()
{
	id_depth = sel_depth = 0;
	nodes = qnodes = 0;
	heuristics.clear();
	root_pv.clear();
	root_value = -Infinite;
}

MainThread::MainThread()
	: Thread(0), helpers(), t0(), t1(), times_up(false)
{
}

/**
 * @brief Same as Thread::initialise() but also initialises helper threads
 * 
 * @param root_position 
 */
void MainThread::initialise(const Position &root_position, const KeyHistory &key_history)
{
	for (auto &thread : helpers)
		thread->initialise(root_position, key_history);

	Thread::initialise(root_position, key_history);
}

/**
 * @brief Same as Thread::clear() but also clears helper threads
 */
void MainThread::clear()
{
	for (auto &thread : helpers)
		thread->clear();

	Thread::clear();
}

/**
 * @brief Adjust the number of helper threads to use
 * 
 * @param n Number of helper threads
 */
void MainThread::resize_helpers(std::size_t n)
{
	while (helpers.size() > n)
		helpers.pop_back();

	while (helpers.size() < n)
		helpers.push_back(std::make_unique<Thread>(helpers.size() + 1));
}

milliseconds MainThread::total_search_time() const
{
	return duration_cast<milliseconds>(high_resolution_clock::now() - t0);
}

milliseconds MainThread::iteration_time() const
{
	return duration_cast<milliseconds>(high_resolution_clock::now() - t1);
}

Nodes MainThread::total_nodes_searched() const
{
	Nodes nodes = nodes_searched() + qnodes_searched();

	for (auto &thread : helpers)
		nodes += thread->nodes_searched() + thread->qnodes_searched();

	return nodes;
}
