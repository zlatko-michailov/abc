/*
MIT License

Copyright (c) 2018 Zlatko Michailov 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


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
