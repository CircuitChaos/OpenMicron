/**
 * \brief	Command-line interface for write applet
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#pragma once

#include "clibase.h"

namespace cli
{
	class CWrite: public CBase
	{
	public:
		CWrite();
		virtual ~CWrite() {}

		const std::string &getPort() const;
		const std::string &getFile() const;
		const std::string &getRefFile() const;

	protected:
		virtual std::string parsed();

	private:
		std::string m_port;
		std::string m_file;
		std::string m_refFile;
	};
}
