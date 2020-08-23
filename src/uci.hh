#pragma once

#include "types.hh"
#include "ucioption.hh"

#include <iostream>

namespace chess::uci
{
	constexpr std::string_view Name = "Mink";
	constexpr unsigned         Version = 1;

	extern Options options;

	extern std::string format_value(const Value value);
	extern std::string format_move(const Move move);
	extern std::string format_variation(const MoveSequence &moves);
	extern Move parse_move(std::string_view move);

	template <typename ...Args>
	inline void message(std::string_view f, Args &&...args)
	{
		fmt::print("{}\n", fmt::vformat(f, fmt::make_format_args(args...)));
#if defined(_WIN32)
		// On Windows, outside of a terminal, newlines don't flush the buffer.
		std::cout << std::flush;
#endif
	}

	extern void init();
	extern int main(int argc, char *argv[]);
} // chess::uci
