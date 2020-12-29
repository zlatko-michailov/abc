/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once

#include "mutex.i.h"


namespace abc {

	template <spin_count_t SpinCount, typename Mutex>
	inline void spin_mutex<SpinCount, Mutex>::lock() {
		for (spin_count_t spin_count = 0; SpinCount < 0 || spin_count < SpinCount; spin_count++) {
			if (!_flag.test_and_set()) {
				break;
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
