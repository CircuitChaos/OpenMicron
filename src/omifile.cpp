/**
 * \brief	.omi file representation
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#include <cstring>
#include <endian.h>
#include "omifile.h"
#include "rawfile.h"
#include "throw.h"
#include "util.h"
#include "log.h"

#define xmin(a, b) ((a) < (b) ? (a) : (b))

void COmiFile::SHdr::fromHost()
{
	magic		= htobe32(magic);
	version		= htobe16(version);
	offset		= htobe16(offset);
	size		= htobe16(size);
	crc32		= htobe32(crc32);
}

void COmiFile::SHdr::toHost()
{
	magic		= be32toh(magic);
	version		= be16toh(version);
	offset		= be16toh(version);
	size		= be16toh(size);
	crc32		= be32toh(crc32);
}

uint32_t COmiFile::SHdr::crcToHost(uint32_t crc)
{
	return be32toh(crc);
}

COmiFile::COmiFile(): m_offset(0)
{
}

// returns "" if read successful, or error string
std::string COmiFile::read(const std::string &path)
{
	logd("About to read file %s", path.c_str());
	CRawReader r(path);
	if (!r.isOpen())
		return util::format("%s: open error", path.c_str());

	SHdr hdr;
	if (!r(&hdr, sizeof(hdr)))
		return util::format("%s: header read error", path.c_str());

	const uint32_t hdrCrc(SHdr::crcToHost(hdr.crc32));
	hdr.crc32 = 0;

	uint32_t calcCrc(util::crc32(0, &hdr, sizeof(hdr)));

	hdr.toHost();
	if (hdr.magic != MAGIC)
		return util::format("%s: invalid magic; maybe not an .omi file?", path.c_str());

	if (hdr.version != 1)
		return util::format("%s: invalid version; maybe written with newer utility?", path.c_str());

	logd("Data offset: %u (0x%04x) bytes", hdr.offset, hdr.offset);
	logd("Data size: %u (0x%04x) bytes", hdr.size, hdr.size);
	logd("Model size: %u (0x%02x) bytes", hdr.modelSize, hdr.modelSize);

	if (hdr.size == 0)
		return util::format("%s: file does not contain data", path.c_str());

	m_offset = hdr.offset;
	m_data.resize(hdr.size);
	m_model.assign((const char *) hdr.model, xmin(hdr.modelSize, sizeof(hdr.model)));

	if (!r(&m_data[0], m_data.size()))
		return util::format("%s: data read error", path.c_str());

	uint8_t testByte;
	if (r(&testByte, sizeof(testByte)))
		return util::format("%s: excessive data at the end of file", path.c_str());

	calcCrc = util::crc32(calcCrc, &m_data[0], m_data.size());
	if (calcCrc != hdrCrc)
		return util::format("%s: CRC32 mismatch (calculated 0x%08x, read 0x%08x)", path.c_str(), calcCrc, hdrCrc);

	return "";
}

// returns "" if write successful, or error string
std::string COmiFile::write(const std::string &path) const
{
	SHdr hdr;

	hdr.magic = MAGIC;
	hdr.version = 1;
	hdr.offset = m_offset;
	hdr.size = m_data.size();
	hdr.modelSize = m_model.size();
	hdr.crc32 = 0;
	hdr.padding = 0;

	memset(hdr.model, 0, sizeof(hdr.model));
	if (!m_model.empty())
		memcpy(hdr.model, m_model.data(), xmin(m_model.size(), sizeof(hdr.model)));

	uint32_t crc(util::crc32(0, &hdr, sizeof(hdr)));

	xassert(!m_data.empty(), "Empty data");
	crc = util::crc32(crc, &m_data[0], m_data.size());

	hdr.crc32 = crc;
	hdr.fromHost();

	CRawWriter w(path);
	if (!w.isOpen())
		return util::format("%s: open error", path.c_str());

	if (!w(&hdr, sizeof(hdr)))
		return util::format("%s: header write error", path.c_str());

	if (!w(&m_data[0], m_data.size()))
		return util::format("%s: data write error", path.c_str());

	if (!w.close())
		return util::format("%s: close error", path.c_str());

	return "";
}

uint16_t COmiFile::getOffset() const
{
	return m_offset;
}

void COmiFile::setOffset(uint16_t offset)
{
	m_offset = offset;
}

const std::vector<uint8_t> &COmiFile::getData() const
{
	return m_data;
}

std::vector<uint8_t> &COmiFile::getData()
{
	return m_data;
}

// beware of using c_str() as model name normally contains NULs!
// std::string supports it, but c_str() doesn't
const std::string &COmiFile::getModel() const
{
	return m_model;
}

void COmiFile::setModel(const std::string &model)
{
	m_model = model;
}
