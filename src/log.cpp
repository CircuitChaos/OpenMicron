/*
 * \brief	Logging
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <string>
#include <cstdarg>
#include <cstdio>
#include "log.h"
#include "config.h"
#include "util.h"

static xlog::ELevel g_minLevel = xlog::LL_NORM;

static const char *levelToString(xlog::ELevel level)
{
	switch (level)
	{
		case xlog::LL_DBG:
			return "dbg";

		case xlog::LL_INFO:
			return "info";

		case xlog::LL_NORM:
			return "norm";

		case xlog::LL_ERR:
			return "err";

		default:
			break;
	}

	// we shouldn't throw here as it might be called
	// in exception handler
	return "?";
}

void xlog::setLevel(ELevel minLevel)
{
	g_minLevel = minLevel;
	logd("Log level set to %s", levelToString(minLevel));
}

void xlog::doLog(ELevel level, const char *file, int line, const char *fmt, ...)
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

void xlog::doDump(ELevel level, const char *file, int line, const char *prefix, const void *data, size_t sz)
{
	if (!sz)
	{
		doLog(level, file, line, "0000 %s | <empty>", prefix);
		return;
	}

	const unsigned char *c((const unsigned char *) data);
	std::string s;

	for (unsigned i(0); i < sz; i += config::HEXDUMP_ROWS)
	{
		s = util::format("%s %04x | ", prefix, i);

		unsigned l(sz - i);
		if (l > config::HEXDUMP_ROWS)
			l = config::HEXDUMP_ROWS;

		for (unsigned j(0); j < l; ++j)
			s += util::format("%02x ", c[i + j]);

		if (l < config::HEXDUMP_ROWS)
			s += std::string((config::HEXDUMP_ROWS - l) * 3, ' ');

		s += "| ";

		for (unsigned j(0); j < l; ++j)
		{
			unsigned char ch(c[i + j]);
			if (ch < 0x20 || ch >= 0x7F)
				ch = '.';

			s.push_back(ch);
		}

		doLog(level, file, line, "%s", s.c_str());
	}
}
