#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>

#include "pool.h"


namespace abc {

	template <typename InstanceId, typename Runnable>
	class runnable;

		template <std::size_t DaemonsCount>
		class thread;
	
		template <typename Runnable>
		class process;
	
			template <std::size_t DaemonsCount>
			class root_process;
			
			template <std::size_t DaemonsCount>
			class daemon_process;
			
			template <std::size_t DaemonsCount>
			class job_process;

			class program;

			class daemon;

			class job;

	template <typename Runnable>
	using basic_runnable_handler = std::function<void(Runnable&)>;
	
		template <std::size_t DaemonsCount>
		using thread_start_handler = basic_runnable_handler<thread<DaemonsCount>>;
		
		template <std::size_t DaemonsCount>
		using thread_crash_handler = basic_runnable_handler<thread<DaemonsCount>>;

		template <std::size_t DaemonsCount>
		using daemon_start_handler = basic_runnable_handler<daemon_process<DaemonsCount>>;
		
		template <std::size_t DaemonsCount>
		using daemon_crash_handler = basic_runnable_handler<daemon_process<DaemonsCount>>;
		
		template <std::size_t DaemonsCount>
		using job_start_handler = basic_runnable_handler<job_process<DaemonsCount>>;

		template <std::size_t DaemonsCount>
		using job_crash_handler = basic_runnable_handler<job_process<DaemonsCount>>;

	template <std::size_t DaemonsCount>
	struct daemon_def;

	typedef std::vector<daemon> daemon_container;

	typedef std::uint8_t	process_kind_t;
	typedef std::uint32_t	process_id_t;
	typedef std::uint32_t	process_cycle_t;
	typedef std::uint32_t	thread_id_t;


	namespace process_kind {
		constexpr process_kind_t invalid	= 0;
		constexpr process_kind_t root		= 1;
		constexpr process_kind_t daemon		= 2;
		constexpr process_kind_t job		= 3;
	}


// --------------------------------------------------------------


	template <typename Runnable>
	struct runnable_def {
		basic_runnable_handler<Runnable>	start_handler;
		basic_runnable_handler<Runnable>	crash_handler;
	};

	template <std::size_t DaemonsCount>
	using thread_def = runnable_def<thread<DaemonsCount>>;

	template <std::size_t DaemonsCount>
	struct daemon_def final
		: public runnable_def<daemon_process<DaemonsCount>> {

		std::size_t		heap_size;
		std::size_t		output_size;
	};

	template <std::size_t DaemonsCount>
	using job_def = runnable_def<job_process<DaemonsCount>>;


	template <typename InstanceId, typename Runnable>
	class runnable
		: public instance<abc::pool<InstanceId>> {

	protected:
		runnable(abc::pool<InstanceId>& pool, runnable_def<Runnable>&& def);

	protected:
		virtual void start() = 0;

	protected:
		basic_runnable_handler<Runnable>	_start_handler;
		basic_runnable_handler<Runnable>	_crash_handler;
	};


	template <std::size_t DaemonsCount>
	class thread final
		: public runnable<thread_id_t, thread<DaemonsCount>> {

	private:
		friend class job_process<DaemonsCount>;

		thread(const job_process<DaemonsCount>& parent, thread_def<DaemonsCount>&& def);

	public:
		virtual void start() override;

	private:
		const job_process<DaemonsCount>	_parent;
	};


	template <typename Runnable>
	class process
		: public runnable<process_id_t, Runnable> {

	protected:
		process(abc::pool<process_id_t>& pool, thread_id_t thread_pool_capacity, process_id_t child_process_pool_capacity, runnable_def<Runnable>&& def);

	protected:
		abc::pool<thread_id_t>		_thread_pool;
		abc::pool<process_id_t>		_child_process_pool;
	};


	template <std::size_t DaemonsCount>
	class root_process final
		: public process {

	public:
		root_process(std::array<daemon_def<DaemonsCount>, DaemonsCount>&& daemon_defs);

	public:
		const std::array<daemon_process, DaemonsCount>& daemons() const noexcept;

	public:
		virtual void start() override;

	private:
		static abc::pool<process_id_t>							_root_process_pool;
		std::array<daemon_process<DaemonsCount>, DaemonsCount>	_daemons;
	};


	template <std::size_t DaemonsCount>
	class daemon_process final
		: public process {

	private:
		friend class root_process<DaemonsCount>;

		daemon_process(const root_process<DaemonsCount>& parent, const daemon_def<DaemonsCount>& def);

	public:
		job_process&& create_job(job_start_handler<DaemonsCount>&& start_handler);

	public:
		const root_process<DaemonsCount>&	parent() const noexcept;
		std::size_t							heap_size() const noexcept;
		std::size_t							output_size() const noexcept;
		const void*							heap() const noexcept;
		void*								heap() noexcept;
		const void*							output() const noexcept;
		void*								output() noexcept;
		process_cycle_t						cycle() const noexcept;

	protected:
		virtual void start() override;

	private:
		const root_process<DaemonsCount>&	_parent;
		daemon_def							_def;
		void*								_heap;
		void*								_output;
		process_cycle_t						_cycle;
	};


	template <std::size_t DaemonsCount>
	class job_process final
		: public process {

	private:
		friend class daemon_process<DaemonsCount>;

		job_process(const daemon_process<DaemonsCount>& parent, job_def<DaemonsCount>&& def);

	public:
		const daemon_process<DaemonsCount>&	parent() const noexcept;
		thread<DaemonsCount>&&				create_thread(thread_def<DaemonsCount>&& def);

	protected:
		virtual void start() override;

	private:
		const daemon_process<DaemonsCount>&	_parent;
		job_start_handler<DaemonsCount> 	_start_handler;
	};


	// --------------------------------------------------------------


	class thread
		: public instance<abc::pool<thread_id_t>> {

	public:
		thread(job& parent);

	public:
		virtual void start() override;

	private:
		const job&	_parent;
	};


	class process
		: public instance<abc::pool<process_id_t>> {

	protected:
		process(abc::pool<process_id_t>& peer_pool, thread_id_t thread_pool_capacity, process_id_t child_pool_capacity);

	protected:
		abc::pool<thread_id_t>		_thread_pool;
		abc::pool<process_id_t>		_child_process_pool;
	};


	class program final
		: public process {

	private:
		friend class daemon;

		void accept_daemon(daemon&& daemon);

	public:
		program();

	public:
		const daemon_container& daemons() const noexcept;

		void start();

	private:
		static abc::pool<process_id_t>	_program_pool;
		daemon_container				_daemons;
	};


	class daemon
		: public process {

	public:
		daemon(program& parent);
		virtual ~daemon();

	public:
		const program&		parent() const noexcept;
		std::size_t			heap_size() const noexcept;

		const void*			heap() const noexcept;
		void*				heap() noexcept;
		std::size_t			output_size() const noexcept;
		const void*			output() const noexcept;
		void*				output() noexcept;
		process_cycle_t		cycle() const noexcept;

	public:
		virtual void start(process_cycle_t cycle);

	private:
		const program&			_parent;
		process_cycle_t			_cycle;

		std::size_t				_heap_size;
		void*					_heap;
		std::size_t				_output_size;
		void*					_output;
	};


	class job
		: public process {

	public:
		job(daemon& daemon);

	public:
		const daemon&	parent() const noexcept;

	public:
		virtual void start();

	private:
		const daemon&	_parent;
	};


	// --------------------------------------------------------------



}
