/*
 * \brief	Logging
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <cstdarg>
#include <cstdio>
#include "log.h"

static log::ELevel g_minLevel = log::LL_NORM;

static const char *levelToString(log::ELevel level)
{
	switch (level)
	{
		case log::LL_DBG:
			return "dbg";

		case log::LL_INFO:
			return "info";

		case log::LL_NORM:
			return "norm";

		case log::LL_ERR:
			return "err";

		default:
			break;
	}

	// we shouldn't throw here as it might be called
	// in exception handler
	return "?";
}

void log::setLevel(ELevel minLevel)
{
	g_minLevel = minLevel;
	logd("Log level set to %s", levelToString(minLevel));
}

void log::doLog(ELevel level, const char *file, int line, const char *fmt, ...)
{
	if (level < g_minLevel)
		return;

	printf("%s:%d: [%s] ", file, line, levelToString(level));

	va_list ap;
	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);

	printf("\n");
}
