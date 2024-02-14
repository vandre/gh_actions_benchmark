#pragma once
#if !defined(COLLECTOR_JSON_HPP_)
#define COLLECTOR_JSON_HPP_

#include <boost/chrono.hpp>
#include <boost/format.hpp>

namespace json_output_helpers
{
	template <typename TIME_UNIT>
	class chrono_formatter
	{
	public:
		chrono_formatter(TIME_UNIT t) :
			t_(t)
		{
		}

		operator TIME_UNIT() const
		{
			return t_;
		}

	private:
		TIME_UNIT t_;
	};

	template <typename TIME_UNIT>
	std::ostream& operator<<(std::ostream& sink, chrono_formatter<TIME_UNIT> const& formatter)
	{
		TIME_UNIT units(formatter);
		sink << boost::format("%0.12f") % boost::chrono::duration<double>(units).count();
		return sink;
	}
}

using namespace json_output_helpers;

class json_output
{
public:
	typedef boost::chrono::nanoseconds time_unit;

public:
	void register_exception(std::exception const& e)
	{
		std::cerr << "[ERROR] " << e.what() << std::endl;
	}

	void register_sample_results(int trials, time_unit total, time_unit low, time_unit high)
	{
		std::cerr << "[RESULTS] trial(s):    " << trials << std::endl;
		std::cerr << "[RESULTS] total time:  " << total  << std::endl;
		std::cerr << "[RESULTS] min time:    " << low << std::endl;
		std::cerr << "[RESULTS] max time:    " << high << std::endl;

		std::cout << "{\"trial_count\": " << trials << ", " <<
			"\"total_seconds\": " << chrono_formatter<time_unit>(total) << ", " <<
			"\"min_seconds\": " << chrono_formatter<time_unit>(low) << ", " <<
			"\"max_seconds\": " << chrono_formatter<time_unit>(high) << ", " <<
			"\"mean_seconds\": " << chrono_formatter<time_unit>(total / trials) << " }" << std::endl;
	}
};

#endif /* !COLLECTOR_JSON_HPP_ */
