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
#include <fstream>
#include <stdexcept>

#include "log.i.h"
#include "timestamp.h"
#include "streambuf.h"
#include "exception.h"


namespace abc {

	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline log<LineSize, Container, View, Filter>::log(Container&& container, View&& view, Filter&& filter) noexcept
		: _container(std::move(container))
		, _view(std::move(view))
		, _filter(std::move(filter)) {
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline log<LineSize, Container, View, Filter>::log(log<LineSize, Container, View, Filter>&& other) noexcept
		: _container(std::move(other._container))
		, _view(std::move(other._view))
		, _filter(std::move(other._filter)) {
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline log<LineSize, Container, View, Filter>& log<LineSize, Container, View, Filter>::operator=(log<LineSize, Container, View, Filter>&& other) noexcept {
		_container = std::move(other._container);
		_view = std::move(other._view);
		_filter = std::move(other._filter);

		return *this;
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline void log<LineSize, Container, View, Filter>::push_back(category_t category, severity_t severity, tag_t tag, const char* format, ...) {
		va_list vlist;
		va_start(vlist, format);

		vpush_back(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline void log<LineSize, Container, View, Filter>::vpush_back(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
		if (_filter.is_enabled(category, severity)) {
			char line[LineSize];
			_view.format(line, LineSize, category, severity, tag, format, vlist);

			_container.push_back(line);
		}
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline void log<LineSize, Container, View, Filter>::push_back_blank(category_t category, severity_t severity) {
		if (_filter.is_enabled(category, severity)) {
			_container.push_back("");
		}
	}


	namespace log_container {
		inline ostream::ostream() noexcept
			: ostream(std::clog.rdbuf()) {
		}


		inline ostream::ostream(std::streambuf* sb) noexcept
			: _mutex()
			, _stream(sb) {
		}


		inline ostream::ostream(ostream&& other) noexcept
			: _mutex()
			, _stream(other._stream.rdbuf()) {
			other._stream.rdbuf(nullptr);
		}


		inline ostream& ostream::operator=(ostream&& other) noexcept {
			_stream.rdbuf(other._stream.rdbuf());
			other._stream.rdbuf(nullptr);

			return *this;
		}


		inline void ostream::push_back(const char* line) {
			std::lock_guard lock(_mutex);
			_stream << line << std::endl;
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::file(const char* path)
			: file<MaxPath, Clock>(path, no_rotation) {
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::file(const char* path, std::chrono::minutes::rep rotation_minutes)
			: _rotation_minutes(rotation_minutes)
			, _ostream(&_filebuf) {

			std::size_t path_length = std::strlen(path);
			if (path_length + 16 > MaxPath) {
				throw exception<std::logic_error>("std::strlen(path)", __TAG__);
			}

			_path_length = path_length;
			std::strcpy(_path, path);

			ensure_filebuf();
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::file(file<MaxPath, Clock>&& other) noexcept
			: _path_length(other._path_length)
			, _rotation_minutes(other._rotation_minutes)
			, _rotation_timestamp(other._rotation_timestamp)
			, _filebuf(std::move(other._filebuf))
			, _ostream(&_filebuf) {
			std::memmove(_path, other._path, sizeof(_path));
		}

		template <std::size_t MaxPath, typename Clock>
		inline void file<MaxPath, Clock>::push_back(const char* line) {
			if (_rotation_minutes > no_rotation) {
				ensure_filebuf();
			}
	
			_ostream.push_back(line);
		}


		template <std::size_t MaxPath, typename Clock>
		inline void file<MaxPath, Clock>::ensure_filebuf() {
			timestamp<Clock> expected_rotation_timestamp;
			if (_rotation_minutes > no_rotation) {
				expected_rotation_timestamp = expected_rotation_timestamp.coerse_minutes(_rotation_minutes);
			}

			if (_rotation_timestamp != expected_rotation_timestamp || !_filebuf.is_open()) {
				log_view::format_timestamp(_path + _path_length, MaxPath + 1 - _path_length, expected_rotation_timestamp, log_view::format::datetime::file);
				_rotation_timestamp = expected_rotation_timestamp;

				if (_filebuf.is_open()) {
					_filebuf.close();
				}

				_filebuf.open(_path, std::ios_base::out);
			}
		}
	}


	namespace log_view {
		template <typename Clock>
		inline void debug<Clock>::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			char buf_timestamp[31];
			format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp<Clock>(), format::datetime::friendly);

			char buf_thread_id[17];
			format_thread_id(buf_thread_id, sizeof(buf_thread_id), std::this_thread::get_id());

			char buf_category[5];
			format_category(buf_category, sizeof(buf_category), category, format::category::friendly);

			char buf_severity[2];
			format_severity(buf_severity, sizeof(buf_severity), severity, format::severity::friendly);

			char buf_tag[17];
			format_tag(buf_tag, sizeof(buf_tag), tag, format::tag::friendly);

			int char_count = std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::friendly, buf_thread_id, format::separator::friendly, buf_category, format::separator::friendly, buf_severity, format::separator::friendly, buf_tag, format::separator::friendly);
			if (0 <= char_count && char_count < line_size) {
				std::vsnprintf(line + char_count, line_size - char_count, format, vlist);
			}
		}


		template <typename Clock>
		inline void diag<Clock>::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			char buf_timestamp[31];
			format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp<Clock>(), format::datetime::iso);

			char buf_thread_id[17];
			format_thread_id(buf_thread_id, sizeof(buf_thread_id), std::this_thread::get_id());

			char buf_category[5];
			format_category(buf_category, sizeof(buf_category), category, format::category::compact);

			char buf_severity[2];
			format_severity(buf_severity, sizeof(buf_severity), severity, format::severity::compact);

			char buf_tag[17];
			format_tag(buf_tag, sizeof(buf_tag), tag, format::tag::compact);

			int char_count = std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::compact, buf_thread_id, format::separator::compact, buf_category, format::separator::compact, buf_severity, format::separator::compact, buf_tag, format::separator::compact);
			if (0 <= char_count && char_count < line_size) {
				std::vsnprintf(line + char_count, line_size - char_count, format, vlist);
			}
		}


		template <typename Clock>
		inline void test<Clock>::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			char buf_timestamp[31];
			format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp<Clock>(), format::datetime::friendly);

			char buf_severity[2 * severity::abc + 1];
			severity = severity <= severity::abc ? severity : severity::abc;
			std::memset(buf_severity, ' ', 2 * severity);
			buf_severity[2 * (severity - 1)] = '\0';

			char buf_tag[17];
			if (tag != tag::none) {
				format_tag(buf_tag, sizeof(buf_tag), tag, format::tag::compact);
			}
			else {
				buf_tag[0] = '\0';
			}

			int char_count = std::snprintf(line, line_size, "%s%s%s%s%s%s", buf_timestamp, format::separator::space, buf_severity, format::separator::space, buf_tag, format::separator::space);
			if (0 <= char_count && char_count < line_size) {
				std::vsnprintf(line + char_count, line_size - char_count, format, vlist);
			}
		}


		inline void blank::format(char* line, std::size_t line_size, category_t /*category*/, severity_t /*severity*/, tag_t /*tag*/, const char* format, va_list vlist) {
			std::vsnprintf(line, line_size, format, vlist);
		}


		template <typename Clock>
		inline int format_timestamp(char* line, std::size_t line_size, const timestamp<Clock>& ts, const char* format) {
			return std::snprintf(line, line_size, format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
		}


		inline int format_thread_id(char* line, std::size_t line_size, std::thread::id thread_id) {
			if (line_size < 17) {
				throw exception<std::logic_error>("line_size", __TAG__);
			}

			streambuf_adapter streambuf(line, &line[line_size - 1]);
			std::ostream stream(&streambuf);
			stream << std::hex << thread_id << '\0';
			return std::strlen(line);
		}


		inline int format_category(char* line, std::size_t line_size, category_t category, const char* format) {
			return std::snprintf(line, line_size, format, category);
		}


		inline int format_severity(char* line, std::size_t line_size, severity_t severity, const char* format) {
			return std::snprintf(line, line_size, format, severity);
		}


		inline int format_tag(char* line, std::size_t line_size, tag_t tag, const char* format) {
			return std::snprintf(line, line_size, format, tag);
		}
	}


	namespace log_filter {
		inline bool none::is_enabled(category_t category, severity_t severity) const noexcept {
			return true;
		}


		inline bool off::is_enabled(category_t category, severity_t severity) const noexcept {
			return false;
		}


		inline severity::severity(severity_t min_severity) noexcept
			: _min_severity(min_severity) {
		}

		inline bool severity::is_enabled(category_t /*category*/, severity_t severity) const noexcept {
			return abc::severity::is_higher_or_equal(severity, _min_severity);
		}
	}


	inline bool severity::is_higher(severity_t severity, severity_t other) noexcept {
		return severity < other;
	}


	inline bool severity::is_higher_or_equal(severity_t severity, severity_t other) noexcept {
		return severity <= other;
	}

}
