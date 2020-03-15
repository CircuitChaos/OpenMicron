/**
 * \brief	Applet to export data
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#pragma once

#include "appletbase.h"
#include "impexp.h"

namespace applet
{
	class CExport: public CBase
	{
	public:
		virtual ~CExport() {}
		virtual bool run(int argc, char * const argv[]);

	private:
		static std::string decodeModel(const uint8_t *p);
		static std::string decodeEncDec(bool dcs, bool cts);
		static std::string decodeFreq(const uint8_t *freq, bool condensed);
		static std::string decodeCts(uint8_t cts);
		static std::string decodeDcs(uint8_t dcs, impexp::SChannel::SDcsFlags flags);
		static std::string decodeDefCts(const uint8_t *defCts);
	};
}
