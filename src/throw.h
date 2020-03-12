/**
 * \brief	Exception throwing
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#define xthrow(...) doThrow(__FILE__, __LINE__, __VA_ARGS__)
#define xassert(e, ...) do { if (!(e)) xthrow(__VA_ARGS__); } while (0)

void doThrow(const char *file, int line, const char *fmt, ...) __attribute__((noreturn, format(printf, 3, 4)));
