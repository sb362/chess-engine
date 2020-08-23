#pragma once

#include "fmt/format.h"

#include <string>

namespace util
{

inline std::string compiler_info()
{
#if defined(__clang__)
	return fmt::format("Clang {}.{}.{}", __clang_major__, __clang_minor__, __clang_patchlevel__);
#elif defined(__INTEL_COMPILER)
	return fmt::format("ICC {}.{}", __INTEL_COMPILER, __INTEL_COMPILER_UPDATE);
#elif defined(__GNUC__)
	return fmt::format("GCC {}.{}.{}", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
#elif defined(_MSC_VER)
	return fmt::format("MSVC {}", _MSC_VER);
#elif defined(__EMSCRIPTEN__)
	return "Emscripten"
#else
	return "Unknown";
#endif
}

inline std::string os_info()
{
#if defined(__asmjs__)
	return "AsmJS"
#elif defined(__ANDROID__)
	return "Android"
#elif defined(__APPLE__)
	return "Apple"
#elif defined(__MINGW64__)
	return "64-bit MinGW";
#elif defined(__MINGW32__)
	return "32-bit MinGW";
#elif defined(__CYGWIN__)
	return "Cygwin";
#elif defined(_WIN64)
	return "64-bit Windows";
#elif defined(_WIN32)
	return "32-bit Windows";
#elif defined(__linux__)
	return "Linux";
#elif defined(__unix__)
	return "Unix";
#else
	return "Unknown";
#endif
}

inline std::string build_time()
{
	return fmt::format("{} {}", __DATE__, __TIME__);
}

inline std::string intrinsics_info()
{
	std::string out;

#if defined(USE_BMI2)
	out += "BMI2 ";
#endif

#if defined(USE_LSB)
	out += "LSB ";
#endif

#if defined(USE_POPCNT)
	out += "POPCNT ";
#endif

	if (!out.empty())
		out.pop_back();
	else
		out = "None";

	return out;
}

inline std::string attack_generation_info()
{
#if defined(USE_KOGGE_STONE)
	return "Kogge-Stone";
#elif defined(USE_FANCY)
	return "Fancy magic bitboards";
#elif defined(USE_PEXT)
	return "PEXT bitboards";
#elif defined(USE_PDEP)
	return "PEXT+PDEP bitboards";
#else
	return "Unknown";
#endif
}

} // util
