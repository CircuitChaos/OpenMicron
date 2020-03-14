/**
 * \brief	Command-line interface for export
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#pragma once

#include "clibase.h"

namespace cli
{
	class CExport: public CBase
	{
	public:
		CExport();
		virtual ~CExport() {}

		const std::string &getInput() const;
		const std::string &getOutput() const;
		bool isText() const;

	protected:
		virtual std::string parsed();

	private:
		std::string m_input;
		std::string m_output;
		bool m_isText;
	};
}
