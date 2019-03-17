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


	template <typename InstanceId>
	class unlimited_pool {
	public:
		typedef InstanceId instance_id_t;
		
	public:
		unlimited_pool() noexcept
			: _instance_id(0) {
		}

	protected:
		friend class instance<InstanceId, unlimited_pool<InstanceId>>;

		InstanceId reserve_instance() noexcept {
			return _instance_id++;
		}

		void release_instance() noexcept {
		}

	private:
		std::atomic<InstanceId>	_instance_id;
	};


	template <typename InstanceId>
	class pool : public unlimited_pool<InstanceId> {
	public:
		pool(InstanceId capacity) noexcept
			: unlimited_pool<InstanceId>()
			, _capacity(capacity)
			, _instance_count(0) {
		}

	protected:
		friend class instance<InstanceId, pool<InstanceId>>;

		InstanceId reserve_instance() {
			if (++(_instance_count) > _capacity) {
				--(_instance_count);
				throw;
			}

			return unlimited_pool<InstanceId>::reserve_instance();
		}

		void release_instance() noexcept {
			--(_instance_count);
		}

	private:
		const InstanceId		_capacity;
		std::atomic<InstanceId>	_instance_count;
	};


	template <typename InstanceId, typename Pool>
	class instance {
	public:
		typedef InstanceId instance_id_t;
		
	public:
		instance(Pool& pool)
			: _pool(pool) {
			_instance_id = pool.reserve_instance();
		}

		~instance() noexcept {
			_pool.release_instance();
		}

		InstanceId instance_id() const noexcept {
			return _instance_id;
		}

		Pool& pool() noexcept {
			return _pool;
		}

	private:
		Pool&			_pool;
		InstanceId		_instance_id;
	};
}
