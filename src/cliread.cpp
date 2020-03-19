/**
 * \brief	Command-line interface for read applet
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <cstdio>
#include "cliread.h"
#include "config.h"
#include "util.h"
#include "log.h"

cli::CRead::CRead()
{
	add('o', true, "Output .omi file path");
	add('p', true, util::format("Port to use (default: %s)", config::DFL_PORT));
	setSummary("read", "-o <output.omi> [-p <port>]");
}

const std::string &cli::CRead::getPort() const
{
	return m_port;
}

const std::string &cli::CRead::getFile() const
{
	return m_file;
}

std::string cli::CRead::parsed()
{
	m_port = exists('p') ? get('p') : config::DFL_PORT;

	if (!exists('o'))
		return "Output file not specified";

	m_file = get('o');
	return "";
}
