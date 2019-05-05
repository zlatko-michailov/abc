#pragma once

#include <cstdint>
#include <functional>


namespace abc {

	class os;

		class posix;


	// --------------------------------------------------------------

	class os {

	public:
		virtual void fork() = 0;
	};


	class posix
		: public os {

	public:
		virtual void fork() override;
	};

	
}
