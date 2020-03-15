/**
 * \brief	Applet to write data to the radio
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <memory>
#include <cstring>
#include "appletwrite.h"
#include "cliwrite.h"
#include "config.h"
#include "log.h"
#include "port.h"
#include "throw.h"
#include "protocol.h"
#include "omifile.h"
#include "util.h"

bool applet::CWrite::run(int argc, char * const argv[])
{
	cli::CWrite cli;
	if (!cli.parse(argc, argv))
		return false;

	COmiFile of;
	if (!of.read(cli.getFile()))
		return false;

	std::auto_ptr<COmiFile> rf;
	if (!cli.getRefFile().empty())
	{
		rf.reset(new COmiFile());
		if (!rf->read(cli.getRefFile()))
			return false;

		if (of.getOffset() != rf->getOffset() || of.getModel() != rf->getModel() || of.getData().size() != rf->getData().size())
		{
			loge("Reference file does not match input file");
			return false;
		}

		if (of.getData() == rf->getData())
		{
			loge("Data in files is identical, wouldn't write anything to radio");
			return false;
		}
	}

	if (of.getOffset() != 0)
	{
		loge("Non-zero offsets not supported; update this utility");
		return false;
	}

	if (of.getData().empty())
	{
		loge("Empty data in input file");
		return false;
	}

	if (of.getData().size() % config::PACKET_SIZE)
	{
		loge("Data size is not a multiple of packet size; update this utility");
		return false;
	}

	xassert(of.getData().size() <= UINT16_MAX, "Vector too large, should not happen with 16-bit size in .omi header");

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
	if (of.getModel() != (model.size() > config::MAX_MODEL_SIZE ? model.substr(0, config::MAX_MODEL_SIZE) : model))
	{
		loge("Radio model mismatch");
		loge("Model in file: %s", util::toPrintable(of.getModel()).c_str());
		loge("Model read from radio: %s", util::toPrintable(model).c_str());
		return false;
	}

	std::vector<uint8_t> &data(of.getData());
	const uint16_t size(of.getData().size());
	for (uint16_t ofs(0); ofs < size; ofs += config::PACKET_SIZE)
	{
		if (rf.get())
		{
			if (!memcmp(&data[ofs], &rf->getData()[ofs], config::PACKET_SIZE))
			{
				logd("Data at offset 0x%04x did not change; not writing", ofs);
				continue;
			}
		}

		logi("Writing offset 0x%04x of 0x%04x (%u%%)", ofs, size, ofs * 100 / size);
		if (!protocol::write(port, &data[ofs], ofs, config::PACKET_SIZE))
		{
			loge("Protocol error during write (offset 0x%04x)", ofs);
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

	return true;
}
