/**
 * \brief	Command-line interface base class
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#include <string>
#include <map>

namespace cli
{
	class CBase
	{
	public:
		CBase();
		virtual ~CBase() {}

		// also calls help() and sets logging verbosity
		bool parse(int argc, char * const argv[]);

		// can call without parse(), called from help() from main
		const std::string &getSummary() const;

	protected:
		// call these two in ctor, but setSummary() after all add()
		void setSummary(const std::string &name, const std::string &opts);
		void add(char option, bool withArg, const std::string &descr);
		bool exists(char option);
		std::string get(char option);

		// called in parse() after parsing all options
		// can return an error string displayed by parse()
		virtual std::string parsed() = 0;

	private:
		struct SOpt
		{
			bool withArg;
			std::string descr;
		};

		std::string m_summary;
		std::map<char, SOpt> m_optsMap;
		std::map<char, std::string> m_opts;

		std::string getOptString() const;
		void help();
		std::string baseParsed();
	};
}
