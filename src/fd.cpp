/**
 * \brief	File descriptor
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <unistd.h>
#include "fd.h"

CFd::CFd(): m_fd(-1)
{
}

CFd::CFd(int fd): m_fd(fd)
{
}

CFd::~CFd()
{
	if (m_fd != -1)
		close(m_fd);
}

CFd &CFd::operator=(int fd)
{
	if (m_fd != -1)
		close(m_fd);

	m_fd = fd;
	return *this;
}

int CFd::release()
{
	const int fd(m_fd);
	m_fd = -1;
	return fd;
}

CFd::operator int() const
{
	return m_fd;
}
