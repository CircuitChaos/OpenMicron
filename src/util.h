/**
 * \brief	Utility functions
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#include <string>
#include <inttypes.h>

namespace util
{
	std::string format(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
	uint32_t crc32(uint32_t crc, const void *data, size_t size);
	std::string toPrintable(const std::string &s);
}
