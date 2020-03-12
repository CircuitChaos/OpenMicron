/**
 * \brief	Applet to read data from the radio
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include "appletread.h"
#include "cliread.h"

applet::CRead::CRead()
{
	// xxx
}

applet::CRead::~CRead()
{
	// xxx
}

bool applet::CRead::run(int argc, char * const argv[])
{
	cli::CRead cli;
	if (!cli.parse(argc, argv))
		return false;

	// xxx
	return false;
}
