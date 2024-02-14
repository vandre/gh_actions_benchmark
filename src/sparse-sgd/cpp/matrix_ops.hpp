#pragma once
#if !defined(MATRIX_OPS_HPP_)
#define MATRIX_OPS_HPP_

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

/* Performance traits for Boost matrix types */
template <typename VALUE_TYPE, class MATRIX_TEMPLATE_TYPE>
class matrix_performance_traits
{
public:
	static const bool fast_iterating = false;
	static const bool fast_indexing = false;
};

/* Specialization for coordinate_matrix performance traits */
template <typename VALUE_TYPE>
class matrix_performance_traits<VALUE_TYPE, boost::numeric::ublas::coordinate_matrix<VALUE_TYPE>>
{
public:
	static const bool fast_iterating = true;
	static const bool fast_indexing = false;
};

/* Specialization for (dense) matrix performance traits */
template <typename VALUE_TYPE>
class matrix_performance_traits<VALUE_TYPE, boost::numeric::ublas::matrix<VALUE_TYPE>>
{
public:
	static const bool fast_iterating = true;
	static const bool fast_indexing = true;
};

/* Matrix multiplication helper with overridden logic driven by performance traits */
template <class MATRIX_1_TYPE, class MATRIX_2_TYPE, class MATRIX_RESULT_TYPE>
void matrix_multiply(MATRIX_1_TYPE const& matrix1, MATRIX_2_TYPE& matrix2, MATRIX_RESULT_TYPE& result)
{
	/* Prepare result storage */
	result.resize(matrix1.size1(), matrix2.size2());

	/* Optimize if the first matrix is efficiently iterable and the second matrix is efficiently indexable */
	if (matrix_performance_traits<typename MATRIX_1_TYPE::value_type, MATRIX_1_TYPE>::fast_iterating && matrix_performance_traits<typename MATRIX_2_TYPE::value_type, MATRIX_2_TYPE>::fast_indexing)
	{
		/* Boost documentation says the resized result matrix will be zero-filled, but Microsoft debug builds violate this expectation (std::allocator issue) */
#if defined(_DEBUG) && defined(_MSC_VER)
		result.clear();
#endif /* _DEBUG && _MSC_VER */

		/* Iterate over (presumably sparse) first-matrix elements and apply to associated result row by multiplying by second-matrix row */
		for (typename MATRIX_1_TYPE::const_iterator1 major = matrix1.begin1(); major != matrix1.end1(); ++major)
		{
			for (typename MATRIX_1_TYPE::const_iterator2 minor = major.begin(); minor != major.end(); ++minor)
			{
				typename MATRIX_1_TYPE::value_type multiplier(*minor);

				boost::numeric::ublas::matrix_row<MATRIX_2_TYPE> matrix2_row(matrix2, minor.index2());
				boost::numeric::ublas::matrix_row<MATRIX_RESULT_TYPE> result_row(result, minor.index1());

				for (typename MATRIX_2_TYPE::size_type i = 0; i < matrix2.size2(); ++i)
				{
					result_row(i) += multiplier * matrix2_row(i);
				}
			}
		}

		return;
	}

	/* Default to Boost matrix multiplication implementation */
	result.assign(boost::numeric::ublas::prod(matrix1, matrix2));
}

#endif /* MATRIX_OPS_HPP_ */

