#pragma once

#include "assert.hh"

#include <type_traits>

namespace util
{
	template <typename S, typename T>
	constexpr std::common_type_t<S, T> min(const S &a, const T &b)
	{
		return a < b ? a : b;
	}

	template <typename S, typename T>
	constexpr std::common_type_t<S, T> max(const S &a, const T &b)
	{
		return a > b ? a : b;
	}

	template <typename S, typename T, typename U>
	constexpr std::common_type_t<S, T, U> clamp(const S &x, const T &lower, const U &upper)
	{
		ASSERT(lower < upper);

		return util::min(util::max(x, lower), upper);
	}

	template <typename T>
	constexpr T sgn(const T &a)
	{
		return (T(0) < a) - (a < T(0));
	}

	template <typename T>
	constexpr std::make_unsigned_t<T> abs(const T &a)
	{
		return a >= T(0) ? a : -a;
	}
}

