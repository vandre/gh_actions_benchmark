#pragma once
#if !defined(PROFILE_CONFIG_HPP_)
#define PROFILE_CONFIG_HPP_

#include <sstream>
#include <string.h>
#include <assert.h>
#include <boost/filesystem.hpp>

/* Helper class for raising exception on bad argument */
class default_error_handler
{
public:
	void bad_argument(char const* name, char const* reason)
	{
		std::ostringstream message;
		message << "Argument error for " << name << ": " << reason;
		throw std::invalid_argument(message.str());
	}
};

/* Wrapper class for command-line arguments with limited iterator support */
class argv_collection
{
public:
	/* Nested class for iteration */
	class const_iterator
	{
	protected:
		/* Initializing constructor supports begin/end placement */
		const_iterator(argv_collection const& collection, bool last = false) :
			current_(last ? collection.sentinel_ : collection.first_),
			sentinel_(collection.sentinel_)
		{
			assert(current_ <= sentinel_);
		}

	public:
		/* Copy constructor supports capturing state from existing iterator */
		const_iterator(const_iterator const& rhs) :
			current_(rhs.current_),
			sentinel_(rhs.sentinel_)
		{
		}

	public:
		/* Dereferencing operator returns currently-referenced argv string */
		char const* operator *() const
		{
			return *current_;
		}

		/* Pre-increment advances to next argv */
		const_iterator& operator ++()
		{
			if (current_ < sentinel_)
			{
				++current_;
			}
			return *this;
		}

		/* Post-increment advances to next argv but returns current */
		const_iterator operator ++(int post_increment)
		{
			const_iterator snapshot(*this);
			++(*this);
			return snapshot;
		}

	public:
		/* Minimum needed for comparison to end() */
		bool operator ==(const_iterator const& rhs)
		{
			return (this->current_ == rhs.current_);
		}

		/* Minimum needed for comparison to end() */
		bool operator !=(const_iterator const& rhs)
		{
			return !(*this == rhs);
		}

	private:
		char const* const* current_;
		char const* const* sentinel_;

		friend class argv_collection;
	};

public:
	/* Initializing constructor expects standard C command line arguments */
	/* In this case, however, argv[0] (program name) is expected to be skipped */
	argv_collection(int argc, char* argv[]) :
		first_(argv),
		sentinel_(argv + argc)
	{
	}

public:
	/* Returns iterator for start of command line argument strings */
	const_iterator begin() const
	{
		return const_iterator(*this);
	}

	/* Returns iterator for end of command line argument strings */
	const_iterator end() const
	{
		return const_iterator(*this, true);
	}

private:
	char** first_;
	char** sentinel_;

	friend class const_iterator;
};

/* Helper class for parsing command line arguments specific to Julia paper experiments */
template <class ARGUMENT_ITERATOR, class ERROR_HANDLER = default_error_handler>
class profile_config
{
protected:
	/* Type aliases */
	typedef ARGUMENT_ITERATOR argument_iterator_type;
	typedef ERROR_HANDLER error_handler_type;
	typedef profile_config<argument_iterator_type, error_handler_type> self_type;
	typedef void (self_type::*argument_consumer)(char const*, argument_iterator_type);

public:
	/* Initializing constructor expects command line argument iterator range and an error-handling object */
	profile_config(argument_iterator_type argument_iter, argument_iterator_type argument_end, error_handler_type error_handler = error_handler_type()) :
		error_handler_(error_handler),
		trial_count_(4), /* default trial count is 4 */
		directory_(argument_end)
	{
		char const* argument_name = NULL;
		argument_consumer consumer = NULL;

		/* Loop over all argument strings */
		while (argument_iter != argument_end)
		{
			argument_iterator_type current(argument_iter++);
			char const* argument = *current;

			/* If a consumer is waiting to ingest the argument string, delegate to it */
			if (consumer != NULL)
			{
				assert(argument_name != NULL);
				std::invoke(consumer, this, argument_name, current);
				consumer = NULL;
				continue;
			}

			/* Check for "trials" switch */
			if (!strcmp(argument, "-t") || !strcmp(argument, "--trials"))
			{
				argument_name = "trials";
				consumer = &self_type::consume_trial_count;
				continue;
			}

			/* Check for "directory" switch */
			if (!strcmp(argument, "-d") || !strcmp(argument, "--directory"))
			{
				argument_name = "directory";
				consumer = &self_type::consume_directory_specifier;
				continue;
			}

			argument_name = NULL;
			error_handler_.bad_argument(argument, "unrecognized option");
		}

		/* Check for missing but expected argument */
		if (consumer != NULL)
		{
			assert(argument_name != NULL);
			error_handler_.bad_argument(argument_name, "missing argument value");
		}

		/* Set the work directory based on argument or default */
		set_work_directory((directory_ != argument_end) ? *directory_ : "../data");
	}

	/* Accessor for trial count */
	int get_trial_count() const
	{
		return trial_count_;
	}

private:
	/* Ingest string argument as integer and assign to this object */
	void consume_trial_count(char const* name, argument_iterator_type value)
	{
		std::istringstream wrapper(*value);
		
		wrapper >> trial_count_;

		if (!wrapper.eof())
		{
			error_handler_.bad_argument(name, "invalid argument value");
		}
	}

	/* Ingest string argument via direct (iterator) assignment */
	void consume_directory_specifier(char const* name, argument_iterator_type value)
	{
		directory_ = value;
	}

	/* Apply directory configuration (either via argument or default) */
	/* This routine will attempt to change to the specifed (relative) directory by walking up the directory tree */
	void set_work_directory(char const* path)
	{
		boost::system::error_code status;

		/* The relative path is assigned as the target directory, the current work directory is the first anchor */
		boost::filesystem::path target(path);
		boost::filesystem::path anchor(boost::filesystem::current_path());

		/* Loop until success or top of directory tree is reached */
		for (;;)
		{
			/* Attempt to use target as work directory */
			boost::filesystem::current_path(target, status);

			/* If target was entered, exit on success */
			if (status == boost::system::errc::success)
			{
				return;
			}

			/* Attempt to walk up one directory */
			boost::filesystem::current_path("..");
			boost::filesystem::path parent(boost::filesystem::current_path());

			/* If no change was made, exit on failure */
			if (boost::filesystem::equivalent(parent, anchor))
			{
				std::string message("failed to resolve work directory from ");
				message += path;
				error_handler_.bad_argument("directory", message.c_str());
			}

			/* Rebase at parent and try again */
			anchor = parent;
		}
	}

private:
	error_handler_type error_handler_;
	int trial_count_;
	argument_iterator_type directory_;
};

#endif /* PROFILE_CONFIG_HPP_ */