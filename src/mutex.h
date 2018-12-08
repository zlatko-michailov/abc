#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>
#include "base.h"


namespace abc {

	typedef std::int32_t spin_count_t;

	namespace spin_for {
		constexpr spin_count_t	memory	= -1;
		constexpr spin_count_t	os		= 25 * 1000;
		constexpr spin_count_t	disk	= 50 * 1000;
		constexpr spin_count_t	network	=  1;
	}


	template <spin_count_t SpinCount, typename Mutex = std::mutex>
	class spin_mutex {
	public:
		void lock() /*throws*/ {
			for (spin_count_t spin_count = 0; SpinCount < 0 || spin_count < SpinCount; spin_count++) {
				if (!_flag.test_and_set()) {
					return;
				}
			}

			_mutex.lock();
		}

		bool try_lock() noexcept {
			if (SpinCount != 0) {
				return !_flag.test_and_set();
			}

			return _mutex.try_lock();
		}

		void unlock() /*throws*/ {
			if (SpinCount != 0) {
				_flag.clear();
			}

			_mutex.unlock();
		}

	private:
		std::atomic_flag	_flag;
		Mutex 				_mutex;
	};


	template <typename Mutex>
	class status_lock {
	public:
		status_lock(Mutex& mutex) noexcept
			: _mutex(mutex)
			, _st(status::success) {
			try {
				_mutex.lock();
			}
			catch(...) {
				_st = status::bad_state;
			}
		}

		~status_lock() noexcept {
			if (status::succeeded(_st)) {
				try {
					_mutex.unlock();
				}
				catch(...) {
				}
			}
		}

	public:
		status_t operator() noexcept {
			return _st;
		}

	private:
		Mutex&		_mutex;
		status_t	_st;
	};
}
