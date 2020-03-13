/**
 * \brief	Command-line interface base class
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include <unistd.h>
#include <cstdio>
#include "clibase.h"
#include "throw.h"
#include "log.h"

cli::CBase::CBase()
{
	add('h', false, "Show help (this screen)");
	add('d', false, "Enable debug output (full verbosity)");
	add('v', false, "Increase verbosity (verbose output)");
	add('q', false, "Decrease verbosity (quiet output)");
}

bool cli::CBase::parse(int argc, char * const argv[])
{
	const std::string optString(getOptString());

	int opt;
	while ((opt = getopt(argc, argv, optString.c_str())) != -1)
	{
		if (opt == '?')
		{
			loge("Option -%c not recognized", optopt);
			help();
			return false;
		}

		if (opt == ':')
		{
			loge("Option -%c requires argument", optopt);
			help();
			return false;
		}

		if (opt == 'h')
		{
			help();
			return false;
		}

		if (m_opts.find(opt) != m_opts.end())
		{
			loge("Option -%c specified twice", opt);
			help();
			return false;
		}

		m_opts[opt] = m_optsMap[opt].withArg ? optarg : "";
	}

	if (optind != argc)
	{
		loge("Excessive arguments");
		help();
		return false;
	}

	std::string s;

	s = baseParsed();
	if (!s.empty())
	{
		loge("Error: %s", s.c_str());
		help();
		return false;
	}

	s = parsed();
	if (!s.empty())
	{
		loge("Error: %s", s.c_str());
		help();
		return false;
	}

	return true;
}

std::string cli::CBase::getOptString() const
{
	std::string s(":");

	for (const auto &o: m_optsMap)
	{
		s.push_back(o.first);
		if (o.second.withArg)
			s.push_back(':');
	}

	return s;
}

void cli::CBase::help()
{
	printf("Usage: %s\n\n", getSummary().c_str());
	printf("Available options:\n");
	for (const auto &o: m_optsMap)
		printf("  -%c: %s\n", o.first, o.second.descr.c_str());
}

std::string cli::CBase::baseParsed()
{
	const std::string err("Conflicting verbosity options used");
	xlog::ELevel ll(xlog::LL_NORM);
	if (exists('d'))
		ll = xlog::LL_DBG;

	if (exists('q'))
	{
		if (ll != xlog::LL_NORM)
			return err;

		ll = xlog::LL_ERR;
	}

	if (exists('v'))
	{
		if (ll != xlog::LL_NORM)
			return err;

		ll = xlog::LL_INFO;
	}

	xlog::setLevel(ll);
	return "";
}

void cli::CBase::setSummary(const std::string &name, const std::string &opts)
{
	std::string nonArgOpts;
	for (const auto &o: m_optsMap)
		nonArgOpts.push_back(o.first);

	m_summary = std::string("omi ") + name + " [-" + nonArgOpts + "]";
	if (!opts.empty())
		m_summary += std::string(" ") + opts;
}

void cli::CBase::add(char option, bool withArg, const std::string &descr)
{
	xassert(m_optsMap.find(option) == m_optsMap.end(), "%c", option);
	SOpt o;
	o.withArg = withArg;
	o.descr = descr;
	m_optsMap[option] = o;
}

bool cli::CBase::exists(char option)
{
	return m_opts.find(option) != m_opts.end();
}

std::string cli::CBase::get(char option)
{
	const auto &o = m_opts.find(option);
	xassert(o != m_opts.end(), "%c", option);
	return o->second;
}

const std::string &cli::CBase::getSummary() const
{
	return m_summary;
}
