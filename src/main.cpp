/**
 * \brief	Open Micron entry point
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include "throw.h"
#include "version.h"
#include "log.h"
#include "appletbase.h"
#include "appletread.h"

static void help()
{
	printf(
		"Open Micron version %s (commit hash %s)\n"
		"Utility to program CRT Micron transceivers under Linux\n"
		"Newest version at: https://github.com/CircuitChaos/OpenMicron\n"
		"\n"
		"Usage: omi <applet name> [applet arguments]\n"
		"\n"
		"Available applets:\n"
		"- read\n"
		"- write\n"
		"- export\n"
		"- import\n"
		"\n"
		"Call applet name without arguments to read its help.\n",
		version::getVersion(), version::getHash());
}

static int main2(int argc, char * const argv[])
{
	if (argc < 2)
	{
		help();
		return EXIT_FAILURE;
	}

	std::string av1(argv[1]);
	std::auto_ptr<applet::CBase> a;
	// xxx uncomment when next applets are ready
	if (av1 == "read")
		a.reset(new applet::CRead());
	/*
	else if (av1 == "write")
		a.reset(new applet::CWrite());
	else if (av1 == "export")
		a.reset(new applet::CExport());
	else if (av1 == "import")
		a.reset(new applet::CImport());
	*/

	if (!a.get())
	{
		help();
		return EXIT_FAILURE;
	}

	return a->run(argc - 1, argv + 1) ? EXIT_SUCCESS : EXIT_FAILURE;
}

int main(int argc, char * const argv[])
{
	try
	{
		return main2(argc, argv);
	}
	catch (const std::runtime_error &e)
	{
		fprintf(stderr, "Fatal error: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
