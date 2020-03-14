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

cli::CRead::CRead(): m_full(false)
{
	add('o', true, "Output .omi file path");
	add('p', true, util::format("Port to use (default: %s)", config::DFL_PORT));
	add('f', false, "Read full memory (only channel table is read by default)");
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

bool cli::CRead::isFull() const
{
	return m_full;
}

std::string cli::CRead::parsed()
{
	m_port = exists('p') ? get('p') : config::DFL_PORT;

	if (!exists('o'))
		return "Output file not specified";

	m_file = get('o');
	m_full = exists('f');
	return "";
}
