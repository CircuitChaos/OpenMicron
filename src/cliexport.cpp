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
	add('o', true, "Output .csv or .txt file path");
	add('t', false, "Use text format (default is CSV)");
	setSummary("export", "-i <file.omi> -o <file.csv>");
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
	if (!exists('i'))
		return "Input file not specified";

	if (!exists('o'))
		return "Output file not specified";

	m_input = get('i');
	m_output = get('o');
	m_isText = exists('t');
	return "";
}
