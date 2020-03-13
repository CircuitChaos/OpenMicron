/**
 * \brief	Raw file reader and writer
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <unistd.h>
#include "rawfile.h"
#include "throw.h"
#include "log.h"

CRawReader::CRawReader(const std::string &path)
{
	logd("Opening file %s for reading", path.c_str());
	m_fp = fopen(path.c_str(), "rb");
	if (!m_fp)
		loge("Cannot open file %s for reading: %m", path.c_str());
}

CRawReader::~CRawReader()
{
	if (m_fp)
		fclose(m_fp);
}

bool CRawReader::isOpen() const
{
	return m_fp;
}

bool CRawReader::operator()(void *p, size_t sz)
{
	xassert(isOpen(), "Attempted read from closed file");
	return fread(p, sz, 1, m_fp) == 1;
}

CRawWriter::CRawWriter(const std::string &path): m_path(path)
{
	logd("Opening file %s.tmp for writing", m_path.c_str());
	m_fp = fopen((m_path + ".tmp").c_str(), "wb");
	if (!m_fp)
		loge("Cannot open file %s.tmp for writing: %m", m_path.c_str());
}

CRawWriter::~CRawWriter()
{
	if (m_fp)
	{
		logd("Writer got out of scope, removing file %s.tmp", m_path.c_str());
		fclose(m_fp);
		unlink((m_path + ".tmp").c_str());
	}
}

bool CRawWriter::isOpen() const
{
	return m_fp;
}

bool CRawWriter::operator()(const void *p, size_t sz)
{
	xassert(isOpen(), "Attempted write to closed file");
	return fwrite(p, sz, 1, m_fp) == 1;
}

bool CRawWriter::close()
{
	logd("Closing file %s.tmp and renaming to %s", m_path.c_str(), m_path.c_str());
	xassert(isOpen(), "Attempted closing of closed file");
	fclose(m_fp);
	m_fp = NULL;
	unlink(m_path.c_str());
	if (rename((m_path + ".tmp").c_str(), m_path.c_str()) != 0)
	{
		logd("Cannot rename file %s.tmp: %m", m_path.c_str());
		return false;
	}

	return true;
}
