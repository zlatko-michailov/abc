#pragma once

#include "mutex.i.h"


namespace abc {

	template <spin_count_t SpinCount, typename Mutex>
	inline void spin_mutex<SpinCount, Mutex>::lock() {
		for (spin_count_t spin_count = 0; SpinCount < 0 || spin_count < SpinCount; spin_count++) {
			if (!_flag.test_and_set()) {
				return;
			}
		}

		_mutex.lock();
	}


	template <spin_count_t SpinCount, typename Mutex>
	inline bool spin_mutex<SpinCount, Mutex>::try_lock() {
		if (SpinCount != 0) {
			return !_flag.test_and_set();
		}

		return _mutex.try_lock();
	}


	template <spin_count_t SpinCount, typename Mutex>
	inline void spin_mutex<SpinCount, Mutex>::unlock() {
		if (SpinCount != 0) {
			_flag.clear();
		}

		_mutex.unlock();
	}

}
