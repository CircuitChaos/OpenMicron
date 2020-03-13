/**
 * \brief	.omi file representation
 * \author	Circuit Chaos
 * \date	2020-03-13
 */

#pragma once

#include <string>
#include <vector>
#include <inttypes.h>

class COmiFile
{
public:
	COmiFile();

	// returns "" if read successful, or error string
	std::string read(const std::string &path);

	// returns "" if write successful, or error string
	std::string write(const std::string &path) const;

	uint16_t getOffset() const;
	void setOffset(uint16_t offset);

	const std::vector<uint8_t> &getData() const;
	std::vector<uint8_t> &getData();	// vector might be resized externally

	// beware of using c_str() as model name normally contains NULs!
	// std::string supports it, but c_str() doesn't
	const std::string &getModel() const;
	void setModel(const std::string &model);

private:
	static const uint32_t MAGIC = 0x4f4d4921;	// "OMI!"

	// on disk, all integers in header are in BE byte order
	// when altering header, change also fromHost() and toHost()
	struct SHdr
	{
		// magic, always OMI!
		uint32_t magic;

		// file version, currently only 1 is supported
		uint16_t version;

		// first memory address that has been read
		uint16_t offset;

		// number of bytes that are present
		uint16_t size;

		// radio model size (actual, not present)
		// - can be longer than 16 bytes
		uint8_t modelSize;

		// crc32 of the whole file, with dataCrc32 filled with zeroes
		uint32_t crc32;

		// padding to 16 bytes (0)
		uint8_t padding;

		// radio model read
		// - if longer than 16 bytes, it's right-trimmed
		// - if shorter than 16 bytes, it's zero-padded
		//   (note that in both these cases, modelSize
		//   contains actual size)
		uint8_t model[16];

		void fromHost();
		void toHost();
		static uint32_t crcToHost(uint32_t crc);
	} __attribute__((packed));

	uint16_t m_offset;
	std::string m_model;
	std::vector<uint8_t> m_data;
};
