/**
 * \brief	File descriptor
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#pragma once

class CFd
{
public:
	CFd();
	CFd(int fd);
	~CFd();

	CFd &operator=(int fd);
	int release();
	operator int() const;

private:
	CFd(const CFd &);
	CFd &operator=(const CFd &);

	int m_fd;
};
