#pragma once

#include <cstdint>
#include <atomic>
#include <memory>


namespace abc {

	template <typename Capacity>
	class pool;

	template <typename Pool>
	class instance;


	// --------------------------------------------------------------


	template <typename Capacity>
	class pool {
	public:
		typedef Capacity capacity_t;

		static constexpr capacity_t unlimited	= -1;
		static constexpr capacity_t disabled	= 0;
		static constexpr capacity_t singleton	= 1;

	public:
		pool(capacity_t capacity) noexcept;
		pool(pool&& other) noexcept;

	protected:
		friend class instance<pool<capacity_t>>;

		capacity_t	reserve();
		void		release() noexcept;

	public:
		capacity_t	capacity() const noexcept;
		capacity_t	count() const noexcept;

	private:
		capacity_t				_capacity;
		std::atomic<capacity_t>	_next_id;
		std::atomic<capacity_t>	_count;
	};


	template <typename Pool>
	class instance {
	public:
		instance(Pool& pool);
		~instance() noexcept;

		typename Pool::capacity_t	id() const noexcept;
		Pool&						pool() noexcept;

	private:
		Pool&						_pool;
		typename Pool::capacity_t	_id;
	};


	// --------------------------------------------------------------
	// pool

	template <typename Capacity>
	inline pool<Capacity>::pool(Capacity capacity) noexcept
		: _capacity(capacity)
		, _next_id(0)
		, _count(0) {
	}


	template <typename Capacity>
	inline pool<Capacity>::pool(pool&& other) noexcept
		: _capacity(other._capacity)
		, _next_id(other._next_id.load())
		, _count(other._count.load()) {
		other._capacity = disabled;
		other._next_id.store(0);
		other._count.store(0);
	}

	template <typename Capacity>
	inline typename pool<Capacity>::capacity_t pool<Capacity>::reserve() {
		capacity_t count = ++(_count);

		if (_capacity != unlimited && count > _capacity) {
			--(_count);
			throw "capacity"; // TODO: What exception?
		}

		return ++(_next_id);
	}


	template <typename Capacity>
	inline void pool<Capacity>::release() noexcept {
		--(_count);
	}


	template <typename Capacity>
	typename pool<Capacity>::capacity_t pool<Capacity>::capacity() const noexcept {
		return _capacity;
	}


	template <typename Capacity>
	typename pool<Capacity>::capacity_t pool<Capacity>::count() const noexcept {
		return _count;
	}


	// --------------------------------------------------------------
	// instance

	template <typename Pool>
	inline instance<Pool>::instance(Pool& pool)
		: _pool(pool) {
		_id = pool.reserve();
	}


	template <typename Pool>
	inline instance<Pool>::~instance() noexcept {
		_pool.release();
	}


	template <typename Pool>
	inline typename Pool::capacity_t instance<Pool>::id() const noexcept {
		return _id;
	}


	template <typename Pool>
	inline Pool& instance<Pool>::pool() noexcept {
		return _pool;
	}

}
