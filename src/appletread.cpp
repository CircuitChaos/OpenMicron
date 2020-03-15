/**
 * \brief	Applet to read data from the radio
 * \author	Circuit Chaos
 * \date	2020-03-12
 */

#include "appletread.h"
#include "cliread.h"
#include "config.h"
#include "log.h"
#include "port.h"
#include "protocol.h"
#include "omifile.h"
#include "util.h"

bool applet::CRead::run(int argc, char * const argv[])
{
	cli::CRead cli;
	if (!cli.parse(argc, argv))
		return false;

	CPort port(cli.getPort(), config::PORT_TIMEOUT);
	if (!port.isOpen())
	{
		loge("Error opening communication port");
		return false;
	}

	std::string model;
	if (!protocol::handshake(port, model))
	{
		loge("Protocol error during handshake");
		return false;
	}

	logn("Radio ID string: %s", util::toPrintable(model).c_str());

	COmiFile of;
	of.setOffset(0);
	of.setModel(model);

	const uint16_t size(cli.isFull() ? config::FULL_MEMORY_SIZE : config::CHAN_MEMORY_SIZE);

	std::vector<uint8_t> &data(of.getData());
	data.resize(size);
	for (uint16_t ofs(0); ofs < size; ofs += config::PACKET_SIZE)
	{
		logi("Reading offset 0x%04x of 0x%04x (%u%%)", ofs, size, ofs * 100 / size);
		if (!protocol::read(port, &data[ofs], ofs, config::PACKET_SIZE))
		{
			loge("Protocol error during read (offset 0x%04x)", ofs);
			if (!protocol::end(port))
				loge("Additional error while trying to terminate session");
			return false;
		}
	}

	if (!protocol::end(port))
	{
		loge("Protocol error during termination");
		return false;
	}

	if (!of.write(cli.getFile()))
		return false;

	return true;
}
