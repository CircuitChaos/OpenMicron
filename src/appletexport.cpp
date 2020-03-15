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

	tf.add(2, impexp::WELCOME, util::toPrintable(std::string((const char *) &infile.getData()[0x1980], 7)).c_str());
	tf.add(0);
	tf.add(-1, impexp::COMMENT,
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

	tf.add(-1, impexp::COMMENT,
		"",
		"5 chr max",
		"",
		"",
		"none, CTCSS, DCS",
		"can be 'defined'",
		"start with 'i' to invert",
		"none, CTCSS, DCS",
		"can be 'defined'",
		"start with 'i' to invert",
		"carrier, ctsdcs, optsig",
		"off, low, med, high",
		"off, rpt, busy",
		"off, dtmf/5tone:begin/end/both",
		"off, dtmf:Mn, 5tone:n",
		"no, yes",
		"no, yes",
		"no, yes",
		"12.5, 20, 25",
		"",
		NULL);

	for (unsigned i(0); i < config::NUM_CHANNELS; ++i)
	{
		logd("Decoding channel %u", i + 1);

		std::string tmp;
		const impexp::SChannel *chan((const impexp::SChannel *) &infile.getData()[i * 32]);

		logd("Dumping channel data");
		logdump("Channel", chan, sizeof(*chan));

		logdump("RX Freq", chan->rxfreq, sizeof(chan->rxfreq));
		logdump("TX Shift", chan->txshift, sizeof(chan->rxfreq));
		logd("Dunno 1: 0x%02x", chan->dunno1);
		logd("Flags 1: talkaround=%d, dunno=0x%02x, txpower=%d, shiftdir=%d",
			chan->flags1.talkaround, chan->flags1.dunno, chan->flags1.txpower, chan->flags1.shiftdir);
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

		if (!memcmp(chan->chname, "\x20\x20\x20\x20\x20", sizeof(chan->chname)) || !memcmp(chan->chname, "\xff\xff\xff\xff\xff", sizeof(chan->chname)))
		{
			// channel is empty
			tf.add(2, "channel", util::format("%u", i + 1).c_str());
			continue;
		}

		std::vector<std::string> v;

		v.push_back(impexp::CHANNEL);
		v.push_back(util::format("%u", i + 1));
		v.push_back(util::toPrintable(std::string((const char *) chan->chname, 5)).c_str());
		v.push_back(decodeFreq(chan->rxfreq, false));

		switch (chan->flags1.shiftdir)
		{
			case impexp::SChannel::SFlags1::SHIFT_DN:
				v.push_back(std::string("-") + decodeFreq(chan->txshift, true));
				break;

			case impexp::SChannel::SFlags1::SHIFT_UP:
				v.push_back(decodeFreq(chan->txshift, true));
				break;

			default:
				v.push_back("0");
				break;
		}

		// RX Decoder
		tmp = decodeEncDec(chan->flags3.rxdcs, chan->flags3.rxcts);
		if (tmp.empty())
		{
			logn("Warning: Could not decode RX Decoder flags for channel %u, setting to none", i);
			tmp = "none";
		}
		v.push_back(tmp);

		v.push_back(decodeCts(chan->rxcts));
		v.push_back(decodeDcs(chan->rxdcs, chan->rxdcsfl));

		// TX Encoder
		tmp = decodeEncDec(chan->flags3.txdcs, chan->flags3.txcts);
		if (tmp.empty())
		{
			logn("Warning: Could not decode TX Encoder flags for channel %u, setting to none", i);
			tmp = "none";
		}
		v.push_back(tmp);

		v.push_back(decodeCts(chan->txcts));
		v.push_back(decodeDcs(chan->txdcs, chan->txdcsfl));

		// Squelch Mode
		switch (chan->sql)
		{
			case impexp::SChannel::SQL_CARRIER:
				v.push_back("carrier");
				break;

			case impexp::SChannel::SQL_CTSDCS:
				v.push_back("ctsdcs");
				break;

			case impexp::SChannel::SQL_OPTSIG:
				v.push_back("optsig");
				break;

			default:
				logn("Warning: Could not decode Squelch Mode for channel %u (0x%02x), setting to carrier", i, chan->sql);
				v.push_back("carrier");
				break;
		}

		// TX Power
		if (chan->flags2.txoff)
			v.push_back("off");
		else switch (chan->flags1.txpower)
		{
			case impexp::SChannel::SFlags1::TXPWR_LOW:
				v.push_back("low");
				break;

			case impexp::SChannel::SFlags1::TXPWR_MED:
				v.push_back("med");
				break;

			case impexp::SChannel::SFlags1::TXPWR_HIGH:
				v.push_back("high");
				break;

			default:
				logn("Warning: Could not decode TX power (%u) for channel %u, setting to low", chan->flags1.txpower, i);
				v.push_back("low");
				break;
		}

		// Busy Channel Lockout
		switch (chan->bcl)
		{
			case impexp::SChannel::BCL_OFF:
				v.push_back("off");
				break;

			case impexp::SChannel::BCL_RPT:
				v.push_back("rpt");
				break;

			case impexp::SChannel::BCL_BUSY:
				v.push_back("busy");
				break;

			default:
				logn("Warning: Could not decode Busy Channel Lockout for channel %u (0x%02x), setting to off", i, chan->bcl);
				v.push_back("off");
				break;
		}

		// PTT ID
		switch (chan->pttid)
		{
			case impexp::SChannel::PTTID_OFF:
				v.push_back("off");
				break;

			case impexp::SChannel::PTTID_DTMF_BEGIN:
				v.push_back("dtmf:begin");
				break;

			case impexp::SChannel::PTTID_DTMF_END:
				v.push_back("dtmf:end");
				break;

			case impexp::SChannel::PTTID_DTMF_BOTH:
				v.push_back("dtmf:both");
				break;

			case impexp::SChannel::PTTID_5TONE_BEGIN:
				v.push_back("5tone:begin");
				break;

			case impexp::SChannel::PTTID_5TONE_END:
				v.push_back("5tone:end");
				break;

			case impexp::SChannel::PTTID_5TONE_BOTH:
				v.push_back("5tone:both");
				break;

			default:
				logn("Warning: Could not decode PTT ID for channel %u (0x%02x), setting to off", i, chan->pttid);
				v.push_back("off");
				break;
		}

		switch (chan->optsig)
		{
			case impexp::SChannel::OPTSIG_OFF:
				v.push_back("off");
				break;

			case impexp::SChannel::OPTSIG_DTMF:
				v.push_back(std::string("dtmf:M") + util::format("%u", chan->flags3.dtmf + 1));
				break;

			case impexp::SChannel::OPTSIG_5TONE:
				v.push_back(std::string("5tone:") + util::format("%u", chan->flags3.dtmf));
				break;

			default:
				logn("Warning: Could not decode Optional Signaling for channel %u (0x%02x), setting to off", i, chan->optsig);
				v.push_back("off");
				break;
		}

		// Scanning
		v.push_back((infile.getData()[0x1960 + i / 8] & (1 << (i % 8))) ? "yes" : "no");
		v.push_back(chan->flags1.talkaround ? "yes" : "no");
		v.push_back(chan->flags2.reverse ? "yes" : "no");

		switch (chan->flags2.spacing)
		{
			case impexp::SChannel::SFlags2::SP_125:
				v.push_back("12.5");
				break;

			case impexp::SChannel::SFlags2::SP_20:
				v.push_back("20");
				break;

			case impexp::SChannel::SFlags2::SP_25:
				v.push_back("25");
				break;

			default:
				logn("Warning: Could not decode Channel Spacing for channel %u (0x%02x), setting to 12.5 kHz", i, chan->flags2.spacing);
				v.push_back("12.5");
				break;
		}

		v.push_back(decodeDefCts(chan->defcts));
		tf.add(v);
	}

	if (!tf.write(cli.getOutput(), cli.isText()))
		return false;

	return true;
}

std::string applet::CExport::decodeModel(const uint8_t *p)
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

std::string applet::CExport::decodeEncDec(bool dcs, bool cts)
{
	if (!dcs && !cts)
		return "none";

	if (!dcs && cts)
		return "CTCSS";

	if (dcs && !cts)
		return "DCS";

	return "";
}

std::string applet::CExport::decodeFreq(const uint8_t *freq, bool condensed)
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
			loge("Warning: Could not decode frequency (byte %u is 0x%02x)", i, freq[i]);
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

std::string applet::CExport::decodeCts(uint8_t cts)
{
	size_t idx(0);
	if (cts >= sizeof(impexp::CTSTBL) / sizeof(impexp::CTSTBL[0]))
		loge("Warning: Could not decode CTCSS (0x%02x), using lowest possible CTCSS", cts);
	else
		idx = cts;

	return impexp::CTSTBL[idx];
}

std::string applet::CExport::decodeDcs(uint8_t dcs, impexp::SChannel::SDcsFlags flags)
{
	std::string rs;

	if (flags.invert)
		rs += "i";

	rs += util::format("%03o", dcs | (flags.msb ? 0x100 : 0));
	return rs;
}

std::string applet::CExport::decodeDefCts(const uint8_t *defCts)
{
	const uint16_t value(defCts[0] | ((uint16_t) defCts[1] << 8));
	return util::format("%u.%u", value / 10, value % 10);
}
