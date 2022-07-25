/**
 * \brief	Utility functions
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#include <vector>
#include <string>
#include <inttypes.h>

namespace util
{
#ifdef __GNUC__
#define FORMAT_ATTR __attribute__((format(printf, 1, 2)))
#else
#define FORMAT_ATTR
#endif
	std::string format(const char *fmt, ...) FORMAT_ATTR;
#undef FORMAT_ATTR
	uint32_t crc32(uint32_t crc, const void *data, size_t size);
	std::string toPrintable(const std::string &s);
	std::string stripRight(const std::string &s);
	std::vector<std::string> tokenize(const std::string &src, char sep);
}
