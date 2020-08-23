#pragma once

#include <type_traits>

namespace util
{
	template <typename Enum> using EnumsOnly = std::enable_if_t<std::is_enum_v<Enum>>;

	/**
	 * @brief Casts an enum to its underlying representation
	 * 
	 * @tparam Enum Enum type
	 * @param e Enum value
	 * @return Underlying representation of enum value @param e
	 */
	template <typename Enum, typename = EnumsOnly<Enum>>
	constexpr auto underlying_value(const Enum e)
	{
		return static_cast<std::underlying_type_t<Enum>>(e);
	}
}
