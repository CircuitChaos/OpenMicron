/**
 * \brief	Command-line interface for export
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#include <cstdio>
#include "cliexport.h"
#include "config.h"
#include "util.h"
#include "log.h"

cli::CExport::CExport(): m_isText(false)
{
	add('i', true, "Input .omi file path");
	add('t', true, "Output text file path");
	add('c', true, "Output .csv file path");
	setSummary("export", "-i <file.omi> -t <file.txt>|-c <file.csv>");
}

const std::string &cli::CExport::getInput() const
{
	return m_input;
}

const std::string &cli::CExport::getOutput() const
{
	return m_output;
}

bool cli::CExport::isText() const
{
	return m_isText;
}

std::string cli::CExport::parsed()
{
	if (!exists('c') && !exists('t'))
		return "One -c or -t must be specified";

	if (exists('c') && exists('t'))
		return "Only one of -c or -t must be specified";

	if (!exists('i'))
		return "Input file not specified";

	if (exists('c'))
	{
		m_output = get('c');
		m_isText = false;
	}
	else
	{
		m_output = get('t');
		m_isText = true;
	}

	m_input = get('i');
	return "";
}
