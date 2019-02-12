#pragma once


#include <atomic>
#include <future>
#include <chrono>
#include <memory>
#include <functional>
#include <queue>

//#include <deque>
//#include <thread>

#include "base.h"
#include "mutex.h"
#include "macros.h"


namespace abc {

	template <typename Value>
	class promise;

	template <typename Value>
	class future_state {
		friend class promise<Value>;

	// For promise
	private:
		future_state() noexcept
			: _mutex()
			, _has_value(false)
			, _value()
			, _thens() {
		}

	private:
		status_t set_value(const Value& value) noexcept {
			if (_has_value) {
				return status::abort;
			}

			status_lock lock(_mutex);
			abc_warning(lock.status(), category::async, __TAG__);

			if (_has_value) {
				return status::abort;
			}

			_value = value;
			_has_value = true;
			return status::success;
		}

	public:
		bool has_value() const noexcept {
			return _has_value;
		}

		result<Value> get_value() const noexcept {
			if (!_has_value) {
				return status::bad_state;
			}

			return _value;
		}

		status_t then(std::function<void(const Value&)>&& func) noexcept {
			if (_has_value) {
				try {
					func(_value);
					return status::success;
				}
				catch (...) {
					abc_warning(status::exception, category::async, __TAG__);
				}
			}

			status_lock lock(_mutex);
			abc_warning(lock.status(), category::async, __TAG__);

			if (_has_value) {
				try {
					func(_value);
					return status::success;
				}
				catch (...) {
					abc_warning(status::exception, category::async, __TAG__);
				}
			}

			try {
				_thens.push(std::move(func));
				return status::success;
			}
			catch (...) {
				abc_warning(status::exception, category::async, __TAG__);
			}
		}

	private:
		spin_mutex<spin_for::memory>					_mutex;
		bool											_has_value;
		Value											_value;
		std::queue<std::function<void(const Value&)>>	_thens;
	};


	template <>
	class future_state<void> {
		friend class promise<void>;

	// For promise
	private:
		future_state() noexcept
			: _mutex()
			, _has_value(false)
			, _thens() {
		}

	private:
		status_t set_value() noexcept {
			if (_has_value) {
				return status::abort;
			}

			_has_value = true;
			return status::success;
		}

	public:
		bool has_value() const noexcept {
			return _has_value;
		}

		status_t then(std::function<void()>&& func) noexcept {
			if (_has_value) {
				try {
					func();
					return status::success;
				}
				catch (...) {
					abc_warning(status::exception, category::async, __TAG__);
				}
			}

			status_lock lock(_mutex);
			abc_warning(lock.status(), category::async, __TAG__);

			if (_has_value) {
				try {
					func();
					return status::success;
				}
				catch (...) {
					abc_warning(status::exception, category::async, __TAG__);
				}
			}

			try {
				_thens.push(std::move(func));
				return status::success;
			}
			catch (...) {
				abc_warning(status::exception, category::async, __TAG__);
			}
		}

	private:
		spin_mutex<spin_for::memory>		_mutex;
		std::atomic_bool					_has_value;
		std::queue<std::function<void()>>	_thens;
	};


	template <typename Value>
	class future {
		friend class promise<Value>;
	
	// For promise
	private:
		future(const std::shared_ptr<future_state<Value>>& state) noexcept
			: _state(state) {
		}

	public:
		future() noexcept
			: _state() {
		}

		future(const future<Value>& other) noexcept
			: _state(other._state) {
		}

	public:
		bool valid() const noexcept {
			return _state;
		}

		result<Value> get() noexcept {
			abc_assert(_state, category::async, __TAG__);
			try {
				return _state->get_value();
			}
			catch (...) {
				return status::bad_state;
			}
		}

	public:
		bool ready() const noexcept {
			abc_assert(_state, category::async, __TAG__);
			return _state->has_value();
		}

		/*template <typename ThenValue>
		result<future<ThenValue>> then(std::function<ThenValue(const Value&)> func) noexcept {
			promise<ThenValue> then_promise;
		}*/

	private:
		std::shared_ptr<future_state<Value>> _state;
	};


	template <typename Value>
	class promise {
	public:
		promise() noexcept
			: _state() {
			try {
				_state.reset(new (std::nothrow) future_state<Value>());
			}
			catch (...) {
			}
		}

		promise(const promise<Value>& other) noexcept
			: _state(other._state) {
		}

	// std-like API
	public:
		result<future<Value>> get_future() const noexcept {
			abc_assert(_state, category::async, __TAG__);
			return future<Value>(_state);
		}

		status_t set_value(const Value& value) noexcept {
			abc_assert(_state, category::async, __TAG__);
			abc_warning(_state->set_value(value), category::async, __TAG__);
			return status::success;
		}

	private:
		std::shared_ptr<future_state<Value>> _state;
	};


	/*template <typename Value>
	class packaged_task {
	public:
		packaged_task(std::function<Value()>&& func) noexcept
			: _mutex()
			, _status(status::not_started)
			, _function(std::move(func))
			, _promise() {
		}

		packaged_task(const packaged_task<Value>& other) noexcept
			: _mutex()
			, _status(other._status)
			, _function(other._function)
			, _promise(other._promise) {
		}

	public:
		status_t status() const noexcept {
			return _status;
		}

		result<future<Value>> get_future() const noexcept {
			return _promise.get_future();
		}

		status_t start() noexcept {
			status_lock lock(_mutex);
			abc_warning(lock.status(), category::async, __TAG__);

			abc_assert(_status == status::not_started, category::async, __TAG__);
			_status = status::not_finished;

			try {
				std::async(std::launch::async, [this]() mutable noexcept -> status_t {
					Value value = this->_function();
					this->_status = this->_promise.set_value(value);
					abc_warning(this->_status, category::async, __TAG__);
					return this->_status;
				});

				return status::success;
			}
			catch (...) {
				this->_status = status::exception;
				abc_warning(this->_status, category::async, __TAG__);
				return this->_status;
			}
		}

	private:
		spin_mutex<spin_for::memory>	_mutex;
		status_t						_status;
		std::function<Value()>			_function;
		promise<Value>					_promise;
	};*/


	/*template <typename Value>
	class then_task {
	public:
		then_task(std::function<void(const Value&)>&& func) noexcept
			: _status(status::not_started)
			, _function(std::move(func))
			, _promise() {
		}

		then_task(const then_task<Value>& other) noexcept
			: _status(other._status)
			, _function(other._function)
			, _promise(other._promise) {
		}

	public:
		status_t status() const noexcept {
			return _status;
		}

		result<future<>> get_future() const noexcept {
			return _promise.get_future();
		}

		status_t start(const Value& value) noexcept {
			abc_assert(_status == status::not_started, category::async, __TAG__);
			_status = status::not_finished;

			try {
				std::async(std::launch::async, [this]() mutable noexcept -> status_t {
					this->_function(value);
					this->_status = this->_promise.set_value();
					abc_warning(this->_status, category::async, __TAG__);
					return this->_status;
				});

				return status::success;
			}
			catch (...) {
				this->_status = status::exception;
				abc_warning(this->_status, category::async, __TAG__);
				return this->_status;
			}
		}

	private:
		status_t					_status;
		std::function<void(Value)>	_function;
		promise<void>				_promise;
	};*/


	// ---------------------------------------------------------
#ifdef DELETE
	template <typename Value>
	class shared_promise;


	template <typename Future, typename Value>
	class basic_future {
	public:
		basic_future() noexcept
			: _future() {
		}

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


	template <typename Value = void>
	class future : public basic_future<std::future<Value>, Value> {
		friend class shared_promise<Value>;

	public:
		future() noexcept
			: basic_future<std::future<Value>, Value>() {
		}

	private:
		future(std::future<Value>&& fut) noexcept
			: basic_future<std::future<Value>, Value>(std::move(fut)) {
		}

	public:
	};


	template <typename Value = void>
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
				return std::move(future<Value>(std::move(_promise->get_future())));
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

		status_t set_value() noexcept {
			abc_warning(ensure_promise(), category::async, __TAG__);

			try {
				_promise->set_value();
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
#endif
	// ---------------------------------------------------------


	class async {
	public:
		/*template <typename Value>
		static result<future<Value>> start(std::function<Value()>&& func) noexcept {
			packaged_task<Value> task(std::move(func));
			abc_warning(task.start(), category::async, __TAG__);
			return task.get_future();
		}*/

		template <typename Value, typename PredValue>
		static result<future<Value>> start(std::function<Value(const PredValue&)>&& func, const PredValue& pred_value) noexcept {
			promise<Value> prom;

			try {
				std::async(std::launch::async, [func, pred_value, prom]() mutable noexcept -> status_t {
					try {
						Value value = func(pred_value);
						abc_warning(prom.set_value(value), category::async, __TAG__);
					}
					catch (...) {
						// TODO: prom.set_error()
						abc_warning(status::exception, category::async, __TAG__);
					}

					return status::success;
				});
			}
			catch (...) {
				// TODO: prom.set_error()
				abc_warning(status::exception, category::async, __TAG__);
			}

			return prom.get_future();
		}


		template <typename Value>
		static result<future<Value>> start(std::function<Value()>&& func) noexcept {
			promise<Value> prom;

			try {
				std::async(std::launch::async, [func, prom]() mutable noexcept -> status_t {
					try {
						Value value = func();
						abc_warning(prom.set_value(value), category::async, __TAG__);
					}
					catch (...) {
						// TODO: prom.set_error()
						abc_warning(status::exception, category::async, __TAG__);
					}

					return status::success;
				});
			}
			catch (...) {
				// TODO: prom.set_error()
				abc_warning(status::exception, category::async, __TAG__);
			}

			return prom.get_future();
		}
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
