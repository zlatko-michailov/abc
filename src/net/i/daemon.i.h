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
#include <string>

#include "../../diag/i/diag_ready.i.h"


namespace abc { namespace net { 

    /**
     * @brief An autonomous daemon.
     */
    class daemon
        : protected diag::diag_ready<const char*>  {

        using diag_base = diag::diag_ready<const char*>;

    public:
        /**
         * @brief     Constructor.
         * @param log `diag::log_ostream` pointer. May be `nullptr`.
         */
        daemon(diag::log_ostream* log);

        /**
         * @brief   Deleted.
         * @details The daemon is not copyable or movable, because it contains a `std::promise` and `std::atomic` members, 
         *          which are not copyable or movable.
         *          Also, the thread function keeps a pointer to the daemon instance, so moving it would cause issues.
         */
        daemon(daemon&& other) = delete;

        /**
         * @brief Deleted.
         */
        daemon(const daemon& other) = delete;

        /**
         * @brief Destructor.
         */
        virtual ~daemon() noexcept = default;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param log    `diag::log_ostream` pointer. May be `nullptr`.
         */
        daemon(const char* origin, diag::log_ostream* log);

    public:
        /**
         * @brief  Starts the daemon on a separate thread.
         * @return `std::shared_future<void>` that will get set after the daemon is stopped.
         */
        std::shared_future<void> start_async();

        /**
         * @brief   Starts the daemon on the current thread.
         * @details This thread will block until the daemon is stopped.
         */
        void start();

        /**
         * @brief  Triggers a stop of the daemon and returns immediately.
         * @return The same `std::shared_future<void>` returned from `start_async()`.
         */
        std::shared_future<void> stop_async();

        /**
         * @brief   Triggers a stop of the daemon and waits for it to complete.
         * @details This thread will block until the daemon is stopped.
         */
        void stop();

    protected:
        /**
         * @brief Returns `true` if the daemon is in a state that it can be stopped; `false` otherwise. Defaults to `true`.
         */
        virtual bool can_stop() const;

        /**
         * @brief Called **once**, **after** the daemon has started. Defaults to no-op.
         */
        virtual void on_started();

        /**
         * @brief Called **in a loop**, when the daemon is idle and can pick new work. Defaults to a brief sleep.
         */
        virtual void on_idle();

        /**
         * @brief Called **in a loop**, when the autonomous thread is waiting for the daemon to stop. Defaults to a brief sleep.
         */
        virtual void on_stopping();

        /**
         * @brief Called **once**, **after** the daemon has stopped. Defaults to no-op.
         */
        virtual void on_stopped();

    private:
        /**
         * @brief Thread function for the autonomous thread.
         */
        static void thread_func(daemon* this_ptr);

    private:
        /**
         * @brief The `std::promise` that is signaled when the daemon stops.
         */
        std::promise<void> _promise;

        /**
         * @brief The `std::shared_future` that is returned by `start_async()`.
         */
        std::shared_future<void> _future;

        /**
         * @brief Flag that gets set when `stop()` or `stop_async()` is called.
         */
        std::atomic<bool> _is_stop_requested{ false };
    };


    // --------------------------------------------------------------

} }
