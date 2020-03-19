/**
 * \brief	Text and CSV file representation
 * \author	Circuit Chaos
 * \date	2020-03-14
 */

#include <cstdarg>
#include "textfile.h"
#include "rawfile.h"
#include "scopedcsv.h"
#include "log.h"

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

bool CTextFile::read(const std::string &path, bool isText)
{
	m_data.clear();

	CRawReader r(path);
	if (!r.isOpen())
		return false;

	// this is not the most efficient way, but the simplest one
	std::vector<char> data;
	for (;;)
	{
		char buf[1024];
		// we don't inspect return value of reader, we inspect
		// returned size instead
		size_t sz;
		r(buf, sizeof(buf), &sz);
		if (!sz)
			break;

		data.insert(data.end(), buf, buf + sz);
	}

	if (data.empty())
	{
		loge("Cannot use empty text file");
		return false;
	}

	return isText ? parseText(data) : parseCsv(data);
}

const std::vector<CTextFile::TLine> &CTextFile::get() const
{
	return m_data;
}

bool CTextFile::parseText(const std::vector<char> &data)
{
	// text file has simple rules:
	// - \n or \r\n separates lines
	// - \t separates fields
	// - none of \t, \r, \n is allowed in the field value

	std::string curField;
	TLine curLine;
	for (auto &ch: data)
	{
		if (ch == '\r')
			continue;

		if (ch == '\t' || ch == '\n')
		{
			// end of field or end of line
			curLine.push_back(curField);
			curField.clear();

			if (ch == '\n')
			{
				// end of line
				m_data.push_back(curLine);
				curLine.clear();
			}

			continue;
		}

		curField.push_back(ch);
	}

	// consider these cases for EOFs:
	// - file ended with \n: curField and curLine empty, no action needed
	// - file ended with \t: curField empty (copied to curLine), curLine not empty -- need to add
	// - file ended mid-field: both curField and curLine not empty, need to add

	if (!curField.empty())
		curLine.push_back(curField);

	if (!curLine.empty())
		m_data.push_back(curLine);

	return true;
}

int CTextFile::isSpace(unsigned char ch)
{
	return (ch == CSV_SPACE || ch == CSV_TAB) ? 1 : 0;
}

int CTextFile::isTerm(unsigned char ch)
{
	return (ch == CSV_CR || ch == CSV_LF) ? 1 : 0;
}

void CTextFile::scb1(void *data, size_t size, void *parent)
{
	((CTextFile *) parent)->cb1(data, size);
}

void CTextFile::scb2(int ch, void *parent)
{
	((CTextFile *) parent)->cb2(ch);
}

void CTextFile::cb1(void *data, size_t size)
{
	m_curLine.push_back(std::string((const char *) data, size));
}

void CTextFile::cb2(int /* ch */)
{
	m_data.push_back(m_curLine);
	m_curLine.clear();
}

bool CTextFile::parseCsv(const std::vector<char> &data)
{
	// csv file cannot be parsed line-by-line, as it can contain line
	// separator mid-field (in enclosing character -- double quote).
	//
	// there's no need to reinvent the wheel, we'll use a library
	// instead.

	CScopedCsv csv(CSV_STRICT_FINI);

	csv_set_space_func(csv, CTextFile::isSpace);
	csv_set_term_func(csv, CTextFile::isTerm);

	const size_t rs1(csv_parse(csv, &data[0], data.size(), CTextFile::scb1, CTextFile::scb2, this));
	if (rs1 != data.size())
	{
		loge("CSV parser error (%zu ne %zu): %s", rs1, data.size(), csv_strerror(csv_error(csv)));
		return false;
	}

	const int rs2(csv_fini(csv, CTextFile::scb1, CTextFile::scb2, this));
	if (rs2 != 0)
	{
		loge("CSV completion error (%d): %s", rs2, csv_strerror(csv_error(csv)));
		return false;
	}

	return true;
}
