#pragma once

#include "host.itf.h"
#include "log.h"
#include "os.h"


namespace abc {

	class posix_host;


	// --------------------------------------------------------------


	class posix_host
		: public host {

	public:
		posix_host(posix_os&& os, log&& diag);

	public:
		virtual abc::os&	os() noexcept override;
		virtual log&		diag() noexcept override;

	private:
		posix_os	_os;
		log			_diag;
	};


	// --------------------------------------------------------------


	inline posix_host::posix_host(posix_os&& os, log&& diag)
		: _os(std::move(os))
		, _diag(std::move(diag)) {
	}


	abc::os& posix_host::os() noexcept {
		return _os;
	}


	log& posix_host::diag() noexcept {
		return _diag;
	}


}
