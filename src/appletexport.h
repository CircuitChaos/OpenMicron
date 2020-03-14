/**
 * \brief	Applet to export data
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#pragma once

#include "appletbase.h"

namespace applet
{
	class CExport: public CBase
	{
	public:
		virtual ~CExport() {}
		virtual bool run(int argc, char * const argv[]);
	};
}
