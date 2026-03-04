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


#include <future>
#include <atomic>
#include <thread>
#include <string>

#include "../diag/diag_ready.h"
#include "i/daemon.i.h"


namespace abc { namespace net { 

    inline daemon::daemon(diag::log_ostream* log)
        : daemon("abc::net::daemon", log) {
    }


    inline daemon::daemon(const char* origin, diag::log_ostream* log)
        : diag_base(copy(origin), log)
        , _is_stop_requested(false) {
    }


    inline std::future<void> daemon::start_async() {
        constexpr const char* suborigin = "start_async()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // We can't use std::async() here because we want to detach the thread and return our own std::future.
        std::thread(thread_func, this).detach();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        // Return our own future.
        return _promise.get_future();
    }


    inline void daemon::thread_func(daemon* this_ptr) {
        this_ptr->start();
    }


    inline void daemon::start() {
        constexpr const char* suborigin = "start()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // The autonomous thread has started.
        on_started();

        // Idle loop.
        while (!_is_stop_requested) {
            on_idle();
        }

        // Wait until the daemon can stop.
        while (!can_stop()) {
            on_stopping();
        }

        // The autonomous thread has stopped.
        on_stopped();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        // Complete the promise after everything is done.
        _promise.set_value();
    }


    inline std::future<void> daemon::stop_async() {
        constexpr const char* suborigin = "stop_async()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        _is_stop_requested = true;

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");

        return _promise.get_future();
    }


    inline void daemon::stop() {
        constexpr const char* suborigin = "stop()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // Block the current thread until the daemon stops.
        stop_async().wait();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline bool daemon::can_stop() const {
        return true;
    }


    inline void daemon::on_started() {
    }


    inline void daemon::on_idle() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }


    inline void daemon::on_stopping() {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }


    inline void daemon::on_stopped() {
    }
        

    // --------------------------------------------------------------

} }
