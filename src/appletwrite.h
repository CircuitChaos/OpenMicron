/**
 * \brief	Applet to write data to the radio
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#pragma once

#include "appletbase.h"

namespace applet
{
	class CWrite: public CBase
	{
	public:
		virtual ~CWrite() {}
		virtual bool run(int argc, char * const argv[]);
	};
}
