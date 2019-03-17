#pragma once

#include <cstdint>
#include <atomic>


namespace abc {

	template <typename InstanceId>
	class unlimited_pool;

	template <typename InstanceId>
	class pool;

	template <typename InstanceId, typename Pool>
	class instance;


	// --------------------------------------------------------------


	template <typename InstanceId>
	class unlimited_pool {
	public:
		typedef InstanceId instance_id_t;
		
	public:
		unlimited_pool() noexcept;

	protected:
		friend class instance<InstanceId, unlimited_pool<InstanceId>>;

		InstanceId reserve_instance() noexcept;
		void release_instance() noexcept;

	private:
		std::atomic<InstanceId>	_instance_id;
	};


	template <typename InstanceId>
	class pool : public unlimited_pool<InstanceId> {
	public:
		pool(InstanceId capacity) noexcept;

	protected:
		friend class instance<InstanceId, pool<InstanceId>>;

		InstanceId reserve_instance();
		void release_instance() noexcept;

	private:
		const InstanceId		_capacity;
		std::atomic<InstanceId>	_instance_count;
	};


	template <typename InstanceId, typename Pool>
	class instance {
	public:
		typedef InstanceId instance_id_t;
		
	public:
		instance(Pool& pool);
		~instance() noexcept;

		InstanceId instance_id() const noexcept;

		Pool& pool() noexcept;

	private:
		Pool&			_pool;
		InstanceId		_instance_id;
	};


	// --------------------------------------------------------------


	template <typename InstanceId>
	inline unlimited_pool<InstanceId>::unlimited_pool() noexcept
		: _instance_id(0) {
	}


	template <typename InstanceId>
	inline InstanceId unlimited_pool<InstanceId>::reserve_instance() noexcept {
		return _instance_id++;
	}


	template <typename InstanceId>
	inline void unlimited_pool<InstanceId>::release_instance() noexcept {
	}


	template <typename InstanceId>
	inline pool<InstanceId>::pool(InstanceId capacity) noexcept
		: unlimited_pool<InstanceId>()
		, _capacity(capacity)
		, _instance_count(0) {
	}


	template <typename InstanceId>
	inline InstanceId pool<InstanceId>::reserve_instance() {
		if (++(_instance_count) > _capacity) {
			--(_instance_count);
			throw; // TODO: What exception?
		}

		return unlimited_pool<InstanceId>::reserve_instance();
	}


	template <typename InstanceId>
	inline void pool<InstanceId>::release_instance() noexcept {
		--(_instance_count);
	}


	template <typename InstanceId, typename Pool>
	inline instance<InstanceId, Pool>::instance(Pool& pool)
		: _pool(pool) {
		_instance_id = pool.reserve_instance();
	}


	template <typename InstanceId, typename Pool>
	inline instance<InstanceId, Pool>::~instance() noexcept {
		_pool.release_instance();
	}


	template <typename InstanceId, typename Pool>
	inline InstanceId instance<InstanceId, Pool>::instance_id() const noexcept {
		return _instance_id;
	}


	template <typename InstanceId, typename Pool>
	inline Pool& instance<InstanceId, Pool>::pool() noexcept {
		return _pool;
	}

}
