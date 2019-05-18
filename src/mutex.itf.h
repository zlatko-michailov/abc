#pragma once

#include <cstdint>
#include <atomic>
#include <mutex>


namespace abc {

	typedef std::int32_t spin_count_t;

	namespace spin_for {
		constexpr spin_count_t	memory	= -1;
		constexpr spin_count_t	os		= 25 * 1000;
		constexpr spin_count_t	disk	= 50 * 1000;
		constexpr spin_count_t	network	=  1;
	}


	template <spin_count_t SpinCount, typename Mutex>
	class spin_mutex;


	// --------------------------------------------------------------


	template <spin_count_t SpinCount, typename Mutex = std::mutex>
	class spin_mutex {
	public:
		void lock();
		bool try_lock();
		void unlock();

	private:
		std::atomic_flag	_flag;
		Mutex 				_mutex;
	};

}
