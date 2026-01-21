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


#pragma once

#include "../diag/diag_ready.h"
#include "i/mutex.i.h"


namespace abc { namespace concurrent {

    inline mutex::mutex(diag::log_ostream* log)
        : diag_base("abc::concurrent::mutex", log)
        , _is_locked(false)
        , _thread_id() {

        constexpr const char* suborigin = "mutex()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10894, "Begin:");

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10895, "End:");
    }


    inline void mutex::lock() {
        constexpr const char* suborigin = "lock()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10896, "Begin:");

        {
            std::unique_lock<std::mutex> state_lock(_state_mutex);

            std::thread::id this_thread_id = std::this_thread::get_id();

            if (_is_locked) {
                diag_base::expect(suborigin, _thread_id != this_thread_id, 0x10897, "_thread_id != this_thread_id");

                _blocker.wait(state_lock, [=]{ return !_is_locked; });
            }

            _is_locked = true;
            _thread_id = this_thread_id;
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10898, "End:");
    }


    inline bool mutex::try_lock() {
        constexpr const char* suborigin = "try_lock()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x10899, "Begin:");

        bool ret = false;
        {
            std::lock_guard<std::mutex> state_lock(_state_mutex);

            std::thread::id this_thread_id = std::this_thread::get_id();

            if (!_is_locked) {
                _is_locked = true;
                _thread_id = this_thread_id;
                ret = true;
            }
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1089a, "End: ret=%d", ret);

        return ret;
    }


    inline void mutex::unlock() {
        constexpr const char* suborigin = "unlock()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x1089b, "Begin:");

        {
            std::lock_guard<std::mutex> state_lock(_state_mutex);

            std::thread::id this_thread_id = std::this_thread::get_id();

            diag_base::expect(suborigin, _is_locked, 0x1089c, "_is_locked");
            diag_base::expect(suborigin, _thread_id == this_thread_id, 0x1089d, "_thread_id == this_thread_id");

            _is_locked = false;
            _thread_id = std::thread::id();
        }

        _blocker.notify_one();

        diag_base::put_any(suborigin, diag::severity::callstack, 0x1089e, "End:");
    }


    inline bool mutex::is_locked() noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);

        return _is_locked;
    }

 
    inline std::thread::id mutex::locking_thread_id() noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);

        return _thread_id;
    }


    inline mutex::operator bool () noexcept {
        std::lock_guard<std::mutex> lock(_state_mutex);

        return _is_locked && (_thread_id == std::this_thread::get_id());
    }


    // --------------------------------------------------------------

} }
