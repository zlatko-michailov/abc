#pragma once

#include <stdexcept>

#include "tag.h"


namespace abc {

	class exception;

	// logic_error
	class unexpected;

	// runtime_error
	class failed;


	// --------------------------------------------------------------


	class exception {

	public:
		exception(tag_t tag);

	public:
		tag_t	tag() const noexcept;

	private:
		tag_t	_tag;
	};


	// --------------------------------------------------------------


	class unexpected
		: public std::logic_error
		, public abc::exception {

	public:
		unexpected(const char* message, tag_t tag);
	};


	// --------------------------------------------------------------


	class failed
		: public std::runtime_error
		, public abc::exception {

	public:
		failed(const char* message, tag_t tag);
	};


	// --------------------------------------------------------------


	inline exception::exception(tag_t tag)
		: _tag(tag) {
	}


	inline tag_t exception::tag() const noexcept {
		return _tag;
	}


	// --------------------------------------------------------------


	inline unexpected::unexpected(const char* message, tag_t tag)
		: std::logic_error(message)
		, abc::exception(tag) {
	}


	// --------------------------------------------------------------


	inline failed::failed(const char* message, tag_t tag)
		: std::runtime_error(message)
		, abc::exception(tag) {
	}

}
