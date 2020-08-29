#pragma once

#include "heuristics.hh"
#include "movegen.hh"
#include "position.hh"
#include "pawns.hh"
#include "types.hh"
#include "uci.hh"

#include "threading/thread.hh"

#include "util/array.hh"
#include "util/hashtable.hh"

#include <atomic>
#include <chrono>

namespace chess::search
{
	using std::chrono::duration_cast;
	using std::chrono::milliseconds;
	using std::chrono::high_resolution_clock;
	using time_point = high_resolution_clock::time_point;

////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(NDEBUG)
	constexpr Nodes CheckTimeEvery = 65536;
#else
	constexpr Nodes CheckTimeEvery = 16384;
#endif

	constexpr Depth LMRDepthLimit = 3;
	constexpr Depth LMRMoveNumber = 3;

	constexpr milliseconds Overhead = milliseconds {50};

////////////////////////////////////////////////////////////////////////////////////////////////////

	/**
	 * @brief Time control
	 */
	struct TimeControl
	{
		milliseconds wtime = {}, winc = {}, btime = {}, binc = {};
		milliseconds movetime = {};
		Depth movestogo = 0;

		constexpr bool is_sudden_death() const
		{
			return movestogo == 0;
		}

		constexpr bool is_nonzero() const
		{
			return wtime != wtime.zero() || winc != winc.zero()
				|| btime != btime.zero() || binc != binc.zero() || movetime != movetime.zero();
		}

		constexpr milliseconds time(const Colour us)
		{
			return us == Colour::White ? wtime : btime;
		}

		constexpr milliseconds inc(const Colour us)
		{
			return us == Colour::White ? winc : binc;
		}
	};

	/**
	* @brief Search parameters
	*/
	struct Limits
	{
		TimeControl tc = {};

		bool ponder = false, infinite = false;
		Depth depth = 0, mate = 0;
		Nodes nodes = 0;
	};

	/**
	 * @brief Stack of zobrist keys used to detect repetitions
	 */
	using KeyHistory = std::vector<Key>;

	/**
	* @brief Search thread
	*/
	class Thread : public threading::Thread
	{
	protected:
		Position root_position;
		KeyHistory key_history;
		Limits limits;

	private:
		Depth id_depth, sel_depth;
		std::atomic<Nodes> nodes, qnodes;

		std::unique_ptr<pawns::Cache> pawn_cache;

		Heuristics heuristics;

		MoveSequence root_pv;
		Value root_value;

	public:
		Thread(std::size_t id);

		Value search(const Position &position, Value alpha, Value beta, const Depth depth,
					 const Depth plies_to_root, MoveSequence &pv);

		Value qsearch(const Position &position, Value alpha, Value beta,
					  const Depth plies_to_root, MoveSequence &pv);

		void think() override;

		void initialise(const Position &root_position, const KeyHistory &key_history);

		void start_thinking(const Limits &limits);
		using threading::Thread::start_thinking;

		void clear();

		bool is_main_thread() const { return id() == 0; }

		Nodes nodes_searched() const { return nodes.load(std::memory_order_relaxed); }
		Nodes qnodes_searched() const { return qnodes.load(std::memory_order_relaxed); }
		Depth depth_reached() const { return id_depth; }
		const MoveSequence &principal_variation() const { return root_pv; }
		Value best_value() const { return root_value; }
	};

	class MainThread : public Thread
	{
	private:
		std::vector<std::unique_ptr<Thread>> helpers;

		time_point t0, t1;
		bool times_up;

	public:
		MainThread();

		void think() override;

		void check_time_fast();
		void check_time_slow();

		void post_statistics();

		void initialise(const Position &root_position, const KeyHistory &key_history);
		void clear();

		void resize_helpers(std::size_t n);

		milliseconds total_search_time() const;
		milliseconds iteration_time() const;

		Nodes total_nodes_searched() const;
	};
} // chess::search
