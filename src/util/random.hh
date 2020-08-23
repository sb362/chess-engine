#pragma once

#include <cstdint>

namespace util
{
	struct PRNG
	{
		using T = std::uint64_t;

		T a, b, c, d;

		constexpr PRNG(T a, T b, T c, T d)
			: a(a), b(b), c(c), d(d)
		{
		}

		constexpr T rand()
		{
			a *= 2688792669u;
			a += 180014855u;
			b ^= b << 5u;
			b ^= b >> 7u;
			b ^= b << 29u;

			T e = 1823811948u * c + d;
			d = e >> 32u;
			c = e;

			return a + b + c;
		}
	};
}

