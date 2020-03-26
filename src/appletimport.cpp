/**
 * \brief	Applet to import data
 * \author	Circuit Chaos
 * \date	2020-03-16
 */

#include <vector>
#include <string>
#include <cstring>
#include <cstdarg>
#include <unistd.h>
#include "appletimport.h"
#include "cliimport.h"
#include "config.h"
#include "log.h"
#include "omifile.h"
#include "textfile.h"
#include "impexp.h"
#include "throw.h"
#include "util.h"

using namespace impexp;

applet::CImport::CImport(): m_chan(NULL)
{
}

bool applet::CImport::run(int argc, char * const argv[])
{
	cli::CImport cli;
	if (!cli.parse(argc, argv))
		return false;

	COmiFile omi;
	if (!omi.read(cli.getInputOmi()))
		return false;

	if (omi.getOffset() != 0)
	{
		logError("input file offset not zero (not supported)");
		return false;
	}

	if (omi.getData().size() != config::MEMORY_SIZE)
	{
		logError("input file does not contain radio memory (size mismatch)");
		return false;
	}

	CTextFile tf;
	if (!tf.read(cli.getInputTextCsv(), cli.inputIsText()))
		return false;

	for (auto &line: tf.get())
	{
		++m_errCtx.lineNo;
		m_errCtx.chanNo = 0;

		if (line.empty() || line[0].empty() || line[0] == strings::COMMENT)
			continue;
		else if (line[0] == strings::WELCOME)
		{
			if (!importWelcome(omi, line))
				return false;
		}
		else if (line[0] == strings::CHANNEL)
		{
			if (!importChannel(omi, line))
				return false;
		}
		else if (line[0] == strings::KEY)
		{
			if (!importKey(omi, line))
				return false;
		}
		else
		{
			logError("command '%s' not recognized", line[0].c_str());
			return false;
		}
	}

	logd("Processed file, writing output");
	if (!omi.write(cli.getOutputOmi()))
		return false;

	return true;
}

void applet::CImport::logError(const char *fmt, ...)
{
	std::string s;

	s += "Error: ";
	if (m_errCtx.lineNo)
		s += util::format("line %u: ", m_errCtx.lineNo);

	if (m_errCtx.chanNo)
		s += util::format("channel %u: ", m_errCtx.chanNo);

	va_list ap;
	va_start(ap, fmt);

	char *p;
	xassert(vasprintf(&p, fmt, ap) != -1, "Allocation error during error reporting");

	va_end(ap);
	s += p;
	free(p);

	loge("%s", s.c_str());
}

bool applet::CImport::importWelcome(COmiFile &omi, const std::vector<std::string> &line)
{
	std::string welcome;
	if (line.size() == 1)
		welcome = std::string(WELCOME_SIZE, 0x20);
	else if (!convertName(welcome, line[1], WELCOME_SIZE))
		return false;

	logd("Importing welcome message: %s", welcome.c_str());
	xassert(welcome.size() == WELCOME_SIZE, "Bug in convertName(), %zu ne %u", welcome.size(), WELCOME_SIZE);
	memcpy(&omi.getData()[WELCOME_OFFSET], welcome.data(), welcome.size());
	return true;
}

bool applet::CImport::importChannel(COmiFile &omi, const std::vector<std::string> &line)
{
	if (line.size() == 1)
	{
		logError("invalid channel format in input file (too short)");
		return false;
	}

	const unsigned chanNo(strtol(line[1].c_str(), NULL, 10));
	if (chanNo < 1 || chanNo > NUM_CHANNELS)
	{
		logError("channel number %s non-numeric or out of range (1-%u)", line[1].c_str(), NUM_CHANNELS);
		return false;
	}

	m_chan = (SChannel *) &omi.getData()[(chanNo - 1) * 32];
	m_errCtx.chanNo = chanNo;
	if (line.size() == 2 || line[2].empty())
	{
		logd("Removing channel %u", chanNo);
		memset(m_chan, 0xff, sizeof(SChannel));
		clrFlag(&omi.getData()[CHAN_EN_OFFSET], chanNo - 1);
		clrFlag(&omi.getData()[SCANNING_OFFSET], chanNo - 1);
		return true;
	}

	if (line.size() < 16)
	{
		logError("invalid field count");
		return false;
	}

	logd("Importing channel %u", chanNo);
	setFlag(&omi.getData()[CHAN_EN_OFFSET], chanNo - 1);

	bool scanning;
	unsigned idx(2);
	if (!importName(line[idx++])) return false;
	if (!importCombinedFreq(line[idx++])) return false;
	if (!importEncDec(line[idx++], false)) return false;
	if (!importEncDec(line[idx++], true)) return false;
	if (!importSquelchMode(line[idx++])) return false;
	if (!importTxPower(line[idx++])) return false;
	if (!importBandwidth(line[idx++])) return false;
	if (!importBcl(line[idx++])) return false;
	if (!importPttId(line[idx++])) return false;
	if (!importOptSig(line[idx++])) return false;
	if (!importScanning(line[idx++], scanning)) return false;
	if (!importTalkaround(line[idx++])) return false;
	if (!importReverse(line[idx++])) return false;
	if (!importDefCts(line[idx++])) return false;

	if (!scanning)
		clrFlag(&omi.getData()[SCANNING_OFFSET], chanNo - 1);
	else
		setFlag(&omi.getData()[SCANNING_OFFSET], chanNo - 1);

	return true;
}

bool applet::CImport::importName(const std::string &field)
{
	std::string name;
	if (!convertName(name, field, sizeof(m_chan->chname)))
		return false;

	xassert(name.size() == sizeof(m_chan->chname), "Bug in convertName(), %zu ne %zu", name.size(), sizeof(m_chan->chname));
	memcpy(m_chan->chname, name.data(), name.size());
	return true;
}

bool applet::CImport::importCombinedFreq(const std::string &field)
{
	static_assert(sizeof(m_chan->rxfreq) == 4, "Wrong RX frequency array size");
	static_assert(sizeof(m_chan->txshift) == 4, "Wrong TX shift array size");

	std::string freq, shift;
	char dir(0);

	const size_t npos(field.find('-')), ppos(field.find('+'));
	if (npos == std::string::npos && ppos == std::string::npos)
		freq = field;
	else if (npos == std::string::npos && ppos != std::string::npos)
	{
		freq = field.substr(0, ppos);
		shift = field.substr(ppos + 1);
		dir = '+';
	}
	else if (npos != std::string::npos && ppos == std::string::npos)
	{
		freq = field.substr(0, npos);
		shift = field.substr(npos + 1);
		dir = '-';
	}
	else
	{
		logError("both + and - found in frequency field (%s)", field.c_str());
		return false;
	}

	if (!convertFreq(m_chan->rxfreq, freq))
		return false;

	if (dir == 0)
	{
		memset(m_chan->txshift, 0, 4);
		m_chan->flags1.shiftdir = SChannel::SFlags1::SHIFT_OFF;
		return true;
	}

	xassert(dir == '+' || dir == '-', "Invalid direction (%c)", dir);

	if (!convertFreq(m_chan->txshift, shift))
		return false;

	if (!memcmp(m_chan->txshift, "\0\0\0\0", 4))
		m_chan->flags1.shiftdir = SChannel::SFlags1::SHIFT_OFF;
	else
		m_chan->flags1.shiftdir = (dir == '+') ? SChannel::SFlags1::SHIFT_UP : SChannel::SFlags1::SHIFT_DN;
	return true;
}

bool applet::CImport::importEncDec(const std::string &field, bool isEncoder)
{
	bool bdcs, bcts;

	if (field == strings::ENCDEC_OFF)
	{
		bdcs = false;
		bcts = false;
	}
	else
	{
		const auto pos(field.find(strings::SEPARATOR));
		if (pos == std::string::npos)
		{
			logError("invalid %s setting (%s)", isEncoder ? "TX Encoder" : "RX Decoder", field.c_str());
			return false;
		}

		const std::string type(field.substr(0, pos));
		const std::string value(field.substr(pos + 1));

		if (type == strings::ENCDEC_CTCSS)
		{
			bdcs = false;
			bcts = true;

			if (!importCts(value, isEncoder))
				return false;
		}
		else if (type == strings::ENCDEC_DCS)
		{
			bdcs = true;
			bcts = false;

			if (!importDcs(value, isEncoder))
				return false;
}
		else
		{
			logError("invalid %s setting (%s)", isEncoder ? "TX Encoder" : "RX Decoder", field.c_str());
			return false;
		}
	}

	if (isEncoder)
	{
		m_chan->flags3.txdcs = bdcs;
		m_chan->flags3.txcts = bcts;
	}
	else
	{
		m_chan->flags3.rxdcs = bdcs;
		m_chan->flags3.rxcts = bcts;
	}

	return true;
}

bool applet::CImport::importCts(const std::string &value, bool isEncoder)
{
	size_t idx;
	if (!getCtsIdx(value, idx, true))
		return false;

	if (isEncoder)
		m_chan->txcts = idx;
	else
		m_chan->rxcts = idx;
	return true;
}

bool applet::CImport::importDcs(const std::string &value, bool isEncoder)
{
	bool invert(false);
	std::string sdcs(value);
	if (value.size() >= 1 && value[0] == strings::DCS_INVERT_FLAG)
	{
		invert = true;
		sdcs = value.substr(1);
	}

	if (sdcs.empty())
	{
		logError("invalid %s DCS setting (empty)", isEncoder ? "TX" : "RX");
		return false;
	}

	if (sdcs.size() < 3)
		sdcs = std::string(3 - sdcs.size(), '0') + sdcs;

	uint16_t dcs16(strtol(sdcs.c_str(), NULL, 8));
	if (util::format("%03o", dcs16) != sdcs)
	{
		logError("invalid %s DCS setting (%s)", isEncoder ? "TX" : "RX", value.c_str());
		return false;
	}

	SChannel::SDcsFlags dcsfl;
	dcsfl.invert = invert;
	dcsfl.msb = dcs16 & 0x100;
	const uint8_t dcs8(dcs16 & 0xff);

	if (isEncoder)
	{
		m_chan->txdcsfl = dcsfl;
		m_chan->txdcs = dcs8;
	}
	else
	{
		m_chan->rxdcsfl = dcsfl;
		m_chan->rxdcs = dcs8;
	}
	return true;
}

bool applet::CImport::importSquelchMode(const std::string &field)
{
	if (field == strings::SQL_CARRIER)
		m_chan->sql = SChannel::SQL_CARRIER;
	else if (field == strings::SQL_CTSDCS)
		m_chan->sql = SChannel::SQL_CTSDCS;
	else if (field == strings::SQL_OPTSIG)
		m_chan->sql = SChannel::SQL_OPTSIG;
	else
	{
		logError("invalid squelch setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importTxPower(const std::string &field)
{
	m_chan->flags2.txoff = false;

	if (field == strings::TXP_OFF)
	{
		m_chan->flags2.txoff = true;
		m_chan->flags1.txpwr = SChannel::SFlags1::TXPWR_LOW;
	}
	else if (field == strings::TXP_LOW)
		m_chan->flags1.txpwr = SChannel::SFlags1::TXPWR_LOW;
	else if (field == strings::TXP_MEDIUM)
		m_chan->flags1.txpwr = SChannel::SFlags1::TXPWR_MED;
	else if (field == strings::TXP_HIGH)
		m_chan->flags1.txpwr = SChannel::SFlags1::TXPWR_HIGH;
	else
	{
		logError("invalid TX power setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importBcl(const std::string &field)
{
	if (field == strings::BCL_OFF)
		m_chan->bcl = SChannel::BCL_OFF;
	else if (field == strings::BCL_RPT)
		m_chan->bcl = SChannel::BCL_RPT;
	else if (field == strings::BCL_BUSY)
		m_chan->bcl = SChannel::BCL_BUSY;
	else
	{
		logError("invalid Busy Channel Lockout setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importPttId(const std::string &field)
{
	if (field == strings::PTTID_OFF)
	{
		m_chan->pttid = SChannel::PTTID_OFF;
		return true;
	}

	const auto pos(field.find(strings::SEPARATOR));
	if (pos == std::string::npos)
	{
		logError("invalid PTT ID setting (%s)", field.c_str());
		return false;
	}

	const std::string first(field.substr(0, pos));
	const std::string last(field.substr(pos + 1));

	if (first != strings::PTTID_DTMF && first != strings::PTTID_5TONE)
	{
		logError("invalid PTT ID setting (%s)", field.c_str());
		return false;
	}

	const bool is5tone(first == strings::PTTID_5TONE);
	if (last == strings::PTTID_BEGIN)
		m_chan->pttid = is5tone ? SChannel::PTTID_5TONE_BEGIN : SChannel::PTTID_DTMF_BEGIN;
	else if (last == strings::PTTID_END)
		m_chan->pttid = is5tone ? SChannel::PTTID_5TONE_END : SChannel::PTTID_DTMF_END;
	else if (last == strings::PTTID_BOTH)
		m_chan->pttid = is5tone ? SChannel::PTTID_5TONE_BOTH : SChannel::PTTID_DTMF_BOTH;
	else
	{
		logError("invalid PTT ID setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importOptSig(const std::string &field)
{
	static const size_t lendtmf(strlen(strings::OPTSIG_DTMF));
	static const size_t len5tone(strlen(strings::OPTSIG_5TONE));

	if (field == strings::OPTSIG_OFF)
		m_chan->optsig = SChannel::OPTSIG_OFF;
	else if (field.size() > lendtmf && field.substr(0, lendtmf) == strings::OPTSIG_DTMF)
	{
		const unsigned param(strtol(field.substr(lendtmf).c_str(), NULL, 10));
		if (param < 1 || param > 16)
		{
			logError("invalid Optional Signaling setting (%s)", field.c_str());
			return false;
		}

		m_chan->optsig = SChannel::OPTSIG_DTMF;
		m_chan->flags3.dtmf = param - 1;
	}
	else if (field.size() > len5tone && field.substr(0, len5tone) == strings::OPTSIG_5TONE)
	{
		const unsigned param(strtol(field.substr(len5tone).c_str(), NULL, 10));
		if (param > 15)
		{
			logError("invalid Optional Signaling setting (%s)", field.c_str());
			return false;
		}

		// xxx not sure if 5tone setting is really in dtmf field
		m_chan->optsig = SChannel::OPTSIG_5TONE;
		m_chan->flags3.dtmf = param;
	}
	else
	{
		logError("invalid Optional Signaling setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importScanning(const std::string &field, bool &scanning)
{
	if (field == strings::YES)
		scanning = true;
	else if (field == strings::NO)
		scanning = false;
	else
	{
		logError("invalid Scanning setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importTalkaround(const std::string &field)
{
	if (field == strings::YES)
		m_chan->flags1.talkaround = true;
	else if (field == strings::NO)
		m_chan->flags1.talkaround = false;
	else
	{
		logError("invalid Talkaround setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importReverse(const std::string &field)
{
	if (field == strings::YES)
		m_chan->flags2.reverse = true;
	else if (field == strings::NO)
		m_chan->flags2.reverse = false;
	else
	{
		logError("invalid Reverse setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importBandwidth(const std::string &field)
{
	if (field == strings::BW_125)
		m_chan->flags2.bandwidth = SChannel::SFlags2::BW_125;
	else if (field == strings::BW_20)
		m_chan->flags2.bandwidth = SChannel::SFlags2::BW_20;
	else if (field == strings::BW_25)
		m_chan->flags2.bandwidth = SChannel::SFlags2::BW_25;
	else
	{
		logError("invalid Bandwidth setting (%s)", field.c_str());
		return false;
	}

	return true;
}

bool applet::CImport::importDefCts(const std::string &field)
{
	// default CTCSS does not seem to conform to the normal CTCSS map
	if (field.empty() || field.size() < 3 || field[field.size() - 2] != '.')
	{
		logError("invalid Def. CTCSS (%s)", field.c_str());
		return false;
	}

	// quick and dirty...
	const std::string scts10(field.substr(0, field.size() - 2) + field[field.size() - 1]);
	const uint16_t cts10(strtol(scts10.c_str(), NULL, 10));
	logd("Converted Def. CTCSS %s to number %u", field.c_str(), cts10);

	m_chan->defcts[0] = cts10 & 0xff;
	m_chan->defcts[1] = cts10 >> 8;
	return true;
}

bool applet::CImport::convertName(std::string &out, const std::string &in, unsigned size)
{
	if (in.size() > size)
	{
		logError("string '%s' too long, must be %u chars max", in.c_str(), size);
		return false;
	}

	for (auto &ch: in)
	{
		if (ch >= 'A' && ch <= 'Z')
			continue;

		if (ch >= '0' && ch <= '9')
			continue;

		if (ch == 0x20 || ch == '-')
			continue;

		// this might not be fully true. feel free to report the issue to relax this test
		logError("invalid character '%c' in string '%s'", ch, in.c_str());
		logError("only uppercase 7-bit ASCII letters, digits, dashes and spaces are allowed");
		return false;
	}

	out = in;
	if (out.size() < size)
		out += std::string(size - out.size(), 0x20);

	return true;
}

bool applet::CImport::getCtsIdx(const std::string &number, size_t &idx, bool allowDef)
{
	for (size_t i(0); i < sizeof(CTSTBL) / sizeof(*CTSTBL) - (allowDef ? 0 : 1); ++i)
	{
		if (CTSTBL[i] == number)
		{
			idx = i;
			return true;
		}
	}

	logError("Could not find CTCSS %s in CTCSS map", number.c_str());
	return false;
}

bool applet::CImport::convertFreq(uint8_t out[4], const std::string &in)
{
	if (in.empty())
	{
		logError("cannot convert empty frequency");
		return false;
	}

	// BCD format expects: DDDFFFFF
	const auto pos(in.find('.'));
	std::string dec, frac;
	if (pos == std::string::npos)
		dec = in;
	else
	{
		dec = in.substr(0, pos);
		frac = in.substr(pos + 1);
	}

	if (dec.size() > 3)
	{
		logError("decimal part (%s) of frequency %s too long", dec.c_str(), in.c_str());
		return false;
	}
	else if (dec.size() < 3)
		dec = std::string(3 - dec.size(), '0') + dec;

	if (frac.size() > 5)
	{
		logError("fractional part (%s) of frequency %s too long", frac.c_str(), in.c_str());
		return false;
	}
	else if (frac.size() < 5)
		frac += std::string(5 - frac.size(), '0');

	logd("Frequency %s split into dec part %s and frac part %s", in.c_str(), dec.c_str(), frac.c_str());

	const std::string combined(dec + frac);
	xassert(combined.size() == 8, "Invalid combined frequency size (%s)", combined.c_str());

	for (size_t idx(0); idx < 4; ++idx)
		out[idx] = ((combined[idx * 2] - '0') << 4) | (combined[idx * 2 + 1] - '0');

	return true;
}

bool applet::CImport::importKey(COmiFile &omi, const std::vector<std::string> &line)
{
	if (line.size() < 3)
	{
		logError("invalid key format in input file (too short)");
		return false;
	}

	for (unsigned i(0); i < sizeof(FUNC_KEY_NAMES) / sizeof(*FUNC_KEY_NAMES); ++i)
		if (line[1] == FUNC_KEY_NAMES[i])
			return importKey(omi, i, line[2], false);

	for (unsigned i(0); i < sizeof(MIC_KEY_NAMES) / sizeof(*MIC_KEY_NAMES); ++i)
		if (line[1] == MIC_KEY_NAMES[i])
			return importKey(omi, i, line[2], true);

	logError("invalid key name (%s)", line[1].c_str());
	return false;
}

bool applet::CImport::importKey(COmiFile &omi, unsigned idx, const std::string &value, bool isMic)
{
	xassert(idx < (isMic ? 4 : 12), "Index out of range (%u)", idx);
	uint8_t *data(&omi.getData()[(isMic ? MIC_KEYS_OFFSET : FUNC_KEYS_OFFSET) + idx]);

	if (value.size() == 4 && value.substr(0, 2) == "0x")
	{
		logn("Warning: importing key as hex literal (%s)", value.c_str());
		*data = strtol(value.substr(2).c_str(), NULL, 16);
		return true;
	}

	for (unsigned i(0); i < sizeof(KEY_FUNCTIONS) / sizeof(*KEY_FUNCTIONS); ++i)
	{
		if (value != KEY_FUNCTIONS[i])
			continue;

		if (isMic && i == 1)
		{
			logError("cannot import A/B key for microphone");
			return false;
		}

		*data = i;
		return true;
	}

	logError("invalid key value (%s)", value.c_str());
	return false;
}
