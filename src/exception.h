/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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
#include "log.i.h"


namespace abc {

	template <typename Exception, typename LogPtr>
	class exception;

	// logic_error
	// runtime_error


	// --------------------------------------------------------------


	template <typename Exception, typename LogPtr = null_log_ptr>
	class exception : public Exception {

	public:
		exception(const char* message, tag_t tag, const LogPtr& log_ptr = nullptr);

	public:
		tag_t	tag() const noexcept;

	private:
		tag_t	_tag;
	};


	// --------------------------------------------------------------


	template <typename Exception, typename LogPtr>
	inline exception<Exception, LogPtr>::exception(const char* message, tag_t tag, const LogPtr& log_ptr)
		: Exception(message)
		, _tag(tag) {
		if (log_ptr != nullptr) {
			log_ptr->push_back(category::abc::exception, severity::warning, __TAG__, "Exception thrown! %s", message);
		}
	}


	template <typename Exception, typename LogPtr>
	inline tag_t exception<Exception, LogPtr>::tag() const noexcept {
		return _tag;
	}

}
