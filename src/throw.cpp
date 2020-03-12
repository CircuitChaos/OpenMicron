/**
 * \brief	Exception throwing
 * \author	Adam Wysocki
 * \date	2018-01-18
 */

#include <string>
#include <stdexcept>
#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include <cstdarg>
#include "throw.h"

void doThrow(const char *file, int line, const char *fmt, ...)
{
	char buf[128];
	snprintf(buf, sizeof(buf), "%s:%d: ", file, line);

	va_list ap;
	va_start(ap, fmt);

	char *p;
	if (vasprintf(&p, fmt, ap) == -1)
		throw std::runtime_error("Allocation error during exception throwing");

	va_end(ap);

	std::string s(std::string(buf) + p);
	free(p);

	throw std::runtime_error(s);
}
