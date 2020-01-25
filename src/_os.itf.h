#pragma once

#include <cstdint>
#include <functional>


namespace abc {

	class os;


	// --------------------------------------------------------------


	class os {

	public:
		virtual void fork() = 0;
	};


}
