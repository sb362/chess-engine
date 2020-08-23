#include "search.hh"
#include "tt.hh"
#include "uci.hh"

#include <sstream>

using namespace chess;
using namespace chess::uci;

Options uci::options;

/**
 * @brief Format value to UCI notation
 * 
 * @param value 
 * @return std::string 
 */
std::string uci::format_value(const Value value)
{
	return is_mate(value)
		 ? fmt::format("mate {}", ((depth_to_mate(value) + 1) / 2) * util::sgn(value))
		 : fmt::format("cp {}", value);
}

/**
 * @brief Format a move to UCI notation
 * 
 * @param move 
 * @return std::string 
 */
std::string uci::format_move(const Move move)
{
	std::string s;

	if (!move.is_valid())
		return "0000";

#if defined(CRAZYHOUSE)
	if (move.is_drop())
		s += fmt::format("{}@{}", to_char(move.drop(), true), move.to());
	else
#endif
	if (move.is_promotion())
		s += fmt::format("{}{}{}", move.from(), move.to(), move.promotion());
	else
		s += fmt::format("{}{}", move.from(), move.to());

	return s;
}

/**
 * @brief Format a sequence of moves to UCI notation
 * 
 * @param moves 
 * @return std::string 
 */
std::string uci::format_variation(const MoveSequence &moves)
{
	std::string s;
	for (const Move &move : moves)
		s += format_move(move) + ' ';

	if (!s.empty())
		s.pop_back();

	return s;
}

/**
 * @brief Parses a move from UCI notation. Assumes @param s is well-formed.
 * 
 * @param s 
 * @return Move 
 */
Move uci::parse_move(std::string_view s)
{
	if (s == "0000" || (s.size() != 4 && s.size() != 5))
		return {}; // Null/invalid move

#if defined(CRAZYHOUSE)
	if (const std::size_t pos = PieceTypeCharsUpper.find(s[0]); pos != std::string::npos);
	{
		const PieceType drop = static_cast<PieceType>(pos);
		const Square to = parse_square(s.substr(2, 2));

		return {to, drop};
	}
	else
#endif
	{
		const Square from = parse_square(s.substr(0, 2));
		if (!is_valid(from))
			return {}; // Null/invalid move

		const Square to = parse_square(s.substr(2, 2));
		PieceType promotion = PieceType::Invalid;

		if (s.size() == 5)
			if (const std::size_t pos = PieceTypeChars.find(s[4]); pos != std::string::npos)
				promotion = static_cast<PieceType>(pos);
		
		return {from, to, promotion};
	}
}

int uci::main(int, char *[])
{
	message("id name {} {}", Name, Version);

	// Initialise options map
	options.add<SpinOption>("Threads", 1, 1, threading::max_threads());
	options.add<SpinOption>("Hash", TranspositionTable::DefaultSize / 1024 / 1024,
							1, 16384, "Transposition table size in MiB");
	
#if defined(CRAZYHOUSE)
	options.add<ComboOption>("UCI_Variant", "standard", {"standard", "crazyhouse"});
#endif

	// Main search thread
	search::MainThread main_thread;

	options.listen("Threads",
		[&] (const Option *option, const std::string &old_threads, const std::string &new_threads)
		{
			uci::message("info string Resizing thread pool from {} to {}...",
						 old_threads, new_threads);
			main_thread.resize_helpers(static_cast<const SpinOption *>(option)->value());
			uci::message("info string Resized thread pool");
		}
	);

	options.listen("Hash",
		[&] (const Option *option, const std::string &old_size, const std::string &new_size)
		{
			uci::message("info string Resizing transposition table from {} MiB to {} MiB...",
						 old_size, new_size);
			tt.resize(static_cast<const SpinOption *>(option)->value() * 1024ull * 1024);
			uci::message("info string Resized transposition table");
		}
	);

	fmt::print("{}", options.to_string());
	message("uciok");

	std::string line;
	while (std::getline(std::cin, line))
	{
		std::string cmd, token;
		std::istringstream iss {line};
		iss >> cmd;

		if (cmd == "isready")
			message("readyok");
		else if (cmd == "setoption")
		{
			std::string name, value;
			iss >> token; // 'name'

			iss >> name;
			while (iss >> token && token != "value")
				name += ' ' + token;
			
			iss >> value;
			while (iss >> token)
				value += ' ' + token;
			
			const bool was_idle = main_thread.is_idle();
			if (!was_idle)
			{
				main_thread.stop_thinking();
				main_thread.wait_until_idle();
			}

			if (int result = options.set(name, value); result != 0)
				message(
					"info string Failed to set option '{}' to '{}' ({})",
					name, value, result == 1 ? "option not found" : "invalid value"
				);
			
			if (!was_idle)
				main_thread.start_thinking();
		}
		else if (cmd == "ucinewgame")
		{
			main_thread.stop_thinking();
			main_thread.wait_until_idle();

			tt.clear();
		}
		else if (cmd == "position")
		{	
			bool bad = false;
			Position position;
			search::KeyHistory key_history;

			iss >> token;
			if (token == "startpos")
			{
				position.set_fen(fens::Startpos);
				iss >> token; // 'moves'
			}
			else if (token == "fen")
			{
				std::string fen;
				iss >> fen;

				while (iss >> token && token != "moves")
					fen += ' ' + token;
				
				position.set_fen(fen);
			}
			else
			{
				bad = true;
				message("info string Unrecognised parameter '{}'", token);
			}

			key_history.push_back(position.key());

			while (!bad && iss >> token)
			{
				const Move move = uci::parse_move(token);

				if (!move.is_valid())
				{
					bad = true;
					message("info string Invalid move '{}'", token);
					continue;
				}

				position.do_move(move);
				key_history.push_back(position.key());
			}

			if (!bad)
			{
				main_thread.stop_thinking();
				main_thread.wait_until_idle();
				main_thread.initialise(position, key_history);
			}
		}
		else if (cmd == "go")
		{
			search::Limits limits;

			while (iss >> token)
			{
				if (token == "ponder") limits.ponder = true;
				else if (token == "wtime")
				{
					int wtime;
					iss >> wtime;
					limits.tc.wtime = search::milliseconds {wtime};
				}
				else if (token == "btime")
				{
					int btime;
					iss >> btime;
					limits.tc.btime = search::milliseconds {btime};
				}
				else if (token == "winc")
				{
					int winc;
					iss >> winc;
					limits.tc.winc = search::milliseconds {winc};
				}
				else if (token == "binc")
				{
					int binc;
					iss >> binc;
					limits.tc.binc = search::milliseconds {binc};
				}
				else if (token == "movestogo")
				{
					int movestogo;
					iss >> movestogo;
					limits.tc.movestogo = movestogo;
				}
				else if (token == "depth")
				{
					int depth;
					iss >> depth;
					limits.depth = depth;
				}
				else if (token == "nodes") iss >> limits.nodes;
				else if (token == "mate")
				{
					int mate;
					iss >> mate;
					limits.mate = mate;
				}
				else if (token == "movetime")
				{
					int movetime;
					iss >> movetime;
					limits.tc.movetime = search::milliseconds {movetime};
				}
				else if (token == "infinite") limits.infinite = true;
				else message("info string Unrecognised parameter '{}'", token);
			}

			main_thread.stop_thinking();
			main_thread.wait_until_idle();
			main_thread.start_thinking(limits);
		}
		else if (cmd == "ponderhit")
		{

		}
		else if (cmd == "stop")
			main_thread.stop_thinking();
		else if (cmd == "quit")
			break;
		else
			message("info string Unknown command");
	}

	main_thread.stop_thinking();
	main_thread.wait_until_idle();

	return 0;
}
