/**
 * \brief	Text and CSV file representation
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#include <cstdarg>
#include "textfile.h"
#include "rawfile.h"

void CTextFile::add(int count, ...)
{
	TLine line;

	va_list ap;
	va_start(ap, count);

	if (count == -1)
	{
		for (;;)
		{
			const char *s(va_arg(ap, const char *));
			if (!s)
				break;
			line.push_back(s);
		}
	}
	else
	{
		for (int i(0); i < count; ++i)
			line.push_back(va_arg(ap, const char *));
	}
	va_end(ap);

	add(line);
}

void CTextFile::add(const std::vector<std::string> &line)
{
	m_data.push_back(line);
}

bool CTextFile::write(const std::string &path, bool isText) const
{
	CRawWriter w(path);
	if (!w.isOpen())
		return false;

	for (auto &i: m_data)
	{
		const std::string s((isText ? toTextLine(i) : toCsvLine(i)) + '\n');
		if (!w(s.c_str(), s.size()))
			return false;
	}

	if (!w.close())
		return false;

	return true;
}

std::string CTextFile::toTextLine(const TLine &line)
{
	std::string s;
	for (auto i(line.begin()); i != line.end(); ++i)
	{
		if (i != line.begin())
			s += '\t';

		s += *i;
	}
	return s;
}

std::string CTextFile::toCsvLine(const TLine &line)
{
	std::string s;
	for (auto i(line.begin()); i != line.end(); ++i)
	{
		if (i != line.begin())
			s += ',';

		s += '"';
		for (auto &j: *i)
		{
			if (j == '"')
				s += '"';
			s += j;
		}
		s += '"';
	}
	return s;
}
