#pragma once

#include <cstdint>


namespace abc {

	typedef std::uint64_t	tag_t;


	constexpr tag_t __TAG__ = 0;

	namespace tag {
		constexpr tag_t none = 0;
	}

}
