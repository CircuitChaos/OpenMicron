/**
 * \brief	Open Micron entry point
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <cstdio>
#include "throw.h"
#include "version.h"
#include "log.h"
#include "appletbase.h"
#include "appletread.h"
#include "appletwrite.h"
#include "appletexport.h"
#include "appletimport.h"
#include "cliread.h"
#include "cliwrite.h"
#include "cliexport.h"
#include "cliimport.h"

static void help()
{
	std::vector<std::string> summaries;

	cli::CRead cr;
	summaries.push_back(cr.getSummary());

	cli::CWrite cw;
	summaries.push_back(cw.getSummary());

	cli::CExport ce;
	summaries.push_back(ce.getSummary());

	cli::CImport ci;
	summaries.push_back(ci.getSummary());

	printf(
		"Open Micron version %s (commit hash %s)\n"
		"Utility to program CRT Micron transceivers under Linux.\n"
		"Newest version at: https://github.com/CircuitChaos/OpenMicron\n"
		"\n",
		version::getVersion(), version::getHash());

	for (const auto &s: summaries)
		printf("Usage: %s\n", s.c_str());

	printf("\nCall omi <applet name> -h (e.g. omi read -h) to get detailed help.\n");
}

static int main2(int argc, char * const argv[])
{
	if (argc < 2)
	{
		help();
		return EXIT_FAILURE;
	}

	std::string av1(argv[1]);
	std::unique_ptr<applet::CBase> a;
	if (av1 == "read")
		a.reset(new applet::CRead());
	else if (av1 == "write")
		a.reset(new applet::CWrite());
	else if (av1 == "export")
		a.reset(new applet::CExport());
	else if (av1 == "import")
		a.reset(new applet::CImport());

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
