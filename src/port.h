/**
 * \brief	Comm port
 * \author	Circuit Chaos
 * \date	2020-03-13
 *
 * Parameters are fixed to 9600 8N1.
 */

#pragma once

#include <string>
#include <inttypes.h>
#include "fd.h"

class CPort
{
public:
	CPort(const std::string &devpath, unsigned timeout);
	~CPort();

	bool isOpen() const;
	bool read(void *data, size_t size);
	bool write(const void *data, size_t size);

private:
	const unsigned m_timeout;
	CFd m_fd;
};
