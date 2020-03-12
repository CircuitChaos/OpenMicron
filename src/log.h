/**
 * \brief	Logging
 * \author	Circuit Chaos
 * \date	2020-03-12
 *
 * It logs to stdout only.
 */

#pragma once

#define LOG_GENERIC(level, ...) log::doLog(level, __FILE__, __LINE__, __VA_ARGS__)
#define logd(...) LOG_GENERIC(log::LL_DBG, __VA_ARGS__)
#define logn(...) LOG_GENERIC(log::LL_NORM, __VA_ARGS__)
#define logw(...) LOG_GENERIC(log::LL_WARN, __VA_ARGS__)
#define loge(...) LOG_GENERIC(log::LL_ERR, __VA_ARGS__)

namespace log
{
	// when changing these, change also levelToString() in log.cpp
	enum ELevel
	{
		LL_DBG,
		LL_NORM,
		LL_WARN,
		LL_ERR,
	};

	void setLevel(ELevel minLevel);
	void doLog(ELevel level, const char *file, int line, const char *fmt, ...);
}
