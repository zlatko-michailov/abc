#pragma once

#include "os.itf.h"


namespace abc {

	class host;

		class posix_host;


	// --------------------------------------------------------------

	class host {

	public:
		virtual abc::os& os() const noexcept = 0;
		// TODO: virtual log& diag() const noexcept = 0;
	};


	class posix_host
		: public host {

	public:
		posix_host(posix&& os);

	public:
		virtual abc::os& os() const noexcept override;
		// TODO: virtual log& diag() const noexcept override;

	private:
		posix	_os;
	};

}
