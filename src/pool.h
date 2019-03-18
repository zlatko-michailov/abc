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

	protected:
		friend class instance<pool<capacity_t>>;

		capacity_t	reserve_instance();
		void		release_instance() noexcept;

		capacity_t	active_instance_count() const noexcept;

	private:
		const capacity_t		_capacity;
		std::atomic<capacity_t>	_next_instance_id;
		std::atomic<capacity_t>	_active_instance_count;
	};


	template <typename Pool>
	class instance {
	public:
		instance(const std::shared_ptr<Pool>& pool);
		~instance() noexcept;

		typename Pool::capacity_t		instance_id() const noexcept;
		const std::shared_ptr<Pool>&	pool() const noexcept;

	private:
		std::shared_ptr<Pool>		_pool;
		typename Pool::capacity_t	_instance_id;
	};


	// --------------------------------------------------------------


	template <typename Capacity>
	inline pool<Capacity>::pool(Capacity capacity) noexcept
		: _capacity(capacity)
		, _next_instance_id(0)
		, _active_instance_count(0) {
	}


	template <typename Capacity>
	inline typename pool<Capacity>::capacity_t pool<Capacity>::reserve_instance() {
		capacity_t active_instance_count = ++(_active_instance_count);

		if (_capacity != unlimited && active_instance_count > _capacity) {
			--(_active_instance_count);
			throw; // TODO: What exception?
		}

		return ++(_next_instance_id);
	}


	template <typename Capacity>
	inline void pool<Capacity>::release_instance() noexcept {
		--(_active_instance_count);
	}


	template <typename Capacity>
	typename pool<Capacity>::capacity_t pool<Capacity>::active_instance_count() const noexcept {
		return _active_instance_count;
	}


	template <typename Pool>
	inline instance<Pool>::instance(const std::shared_ptr<Pool>& pool)
		: _pool(pool) {
		_instance_id = pool->reserve_instance();
	}


	template <typename Pool>
	inline instance<Pool>::~instance() noexcept {
		_pool->release_instance();
	}


	template <typename Pool>
	inline typename Pool::capacity_t instance<Pool>::instance_id() const noexcept {
		return _instance_id;
	}


	template <typename Pool>
	inline const std::shared_ptr<Pool>& instance<Pool>::pool() const noexcept {
		return _pool;
	}

}
