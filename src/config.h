/**
 * \brief	Compile-time configuration
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#pragma once

namespace config
{
	static const unsigned HEXDUMP_ROWS	= 16;

	static const char DFL_PORT[]		= "/dev/ttyUSB0";

	// in seconds
	static const unsigned PORT_TIMEOUT	= 5;

	// currently fixed; might be changed to argument
	// for now, ..._MEMORY_SIZE must be multiple of PACKET_SIZE
	static const unsigned PACKET_SIZE	= 0x10;

	// FULL_MEMORY_SIZE is number of bytes read by the original utility
	static const unsigned MEMORY_SIZE	= 0x4000;
	// static const unsigned CHAN_MEMORY_SIZE	= 0x1990;

	// this is the theoretical size of model name that radio can
	// send (it's truncated to 16 bytes in .omi file anyway). any
	// data longer than this will trigger protocol error.
	static const unsigned MAX_MODEL_SIZE	= 64;
}
