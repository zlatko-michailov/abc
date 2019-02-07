#pragma once


#include <future>
#include <chrono>
#include <memory>

#include <queue>
#include <deque>
#include <thread>

#include "base.h"
#include "mutex.h"
#include "macros.h"


namespace abc {

	template <typename Value>
	class shared_promise;


	template <typename Future, typename Value>
	class basic_future {
		friend class shared_promise<Value>;

	public:
		basic_future(basic_future<Future, Value>&& other) noexcept
			: _future(std::move(other._future)) {
		}

	protected:
		basic_future(Future&& future) noexcept
			: _future(std::move(future)) {
		}

	// std-like API
	public:
		result<Value> get() noexcept {
			try {
				return _future.get();
			}
			catch (...) {
				return status::bad_state;
			}
		}

		bool valid() const noexcept {
			return _future.valid();
		}

		status_t wait() const noexcept {
			try {
				_future.wait();
				return status::success;
			}
			catch (...) {
				return status::bad_state;
			}
		}

		template <typename Clock, typename Duration>
		status_t wait_until(std::chrono::time_point<Clock, Duration>& time_point) const noexcept {
			try {
				_future.wait_until(time_point);
				return status::success;
			}
			catch (...) {
				return status::bad_state;
			}
		}

	// abc API
	public:
		bool ready() const noexcept {
			return _future.wait_for(std::chrono::nanoseconds(0)) == std::future_status::ready;
		}

	protected:
		Future _future;
	};


	template <typename Value>
	class future : public basic_future<std::future<Value>, Value> {
	public:
	};


	template <typename Value>
	class shared_future : public basic_future<std::shared_future<Value>, Value> {
	public:
		shared_future(const shared_future<Value>& other) noexcept {
			this->_future.reset(other._future);
		}
	};


	template <typename Value>
	class shared_promise {
	public:
		shared_promise() noexcept
			: _promise() {
		}

		shared_promise(shared_promise&& other) noexcept
			: _promise(std::move(other._promise)){
		}

		shared_promise(const shared_promise& other) noexcept
			: _promise(other._promise){
		}

	// std-like API
	public:
		result<future<Value>> get_future() noexcept {
			abc_warning(ensure_promise(), category::async, __TAG__);

			try {
				return std::move(_promise->get_future());
			}
			catch (...) {
				return status::bad_state;
			}
		}

		status_t set_value(const Value& value) noexcept {
			abc_warning(ensure_promise(), category::async, __TAG__);

			try {
				_promise->set_value(value);
			}
			catch (...) {
				return status::bad_state;
			}

			return status::success;
		}

		status_t set_value(Value&& value) noexcept {
			abc_warning(ensure_promise(), category::async, __TAG__);

			try {
				_promise->set_value(std::move(value));
			}
			catch (...) {
				return status::bad_state;
			}

			return status::success;
		}

		status_t set_exception(std::exception_ptr ep) noexcept {
			abc_warning(ensure_promise(), category::async, __TAG__);

			try {
				_promise->set_exception(ep);
			}
			catch (...) {
				return status::bad_state;
			}

			return status::success;
		}

	private:
		status_t ensure_promise() noexcept {
			if (!_promise) {
				try {
					_promise = std::make_shared<std::promise<Value>>();
				}
				catch (...) {
					return status::out_of_memory;
				}
			}

			return status::success;
		}

	protected:
		std::shared_ptr<std::promise<Value>> _promise;
	};


	class async {
	public:
		template <typename Function, typename... Args>
		static result<std::future<std::invoke_result_t<std::decay_t<Function>, std::decay_t<Args>...>>> start(Function&& func, Args&&... args) noexcept {
			try {
				return std::move(std::async(std::launch::async, func, args...));
			}
			catch(...) {
				abc_warning(status::exception, category::async, __TAG__);
			}

			abc_assert(false, category::async, __TAG__);
		}

	public:
	
	private:
	};

#ifdef DELETE
	template <typename In, typename Out>
	using in_out_function_t = std::function<status_t(In&, Out&) noexcept>;

	typedef std::function<void(status_t) noexcept>	on_error_function_t;


	template <typename Duration = std::chrono::milliseconds>
	class async;


	template <typename Future, typename Duration = std::chrono::milliseconds>
	class basic_future : public Future {
		friend class basic_promise;

	private:
		basic_future(async<Duration>& async) noexcept
			: Future()
			, _async(async) {
		}

		basic_future(Future&& other_future, async<Duration>& async) noexcept
			: Future(std::move(other_future))
			, _async(async) {
		}

		basic_future(basic_future<Future>&& other) noexcept
			: Future(std::move(static_cast<Future>(other)))
			, _async(other._async) {
		}

	public:
		bool ready() const noexcept {
			return wait_for(Duration(0)) == std::future_status::ready;
		}

	protected:
		template <typename Value, typename ThenValue>
		static bool weakup(const std::shared_future<Value>& future, in_out_function_t<Value, ThenValue>&& on_success, on_error_function_t&& on_error) noexcept {
			if (!ready(future)) {
				return false;
			}

			try {
				Value value = future->get();

			}
			catch(...) {

			}
		}

		static bool ready(const std::shared_future<Value>& future) noexcept {
			return future->wait_for(Duration(0)) == std::future_status::ready;
		}

	protected:
		async<Duration>	_async;
	};


	template <typename Value, typename Duration = std::chrono::milliseconds>
	class future : public basic_future<Value, std::future<Value>> {

	public:
		template <typename ThenValue>
		status_t then(in_out_function_t<Value, ThenValue>&& on_success, on_error_function_t&& on_error, future<ThenValue>& then_future) noexcept {
			if (!valid()) {
				abc_warning(status::bad_state, category::async, __TAG__);
			}

			std::shared_future future_copy = share();

			abc_warning(
				_asunc.push_then(
					[=this] () noexcept { return this->ready(); }, 
					[=this] () noexcept { this->wakeup(); } ),
				category::async, __TAG__);

			return status::success;
		}
	};



	template <typename Value, typename Duration = std::chrono::milliseconds>
	class future : public std::future<Value> {
		friend class promise;

	private:
		future(async<Duration>& async) noexcept
			: std::future<Value>()
			, _async(async) {
		}

		future(future<Value>&& other) noexcept
			: std::future<Value>(std::move(static_cast<std::future<Value>>(other)))
			, _async(other._async) {
		}

	public:
		template <typename ThenValue>
		status_t then(in_out_function_t<Value, ThenValue>&& on_success, on_error_function_t&& on_error, future<ThenValue>& then_future) noexcept {
			if (!valid()) {
				abc_warning(status::bad_state, category::async, __TAG__);
			}

			abc_warning(
				_asunc.push_then(
					[=this] () noexcept { return this->ready(); }, 
					[=this] () noexcept { this->wakeup(); } ),
				category::async, __TAG__);

			return status::success;
		}

	private:
		bool ready() const noexcept {
			return >wait_for(Duration(0)) == std::future_status::ready;
		}

	private:
		async<Duration>	_async;
	};


	template <typename Duration = std::chrono::milliseconds>
	class async {
	public:
		template <typename In, typename Mid, typename Out>
		static status_t then(in_out_function_t&& func, In& in, in_out_function_t&& on_success, Out& out, on_error_function_t&& on_error) noexcept {
			Mid mid;
			status_t st = func(in, mid);

			if (status::succeeded(st)) {
				st = on_success(mid, out);
			}
			else {
				on_error(st);
			}

			return st;
		}




	public:
		typedef std::chrono::system_clock				clock;
		typedef std::function<status_t() noexcept>		Function;


	public:
		async(const Duration& weakup_interval) noexcept
			: _wakeup_interval(wakeup_interval)
			, _stopping(false) {
		}


		std::future<status_t> after(const Duration& after, Function&& func) noexcept {
			return at(clock::now() + after, std::move(func));
		}


		std::future<status_t> at(const clock::time_point& at, Function&& func) noexcept {
			if (at <= clock::now()) {
				try {
					return std::async(std::launch::async, std::move(func));
				}
				catch(...) {
					// TODO: log
					return completed_future(status::exception);
				}
			}


			task t;
			t.at = at;
			t.func = std::move(func);

			{
				status_lock lock(_mutex);
				if (status::failed(lock.status())) {
					// TODO: log
					return completed_future(status::abort);
				}

				try {
					_queue.push(task);
				}
				catch(...) {
					// TODO: log
					return completed_future(status::out_of_memory);
				}
			}

			return task.promise.get_future();
		}


	private:
		void loop() noexcept {
			for (;;) {
				if (_stopping) {
					return;
				}
				std::this_thread::sleep_for(_weakup_interval);
				if (_stopping) {
					return;
				}

				clock::time_point now = clock::now();
				{
					status_lock lock(_mutex);
					if (status::failed(lock.status())) {
						// TODO: log
						continue;
					}

					task t = _queue.top();
					while (t.at <= now) {
						try {
							_queue.pop();
							std::async(call, t);
						}
						catch(...) {
							// TODO: log
						}
					}
				}
			}

		}

		static void call(const task& e) noexcept {
			status_t st = func();

			try {
				e.promise.set_value(st);
			}
			catch(...) {
				// TODO: log
			}
		}


		static std::future<status_t> completed_future(status_t st) noexcept {
			try {
				std::promise<status_t> promise;
				promise.set_value(st);
				return std::move(promise.get_future());
			}
			catch(...) {
				// TODO: log
				return std::move(future());
			}
		}

		static status_t try_set_value(std::promise<status_t>& promise, status_t st) noexcept{
			try {
				promise.set_value(st);
				return status::success;
			}
			catch(...) {
				// TODO: log
				return status::exception;
			}
		}

		static std::future<status_t> try_get_future(std::promise<status_t>& promise) noexcept {
			try {
				return std::move(promise.get_future());
			}
			catch(...) {
				// TODO: log
				return std::move(future<status_t>());
			}
		}


	private:
		struct task {
			clock::time_point			at;
			Function					func;
			std::promise<status_t>		promise;

			bool operator>(const task& other) noexcept {
				return at > other.at;
			}
		};


	private:
		Duration 														_weakup_interval;
		spin_mutex<spin_for::memory>									_mutex;
		std::priority_queue<task, std::deque<task>, std::greater>		_queue;
		bool															_stopping;
	};
#endif

}
