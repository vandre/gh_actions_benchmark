#if !defined(DEBUG) && !defined(_DEBUG) && !defined(NDEBUG)
#pragma message("Warning: boost will compiled with checks on non-debug build (NDEBUG should be defined but is not)")
#endif

#include <iostream>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_sparse.hpp>

#include "matrix_io.hpp"
#include "matrix_ops.hpp"

#if defined(DEBUG) || defined(_DEBUG) || defined(INCLUDE_MATRIX_DEBUG)
#include "matrix_debug.hpp"
#endif

#include "profile.hpp"
#include "profile_config.hpp"
#include "collector/json.hpp"

using namespace boost;
using namespace boost::numeric::ublas;

class profiler_subject
{
protected:
	typedef double sparse_matrix_double_t;
	typedef coordinate_matrix<sparse_matrix_double_t> sparse_double_matrix_t;
	typedef matrix<sparse_matrix_double_t> dense_double_matrix_t;

protected:
	std::ostream& progress_line(char const* text = NULL)
	{
		std::cerr << "[PROGRESS] ";

		if (text != NULL)
		{
			std::cerr << text;
		}

		return std::cerr;
	}

	template <class MATRIX_TYPE>
	void emit_progress(MATRIX_TYPE const& matrix, char const* name, char const* status = NULL)
	{
		if (status == NULL)
		{
			status = "ready";
		}

		progress_line() << "matrix " << name << " (" << matrix.size1() << ',' << matrix.size2() << ") " << status << "..." << std::endl;
		dump_matrix(matrix, name);
	}

protected:
	static void sgd_V(dense_double_matrix_t& result, sparse_double_matrix_t& x, dense_double_matrix_t& total_losses, dense_double_matrix_t& cross_terms, dense_double_matrix_t& v, dense_double_matrix_t const& dv, int k = 10, double alpha = 0.99, double gamma = 0.1, double lambda = 0.1)
	{
		double x_row_count_reciprocal = 1.0 / static_cast<double>(x.size1());

		coordinate_matrix<sparse_double_matrix_t::value_type> x_loss(x.size2(), x.size1(), x.nnz());
		vector<sparse_double_matrix_t::value_type> xxl(x.size2());

		/* Boost documentation says the vector above will be zero-filled, but Microsoft debug builds violate this expectation (std::allocator issue) */
#if defined(_DEBUG) && defined(_MSC_VER)
		xxl.clear();
#endif /* _DEBUG && _MSC_VER */

		for (sparse_double_matrix_t::iterator1 major = x.begin1(); major != x.end1(); ++major)
		{
			for (sparse_double_matrix_t::iterator2 minor = major.begin(); minor != major.end(); ++minor)
			{
				sparse_double_matrix_t::value_type xelement = *minor;
				sparse_double_matrix_t::value_type loss = xelement * total_losses(minor.index1(), 0);
				x_loss.append_element(minor.index2(), minor.index1(), loss * x_row_count_reciprocal);
				xxl[minor.index2()] += loss * xelement;
			}
		}

		xxl *= x_row_count_reciprocal;

		dense_double_matrix_t xvxl;
		matrix_multiply(x_loss, cross_terms, xvxl);

		dense_double_matrix_t v_modified(v);

		for (size_t f = 0; f < static_cast<size_t>(k); ++f)
		{
			for (size_t i = 0; i < x.size2(); ++i)
			{
				dense_double_matrix_t::value_type& vref(v_modified.at_element(i, f));
				vref -= alpha * ((xvxl(i, f) - xxl[i] * vref) + gamma * dv(i, f) + lambda * vref);
			}
		}

		v_modified *= -1.0;
		v_modified += v;
		result.assign_temporary(v_modified);
	}

	void setup()
	{
		static char const* dense_load_status = "loaded from dense (tabular) datafile";
		static char const* computed_status = "computed";

		progress_line("preparing data...") << std::endl;

		load_cartesian_data(x_, "x_sparse_1.csv", "x_sparse_2.csv");
		emit_progress(x_, "x", "loaded from sparse (coordinate) datafile");

		load_dense_data(y_, "y.csv");
		emit_progress(y_, "y", dense_load_status);
		
		dense_double_matrix_t v;
		load_dense_data(v, "v1.csv");
		emit_progress(v, "v[temp]", dense_load_status);

		matrix_multiply(x_, v, cross_terms_);
		emit_progress(cross_terms_, "cross-terms", computed_status);

		load_dense_data(v_, "v.csv");
		emit_progress(v_, "v", dense_load_status);

		load_dense_data(dv_, "dv.csv");
		emit_progress(dv_, "dv", dense_load_status);
	}

	void begin_sample(int trial)
	{
		progress_line() << "starting trial #" << trial << "..." << std::endl;
	}

	void sample(int trial)
	{
		sgd_V(result_, x_, y_, cross_terms_, v_, dv_);
	}

	void end_sample(int trial)
	{
		emit_progress(result_, "result", "computed");
		progress_line() << "completed trial #" << trial << "..." << std::endl;
	}

	void teardown()
	{
		progress_line("done") << std::endl;
	}

private:
	sparse_double_matrix_t x_;
	dense_double_matrix_t y_;
	dense_double_matrix_t v_;
	dense_double_matrix_t cross_terms_;
	dense_double_matrix_t dv_;
	dense_double_matrix_t result_;
};

int main(int argc, char* argv[])
{
	int result = 0;
	json_output collector;
	argv_collection arguments(argc - 1, argv + 1);
	
	try
	{
		profile_config<argv_collection::const_iterator> config(arguments.begin(), arguments.end());
		profiler<profiler_subject, json_output> metrics(config.get_trial_count(), collector);
		metrics.run();
	}
	catch (std::exception const& e)
	{
		std::cerr << "[ERROR] " << e.what() << std::endl;
		result = 1;
	}
	
	return result;
}
