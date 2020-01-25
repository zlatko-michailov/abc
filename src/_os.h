#pragma once

#include <cstdint>
#include <functional>

#include "os.itf.h"

namespace abc {

	class posix_os;


	// --------------------------------------------------------------


	class posix_os
		: public os {

	public:
		virtual void fork() override;
	};

	
}
