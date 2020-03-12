/**
 * \brief	Utility functions
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#include <string>

namespace util
{
	std::string format(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
}
