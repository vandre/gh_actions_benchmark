#pragma once
#if !defined(PROFILE_HPP_)
#define PROFILE_HPP_

#include <boost/chrono.hpp>

template <class PROFILEE, class COLLECTOR>
class profiler : protected PROFILEE
{
private:
	typedef PROFILEE superclass;
	typedef typename COLLECTOR::time_unit time_unit;

public:
	profiler(int trials, COLLECTOR& collector) :
		trials_(std::max(1, trials)),
		collector_(collector)
	{
	}

public:
	void run()
	{
		try
		{
			superclass::setup();
			run_trials();
			superclass::teardown();
		}
		catch (std::exception const& e)
		{
			collector_.register_exception(e);
		}
	}

protected:
	void run_trials()
	{
		time_unit sample(0);
		time_unit minimum(0);
		time_unit maximum(0);
		time_unit total(0);

		sample = run_sample(1);

		minimum = sample;
		maximum = sample;
		total = sample;

		for (int trial = 2; trial <= trials_; ++trial)
		{
			sample = run_sample(trial);

			if (sample < minimum)
			{
				minimum = sample;
			}

			if (sample > maximum)
			{
				maximum = sample;
			}

			total += sample;
		}

		collector_.register_sample_results(trials_, total, minimum, maximum);
	}

	time_unit run_sample(int trial)
	{
		superclass::begin_sample(trial);

		boost::chrono::high_resolution_clock::time_point t0 = boost::chrono::high_resolution_clock::now();

		superclass::sample(trial);

		time_unit sample = boost::chrono::duration_cast<time_unit>(boost::chrono::high_resolution_clock::now() - t0);

		superclass::end_sample(trial);
		return sample;
	}
	

private:
	int trials_;
	COLLECTOR& collector_;
};

#endif /* !PROFILE_HPP_ */
