#pragma once

#include <iostream> // TODO: remove

#include "process.itf.h"


namespace abc {

	// --------------------------------------------------------------
	// thread

	/*static*/ thread_id_t thread::current_thread_id() noexcept {
		return _current != nullptr ? _current->id() : 0;
	}


	/*static*/ inline thread*	thread::_current = nullptr;


	// --------------------------------------------------------------
	// process

	/*static*/ process_id_t process::current_process_id() noexcept {
		return _current->id();
	}


	/*static*/ process& process::current() noexcept {
		return *_current;
	}


	process::process(abc::pool<process_id_t>& peer_pool, thread_id_t thread_pool_capacity, process_id_t child_pool_capacity)
		: instance<abc::pool<process_id_t>>(peer_pool)
		, _thread_pool(thread_pool_capacity)
		, _child_pool(child_pool_capacity) {
	}


	abc::pool<thread_id_t>& process::thread_pool() noexcept {
		return _thread_pool;
	}


	abc::pool<process_id_t>& process::child_pool() noexcept {
		return _child_pool;
	}


	/*static*/ inline abc::pool<process_id_t>	process::_launcher_pool(abc::pool<process_id_t>::singleton);
	/*static*/ inline launcher					process::_launcher;
	/*static*/ inline process*					process::_current = &process::_launcher;


	// --------------------------------------------------------------
	// launcher

	launcher::launcher()
		: process(_launcher_pool, abc::pool<thread_id_t>::disabled, abc::pool<process_id_t>::singleton) {
	}


	process_kind_t	launcher::kind() const noexcept {
		return process_kind::launcher;
	}


	// --------------------------------------------------------------
	// program

	program::program()
		: process(_program_pool, abc::pool<thread_id_t>::disabled, abc::pool<process_id_t>::unlimited)
		, _daemons() {
	}


	template<typename... Args>
	inline void program::emplace_back_daemon(Args&&... args) {
		_daemons.emplace_back(args...);
	} 


	const daemon_container& program::daemons() const noexcept {
		return _daemons;
	}


	void program::start() {
		// TODO: Log instead of std::cout

		_current = this;

		std::cout << "[" << current().kind() << " " << current().id() << "] Starting..." << std::endl;
		for (daemon& daemon : _daemons) {
			daemon.start(1);
		}
	}


	process_kind_t	program::kind() const noexcept {
		return process_kind::program;
	}


	/*static*/ inline abc::pool<process_id_t>	program::_program_pool(abc::pool<process_id_t>::singleton);


	// --------------------------------------------------------------
	// daemon

	daemon::daemon(program& parent, daemon_start_handler&& start_handler, std::size_t heap_size, std::size_t output_size)
		: process(parent.child_pool(), abc::pool<thread_id_t>::disabled, abc::pool<process_id_t>::unlimited)
		, _parent(parent)
		, _start_handler(std::move(start_handler))
		, _cycle(0)
		, _heap_size(heap_size)
		, _heap(nullptr)
		, _output_size(output_size)
		, _output(nullptr) {
	}


	void daemon::start(process_cycle_t cycle) {
		_current = this;

		_cycle = cycle;
		_start_handler(*this, cycle);
	}


	process_kind_t	daemon::kind() const noexcept {
		return process_kind::daemon;
	}


	// --------------------------------------------------------------
	// job

	
}
