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


#include <thread>
#include <condition_variable>
#include <atomic>
#include <chrono>

#include "../src/test.h"
#include "../src/mutex.h"


namespace abc { namespace test { namespace mutex {

	template <spin_count_t SpinCount>
	static bool test_spin_mutex(test_context<abc::test_log>& context, bool spin) {
		using namespace std::chrono_literals;

		const std::size_t thread_count = 2;
		const std::size_t inc_count = 1000000;
		const std::uint32_t init = 0;

		std::mutex mut;
		std::condition_variable cond;
		abc::spin_mutex<SpinCount> spin_mut;

		volatile std::uint32_t value = init;
		volatile std::size_t ready_count = 0;

		std::thread threads[thread_count];
		for (std::size_t i = 0; i < thread_count; i++) {
			std::thread th([&mut, &cond, &value, &ready_count, &spin_mut, spin]() {
				// Wait until all the thread are ready.
				//std::unique_lock<std::mutex> lock(mut);
				//ready_count++;
				//cond.wait(lock);
				////cond.wait(lock, [&ready_count](){ return ready_count == thread_count; });
				////cond.notify_all();

				// Increment N times.
				for (size_t j = 0; j < inc_count; j++) {
					if (spin) {
						std::lock_guard<abc::spin_mutex<SpinCount>> spin_lock(spin_mut);

						std::uint32_t x = value + 1;
						//std::this_thread::sleep_for(1ms);
						std::this_thread::yield();
						value = x;
					}
					else {
						std::lock_guard lock(mut);

						std::uint32_t x = value + 1;
						//std::this_thread::sleep_for(1ms);
						std::this_thread::yield();
						value = x;
					}
////std::cout << "thread_id=" << std::this_thread::get_id() << ", j=" << j << std::endl;
				}
			});

			threads[i] = std::move(th);
		}

		//std::this_thread::sleep_for(100ms);
		//cond.notify_all();

		for (std::size_t i = 0; i < thread_count; i++) {
			threads[i].join();
		}

		return context.are_equal((std::uint32_t)value, (std::uint32_t)(init + thread_count * inc_count), 0x101, "%u");
	}


	bool test_spin_mutex_memory(test_context<abc::test_log>& context) {
		return test_spin_mutex<abc::spin_for::memory>(context, true);
	}


	bool test_spin_mutex_os(test_context<abc::test_log>& context) {
		return test_spin_mutex<abc::spin_for::os>(context, true);
	}


	bool test_spin_mutex_disk(test_context<abc::test_log>& context) {
		return test_spin_mutex<abc::spin_for::disk>(context, true);
	}


	bool test_spin_mutex_network(test_context<abc::test_log>& context) {
		return test_spin_mutex<abc::spin_for::network>(context, true);
	}


	bool test_mutex(test_context<abc::test_log>& context) {
		return test_spin_mutex<abc::spin_for::network>(context, false);
	}

}}}

