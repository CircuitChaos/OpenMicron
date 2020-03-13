/**
 * \brief	Raw file reader and writer
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#pragma once

#include <string>
#include <cstdio>

class CRawReader
{
public:
	CRawReader(const std::string &path);
	~CRawReader();

	bool isOpen() const;
	bool operator()(void *p, size_t sz);

private:
	FILE *m_fp;
};

class CRawWriter
{
public:
	CRawWriter(const std::string &path);
	~CRawWriter();

	bool isOpen() const;
	bool operator()(const void *p, size_t sz);
	bool close();

private:
	const std::string m_path;
	FILE *m_fp;
};
