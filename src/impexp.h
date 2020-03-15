/**
 * \brief	Common import and export constants and structures
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#pragma once

#include <string>
#include <inttypes.h>

namespace impexp
{
	static const char COMMENT[]	= "comment";
	static const char WELCOME[]	= "welcome message";
	static const char CHANNEL[]	= "channel";

	static const char * const CTSTBL[] =
	{
		"62.5",		// 0x00
		"67.0", 	// 0x01
		"69.3",		// 0x02
		"71.9",		// 0x03
		"74.4",		// 0x04
		"77.0",		// 0x05
		"79.7",		// 0x06
		"82.5",		// 0x07
		"85.4",		// 0x08
		"88.5",		// 0x09
		"91.5",		// 0x0a
		"94.8",		// 0x0b
		"97.4",		// 0x0c
		"100.0",	// 0x0d
		"103.5",	// 0x0e
		"107.2",	// 0x0f
		"110.9",	// 0x10
		"114.8",	// 0x11
		"118.0",	// 0x12
		"123.0",	// 0x13
		"127.3",	// 0x14
		"131.8",	// 0x15
		"136.5",	// 0x16
		"141.3",	// 0x17
		"146.2",	// 0x18
		"151.4",	// 0x19
		"156.7",	// 0x1a
		"159.8",	// 0x1b
		"162.2",	// 0x1c
		"165.5",	// 0x1d
		"167.9",	// 0x1e
		"171.3",	// 0x1f
		"173.8",	// 0x20
		"177.3",	// 0x21
		"179.9",	// 0x22
		"183.5",	// 0x23
		"186.2",	// 0x24
		"189.9",	// 0x25
		"192.8",	// 0x26
		"196.6",	// 0x27
		"199.5",	// 0x28
		"203.5",	// 0x29
		"206.5",	// 0x2a
		"210.7",	// 0x2b
		"218.1",	// 0x2c
		"225.7",	// 0x2d
		"229.1",	// 0x2e
		"233.6",	// 0x2f
		"241.8",	// 0x30
		"250.3",	// 0x31
		"254.1",	// 0x32
		"defined",	// 0x33
	};

	struct SChannel
	{
		struct SFlags1
		{
			enum ETxPwr
			{
				TXPWR_LOW = 0,
				TXPWR_MED = 1,
				TXPWR_HIGH = 2,
			};

			enum EShiftDir
			{
				SHIFT_OFF = 0,
				SHIFT_UP = 1,
				SHIFT_DN = 2,
			};

			EShiftDir shiftdir:2;
			uint8_t txpower:2;
			uint8_t dunno:3;
			bool talkaround:1;
		} __attribute__((packed));

		struct SFlags2
		{
			enum ESpacing
			{
				SP_125 = 0,
				SP_20 = 1,
				SP_25 = 2,
			};

			bool txoff:1;
			bool reverse:1;
			ESpacing spacing:2;
			uint8_t dunno:4;
		} __attribute__((packed));

		struct SFlags3
		{
			bool txcts:1;
			bool txdcs:1;
			bool rxcts:1;
			bool rxdcs:1;
			uint8_t dtmf:4;
		} __attribute__((packed));

		struct SDcsFlags
		{
			bool msb:1;
			bool invert:1;
			uint8_t dunno:6;
		} __attribute__((packed));;

		enum ESql
		{
			SQL_CARRIER = 0,
			SQL_CTSDCS = 1,
			SQL_OPTSIG = 2,
		};

		enum EBcl
		{
			BCL_OFF = 0,
			BCL_RPT = 1,
			BCL_BUSY = 2,
		};

		enum EPttId
		{
			PTTID_OFF = 0,
			PTTID_DTMF_BEGIN = 1,
			PTTID_DTMF_END = 2,
			PTTID_DTMF_BOTH = 3,
			PTTID_5TONE_BEGIN = 0x10,
			PTTID_5TONE_END = 0x20,
			PTTID_5TONE_BOTH = 0x30,
		};

		enum EOptSig
		{
			OPTSIG_OFF = 0,
			OPTSIG_DTMF = 1,
			OPTSIG_5TONE = 3,
		};

		uint8_t rxfreq[4];
		uint8_t txshift[4];
		uint8_t dunno1;
		SFlags1 flags1;
		SFlags2 flags2;
		SFlags3 flags3;
		uint8_t rxcts;
		uint8_t txcts;
		uint8_t rxdcs;
		SDcsFlags rxdcsfl;
		uint8_t txdcs;
		SDcsFlags txdcsfl;
		uint8_t bcl;
		uint8_t pttid;
		uint8_t sql;
		uint8_t optsig;
		uint8_t dunno2;
		uint8_t dunno3;
		uint8_t dunno4;
		uint8_t chname[5];
		uint8_t defcts[2];
	} __attribute__((packed));
}
