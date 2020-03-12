/**
 * \brief	Utility functions
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <cstdarg>
#include <cstdlib>
#include <cstdio>
#include "util.h"
#include "throw.h"

std::string util::format(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	char *p;
	if (vasprintf(&p, fmt, ap) == -1)
		xthrow("Memory allocation error");

	va_end(ap);

	std::string s(p);
	free(p);

	return s;
}
