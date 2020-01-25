#pragma once

#include "os.itf.h"
#include "log.itf.h"


namespace abc {

	class host;


	// --------------------------------------------------------------


	class host {

	public:
		virtual abc::os&	os() noexcept = 0;
		virtual log&		diag() noexcept = 0;
	};


}
