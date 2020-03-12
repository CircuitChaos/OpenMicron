/**
 * \brief	Base class for all applets
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

namespace applet
{
	class CBase
	{
	public:
		virtual ~CBase() {}
		virtual bool run(int argc, char * const argv[]) = 0;
	};
}
