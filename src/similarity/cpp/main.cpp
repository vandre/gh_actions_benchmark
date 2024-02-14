#include <iostream>
#include <functional>
#include <boost/chrono.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/dynamic_bitset.hpp>
#include <boost/iterator/function_input_iterator.hpp>
#include <boost/iterator/function_output_iterator.hpp>

#include "profile.hpp"
#include "profile_config.hpp"
#include "collector/json.hpp"

using namespace boost;

const size_t BIT_CAPACITY = 100000000;

typedef random::mt19937 prng_type;
typedef prng_type::result_type block_type;
typedef dynamic_bitset<block_type> bitvector_type;
typedef bitvector_type::size_type size_type;

class specialization_tag
{
};

template <typename SPECIALIZATION>
size_type count_bits(bitvector_type const& bitvector, SPECIALIZATION const& specialization)
{
	return bitvector.count();
}

#if defined(USE_KERNIGHAN_BIT_COUNT_ALGORITHM)

class bit_counter
{
public:
	bit_counter(size_type* counter) :
		counter_(counter)
	{
	}

	bit_counter(bit_counter const& other) :
		counter_(other.counter_)
	{
	}

	bit_counter& operator=(bit_counter const& other)
	{
		counter_ = other.counter_;
		return *this;
	}

	void operator()(block_type integer)
	{
		size_type bits = 0;
		while (integer)
		{
			++bits;
			integer &= integer - 1;
		}
		*counter_ += bits;
	}

private:
	size_type* counter_;
};

template <>
size_type count_bits<specialization_tag>(bitvector_type const& bitvector, specialization_tag const& specialization)
{
	size_type count = 0;
	bit_counter counter(&count);
	to_block_range(bitvector, make_function_output_iterator(counter));
	return count;
}

#endif /* USE_KERNIGHAN_BIT_COUNT_ALGORITHM */

class profiler_subject
{
protected:
	profiler_subject() :
		x_bitvector_(BIT_CAPACITY),
		y_bitvector_(BIT_CAPACITY),
		bitcount_(0),
		prng_(static_cast<block_type>(std::time(0)))
	{
	}

protected:
	void setup()
	{
	}

	void begin_sample(int trial)
	{
		size_type zero(0);

		std::cerr << "[PROGRESS] starting trial #" << trial << "..." << std::endl;

		from_block_range(make_function_input_iterator(prng_, zero), make_function_input_iterator(prng_, x_bitvector_.num_blocks()), x_bitvector_);
		from_block_range(make_function_input_iterator(prng_, zero), make_function_input_iterator(prng_, y_bitvector_.num_blocks()), y_bitvector_);
	}

	void sample(int trial)
	{
		dynamic_bitset<block_type> intersection(x_bitvector_);
		intersection &= y_bitvector_;
		bitcount_ = count_bits(intersection, specialization_tag());
	}

	void end_sample(int trial)
	{
		std::cerr << "[PROGRESS] bit count for trial #" << trial << " of " << BIT_CAPACITY << ": " << bitcount_ << std::endl;
	}

	void teardown()
	{
	}

private:
	bitvector_type x_bitvector_;
	bitvector_type y_bitvector_;
	size_type bitcount_;
	prng_type prng_;
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