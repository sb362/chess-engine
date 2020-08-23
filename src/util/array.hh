#pragma once

#include <array>

//
// Adapted from:
// https://stackoverflow.com/questions/35008089/
//

namespace util
{
	template <typename T, std::size_t S, std::size_t ...Next>
	struct array
	{
		using next_type = typename array<T, Next...>::type;
		using type = std::array<next_type, S>;
	};

	template <typename T, std::size_t S>
	struct array<T, S>
	{
		using type = std::array<T, S>;
	};

	/**
	 * @brief Alias for creating multidimensional arrays
	 */
	template <typename T, std::size_t I, std::size_t ...Next>
	using array_t = typename array<T, I, Next...>::type;
}
