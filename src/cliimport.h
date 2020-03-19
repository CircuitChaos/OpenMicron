/**
 * \brief	Command-line interface for import
 * \author	Circuit Chaos
 * \date	2020-03-16
 */

#pragma once

#include "clibase.h"

namespace cli
{
	class CImport: public CBase
	{
	public:
		CImport();
		virtual ~CImport() {}

		const std::string &getInputTextCsv() const;
		const std::string &getInputOmi() const;
		const std::string &getOutputOmi() const;
		bool inputIsText() const;

	protected:
		virtual std::string parsed();

	private:
		std::string m_inputTextCsv;
		std::string m_inputOmi;
		std::string m_outputOmi;
		bool m_inputIsText;
	};
}
