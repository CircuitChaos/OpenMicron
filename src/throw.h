/**
 * \brief	Exception throwing
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#define xthrow(...) doThrow(__FILE__, __LINE__, __VA_ARGS__)
#define xassert(e, ...) do { if (!(e)) xthrow(__VA_ARGS__); } while (0)

#ifdef __GNUC__
#define THROW_ATTR __attribute__((noreturn, format(printf, 3, 4)))
#else
#define THROW_ATTR
#endif
void doThrow(const char *file, int line, const char *fmt, ...) THROW_ATTR;
#undef THROW_ATTR
