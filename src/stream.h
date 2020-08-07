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

#include "log.h"
#include "stream.i.h"


namespace abc {

	template <typename Stream, typename LogPtr>
	inline _stream<Stream, LogPtr>::_stream(std::streambuf* sb, const LogPtr& log_ptr)
		: base(sb)
		, _log_ptr(log_ptr) {
		if (_log_ptr != nullptr) {
			_log_ptr->put_any(category::abc::stream, severity::abc, __TAG__, "_stream::_stream()");
		}
	}


	template <typename Stream, typename LogPtr>
	inline void _stream<Stream, LogPtr>::reset() {
		if (_log_ptr != nullptr) {
			_log_ptr->put_any(category::abc::stream, severity::abc, __TAG__, "_stream::reset()");
		}

		base::clear(base::goodbit);
	}


	template <typename Stream, typename LogPtr>
	inline bool _stream<Stream, LogPtr>::eof() const {
		return base::eof();
	}


	template <typename Stream, typename LogPtr>
	inline bool _stream<Stream, LogPtr>::good() const {
		return base::good();
	}


	template <typename Stream, typename LogPtr>
	inline bool _stream<Stream, LogPtr>::bad() const {
		return base::bad();
	}


	template <typename Stream, typename LogPtr>
	inline bool _stream<Stream, LogPtr>::fail() const {
		return base::fail();
	}


	template <typename Stream, typename LogPtr>
	inline bool _stream<Stream, LogPtr>::operator!() const {
		return base::operator!();
	}


	template <typename Stream, typename LogPtr>
	inline _stream<Stream, LogPtr>::operator bool() const {
		return base::operator bool();
	}


	template <typename Stream, typename LogPtr>
	inline bool _stream<Stream, LogPtr>::is_good() const {
		return base::good() && !Stream::eof();
	}


	template <typename Stream, typename LogPtr>
	inline void _stream<Stream, LogPtr>::set_bad() {
		base::clear(base::badbit | base::failbit);
	}


	template <typename Stream, typename LogPtr>
	inline void _stream<Stream, LogPtr>::set_fail() {
		base::setstate(base::failbit);
	}


	template <typename Stream, typename LogPtr>
	inline const LogPtr& _stream<Stream, LogPtr>::log_ptr() const noexcept {
		return _log_ptr;
	}


	// --------------------------------------------------------------


	template <typename LogPtr>
	inline _istream<LogPtr>::_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: base(sb, log_ptr)
		, _gcount(gcount) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::stream, severity::abc, __TAG__, "_istream::_istream()");
		}
	}


	template <typename LogPtr>
	inline void _istream<LogPtr>::reset() {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::stream, severity::abc, __TAG__, "_stream::reset()");
		}

		base::reset();
		_gcount = 0;
	}


	template <typename LogPtr>
	inline std::size_t _istream<LogPtr>::gcount() const noexcept {
		return _gcount;
	}


	template <typename LogPtr>
	inline void _istream<LogPtr>::set_gcount(std::size_t gcount) noexcept {
		_gcount = gcount;
	}

}

