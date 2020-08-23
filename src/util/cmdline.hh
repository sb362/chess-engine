#pragma once

#include <algorithm>
#include <string_view>

namespace util
{
	inline bool option_exists(int argc, char *argv[], std::string_view option)
	{
		return std::find(argv, argv + argc, option) != (argv + argc);
	}

	inline std::string_view option_value(int argc, char *argv[], std::string_view option)
	{
		return *std::find(argv, argv + argc, option);
	}
}
