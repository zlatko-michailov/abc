#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>

#include "pool.h"


namespace abc {

	class process;
	class root;
	struct daemon_def;
	class daemon;
	class transaction;

	typedef std::function<void(daemon&, const std::vector<daemon>&)>	daemon_start;


	// --------------------------------------------------------------


	typedef std::uint8_t	process_kind_t;
	typedef std::uint32_t	process_id_t;
	typedef std::uint32_t	process_cycle_t;
	typedef std::uint32_t	thread_id_t;


	namespace process_kind {
		constexpr process_kind_t invalid		= 0;
		constexpr process_kind_t root			= 1;
		constexpr process_kind_t daemon			= 2;
		constexpr process_kind_t transaction	= 3;
	}


	struct daemon_def {
		process_id_t	id;
		std::size_t		heap_size;
		std::size_t		output_size;
		daemon_start	start;
	};


	class thread : public instance<abc::pool<thread_id_t>> {
	public:
		thread(const std::shared_ptr<abc::pool<thread_id_t>>& pool);

	public:
		void set_exception_handler() noexcept;

	public:
		void start();
	};


	class process : public instance<abc::pool<process_id_t>> {
	public:
		process(
			const std::shared_ptr<abc::pool<process_id_t>>&	pool,
			const std::shared_ptr<abc::pool<thread_id_t>>&	thread_pool,
			const std::shared_ptr<abc::pool<process_id_t>>&	child_process_pool);

	public:
		void set_exception_handler() noexcept;
		void set_child_crash_handler() noexcept;

	public:
		void start();
		void stop();
		void recycle();

	public:
		const std::shared_ptr<abc::pool<thread_id_t>>&	thread_pool() const noexcept;
		const std::shared_ptr<abc::pool<process_id_t>>&	child_process_pool() const noexcept;

	protected:
		std::shared_ptr<abc::pool<thread_id_t>>		_thread_pool;
		std::shared_ptr<abc::pool<process_id_t>>	_child_process_pool;
	};


	class root_process : public process {
	public:
		root_process(const std::vector<daemon_def>& daemon_defs);
	};


	class daemon {
	private:
		friend class root;

		daemon();
		daemon(const daemon_def& def);
		daemon(daemon&& other);

		void start(daemon& this_daemon, const std::vector<daemon>& all_daemons);
		void stop();
		void recycle();

	public:
		process_id_t		id() const noexcept;
		std::size_t			heap_size() const noexcept;
		std::size_t			output_size() const noexcept;
		const void*			heap() const noexcept;
		void*				heap() noexcept;
		const void*			output() const noexcept;
		void*				output() noexcept;
		process_cycle_t		cycle() const noexcept;

	private:
		daemon_def		_def;
		void*			_heap;
		void*			_output;
		process_cycle_t	_cycle;

	private:
		instance<process_id_t, pool<process_id_t>>	_daemon_instance;
	};


	// --------------------------------------------------------------


	inline pool<process_id_t> root::_root_pool(1);

}
