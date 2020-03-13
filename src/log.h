/**
 * \brief	Logging
 * \author	Circuit Chaos
 * \date	2020-03-12
 *
 * It logs to stdout only.
 */

#pragma once

#define LOG_GENERIC(level, ...) xlog::doLog(level, __FILE__, __LINE__, __VA_ARGS__)
#define logdump(prefix, data, size) xlog::doDump(xlog::LL_DBG, __FILE__, __LINE__, prefix, data, size)
#define logd(...) LOG_GENERIC(xlog::LL_DBG, __VA_ARGS__)
#define logi(...) LOG_GENERIC(xlog::LL_INFO, __VA_ARGS__)
#define logn(...) LOG_GENERIC(xlog::LL_NORM, __VA_ARGS__)
#define loge(...) LOG_GENERIC(xlog::LL_ERR, __VA_ARGS__)

namespace xlog
{
	// when changing these, change also levelToString() in log.cpp
	enum ELevel
	{
		LL_DBG,
		LL_INFO,
		LL_NORM,
		LL_ERR,
	};

	void setLevel(ELevel minLevel);
	void doLog(ELevel level, const char *file, int line, const char *fmt, ...) __attribute__((format(printf, 4, 5)));
	void doDump(ELevel level, const char *file, int line, const char *prefix, const void *data, size_t sz);
}
