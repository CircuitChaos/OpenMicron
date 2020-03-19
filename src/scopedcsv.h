/**
 * \brief	Scoped libcsv structure
 * \author	Circuit Chaos
 * \date	2020-03-16
 *
 * It initializes the object and throws if it fails.
 */

#pragma once

#include "csv.h"

class CScopedCsv
{
public:
	CScopedCsv(unsigned char options);
	~CScopedCsv();

	operator csv_parser *();

private:
	csv_parser m_parser;
	bool m_init;
};
