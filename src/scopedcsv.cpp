/**
 * \brief	Scoped libcsv structure
 * \author	Circuit Chaos
 * \date	2020-03-16
 *
 * It initializes the object and throws if it fails.
 */

#include "scopedcsv.h"
#include "throw.h"

CScopedCsv::CScopedCsv(unsigned char options): m_init(false)
{
	const int rs(csv_init(&m_parser, options));
	xassert(rs == 0, "csv_init() failed: %d", rs);
	m_init = true;
}

CScopedCsv::~CScopedCsv()
{
	if (!m_init)
		return;

	csv_free(&m_parser);
}

CScopedCsv::operator csv_parser *()
{
	xassert(m_init, "Cannot get uninitialized parser");
	return &m_parser;
}
