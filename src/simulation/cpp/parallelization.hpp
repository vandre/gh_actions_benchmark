#pragma once
#if !defined(PARALLELIZATION_HPP_)
#define PARALLELIZATION_HPP_

#include <boost/asio.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/thread.hpp>

namespace parallelism
{
	typedef enum
	{
		single_threaded = 0,
		multi_threaded = 1
	}
	strategy;
}

template <parallelism::strategy STRATEGY>
class parallelization
{
public:
	template <class TOKEN>
	void post(TOKEN token)
	{
		boost::asio::post(pool_, token);
	}

	void join()
	{
		pool_.join();
	}

private:
	boost::asio::thread_pool pool_;
};

template <>
class parallelization<parallelism::single_threaded>
{
public:
	template <class TOKEN>
	void post(TOKEN token)
	{
		token();
	}

	void join()
	{
	}
};

#endif /* PARALLELIZATION_HPP_ */
