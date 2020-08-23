#include "perft.hh"
#include "uci.hh"

#include <chrono>

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using time_point = high_resolution_clock::time_point;

chess::Nodes chess::perft(const Position &position, const Depth depth)
{
	MoveList move_list {position};

	if (depth == 1)
		return move_list.size();

	Nodes nodes = 0;

	for (const Move move : move_list)
	{
		Position next_position {position};
		next_position.do_move(move);
		nodes += perft(next_position, depth - 1);
	}

	return nodes;
}

chess::Nodes chess::divide(const Position &position, const Depth depth)
{
	MoveList move_list {position};

	if (depth == 0)
		return 1;

	Nodes nodes = 0, count;

	for (const Move move : move_list)
	{
		Position next_position {position};
		next_position.do_move(move);

		count = depth == 1 ? 1 : perft(next_position, depth - 1);
		nodes += count;

		fmt::print("{}: {}\n", uci::format_move(move), count);
	}

	return nodes;
}

int chess::perft(int argc, char *argv[])
{
	if (argc < 3)
	{
		fmt::print("Usage: {} [perft | divide] [depth] [fen string = startpos]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	int status = 0;
	unsigned depth = 0;
	std::vector<std::string> parts {argv, argv + argc};
	std::string fen;

	// Parse FEN
	for (int i = 3; i < argc; ++i)
		fen += parts[i] + ' ';

	fen.empty() ? (void)(fen = fens::Startpos) : fen.pop_back();

	// todo: move into lookup table
	if (fen == "startpos")
		fen = fens::Startpos;
	else if (fen == "kiwipete")
		fen = fens::Kiwipete;

	// Parse depth
	try
	{
		depth = std::stoul(parts[2]);
	}
	catch (const std::invalid_argument &e)
	{
		fmt::print("Failed to parse depth (invalid argument): {}\n", e.what());
		status = EXIT_FAILURE;
	}
	catch (const std::out_of_range &e)
	{
		fmt::print("Failed to parse depth (out of range): {}\n", e.what());
		status = EXIT_FAILURE;
	}
	catch (const std::exception &e)
	{
		fmt::print("Error: {}\n", e.what());
		status = EXIT_FAILURE;
	}

	const Position position {fen};
	fmt::print("{}\n", position.to_string());

	const time_point t0   = high_resolution_clock::now();
	const Nodes nodes     = parts[1] == "divide" ? divide(position, depth) : perft(position, depth);
	const time_point t1   = high_resolution_clock::now();
	const microseconds dt = duration_cast<microseconds>(t1 - t0);

	fmt::print(
		"nodes:      {}\nknodes/sec: {:.0f}\ntime taken: {} ms\n",
		nodes, (1e3 * nodes) / (dt.count() + 1), duration_cast<milliseconds>(dt).count()
	);

	return status;
}
