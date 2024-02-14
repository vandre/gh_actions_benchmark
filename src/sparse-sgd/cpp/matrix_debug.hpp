#pragma once
#if !defined(MATRIX_DEBUG_HPP_)
#define MATRIX_DEBUG_HPP_

#include <iostream>
#include "matrix_io.hpp"

#if !defined(MATRIX_DUMP_COUNT)
#define MATRIX_DUMP_COUNT 20
#endif /* !MATRIX_DUMP_COUNT */

template <class MATRIX_TYPE>
class matrix_dumper<MATRIX_TYPE, dump_selector_tag>
{
private:
	typedef typename MATRIX_TYPE::size_type size_type;

public:
	static void dump(MATRIX_TYPE const& matrix, char const* name)
	{
		std::cerr << "[DEBUG] Partial dump of matrix " << name << " (" << matrix.size1() << ',' << matrix.size2() << "):" << std::endl;

		if ((matrix.size1() > 0) && (matrix.size2() > 0))
		{
			for (size_type n = 0; n < MATRIX_DUMP_COUNT; ++n)
			{
				bool i_valid = n < matrix.size1();
				bool j_valid = n < matrix.size2();
				size_type i = i_valid ? n : (matrix.size1() - 1);
				size_type j = j_valid ? n : (matrix.size2() - 1);

				std::cerr << "[DEBUG] " << i << ',' << j << ',' << matrix(i, j) << std::endl;

				if (!(i_valid || j_valid))
				{
					break;
				}
			}
		}
	}
};

#endif /* MATRIX_DEBUG_HPP_ */

