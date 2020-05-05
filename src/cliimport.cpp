/**
 * \brief	Command-line interface for import
 * \author	Circuit Chaos
 * \date	2020-03-16
 */

#include <cstdio>
#include "cliimport.h"
#include "config.h"
#include "util.h"
#include "log.h"

cli::CImport::CImport(): m_inputIsText(false)
{
	add('t', true, "Input text file path");
	add('c', true, "Input .csv file path");
	add('i', true, "Input .omi file path");
	add('o', true, "Output .omi file path (can be the same as -i)");
	setSummary("import", "-t <file.txt>|-c <file.csv> -i <file.omi> -o <file.omi>");
}

const std::string &cli::CImport::getInputTextCsv() const
{
	return m_inputTextCsv;
}

const std::string &cli::CImport::getInputOmi() const
{
	return m_inputOmi;
}

const std::string &cli::CImport::getOutputOmi() const
{
	return m_outputOmi;
}

bool cli::CImport::inputIsText() const
{
	return m_inputIsText;
}

std::string cli::CImport::parsed()
{
	if (!exists('c') && !exists('t'))
		return "One -c or -t must be specified";

	if (exists('c') && exists('t'))
		return "Only one of -c or -t must be specified";

	if (!exists('i'))
		return "Input .omi file not specified";

	if (!exists('o'))
		return "Output .omi file not specified";

	if (exists('c'))
	{
		m_inputTextCsv = get('c');
		m_inputIsText = false;
	}
	else
	{
		m_inputTextCsv = get('t');
		m_inputIsText = true;
	}

	m_inputOmi = get('i');
	m_outputOmi = get('o');
	return "";
}
