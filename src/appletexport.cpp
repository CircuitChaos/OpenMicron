/**
 * \brief	Applet to export data
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#include <vector>
#include <string>
#include <cstring>
#include "appletexport.h"
#include "cliexport.h"
#include "config.h"
#include "log.h"
#include "omifile.h"
#include "textfile.h"
#include "impexp.h"
#include "util.h"

using namespace impexp;

bool applet::CExport::run(int argc, char * const argv[])
{
	cli::CExport cli;
	if (!cli.parse(argc, argv))
		return false;

	COmiFile infile;
	if (!infile.read(cli.getInput()))
		return false;

	if (infile.getOffset() != 0)
	{
		loge("Input file offset not zero (not supported)");
		return false;
	}

	if (infile.getData().size() < config::CHAN_MEMORY_SIZE)
	{
		loge("Too little data in input file");
		return false;
	}

	CTextFile tf;

	tf.add(2, strings::WELCOME, util::toPrintable(std::string((const char *) &infile.getData()[WELCOME_OFFSET], WELCOME_SIZE)).c_str());
	tf.add(0);
	outputChannelComment(tf);
	outputChannels(tf, infile.getData());

	if (!tf.write(cli.getOutput(), cli.isText()))
		return false;

	return true;
}

void applet::CExport::outputChannelComment(CTextFile &tf)
{
	tf.add(-1, strings::COMMENT,
		"Number",
		"Name",
		"RX Freq",
		"TX Shift",
		"RX Decoder",
		"RX CTCSS",
		"RX DCS",
		"TX Encoder",
		"TX CTCSS",
		"TX DCS",
		"Squelch",
		"TX Power",
		"BCL",
		"PTT ID",
		"Opt Sig",
		"Scanning",
		"Talkaround",
		"Reverse",
		"Spacing",
		"Def. CTCSS",
		NULL);

	tf.add(-1, strings::COMMENT,
		"",
		"5 chr max",
		"",
		"",
		util::format("%s, %s, %s", strings::ENCDEC_NONE, strings::ENCDEC_CTCSS, strings::ENCDEC_DCS).c_str(),
		util::format("can be '%s'", CTSTBL[sizeof(CTSTBL) / sizeof(CTSTBL[0]) - 1]).c_str(),
		util::format("start with '%c' to invert", strings::DCS_INVERT_FLAG).c_str(),
		util::format("%s, %s, %s", strings::ENCDEC_NONE, strings::ENCDEC_CTCSS, strings::ENCDEC_DCS).c_str(),
		util::format("can be '%s'", CTSTBL[sizeof(CTSTBL) / sizeof(CTSTBL[0]) - 1]).c_str(),
		util::format("start with '%c' to invert", strings::DCS_INVERT_FLAG).c_str(),
		util::format("%s, %s, %s", strings::SQL_CARRIER, strings::SQL_CTSDCS, strings::SQL_OPTSIG).c_str(),
		util::format("%s, %s, %s, %s", strings::TXP_OFF, strings::TXP_LOW, strings::TXP_MEDIUM, strings::TXP_HIGH).c_str(),
		util::format("%s, %s, %s", strings::BCL_OFF, strings::BCL_RPT, strings::BCL_BUSY).c_str(),
		util::format("%s, %s/%s%c%s/%s/%s", strings::PTTID_OFF, strings::PTTID_DTMF, strings::PTTID_5TONE,
			strings::PTTID_SEPARATOR, strings::PTTID_BEGIN, strings::PTTID_END, strings::PTTID_BOTH).c_str(),
		util::format("%s, %sn, %sn", strings::OPTSIG_OFF, strings::OPTSIG_DTMF, strings::OPTSIG_5TONE).c_str(),
		util::format("%s, %s", strings::NO, strings::YES).c_str(),
		util::format("%s, %s", strings::NO, strings::YES).c_str(),
		util::format("%s, %s", strings::NO, strings::YES).c_str(),
		util::format("%s, %s, %s", strings::SPACING_125, strings::SPACING_20, strings::SPACING_25).c_str(),
		"",
		NULL);
}

void applet::CExport::outputChannels(CTextFile &tf, const std::vector<uint8_t> &data)
{
	for (unsigned i(0); i < NUM_CHANNELS; ++i)
	{
		const unsigned chanNo(i + 1);
		logd("Decoding channel %u", chanNo);

		const SChannel *chan((const SChannel *) &data[i * 32]);
		debugDumpChannel(chan);

		if (!memcmp(chan->chname, "\x20\x20\x20\x20\x20", sizeof(chan->chname)) || !memcmp(chan->chname, "\xff\xff\xff\xff\xff", sizeof(chan->chname)))
		{
			logd("Channel %u is empty", chanNo);
			tf.add(2, "channel", util::format("%u", chanNo).c_str());
			continue;
		}

		std::vector<std::string> v;

		// xxx fix this chanNo mess with instance-wide state (like in appletimport)
		v.push_back(strings::CHANNEL);
		v.push_back(util::format("%u", chanNo));
		v.push_back(util::toPrintable(std::string((const char *) chan->chname, 5)).c_str());
		v.push_back(getFreq(chanNo, chan->rxfreq, false));
		v.push_back(getShift(chanNo, chan->flags1.shiftdir, chan->txshift));
		v.push_back(getDecoder(chanNo, chan->flags3.rxdcs, chan->flags3.rxcts));
		v.push_back(getCts(chanNo, chan->rxcts));
		v.push_back(getDcs(chanNo, chan->rxdcs, chan->rxdcsfl));
		v.push_back(getDecoder(chanNo, chan->flags3.txdcs, chan->flags3.txcts));
		v.push_back(getCts(chanNo, chan->txcts));
		v.push_back(getDcs(chanNo, chan->txdcs, chan->txdcsfl));
		v.push_back(getSquelchMode(chanNo, chan->sql));
		v.push_back(getTxPower(chanNo, chan->flags2.txoff, chan->flags1.txpwr));
		v.push_back(getBcl(chanNo, chan->bcl));
		v.push_back(getPttId(chanNo, chan->pttid));
		v.push_back(getOptSig(chanNo, chan->optsig, chan->flags3.dtmf));
		v.push_back((data[SCANNING_OFFSET + i / 8] & (1 << (i % 8))) ? strings::YES : strings::NO);	// scanning
		v.push_back(chan->flags1.talkaround ? strings::YES : strings::NO);
		v.push_back(chan->flags2.reverse ? strings::YES : strings::NO);
		v.push_back(getSpacing(chanNo, chan->flags2.spacing));
		v.push_back(getDefCts(chanNo, chan->defcts));
		tf.add(v);
	}
}

void applet::CExport::debugDumpChannel(const SChannel *chan)
{
	logd("Dumping channel data");
	logdump("Channel", chan, sizeof(*chan));

	logdump("RX Freq", chan->rxfreq, sizeof(chan->rxfreq));
	logdump("TX Shift", chan->txshift, sizeof(chan->rxfreq));
	logd("Dunno 1: 0x%02x", chan->dunno1);
	logd("Flags 1: talkaround=%d, dunno=0x%02x, txpower=%d, shiftdir=%d",
		chan->flags1.talkaround, chan->flags1.dunno, chan->flags1.txpwr, chan->flags1.shiftdir);
	logd("Flags 2: dunno=0x%02x, spacing=%d, reverse=%d, txoff=%d",
		chan->flags2.dunno, chan->flags2.spacing, chan->flags2.reverse, chan->flags2.txoff);
	logd("Flags 3: dtmf=%d, rxdcs=%d, rxcts=%d, txdcs=%d, txcts=%d",
		chan->flags3.dtmf, chan->flags3.rxdcs, chan->flags3.rxcts, chan->flags3.txdcs, chan->flags3.txcts);
	logd("rxcts: 0x%02x, txcts: 0x%02x", chan->rxcts, chan->txcts);
	logd("rxdcs: 0x%02x (flags: dunno=%d, invert=%d, msb=%d), txdcs: 0x%02x (flags: dunno=%d, invert=%d, msb=%d)",
		chan->rxdcs, chan->rxdcsfl.dunno, chan->rxdcsfl.invert, chan->rxdcsfl.msb,
		chan->txdcs, chan->txdcsfl.dunno, chan->txdcsfl.invert, chan->txdcsfl.msb);
	logd("bcl: 0x%02x, pttid: 0x%02x, sql: 0x%02x, optsig: 0x%02x",
		chan->bcl, chan->pttid, chan->sql, chan->optsig);
	logd("dunno2: 0x%02x, dunno3: 0x%02x, dunno4: 0x%02x",
		chan->dunno2, chan->dunno3, chan->dunno4);
	logdump("Channel name", chan->chname, sizeof(chan->chname));
	logdump("Def CTS", chan->defcts, sizeof(chan->defcts));
}

std::string applet::CExport::getShift(unsigned chanNo, SChannel::SFlags1::EShiftDir dir, const uint8_t *freq)
{
	switch (dir)
	{
		case SChannel::SFlags1::SHIFT_DN:
			return std::string("-") + getFreq(chanNo, freq, true);

		case SChannel::SFlags1::SHIFT_UP:
			return getFreq(chanNo, freq, true);

		default:
			break;
	}

	return "0";
}

std::string applet::CExport::getModel(const uint8_t *p)
{
	std::string rs;

	for (int i(0); i < 11; ++i)
	{
		if (p[i] >= 0x20 && p[i] < 0x7F)
			rs.push_back(p[i]);
		else
			rs += util::format("<%02x>", p[i]);
	}

	return rs;
}

std::string applet::CExport::getDecoder(unsigned chanNo, bool dcs, bool cts)
{
	if (!dcs && !cts)
		return strings::ENCDEC_NONE;

	if (!dcs && cts)
		return strings::ENCDEC_CTCSS;

	if (dcs && !cts)
		return strings::ENCDEC_DCS;

	loge("Warning: channel %u: both CTCSS and DCS set, assuming none", chanNo);
	return strings::ENCDEC_NONE;
}

std::string applet::CExport::getFreq(unsigned chanNo, const uint8_t *freq, bool condensed)
{
	// freq is in BCD format
	// 43 93 50 00 = 439.35000
	// 00 76 00 00 for 7.60000 MHz

	std::string bcd;

	for (int i(0); i < 4; ++i)
	{
		const uint8_t hi(freq[i] >> 4);
		const uint8_t lo(freq[i] & 0x0f);

		if (hi > 9 || lo > 9)
		{
			loge("Warning: channel %u: could not decode frequency (byte %u is 0x%02x)", chanNo, i, freq[i]);
			return "";
		}

		bcd += util::format("%u%u", hi, lo);
	}

	std::string dec(bcd.substr(0, 3));
	std::string frac(bcd.substr(3, 5));

	if (condensed)
	{
		for (int i(0); i < 2; ++i)
		{
			if (dec[0] == '0')
				dec.erase(dec.begin());
			else
				break;
		}

		for (int i(0); i < 5; ++i)
		{
			if (frac[frac.size() - 1] == '0')
				frac.erase(frac.end() - 1);
			else
				break;
		}

		if (frac.empty())
			return dec;
	}

	return dec + "." + frac;
}

std::string applet::CExport::getCts(unsigned chanNo, uint8_t cts)
{
	size_t idx(0);
	if (cts >= sizeof(CTSTBL) / sizeof(CTSTBL[0]))
		loge("Warning: channel %u: could not decode CTCSS (0x%02x), using lowest possible CTCSS", chanNo, cts);
	else
		idx = cts;

	return CTSTBL[idx];
}

std::string applet::CExport::getDcs(unsigned /* chanNo */, uint8_t dcs, SChannel::SDcsFlags flags)
{
	std::string rs;

	if (flags.invert)
		rs.push_back(strings::DCS_INVERT_FLAG);

	rs += util::format("%03o", dcs | (flags.msb ? 0x100 : 0));
	return rs;
}

std::string applet::CExport::getSquelchMode(unsigned chanNo, SChannel::ESql sql)
{
	switch (sql)
	{
		case SChannel::SQL_CARRIER:
			return strings::SQL_CARRIER;

		case SChannel::SQL_CTSDCS:
			return strings::SQL_CTSDCS;

		case SChannel::SQL_OPTSIG:
			return strings::SQL_OPTSIG;

		default:
			break;
	}

	loge("Warning: channel %u: could not decode Squelch Mode (0x%02x), setting to carrier", chanNo, sql);
	return strings::SQL_CARRIER;
}

std::string applet::CExport::getTxPower(unsigned chanNo, bool txoff, SChannel::SFlags1::ETxPwr pwr)
{
	if (txoff)
		return strings::TXP_OFF;

	switch (pwr)
	{
		case SChannel::SFlags1::TXPWR_LOW:
			return strings::TXP_LOW;

		case SChannel::SFlags1::TXPWR_MED:
			return strings::TXP_MEDIUM;

		case SChannel::SFlags1::TXPWR_HIGH:
			return strings::TXP_HIGH;

		default:
			break;
	}

	loge("Warning: channel %u: could not decode TX power (%u), setting to low", chanNo, pwr);
	return strings::TXP_LOW;
}

std::string applet::CExport::getBcl(unsigned chanNo, SChannel::EBcl bcl)
{
	switch (bcl)
	{
		case SChannel::BCL_OFF:
			return strings::BCL_OFF;

		case SChannel::BCL_RPT:
			return strings::BCL_RPT;

		case SChannel::BCL_BUSY:
			return strings::BCL_BUSY;

		default:
			break;
	}

	loge("Warning: channel %u: could not decode Busy Channel Lockout (0x%02x), setting to off", chanNo, bcl);
	return strings::BCL_OFF;
}

std::string applet::CExport::getPttId(unsigned chanNo, SChannel::EPttId pttid)
{
	switch (pttid)
	{
		case SChannel::PTTID_OFF:
			return strings::PTTID_OFF;

		case SChannel::PTTID_DTMF_BEGIN:
			return util::format("%s%c%s", strings::PTTID_DTMF, strings::PTTID_SEPARATOR, strings::PTTID_BEGIN);

		case SChannel::PTTID_DTMF_END:
			return util::format("%s%c%s", strings::PTTID_DTMF, strings::PTTID_SEPARATOR, strings::PTTID_END);

		case SChannel::PTTID_DTMF_BOTH:
			return util::format("%s%c%s", strings::PTTID_DTMF, strings::PTTID_SEPARATOR, strings::PTTID_BOTH);

		case SChannel::PTTID_5TONE_BEGIN:
			return util::format("%s%c%s", strings::PTTID_5TONE, strings::PTTID_SEPARATOR, strings::PTTID_BEGIN);

		case SChannel::PTTID_5TONE_END:
			return util::format("%s%c%s", strings::PTTID_5TONE, strings::PTTID_SEPARATOR, strings::PTTID_END);

		case SChannel::PTTID_5TONE_BOTH:
			return util::format("%s%c%s", strings::PTTID_5TONE, strings::PTTID_SEPARATOR, strings::PTTID_BOTH);

		default:
			break;
	}

	loge("Warning: channel %u: could not decode PTT ID (0x%02x), setting to off", chanNo, pttid);
	return strings::PTTID_OFF;
}

std::string applet::CExport::getOptSig(unsigned chanNo, SChannel::EOptSig optsig, uint8_t dtmf)
{
	switch (optsig)
	{
		case SChannel::OPTSIG_OFF:
			return strings::OPTSIG_OFF;

		case SChannel::OPTSIG_DTMF:
			return util::format("%s%u", strings::OPTSIG_DTMF, dtmf + 1);

		case SChannel::OPTSIG_5TONE:
			// xxx not sure if 5tone setting is really in dtmf field
			return util::format("%s%u", strings::OPTSIG_5TONE, dtmf);

		default:
			break;
	}

	loge("Warning: channel %u: could not decode Optional Signaling (0x%02x), setting to off", chanNo, optsig);
	return strings::OPTSIG_OFF;
}

std::string applet::CExport::getSpacing(unsigned chanNo, SChannel::SFlags2::ESpacing spacing)
{
	switch (spacing)
	{
		case SChannel::SFlags2::SP_125:
			return strings::SPACING_125;

		case SChannel::SFlags2::SP_20:
			return strings::SPACING_20;

		case SChannel::SFlags2::SP_25:
			return strings::SPACING_25;

		default:
			break;
	}

	loge("Warning: channel %u: could not decode Channel Spacing (0x%02x), setting to 12.5 kHz", chanNo, spacing);
	return strings::SPACING_125;
}

std::string applet::CExport::getDefCts(unsigned /* chanNo */, const uint8_t *defCts)
{
	const uint16_t value(defCts[0] | ((uint16_t) defCts[1] << 8));
	return util::format("%u.%u", value / 10, value % 10);
}
