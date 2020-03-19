/**
 * \brief	Command-line interface for read applet
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#include "clibase.h"

namespace cli
{
	class CRead: public CBase
	{
	public:
		CRead();
		virtual ~CRead() {}

		const std::string &getPort() const;
		const std::string &getFile() const;

	protected:
		virtual std::string parsed();

	private:
		std::string m_port;
		std::string m_file;
	};
}
