/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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


#include "inc/mutex.h"


static bool mutex_1_thread(abc::concurrent::mutex& mutex, test_context& context);
static bool mutex_M_threads(abc::concurrent::mutex& mutex, std::condition_variable_any& sync, test_context& context);


bool test_mutex_1_thread_1_use(test_context& context) {
    bool passed = true;

    abc::concurrent::mutex mutex;

    passed = passed && mutex_1_thread(mutex, context);

    return passed;
}


bool test_mutex_1_thread_M_uses(test_context& context) {
    bool passed = true;

    abc::concurrent::mutex mutex;

    passed = passed && mutex_1_thread(mutex, context);
    passed = passed && mutex_1_thread(mutex, context);
    passed = passed && mutex_1_thread(mutex, context);
    passed = passed && mutex_1_thread(mutex, context);

    return passed;
}


bool test_mutex_M_threads_1_use(test_context& context) {
    bool passed = true;

    abc::concurrent::mutex mutex;

    std::condition_variable_any sync;

    std::thread thread1([&mutex, &sync, &context, &passed]() {
        passed = passed && mutex_M_threads(mutex, sync, context);
    });

    std::thread thread2([&mutex, &sync, &context, &passed]() {
        passed = passed && mutex_M_threads(mutex, sync, context);
    });

    std::thread thread3([&mutex, &sync, &context, &passed]() {
        passed = passed && mutex_M_threads(mutex, sync, context);
    });

    std::thread thread4([&mutex, &sync, &context, &passed]() {
        passed = passed && mutex_M_threads(mutex, sync, context);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    sync.notify_all();

    thread1.join();
    thread2.join();
    thread3.join();
    thread4.join();

    return passed;

}


static bool mutex_1_thread(abc::concurrent::mutex& mutex, test_context& context) {
    bool passed = true;

    passed = passed && context.are_equal(mutex.is_locked(), false, 0x10c64, "%d");
    passed = passed && context.are_equal(static_cast<bool>(mutex), false, 0x10c65, "%d");

    std::thread::id this_thread_id = std::this_thread::get_id();

    {
        std::lock_guard<abc::concurrent::mutex> lock(mutex);
        passed = passed && context.are_equal(mutex.is_locked(), true, 0x10c66, "%d");
        passed = passed && context.are_equal(mutex.locking_thread_id() == this_thread_id, true, 0x10c67, "%d");
        passed = passed && context.are_equal(static_cast<bool>(mutex), true, 0x10c68, "%d");
    }

    passed = passed && context.are_equal(mutex.is_locked(), false, 0x10c69, "%d");
    passed = passed && context.are_equal(static_cast<bool>(mutex), false, 0x10c6a, "%d");

    return passed;
}


static bool mutex_M_threads(abc::concurrent::mutex& mutex, std::condition_variable_any& sync, test_context& context) {
    bool passed = true;

    std::thread::id this_thread_id = std::this_thread::get_id();

    {
        std::unique_lock<abc::concurrent::mutex> lock(mutex);
        sync.wait(lock);

        passed = passed && context.are_equal(mutex.is_locked(), true, 0x10c6b, "%d");
        passed = passed && context.are_equal(mutex.locking_thread_id() == this_thread_id, true, 0x10c6c, "%d");
        passed = passed && context.are_equal(static_cast<bool>(mutex), true, 0x10c6d, "%d");

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return passed;
}


