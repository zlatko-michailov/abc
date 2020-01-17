#pragma once

#include <cstdint>
#include <functional>

#include "pool.h"


namespace abc {

	class thread;

	class process;

		class launcher;

		class program;

		class daemon;

		class job;


	typedef char			process_kind_t;
	typedef std::uint32_t	process_id_t;
	typedef std::uint32_t	process_cycle_t;
	typedef std::uint32_t	thread_id_t;


	namespace process_kind {
		constexpr process_kind_t invalid	= '?';
		constexpr process_kind_t launcher	= 'L';
		constexpr process_kind_t program	= 'P';
		constexpr process_kind_t daemon		= 'D';
		constexpr process_kind_t job		= 'J';
	}


	using daemon_container		= std::vector<daemon>;

	using daemon_start_handler	= std::function<void(daemon&, process_cycle_t)>;
	using job_start_handler		= std::function<void(job&, process_cycle_t)>;
	using thread_start_handler	= std::function<void(thread&)>;


	// --------------------------------------------------------------


	class thread final
		: public instance<abc::pool<thread_id_t>> {

	public:
		thread(job& parent, thread_start_handler&& start_handler);

	public:
		static thread_id_t		current_thread_id() noexcept;

		const process&			parent() const noexcept;

	public:
		void start();

	private:
		static thread*					_current;

		const process&					_parent;
		thread_start_handler			_start_handler;
	};


	class process
		: public instance<abc::pool<process_id_t>> {

	public:
		static process_id_t			current_process_id() noexcept;
		static process&				current() noexcept;

	protected:
		process(abc::pool<process_id_t>& peer_pool, thread_id_t thread_pool_capacity, process_id_t child_pool_capacity);

	public:
		abc::pool<thread_id_t>&		thread_pool() noexcept;
		abc::pool<process_id_t>&	child_pool() noexcept;

	public:
		virtual process_kind_t		kind() const noexcept = 0;

	protected:
		static abc::pool<process_id_t>	_launcher_pool;
		static launcher					_launcher;
		static process*					_current;

		abc::pool<thread_id_t>			_thread_pool;
		abc::pool<process_id_t>			_child_pool;
	};


	class launcher final
		: public process {

	private:
		friend class process;

		launcher();

	public:
		virtual process_kind_t	kind() const noexcept override;
	};


	class program final
		: public process {

	public:
		program();

	public:
		template <typename... Args>
		void emplace_back_daemon(Args&&... args);

		const daemon_container& daemons() const noexcept;

		void start();

	public:
		virtual process_kind_t	kind() const noexcept override;

	private:
		static abc::pool<process_id_t>	_program_pool;
		daemon_container				_daemons;
	};


	class daemon final
		: public process {

	public:
		daemon(program& parent, daemon_start_handler&& start_handler, std::size_t heap_size, std::size_t output_size);

	public:
		const program&		parent() const noexcept;
		process_cycle_t		cycle() const noexcept;

		std::size_t			heap_size() const noexcept;
		const void*			heap() const noexcept;
		void*				heap() noexcept;
		std::size_t			output_size() const noexcept;
		const void*			output() const noexcept;
		void*				output() noexcept;

	public:
		virtual process_kind_t	kind() const noexcept override;

	private:
		friend class program;

		void start(process_cycle_t cycle);

	private:
		const program&			_parent;
		daemon_start_handler	_start_handler;
		process_cycle_t			_cycle;

		std::size_t				_heap_size;
		void*					_heap;
		std::size_t				_output_size;
		void*					_output;
	};


	class job final
		: public process {

	public:
		job(daemon& parent, job_start_handler&& start_handler);

	public:
		const daemon&		parent() const noexcept;
		process_cycle_t		cycle() const noexcept;

		void	start(process_cycle_t cycle);

	public:
		virtual process_kind_t	kind() const noexcept override;

	private:
		const daemon&		_parent;
		process_cycle_t		_cycle;
		job_start_handler	_start_handler;
	};
	
}
