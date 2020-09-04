#include <chrono>
#include <string>
#include <vector>

#define CATCH_CONFIG_RUNNER
#include "catch2/catch.hpp"

#include "perft.hh"

using namespace chess;

using std::chrono::duration_cast;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::high_resolution_clock;
using time_point = high_resolution_clock::time_point;

struct PerftData
{
	std::string name, fen;
	std::vector<Nodes> counts;
	Depth depth;
};

static PerftData perft_data[]
{
	//
	// Source: https://www.chessprogramming.org/Perft_Results
	//
	{
		"Startpos",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -",
		{20, 400, 8902, 197281, 4865609, 119060324, 3195901860, 84998978956}, 6
	},
	{
		"Kiwipete",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
		{48, 2039, 97862, 4085603, 193690690, 8031647685}, 5
	},
	{
		"CPW #3",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
		{14, 191, 2812, 43238, 674624, 11030083, 178633661, 3009794393}, 7
	},
	{
		"CPW #4",
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -",
		{6, 264, 9467, 422333, 15833292, 706045033}, 6
	},
	{
		"CPW #4, mirrored",
		"r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ -",
		{6, 264, 9467, 422333, 15833292, 706045033}, 6
	},
	{
		"CPW #5",
		"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ -",
		{44, 1486, 62379, 2103487, 89941194}, 5
	},
	{
		"CPW #6",
		"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - -",
		{46, 2079, 89890, 3894594, 164075551, 6923051137, 287188994746}, 5
	},
	
	//
	// Source: http://www.rocechess.ch/perft.html
	//
	{
		"Promotions",
		"n1n5/PPPk4/8/8/8/8/4Kppp/5N1N b - -",
		{24, 496, 9483, 182383, 3605103, 71179139}, 6
	},

	//
	// Crazyhouse
	//
#if defined(CRAZYHOUSE)
	{
		"Startpos (CH)",
		"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR/ w KQkq -",
		{20, 400, 8902, 197281, 4888832, 120812942, }, 6
	},
	{
		"Kiwipete (CH)",
		"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R/ w KQkq -",
		{48, 2039, 106456, 4916262, 271252235, 13608599763}, 5
	},
	{
		"CPW #3 (CH)",
		"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8/ w - -",
		{14, 191, 2890, 47686, 886796, 16503315, 345665347}, 6
	},
	{
		"CPW #4 (CH)",
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1/ w kq -",
		{6, 264, 9467, 509696, 20499292, 12026113899}, 5
	},
	{
		"CPW #5 (CH)",
		"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R/ w KQ -",
		{44, 1486, 69110, 2658139, 133038868, 5557316247}, 6
	},
	{
		"Promotions (CH)",
		"n1n5/PPPk4/8/8/8/8/4Kppp/5N1N/ b - -",
		{24, 496, 18107, 642161, 21453773, 703022093}, 6
	},
	{
		"CPW #4 (CH, PNnnq)",
		"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1/PNnnq w kq -",
		{14, 1520, 108958, 11252119, 709447833}, 5
	}
#endif
};

TEST_CASE("Perft", "[perft]")
{
	using clock = std::chrono::high_resolution_clock;
	using std::chrono::milliseconds;
	using std::chrono::microseconds;
	using std::chrono::duration_cast;

	fmt::print("{: <18} {: <6} {: <12} {: <20} {: <16}\n",
				"Name", "Depth", "Nodes", "Time to depth (ms)", "Performance (kn/s)");

	for (const PerftData &data : perft_data)
	{
		Position position {data.fen};

		time_point t0 = clock::now();
		Nodes nodes = perft(position, data.depth);
		time_point t1 = clock::now();
		microseconds dt = duration_cast<microseconds>(t1 - t0);

		fmt::print("{: <18} {: <6} {: <12} {: <20} {: <16}\n",
			data.name, data.depth, nodes,
			duration_cast<milliseconds>(dt).count(),
			int((1e3 * nodes) / dt.count())
		);

		REQUIRE(data.counts[data.depth - 1] == nodes);
	}
}

int main(int argc, char *argv[])
{
	bitboards::init();
	magics::init();

	Catch::Session session;

	int status = session.applyCommandLine(argc, argv);
	if (status == 0)
		status = session.run();

	return status;
}
