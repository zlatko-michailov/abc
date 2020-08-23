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

	template <typename Log, std::size_t MaxPath, typename Clock>
	inline multifile_streambuf<Log, MaxPath, Clock>::multifile_streambuf(const char* path, std::ios_base::openmode mode, Log* log)
		: base()
		, _mode(mode)
		, _log(log) {
		if (path == nullptr) {
			throw exception<std::logic_error, Log>("path", __TAG__, log);
		}

		_path_length = std::strlen(path);
		if (_path_length + 1 + filename_length > MaxPath) {
			throw exception<std::logic_error, Log>("std::strlen(path)", __TAG__, log);
		}

		std::strncpy(_path, path, _path_length);
		if (_path[_path_length - 1] != path_separator) {
			_path[_path_length++] = path_separator;
		}

		reopen();
	}


	template <typename Log, std::size_t MaxPath, typename Clock>
	inline multifile_streambuf<Log, MaxPath, Clock>::multifile_streambuf(multifile_streambuf&& other) noexcept
		: base(std::move(other)) {
		std::strncpy(_path, other._path, MaxPath);
		_path_length = other._path_length;
		_mode = other._mode;
		_log = other._log;

		other._path[0] = 0;
		other._path_length = 0;
		other._log = nullptr;
	}


	template <typename Log, std::size_t MaxPath, typename Clock>
	inline void multifile_streambuf<Log, MaxPath, Clock>::reopen() {
		if (base::is_open()) {
			base::close();

			if (_log != nullptr) {
				_log->put_any(category::abc::multifile, severity::abc, __TAG__, "multifile_streambuf::reopen() Close path=%s", _path);
			}
		}

		timestamp<Clock> ts;
		std::snprintf(_path + _path_length, MaxPath - _path_length, filename_format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds());

		std::filebuf* op = base::open(_path, _mode);

		if (_log != nullptr) {
			_log->put_any(category::abc::multifile, severity::abc, __TAG__, "multifile_streambuf::reopen() Open path=%s, succes=%u", _path, op != nullptr);
		}
	}


	// --------------------------------------------------------------

}
