/*
MIT License

Copyright (c) 2018-2025 Zlatko Michailov

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

#include <thread>
#include <mutex>
#include <condition_variable>

#include "../../diag/i/diag_ready.i.h"


namespace abc { namespace concurrent {

    /**
     * @brief Mutex that exposes the ID of the thread that has locked it. 
     */
    class mutex
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief     Constructor.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        mutex(diag::log_ostream* log = nullptr);

        /**
         * @brief Deleted.
         */
        mutex(mutex&& other) noexcept = delete;

        /**
         * @brief Deleted.
         */
        mutex(const mutex& other) = delete;

    public:
        /**
         * @brief Locks the mutex.
         */
        void lock();

        /**
         * @brief  Tries to lock the mutex. Returns immediately.
         * @return `true` = a lock was acquired. `false` = a lock was not acquired. 
         */
        bool try_lock();

        /**
         * @brief Unlocks the mutex.
         */
        void unlock();

    public:
        /**
         * @brief Returns `true` if the mutex is currently locked, `false` otherwise.
         */
        bool is_locked() noexcept;

        /**
         * @brief Returns the ID of the thread that has locked the mutex.
         */
        std::thread::id get_locking_thread_id() noexcept;

        /**
         * @brief Returns `true` if the mutex is currently locked by the current thread, `false` otherwise.
         */
        operator bool () noexcept;

    private:
        /**
         * @brief Flag that indicates whether the mutex is currently locked.
         */
        bool _is_locked;

        /**
         * @brief The ID of the thread that has locked the mutex.
         */
        std::thread::id _thread_id;

        /**
         * @brief Mutex to protect the internal state.
         */
        std::mutex _state_mutex;

        /**
         * @brief Condition variable to block on lock attempts.
         */
        std::condition_variable _blocker;
    };


    // --------------------------------------------------------------

} }
