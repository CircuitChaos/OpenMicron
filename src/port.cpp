/**
 * \brief	Comm port
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "port.h"
#include "log.h"
#include "fd.h"
#include "throw.h"

CPort::CPort(const std::string &devpath, unsigned timeout): m_timeout(timeout)
{
	logd("Opening port %s", devpath.c_str());

	CFd fd(open(devpath.c_str(), O_RDWR | O_NOCTTY));
	if (fd == -1)
	{
		loge("Cannot open device: %s: %m", devpath.c_str());
		return;
	}

	struct termios t;
	if (tcgetattr(fd, &t) == -1)
	{
		loge("Cannot get port attributes: %m");
		return;
	}

	t.c_iflag = 0;
	t.c_oflag = 0;
	t.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
	t.c_cflag &= ~(CSIZE | PARENB);
	t.c_cflag |= CS8;

	t.c_cc[VMIN]  = 1;
	t.c_cc[VTIME] = 0;

	if (cfsetispeed(&t, B9600) == -1)
	{
		loge("Cannot set input speed in termios struct: %m");
		return;
	}

	if (cfsetospeed(&t, B9600) == -1)
	{
		loge("Cannot set output speed in termios struct: %m");
		return;
	}

	if (tcsetattr(fd, TCSAFLUSH, &t) == -1)
	{
		loge("Cannot set port attributes: %m");
		return;
	}

	m_fd = fd.release();
	logd("Port %s opened with fd %d", devpath.c_str(), (int) m_fd);
}

CPort::~CPort()
{
	if (m_fd == -1)
		return;

	logd("Draining port");
	tcdrain(m_fd);
}

bool CPort::isOpen() const
{
	return m_fd != -1;
}

bool CPort::read(void *data, size_t size)
{
	char *p((char *) data);
	unsigned rem(size);

	while (rem)
	{
		fd_set rfd;

		FD_ZERO(&rfd);
		FD_SET(m_fd, &rfd);

		struct timeval tv;

		tv.tv_sec = m_timeout;
		tv.tv_usec = 0;

		int selrs(select(m_fd + 1, &rfd, 0, 0, &tv));
		if (selrs == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;

			loge("Port select error: %m");
			return false;
		}

		if (selrs == 0)
		{
			loge("Radio not responding");
			if (rem != size)
			{
				logd("Dumping data read so far");
				logdump("<<", data, size - rem);
			}
			return false;
		}

		const int rs(::read(m_fd, p, rem));

		if (!rs)
		{
			loge("EOF reading from device (radio disconnected?)");
			if (rem != size)
			{
				logd("Dumping data read so far");
				logdump("<<", data, size - rem);
			}
			return false;
		}

		if (rs == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;

			loge("Port read error: %m");
			if (rem != size)
			{
				logd("Dumping data read so far");
				logdump("<<", data, size - rem);
			}
			return false;
		}

		xassert((unsigned) rs <= rem, "read() on port returned nonsense");

		p += rs;
		rem -= rs;
	}

	logdump("<<", data, size);
	return true;
}

bool CPort::write(const void *data, size_t size)
{
	// it seems to be needed as without it I'm getting random "radio not responding" errors...
	usleep(5000);

	logdump(">>", data, size);

	const char *p((const char *) data);
	unsigned rem(size);

	while (rem)
	{
		const int rs(::write(m_fd, p, rem));

		if (rs == -1)
		{
			if (errno == EAGAIN || errno == EINTR)
				continue;

			loge("Port write() error: %m");
			return false;
		}

		xassert(rs && (unsigned) rs <= rem, "write() on port returned nonsense");
		p += rs;
		rem -= rs;
	}

	if (tcdrain(m_fd) == -1)
		logn("Port tcdrain error (not fatal, happens on Cygwin): %m");

	return true;
}
