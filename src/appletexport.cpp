/**
 * \brief	Applet to export data
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#include "appletexport.h"
#include "cliexport.h"
#include "config.h"
#include "log.h"
#include "omifile.h"
#include "util.h"

bool applet::CExport::run(int argc, char * const argv[])
{
	cli::CExport cli;
	if (!cli.parse(argc, argv))
		return false;

	// xxx TBD
	return false;
}
