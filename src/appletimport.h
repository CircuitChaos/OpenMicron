/**
 * \brief	Applet to import data
 * \author	Circuit Chaos
 * \date	2020-03-16
 */

#pragma once

#include <vector>
#include <string>
#include "appletbase.h"
#include "omifile.h"
#include "impexp.h"

namespace applet
{
	class CImport: public CBase
	{
	public:
		CImport();
		virtual ~CImport() {}
		virtual bool run(int argc, char * const argv[]);

	private:
		// for error reporting in methods
		struct SErrCtx
		{
			SErrCtx(): chanNo(0), lineNo(0) {}
			unsigned chanNo;
			unsigned lineNo;
		};

		SErrCtx m_errCtx;

		// used by all import functions except importWelcome and importChannel
		impexp::SChannel *m_chan;

		// ouch, gcc bug? __attribute__((format(printf, 1, 2))) complains about non-string
		// argument ("format string argument is not a string type"). maybe it counts
		// "this" as a first argument?

#ifdef __GNUC__
#define LOG_ERROR_ATTR __attribute__((format(printf, 2, 3)))
#else
#define LOG_ERROR_ATTR
#endif
		void logError(const char *fmt, ...) LOG_ERROR_ATTR;
#undef LOG_ERROR_ATTR

		bool importWelcome(COmiFile &omi, const std::vector<std::string> &line);
		bool importChannel(COmiFile &omi, const std::vector<std::string> &line);
		bool importKeys(COmiFile &omi, const std::vector<std::string> &line);

		bool importName(const std::string &field);
		bool importRxFreq(const std::string &field);
		bool importTxShift(const std::string &field);
		bool importEncDec(const std::string &field, bool isEncoder);
		bool importCts(const std::string &field, bool isEncoder);
		bool importDcs(const std::string &field, bool isEncoder);
		bool importSquelchMode(const std::string &field);
		bool importTxPower(const std::string &field);
		bool importBcl(const std::string &field);
		bool importPttId(const std::string &field);
		bool importOptSig(const std::string &field);
		bool importScanning(const std::string &field, bool &scanning);
		bool importTalkaround(const std::string &field);
		bool importReverse(const std::string &field);
		bool importSpacing(const std::string &field);
		bool importDefCts(const std::string &field);

		bool convertName(std::string &out, const std::string &in, unsigned size);
		bool getCtsIdx(const std::string &number, size_t &idx, bool allowDef);
		bool convertFreq(uint8_t out[4], const std::string &in);
		bool checkFreqSanity(const uint8_t freq[4]);

		bool importFuncKey(uint8_t *out, const std::string &in);
		bool importMicKey(uint8_t *out, const std::string &in);
	};
}
