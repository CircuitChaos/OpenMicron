/**
 * \brief	Version file
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include "version.h"

static const char VERSION[] = "0.1";

#define TO_STR2(x) #x
#define TO_STR(x) TO_STR2(x)

const char *version::getVersion()
{
	return VERSION;
}

const char *version::getHash()
{
	return TO_STR(GIT_HASH);
}
