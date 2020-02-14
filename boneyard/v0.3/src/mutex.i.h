/*
MIT License

Copyright (c) 2018 Zlatko Michailov 

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

#include <cstdint>
#include <atomic>
#include <mutex>


namespace abc {

	typedef std::int32_t spin_count_t;

	namespace spin_for {
		constexpr spin_count_t	memory	= -1;
		constexpr spin_count_t	os		=  25 * 1000;
		constexpr spin_count_t	disk	= 100 * 1000;
		constexpr spin_count_t	network	=  1;
	}


	template <spin_count_t SpinCount, typename Mutex>
	class spin_mutex;


	// --------------------------------------------------------------


	template <spin_count_t SpinCount, typename Mutex = std::mutex>
	class spin_mutex {
	public:
		spin_mutex() noexcept = default;

	public:
		void lock();
		bool try_lock();
		void unlock();

	private:
		volatile std::atomic_flag	_flag = ATOMIC_FLAG_INIT;
		Mutex 						_mutex;
	};

}
