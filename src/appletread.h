/**
 * \brief	Applet to read data from the radio
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

#include "appletbase.h"

namespace applet
{
	class CRead: public CBase
	{
	public:
		CRead();
		virtual ~CRead();

		virtual bool run(int argc, char * const argv[]);
	};
}
