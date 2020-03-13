/**
 * \brief	Comm protocol
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#pragma once

#include <string>
#include "port.h"

namespace protocol
{
	bool handshake(CPort &port, std::string &model);
	bool read(CPort &port, uint8_t *data, uint16_t offset, uint8_t size);
	bool write(CPort &port, const uint8_t *data, uint16_t offset, uint8_t size);
	bool end(CPort &port);
}
