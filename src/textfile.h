/**
 * \brief	Text and CSV file representation
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#pragma once

#include <vector>
#include <string>

class CTextFile
{
public:
	typedef std::vector<std::string> TLine;

	void add(int count, ...);
	void add(const TLine &line);
	bool write(const std::string &path, bool isText) const;

	bool read(const std::string &path, bool isText);
	// no need to implement custom reading methods...
	const std::vector<TLine> &get() const;

private:
	std::vector<TLine> m_data;
	TLine m_curLine;

	static std::string toTextLine(const TLine &line);
	static std::string toCsvLine(const TLine &line);

	bool parseText(const std::vector<char> &data);
	bool parseCsv(const std::vector<char> &data);

	// for libcsv
	void cb1(void *data, size_t size);
	void cb2(int ch);
	static int isSpace(unsigned char ch);
	static int isTerm(unsigned char ch);
	static void scb1(void *data, size_t size, void *parent);
	static void scb2(int ch, void *parent);
};
