#pragma once

#include "array.hh"

namespace util
{
	template <typename T, std::size_t S>
	struct Tuple : util::array_t<T, S>
	{
		constexpr Tuple<T, S> operator+(const Tuple<T, S> &b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] += b[i];

			return out;
		}

		constexpr Tuple<T, S> operator-(const Tuple<T, S> &b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] -= b[i];

			return out;
		}

		constexpr Tuple<T, S> operator*(const Tuple<T, S> &b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] *= b[i];

			return out;
		}

		constexpr Tuple<T, S> operator/(const Tuple<T, S> &b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] /= b[i];

			return out;
		}

		constexpr Tuple<T, S> operator+(const T b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] += b;

			return out;
		}

		constexpr Tuple<T, S> operator-(const T b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] -= b;

			return out;
		}

		constexpr Tuple<T, S> operator*(const T b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] *= b;

			return out;
		}

		constexpr Tuple<T, S> operator/(const T b) const
		{
			Tuple<T, S> out {*this};

			for (unsigned i = 0; i < S; ++i)
				out[i] /= b;

			return out;
		}

		inline Tuple<T, S> &operator+=(const Tuple<T, S> &b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] += b[i];

			return *this;
		}

		inline Tuple<T, S> &operator-=(const Tuple<T, S> &b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] -= b[i];

			return *this;
		}

		inline Tuple<T, S> &operator*=(const Tuple<T, S> &b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] *= b[i];

			return *this;
		}

		inline Tuple<T, S> &operator/=(const Tuple<T, S> &b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] /= b[i];

			return *this;
		}

		inline Tuple<T, S> &operator+=(const T b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] += b;

			return *this;
		}

		inline Tuple<T, S> &operator-=(const T b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] -= b;

			return *this;
		}

		inline Tuple<T, S> &operator*=(const T b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] *= b;

			return *this;
		}

		constexpr inline Tuple<T, S> &operator/=(const T b)
		{
			for (unsigned i = 0; i < S; ++i)
				*this[i] /= b;

			return *this;
		}
	};
}
