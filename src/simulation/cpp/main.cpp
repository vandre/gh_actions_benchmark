
#include <iostream>
#include <assert.h>

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/thread/latch.hpp>
#include <boost/numeric/ublas/matrix.hpp>
#include <boost/numeric/ublas/matrix_proxy.hpp>

#include "parallelization.hpp"
#include "matrix_io.hpp"
#include "profile.hpp"
#include "profile_config.hpp"
#include "collector/json.hpp"

using namespace std;
using namespace boost;
using namespace boost::asio;

#if defined(DISABLE_PARALLELIZATION)
typedef parallelization<parallelism::single_threaded> parallelization_type;
#else
typedef parallelization<parallelism::multi_threaded> parallelization_type;
#endif /* DISABLE_PARALLELIZATION */

/* Constants */
const size_t POLICY_COUNT = 10000;
const size_t TIMESTEP_COUNT = 12 * 120;

/* Shorthand for real vector type */
typedef vector<double> real_vector_type;

/* Modeled policy fields */
struct policy_record
{
	double av;
	double benefit;
};

/* Shorthand for policy vector type */
typedef vector<policy_record> policy_vector_type;

/* Simulation inputs (read-only during simulation phase) */
struct simulation_input
{
	real_vector_type mortality;
	real_vector_type survival;
	real_vector_type yield;
	policy_vector_type inforce;
};

/* Simulation output(s) (mutable during simulation phase) */
struct simulation_output
{
	real_vector_type reserves;
};

/* Boost function object called to execute parallel tasks */
class simulation_tasks
{
public:
	/* Required of function object */
	typedef size_t result_type;

public:
	/* Constructor assigns task count and references to synchronization helper, read-only input and muable output */
	simulation_tasks(size_t count, latch* synchronizer, simulation_input const* input, simulation_output* output) :
		count_(count),
		synchronizer_(synchronizer),
		input_(input),
		output_(output)
	{
		assert(input != NULL);
		assert(output != NULL);
	}

	/* Required of function object (bound to task number in scheduling loop) */
	result_type operator ()(size_t task_number)
	{
		assert(task_number < count_);

		double reserve = 0.0;
		double yield = input().yield[task_number];

		/* Loop over policies */
		for (policy_vector_type::const_iterator policy_iter = input().inforce.begin(); policy_iter != input().inforce.end(); ++policy_iter)
		{
			double compounded_yield = 1.0;
			double deficiency = 0.0;
			double av = 1.0;

			/* Loop over timesteps */
			for (size_t timestep = 0; timestep < TIMESTEP_COUNT; ++timestep)
			{
				double charge = av * policy_iter->av;

				av -= charge;
				av *= yield;

				double benefit = (policy_iter->benefit - av) * input().survival[timestep];

				compounded_yield *= yield;
				double exposure = (benefit - charge) / compounded_yield;

				if (exposure > deficiency)
				{
					deficiency = exposure;
				}
			}

			/* Accumulate total reserves over all policies */
			reserve += deficiency;
		}

		/* Commit reserve for given scenario */
		output().reserves[task_number] = reserve;

		/* Deduct number of outstanding scenario-specific parallel calculations */
		synchronizer_->count_down();

		return task_number;
	}

	/* Convenience accessor */
	simulation_input const& input() const
	{
		return *input_;
	}

	/* Convenience accessor */
	simulation_output& output()
	{
		return *output_;
	}

private:
	size_t count_;
	latch* synchronizer_;
	simulation_input const* input_;
	simulation_output* output_;
};

class profiler_subject
{
protected:
	profiler_subject()
	{
	}

	std::ostream& progress_line(char const* text = NULL)
	{
		std::cerr << "[PROGRESS] ";

		if (text != NULL)
		{
			std::cerr << text;
		}

		return std::cerr;
	}

private:
	/* Helper to load tabular CSV source into 1D vector */
	void load_1d_csv(vector<double>& target, char const* filename)
	{
		boost::numeric::ublas::matrix<double> intermediate;
		load_dense_data(intermediate, filename);

		assert(intermediate.size2() == 1);
		boost::numeric::ublas::matrix_column<boost::numeric::ublas::matrix<double>> slice = boost::numeric::ublas::column(intermediate, 0);

		target.resize(intermediate.size1());
		target.assign(slice.begin(), slice.end());
	}

	/* Helper just for input data */
	void prepare_input(simulation_input& input)
	{
		/* Load vectorized data from disk */
		load_1d_csv(input.mortality, "mortality.csv");
		load_1d_csv(input.yield, "sigma.csv");
		
		input.survival.reserve(std::max(input.mortality.size(), TIMESTEP_COUNT));

		/* Calculate survival by timestep from mortality rates */
		double survival = 1.0;
		for (real_vector_type::const_iterator iter = input.mortality.begin(); iter != input.mortality.end(); ++iter)
		{
			survival *= 1.0 - (*iter);
			/* REVIEW: is application of mortality rate correct? */
			input.survival.push_back(survival * (*iter));
		}

		/* Pad survival for timesteps beyond mortality with zero */
		while (input.survival.size() < TIMESTEP_COUNT)
		{
			input.survival.push_back(0.0);
		}

		/* Adjust returns by 1.0 */
		for (real_vector_type::iterator iter = input.yield.begin(); iter != input.yield.end(); ++iter)
		{
			*iter += 1.0;
		}

		/* Synthesize (invariant) policy data */
		input.inforce.resize(POLICY_COUNT);
		for (policy_vector_type::iterator iter = input.inforce.begin(); iter != input.inforce.end(); ++iter)
		{
			iter->av = 0.02 / 12.0;
			iter->benefit = 1000;
		}
	}

	/* Helper just for output data */
	void prepare_output(simulation_input const& input, simulation_output& output)
	{
		output.reserves.resize(input.yield.size());
	}

protected:
	/* Setup prepares input and output structures for all trials */
	void setup()
	{
		progress_line("preparing data...") << std::endl;

		prepare_input(input_);
		prepare_output(input_, output_);

		progress_line("data preparation complete") << std::endl;
	}

	void begin_sample(int trial)
	{
		progress_line() << "starting trial #" << trial << "..." << std::endl;
	}

	/* Each sample of performance data processes all scenarios, policies and timesteps */
	void sample(int trial)
	{
		size_t scenario_count = output_.reserves.size();

		/* Prepare tasks one-to-one with scenarios */
		latch synchonizer(scenario_count);
		simulation_tasks tasks(scenario_count, &synchonizer, &input_, &output_);

		/* Zero reserves across all scenarios */
		fill(output_.reserves.begin(), output_.reserves.end(), 0.0);

		/* Loop over scenarios */
		for (size_t i = 0; i < scenario_count; ++i)
		{
			/* Schedule each task (all-tasks object bound to a scenario selector (task number) */
			parallelizer_.post(boost::bind(tasks, i));
		}

		/* Sample is not complete until all tasks are complete */
		/* Note the thread pool itsef remains populated for the next trial */
		synchonizer.wait();
	}

	void end_sample(int trial)
	{
#if defined(DEBUG) || defined(DEBUG_) || defined(DUMP_SELECT_SCENARIO_RESERVES)

#if !defined(DEBUG_SCENARIO_SELECTIONS)
	#define DEBUG_SCENARIO_SELECTIONS 0,1,2,499,999
#endif /* !DEBUG_SCENARIO_SELECTIONS */

		size_t selections[] = { DEBUG_SCENARIO_SELECTIONS };		
		for (size_t i = 0; i < sizeof(selections) / sizeof(selections[0]); ++i)
		{
			std::cerr << "[DEBUG] reserve(" << selections[i] << "): " << output_.reserves[selections[i]] << std::endl;
		}

#endif /* DEBUG */

		progress_line() << "completed trial #" << trial << "..." << std::endl;
	}

	void teardown()
	{
		parallelizer_.join();
	}

private:
	parallelization_type parallelizer_;
	simulation_input input_;
	simulation_output output_;
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