/**
 * \brief	Command-line interface for write applet
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <cstdio>
#include "cliwrite.h"
#include "config.h"
#include "util.h"
#include "log.h"

cli::CWrite::CWrite()
{
	setSummary("write", "-i <input.omi> [-r <reference.omi>] [-p <port>]");
	add('i', true, "Input .omi file path");
	add('r', true, "Original (reference) .omi file path for differential upload");
	add('p', true, util::format("Port to use (default: %s)", config::DFL_PORT));
}

const std::string &cli::CWrite::getPort() const
{
	return m_port;
}

const std::string &cli::CWrite::getFile() const
{
	return m_file;
}

const std::string &cli::CWrite::getRefFile() const
{
	return m_refFile;
}

std::string cli::CWrite::parsed()
{
	m_port = exists('p') ? get('p') : config::DFL_PORT;

	if (!exists('i'))
		return "Input file not specified";

	m_file = get('i');

	if (exists('r'))
		m_refFile = get('r');

	return "";
}
