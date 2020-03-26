/**
 * \brief	Applet to export data
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#pragma once

#include <vector>
#include <string>
#include "appletbase.h"
#include "textfile.h"
#include "impexp.h"

namespace applet
{
	class CExport: public CBase
	{
	public:
		virtual ~CExport() {}
		virtual bool run(int argc, char * const argv[]);

	private:
		static void outputChannelComment(CTextFile &tf);
		static void outputKeysComment(CTextFile &tf);
		static void outputChannels(CTextFile &tf, const std::vector<uint8_t> &data);
		static void outputKeys(CTextFile &tf, const std::vector<uint8_t> &data);
		static void debugDumpChannel(const impexp::SChannel *chan);
		static std::string getModel(const uint8_t *p);
		static std::string getCombinedFreq(unsigned chanNo, const uint8_t *rxfreq, const uint8_t *txshift, impexp::SChannel::SFlags1::EShiftDir dir);
		static std::string getDecoder(unsigned chanNo, bool dcs, bool cts);
		static std::string getCts(unsigned chanNo, uint8_t cts);
		static std::string getDcs(unsigned chanNo, uint8_t dcs, impexp::SChannel::SDcsFlags flags);
		static std::string getSquelchMode(unsigned chanNo, impexp::SChannel::ESql sql);
		static std::string getTxPower(unsigned chanNo, bool txoff, impexp::SChannel::SFlags1::ETxPwr pwr);
		static std::string getBcl(unsigned chanNo, impexp::SChannel::EBcl bcl);
		static std::string getPttId(unsigned chanNo, impexp::SChannel::EPttId pttid);
		static std::string getOptSig(unsigned chanNo, impexp::SChannel::EOptSig optsig, uint8_t dtmf);
		static std::string getBandwidth(unsigned chanNo, impexp::SChannel::SFlags2::EBandwidth spacing);
		static std::string getDefCts(unsigned chanNo, const uint8_t *defCts);
		static std::string getFuncKey(unsigned key);
		static std::string getMicKey(unsigned key);
		static std::string getSingleFreq(unsigned chanNo, const uint8_t *freq, bool condensed);
	};
}
