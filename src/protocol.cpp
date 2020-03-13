/**
 * \brief	Comm protocol
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <vector>
#include <string>
#include <cstring>
#include "protocol.h"
#include "throw.h"
#include "log.h"
#include "config.h"

static void logIssue()
{
	loge("If it persists:");
	loge("- Check if you have newest version");
	loge("- If yes, re-run program with -d and file a bug report");
	loge("  (remember to specify radio model)");
}

static bool exchange(CPort &port, const std::vector<uint8_t> &v)
{
	xassert(!v.empty(), "Trying to send empty vector");
	if (!port.write(&v[0], v.size()))
	{
		loge("Port write error");
		return false;
	}

	std::vector<uint8_t> rv;
	rv.resize(v.size());
	if (!port.read(&rv[0], rv.size()))
	{
		loge("Port read error during echo read");
		return false;
	}

	if (v != rv)
	{
		loge("Echo did not match sent data, check cable");
		return false;
	}

	return true;
}

static bool exchange(CPort &port, const std::string &s)
{
	const std::vector<uint8_t> v(s.begin(), s.end());
	return exchange(port, v);
}

bool protocol::handshake(CPort &port, std::string &model)
{
	if (!exchange(port, "PROGRAM"))
		return false;

	char qx[3];
	if (!port.read(qx, sizeof(qx)))
		return false;

	if (memcmp(qx, "QX\x06", sizeof(qx)))
	{
		loge("Radio returned unrecognized data");
		logIssue();
		return false;
	}

	if (!exchange(port, "\x02"))
		return false;

	model.clear();
	// we don't know the size, so we need to read until we receive ACK
	// or until we run out of space
	for (;;)
	{
		uint8_t ch;
		if (!port.read(&ch, sizeof(ch)))
			return false;

		if (ch == 0x06)
			break;

		if (model.size() > config::MAX_MODEL_SIZE)
		{
			loge("Model size too large");
			logIssue();
			return false;
		}

		model.push_back(ch);
	}

	return true;
}

bool protocol::read(CPort &port, uint8_t *data, uint16_t offset, uint8_t size)
{
	std::vector<uint8_t> req;

	req.push_back('R');
	req.push_back(offset >> 8);
	req.push_back(offset & 0xff);
	req.push_back(size);

	if (!exchange(port, req))
		return false;

	std::vector<uint8_t> rsp;
	rsp.resize(size + 6);
	if (!port.read(&rsp[0], rsp.size()))
		return false;

	if (rsp[0] != 'W' || rsp[1] != req[1] || rsp[2] != req[2] || rsp[3] != req[3] || rsp[rsp.size() - 1] != 0x06)
	{
		loge("Invalid response from radio to read packet");
		logIssue();
		return false;
	}

	uint8_t checksum(0);
	for (size_t i(1); i < rsp.size() - 2; ++i)
		checksum += rsp[i];

	if (rsp[rsp.size() - 2] != checksum)
	{
		loge("Checksum error in read packet");
		logIssue();
		return false;
	}

	memcpy(data, &rsp[4], size);
	return true;
}

bool protocol::write(CPort &port, const uint8_t *data, uint16_t offset, uint8_t size)
{
	xassert(size != 0, "Refusing to write empty buffer");

	std::vector<uint8_t> req;
	req.resize(size + 6);

	req[0] = 'W';
	req[1] = offset >> 8;
	req[2] = offset & 0xff;
	req[3] = size;
	req[size + 4] = req[1] + req[2] + req[3];
	req[size + 5] = 0x06;

	for (size_t i(0); i < size; ++i)
	{
		req[i + 4] = data[i];
		req[size + 4] += data[i];
	}

	if (!exchange(port, req))
		return false;

	uint8_t ack;
	if (!port.read(&ack, sizeof(ack)))
		return false;

	if (ack != 0x06)
	{
		loge("Radio did not acknowledge write packet correctly");
		return false;
	}

	return true;
}

bool protocol::end(CPort &port)
{
	if (!exchange(port, "END"))
		return false;

	uint8_t ack;
	if (!port.read(&ack, sizeof(ack)))
		return false;

	if (ack != 0x06)
	{
		loge("Radio did not acknowledge termination correctly");
		return false;
	}

	return true;
}
