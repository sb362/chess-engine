#pragma once

#include "fmt/format.h"
#include "fmt/compile.h"

#include <cstdint>
#include <string_view>

#if defined(__PRETTY_FUNCTION__)
#	define CUR_FUNC __PRETTY_FUNCTION__
#elif defined(__FUNCSIG__)
#	define CUR_FUNC __FUNCSIG__
#else
#	define CUR_FUNC __FUNCTION__
#endif

#define CUR_FILE __FILE__
#define CUR_LINE __LINE__

namespace util
{
	inline int assertion_failed(std::string_view func, std::string_view file,
								std::size_t line, std::string_view cond)
	{
		fmt::print(stderr, "{}:{} {}: expected '{}'\n", file, line, func, cond);
		std::terminate();
		return 0;
	}
}

#if defined(NDEBUG)
#	define ASSERT(cond) (void)(0)
#else
#	define ASSERT(cond) (void)(!(cond) ? util::assertion_failed(CUR_FUNC, CUR_FILE, CUR_LINE, #cond) : 0)
#endif
