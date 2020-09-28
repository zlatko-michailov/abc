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

#include <cstring>
#include <cstdio>

#include "multifile.i.h"
#include "timestamp.h"
#include "exception.h"
#include "tag.h"


namespace abc {

	template <std::size_t MaxPath, typename Clock, typename Log>
	inline multifile_streambuf<MaxPath, Clock, Log>::multifile_streambuf(const char* path, std::ios_base::openmode mode, Log* log)
		: base()
		, _mode(mode)
		, _log(log) {
		if (path == nullptr) {
			throw exception<std::logic_error, Log>("path", 0x102b0, log);
		}

		_path_length = std::strlen(path);
		if (_path_length + 1 + filename_length > MaxPath) {
			throw exception<std::logic_error, Log>("std::strlen(path)", 0x102b1, log);
		}

		std::strncpy(_path, path, _path_length);
		if (_path[_path_length - 1] != path_separator) {
			_path[_path_length++] = path_separator;
		}

		reopen();
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline multifile_streambuf<MaxPath, Clock, Log>::multifile_streambuf(multifile_streambuf&& other) noexcept
		: base(std::move(other)) {
		std::strncpy(_path, other._path, MaxPath);
		_path_length = other._path_length;
		_mode = other._mode;
		_log = other._log;

		other._path[0] = 0;
		other._path_length = 0;
		other._log = nullptr;
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline void multifile_streambuf<MaxPath, Clock, Log>::reopen() {
		if (base::is_open()) {
			base::close();

			if (_log != nullptr) {
				_log->put_any(category::abc::multifile, severity::abc::debug, 0x102b2, "multifile_streambuf::reopen() Close path=%s", _path);
			}
		}

		timestamp<Clock> ts;
		std::snprintf(_path + _path_length, MaxPath - _path_length, filename_format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds());

		std::filebuf* op = base::open(_path, _mode);

		if (_log != nullptr) {
			_log->put_any(category::abc::multifile, severity::abc::optional, 0x102b3, "multifile_streambuf::reopen() Open path=%s, succes=%u", _path, op != nullptr);
		}
	}


	// --------------------------------------------------------------


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline duration_multifile_streambuf<MaxPath, Clock, Log>::duration_multifile_streambuf(typename Clock::duration duration, const char* path, std::ios_base::openmode mode, Log* log)
		: base(path, mode, log)
		, _duration(duration)
		, _ts() {
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline void duration_multifile_streambuf<MaxPath, Clock, Log>::reopen() {
		base::reopen();

		_ts = std::move(timestamp<Clock>());
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline int duration_multifile_streambuf<MaxPath, Clock, Log>::sync() {
		base::sync();

		timestamp<Clock> ts;
		if (ts - _ts >= _duration) {
			reopen();
		}

		return 0;
	}


	// --------------------------------------------------------------


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline size_multifile_streambuf<MaxPath, Clock, Log>::size_multifile_streambuf(std::size_t size, const char* path, std::ios_base::openmode mode, Log* log)
		: base(path, mode, log)
		, _size(size)
		, _current_size(0) {
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline void size_multifile_streambuf<MaxPath, Clock, Log>::reopen() {
		base::reopen();

		_current_size = 0;
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline std::streamsize size_multifile_streambuf<MaxPath, Clock, Log>::xsputn(const char* s, std::streamsize count) {
		_current_size += count;

		return base::xsputn(s, count);
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline int size_multifile_streambuf<MaxPath, Clock, Log>::sync() {
		_current_size += pcount();

		base::sync();

		if (_current_size >= _size) {
			reopen();
		}

		return 0;
	}


	template <std::size_t MaxPath, typename Clock, typename Log>
	inline std::size_t size_multifile_streambuf<MaxPath, Clock, Log>::pcount() const noexcept {
		return base::pptr() - base::pbase();
	}


	// --------------------------------------------------------------
}
