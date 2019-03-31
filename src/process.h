#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>

#include "pool.h"


namespace abc {

	template <typename InstanceId> class runnable;
	class thread;
	class process;
	class root_process;
	class daemon_process;
	class transaction_process;

	template <typename InstanceId> using basic_runnable_handler = std::function<void(runnable<InstanceId>&)>;
	typedef std::function<void(thread&)>	basic_thread_handler_t;
	typedef std::function<void(process&)>	basic_process_handler_t;

	struct daemon_def;
	typedef std::function<void(daemon_process&, const std::vector<daemon_process>&)>	daemon_start;


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


	template <typename InstanceId>
	class runnable : public instance<abc::pool<InstanceId>> {
	protected:
		runnable(std::shared_ptr<abc::pool<InstanceId>>&& pool);
		runnable(std::shared_ptr<abc::pool<InstanceId>>&& pool, basic_runnable_handler<InstanceId>&& start_handler);

	public:
		virtual void start();

	private:
		basic_runnable_handler<InstanceId> _start_handler;
	};


	class thread : public runnable<thread_id_t> {
	private:
		friend class process;

		thread(const std::shared_ptr<abc::pool<thread_id_t>>& pool, basic_thread_handler_t&& start_handler);
	};


	class process : public runnable<process_id_t> {
	protected:
		static basic_process_handler_t	noop;

	protected:
		process(
			const std::shared_ptr<abc::pool<process_id_t>>&	pool,
			const std::shared_ptr<abc::pool<thread_id_t>>&	thread_pool,
			const std::shared_ptr<abc::pool<process_id_t>>&	child_process_pool);

	protected:
		virtual void child_process_crashed() = 0;

	protected:
		std::shared_ptr<abc::pool<thread_id_t>>		_thread_pool;
		std::shared_ptr<abc::pool<process_id_t>>	_child_process_pool;
	};


	class root_process final : public process {
	public:
		root_process(
			const std::shared_ptr<abc::pool<process_id_t>>&	singleton_pool,
			const std::shared_ptr<abc::pool<thread_id_t>>&	disabled_thread_pool,
			const std::shared_ptr<abc::pool<process_id_t>>&	daemon_process_pool);

	public:
		virtual void start() override;

	protected:
		virtual void child_process_crashed() override;

	};


	class daemon_process : public process {
	private:
		friend class root_process;

		daemon_process();
		daemon_process(const daemon_def& def);
		daemon_process(daemon_process&& other);

		void start(const std::vector<daemon_process>& all_daemons);

	public:
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
	};


	// --------------------------------------------------------------



}
