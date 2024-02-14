#pragma once
#if !defined(MATRIX_IO_HPP_)
#define MATRIX_IO_HPP_

#include <iostream>
#include <fstream>

#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

/* Used only for template parameterization */
class dump_selector_tag
{
};

/* Used only for template parameterization */
class line_discarder
{
};

/* Template class for dumping diagnostics info on matrix content */
template <class MATRIX_TYPE, class SELECTOR_TAG>
class matrix_dumper
{
public:
	/* Default implementation unless a partial specializtion is defined for the tag */
	static void dump(MATRIX_TYPE const& /* matrix */, char const* /* name */)
	{
	}
};

/* Functional interface for matrix_dumper */
template <class MATRIX_TYPE>
void dump_matrix(MATRIX_TYPE const& matrix, char const* name)
{
	matrix_dumper<MATRIX_TYPE, dump_selector_tag>::dump(matrix, name);
}

/* Helper class for delimiter parsing */
class delimiter_matcher
{
public:
	delimiter_matcher(int delimiter) : expectation_(delimiter)
	{
	}
	
	int test(int candidate) const
	{
		return candidate == expectation_;
	}

	void reset()
	{
	}

private:
	int expectation_;
};

/* Input stream overload for consuming an expected delimiter */
std::istream& operator>>(std::istream& source, delimiter_matcher const& matcher)
{
	char draw = 0;

	source >> draw;

	if (source.good() && !matcher.test(draw))
	{
		throw new std::runtime_error("Unexpected delimiter");
	}

	return source;
}

/* Input stream overload for consuming the remainder of an input iine (to be discarded) */
std::istream& operator>>(std::istream& source, line_discarder const& specializer)
{
	source.ignore(std::numeric_limits<int>::max(), source.widen('\n'));
	return source;
}

/* Helper class parsing field or record delimiiters in CSV(-like) input */
template <class FIELD_DELIMITER_MATCHER, class RECORD_DELIMITER_MATCHER>
class field_or_record_delimiter_matcher
{
public:
	enum result_t
	{
		NoMatch,
		FieldBreak,
		RecordBreak
	};

	/* This matcher wraps two others (one for field, one for record) */
	field_or_record_delimiter_matcher(FIELD_DELIMITER_MATCHER fmatcher, RECORD_DELIMITER_MATCHER rmatcher) :
		match_state(NoMatch),
		field_matcher(fmatcher),
		record_matcher(rmatcher)
	{
	}

	/* A reset propagates to the wrapped matchers */
	void reset()
	{
		match_state = NoMatch;
		field_matcher.reset();
		record_matcher.reset();
	}

	/* This tests an input character against the field and record matchers (returns false on mismatch) */
	bool accept(int candidate)
	{
		if ((match_state != RecordBreak) && (field_matcher.test(candidate)))
		{
			match_state = FieldBreak;
			return true;
		}

		if ((match_state != FieldBreak) && ((candidate == EOF) || (record_matcher.test(candidate))))
		{
			match_state = RecordBreak;
			return true;
		}

		return false;
	}

	/* Returns the last match state */
	result_t get_result() const
	{
		return match_state;
	}

	/* Returns the last match state */
	operator result_t() const
	{
		return get_result();
	}

private:
	result_t match_state;
	FIELD_DELIMITER_MATCHER field_matcher;
	RECORD_DELIMITER_MATCHER record_matcher;
};

/* Input stream overload for consuming a field or record delimiter */
template <class FIELD_DELIMITER_MATCHER, class RECORD_DELIMITER_MATCHER>
std::istream& operator>>(std::istream& source, field_or_record_delimiter_matcher<FIELD_DELIMITER_MATCHER, RECORD_DELIMITER_MATCHER>& matcher)
{
	matcher.reset();

	if (source.good())
	{
		char peek = static_cast<char>(source.peek());
		if (matcher.accept(peek))
		{
			source.get();
		}
	}

	return source;
}

/* Adapter class to facilitate populating a coordinate matrix for any target matrix type */
template <typename VALUE_TYPE, class MATRIX_TYPE>
class coordinate_matrix_adapter
{
public:
	typedef boost::numeric::ublas::coordinate_matrix<VALUE_TYPE> coordinate_matrix_type;

public:
	coordinate_matrix_adapter(MATRIX_TYPE& external) :
		external_(external)
	{
	}

	~coordinate_matrix_adapter()
	{
		external_.resize(coordinate_.size1(), coordinate_.size2(), false);
		external_.assign(coordinate_);
	}

	coordinate_matrix_type& operator()()
	{
		return coordinate_;
	}

private:
	coordinate_matrix_type coordinate_;
	MATRIX_TYPE& external_;
};

/* Specialized adapter for a target of type coordinate matrix */
template <typename VALUE_TYPE>
class coordinate_matrix_adapter<VALUE_TYPE, boost::numeric::ublas::coordinate_matrix<VALUE_TYPE>>
{
public:
	typedef boost::numeric::ublas::coordinate_matrix<VALUE_TYPE> coordinate_matrix_type;

public:
	coordinate_matrix_adapter(coordinate_matrix_type& external) :
		external_(external)
	{
	}

	~coordinate_matrix_adapter()
	{
	}

	coordinate_matrix_type& operator()()
	{
		return external_;
	}

private:
	coordinate_matrix_type& external_;
};

/* This function attempts to populate a matrix with elements given from a chain of CSV inputs as (row, column, value) triplets */
template <class MATRIX_TYPE, class ... FILENAME_ARGS>
void load_cartesian_data(MATRIX_TYPE& matrix, FILENAME_ARGS... filenames)
{
	typedef typename MATRIX_TYPE::value_type value_type;
	typedef typename MATRIX_TYPE::size_type index_type;

	char const* filename_array[] = { filenames... };
	char const** filename_ptr = filename_array;
	char const** filename_sentinel = filename_ptr + sizeof(filename_array) / sizeof(filename_array[0]);

	index_type i = 0;
	index_type j = 0;
	value_type value;

	/* An adapter allows loading into a coordinate matrix for performance */
	/* If the target matrix is a coordinate matrix, a specialized adapter avoids a copy */
	coordinate_matrix_adapter<value_type, MATRIX_TYPE> adapter(matrix);
	boost::numeric::ublas::coordinate_matrix<value_type>& staging(adapter());

	static delimiter_matcher const& comma = delimiter_matcher(',');
	static line_discarder const& anything = line_discarder();

	std::ifstream source(*filename_ptr);
	if (source.fail())
	{
		throw std::runtime_error("Failed to open source data file");
	}

	/* This first row is a pair (or triplet) expression row count and column count */
	source >> i >> comma >> j >> anything;
	if (!source.good())
	{
		throw std::runtime_error("Failed to parse shape");
	}

	/* Assign shape to coordinate matrix */
	staging.resize(i, j, false);

	/* Loop over input chain */
	do
	{
		/* Read first triplet */
		source >> i >> comma >> j >> comma >> value >> anything;
		while (!source.eof())
		{
			if (!source.good())
			{
				throw std::runtime_error("Failed to parse cartesian triplet");
			}

			/* Assign last-read triplet and read next */
			staging.append_element(i, j, value);
			source >> i >> comma >> j >> comma >> value >> anything;
		}

		/* Current source is exhausted */
		source.close();

		/* Advance to next available source */
		if ((++filename_ptr) < filename_sentinel)
		{
			source.open(*filename_ptr, std::ifstream::in);
			if (source.fail())
			{
				throw std::runtime_error("Failed to open source data file");
			}
		}
	}
	while (source.is_open());

	/* The adapter destructor finalizes population of the target matrix */
}

/* Parser for consuming CSV input data */
template <typename VALUE_TYPE, class CONSUMER, char FIELD_DELIMITER = ',', char RECORD_DELIMITER = '\n'>
class dense_data_parser
{
public:
	typedef field_or_record_delimiter_matcher<delimiter_matcher, delimiter_matcher> delimiter_matcher_type;

public:
	static void consume(std::istream& source, CONSUMER& consumer)
	{
		delimiter_matcher field_matcher(FIELD_DELIMITER);
		delimiter_matcher record_matcher(RECORD_DELIMITER);
		delimiter_matcher_type delimiter_matcher(field_matcher, record_matcher);
		VALUE_TYPE value;

		source.peek();

		while (source.good())
		{
			source >> value;
			source >> delimiter_matcher;

			switch (delimiter_matcher)
			{
			case delimiter_matcher_type::NoMatch:
				throw std::runtime_error("Failed to parse dense data element");

			case delimiter_matcher_type::FieldBreak:
				consumer.consume_value(value);
				break;

			case delimiter_matcher_type::RecordBreak:
				consumer.consume_value(value);
				consumer.consume_record_break();
				break;
			}

			source.peek();
		}
	}
};

/* CSV consumer for use with dense_data_parser to ascertain matrix shape */
template <typename VALUE_TYPE>
class dense_data_shape_consumer
{
public:
	dense_data_shape_consumer() :
		rows(0),
		columns(0)
	{
		;
	}

	void consume_value(VALUE_TYPE value)
	{
		if (rows == 0)
		{
			++columns;
		}
	}

	void consume_record_break()
	{
		++rows;
	}

public:
	uint32_t rows;
	uint32_t columns;
};

/* CSV consumer for use with dense_data_parser to populate target matrix */
template <class MATRIX_TYPE, typename VALUE_TYPE = typename MATRIX_TYPE::value_type>
class dense_data_load_consumer
{
public:
	typedef typename MATRIX_TYPE::size_type index_type;

public:
	dense_data_load_consumer(MATRIX_TYPE& target, index_type rows, index_type columns) :
		target_(target),
		i(0),
		j(0)
	{
		target_.resize(rows, columns, false);
	}

	void consume_value(VALUE_TYPE value)
	{
		target_(i, j++) = value;
	}

	void consume_record_break()
	{
		++i;
		j = 0;
	}

private:
	MATRIX_TYPE& target_;
	index_type i;
	index_type j;
};

/* This function attempts to populate a matrix from a row-major CSV file */
template <class MATRIX_TYPE, typename VALUE_TYPE = typename MATRIX_TYPE::value_type>
void load_dense_data(MATRIX_TYPE& matrix, char const* filename)
{
	std::ifstream source(filename);

	if (source.fail())
	{
		throw std::runtime_error("Failed to open source data file");
	}

	/* Prepare to process the file twice (once to size it, once to load it) */
	std::streampos start = source.tellg();

	/* Determine size of available CSV data */
	dense_data_shape_consumer<VALUE_TYPE> shape_consumer;
	dense_data_parser<VALUE_TYPE, dense_data_shape_consumer<VALUE_TYPE>>::consume(source, shape_consumer);

	if (source.fail())
	{
		throw std::runtime_error("Failed to analyze source data file");
	}

	/* Prepare to re-process input */
	source.seekg(start);

	if (source.fail() || source.bad())
	{
		throw std::runtime_error("Failed to rewind source data file");
	}

	/* Load CSV data into target matrix */
	dense_data_load_consumer<MATRIX_TYPE, VALUE_TYPE> load_consumer(matrix, shape_consumer.rows, shape_consumer.columns);
	dense_data_parser<VALUE_TYPE, dense_data_load_consumer<MATRIX_TYPE, VALUE_TYPE>>::consume(source, load_consumer);
}

#endif /* !MATRIX_IO_HPP_ */