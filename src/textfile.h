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
	// xxx add reading and access

private:
	std::vector<TLine> m_data;

	static std::string toTextLine(const TLine &line);
	static std::string toCsvLine(const TLine &line);
};
