#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <atomic>

#include "pool.h"


namespace abc {

	template <typename InstanceId>
	class runnable;

		template <std::size_t DaemonsCount>
		class thread;
	
		class process;
	
			template <std::size_t DaemonsCount>
			class root_process;
			
			template <std::size_t DaemonsCount>
			class daemon_process;
			
			template <std::size_t DaemonsCount>
			class job_process;

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


	template <std::size_t DaemonsCount>
	struct daemon_def {
		process_id_t						id;
		std::size_t							heap_size;
		std::size_t							output_size;
		daemon_start_handler<DaemonsCount>	start_handler;
	};


	template <typename InstanceId>
	class runnable
		: public instance<abc::pool<InstanceId>> {

	protected:
		runnable(abc::pool<InstanceId>& pool);

	protected:
		virtual void start() = 0;
	};


	template <std::size_t DaemonsCount>
	class thread final
		: public runnable<thread_id_t> {

	private:
		friend class job_process<DaemonsCount>;

		thread(const job_process<DaemonsCount>& parent, thread_start_handler&& start_handler);
		thread(thread<DaemonsCount>&& other);

	public:
		virtual void start() override;

	private:
		const job_process<DaemonsCount>	_parent;
		thread_start_handler			_start_handler;
	};


	class process
		: public runnable<process_id_t> {

	protected:
		process(abc::pool<process_id_t>& pool, abc::pool<thread_id_t>& thread_pool, abc::pool<process_id_t>& child_process_pool);
		process(process&& other);

	protected:
		abc::pool<thread_id_t>		_thread_pool;
		abc::pool<process_id_t>		_child_process_pool;
	};


	template <std::size_t DaemonsCount>
	class root_process final
		: public process {

	public:
		root_process(std::array<daemon_def, DaemonsCount>&& daemon_defs);
		root_process(root_process&& other);

	public:
		const std::array<daemon_process, DaemonsCount>& daemons() const noexcept;

	public:
		virtual void start() override;

	private:
		std::array<daemon_process, DaemonsCount> _daemons;
	};


	template <std::size_t DaemonsCount>
	class daemon_process final
		: public process {

	private:
		friend class root_process<DaemonsCount>;

		daemon_process(const root_process& parent, const daemon_def& def);
		daemon_process(daemon_process&& other);

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

		job_process(const daemon_process& parent, job_start_handler&& start_handler);
		job_process(job_process&& other);

	public:
		const daemon_process<DaemonsCount>&	parent() const noexcept;
		thread&& create_thread(thread_start_handler&& start_handler);

	protected:
		virtual void start() override;

	private:
		const daemon_process<DaemonsCount>&	_parent;
		job_start_handler<DaemonsCount> 	_start_handler;
	};


	// --------------------------------------------------------------



}
