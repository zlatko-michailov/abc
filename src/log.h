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

#include <cstdarg>
#include <cstring>
#include <stdexcept>

#include "log.i.h"
#include "timestamp.h"
#include "buffer_streambuf.h"
#include "exception.h"
#include "table.h"

namespace abc {

	template <std::size_t Size>
	inline debug_line_ostream<Size>::debug_line_ostream()
		: base() {
	}


	template <std::size_t Size>
	inline debug_line_ostream<Size>::debug_line_ostream(table_ostream* table)
		: base(table) {
	}


	template <std::size_t Size>
	inline void debug_line_ostream<Size>::put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t Size>
	inline void debug_line_ostream<Size>::put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
		put_props(category, severity, tag);

		base::put_anyv(format, vlist);
	}


	template <std::size_t Size>
	inline void debug_line_ostream<Size>::put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) {
		std::size_t buffer_offset = 0;
		std::size_t pcount = 1;

		while (pcount != 0) {
			if (buffer_offset != 0) {
				base::flush();
			}

			put_props(category, severity, tag);

			pcount = base::put_binary(buffer, buffer_size, buffer_offset);
		}
	}

	template <std::size_t Size>
	inline void debug_line_ostream<Size>::put_props(category_t category, severity_t severity, tag_t tag) {
		base::put_timestamp(timestamp<>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u |");
		base::put_thread_id(std::this_thread::get_id(), " %16s |");
		base::put_any(" %4.4x |", category);
		base::put_any(" %1.1x |", severity);
		base::put_any(" %16llx | ", tag);
	}


	// --------------------------------------------------------------


	template <std::size_t Size>
	inline diag_line_ostream<Size>::diag_line_ostream()
		: base() {
	}


	template <std::size_t Size>
	inline diag_line_ostream<Size>::diag_line_ostream(table_ostream* table)
		: base(table) {
	}


	template <std::size_t Size>
	inline void diag_line_ostream<Size>::put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t Size>
	inline void diag_line_ostream<Size>::put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
		put_props(category, severity, tag);

		base::put_anyv(format, vlist);
	}


	template <std::size_t Size>
	inline void diag_line_ostream<Size>::put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) {
		std::size_t buffer_offset = 0;
		std::size_t pcount = 1;

		while (pcount != 0) {
			if (buffer_offset != 0) {
				base::flush();
			}

			put_props(category, severity, tag);

			pcount = base::put_binary(buffer, buffer_size, buffer_offset);
		}
	}

	template <std::size_t Size>
	inline void diag_line_ostream<Size>::put_props(category_t category, severity_t severity, tag_t tag) {
		base::put_timestamp(timestamp<>(), "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ,");
		base::put_thread_id(std::this_thread::get_id(), "%s,");
		base::put_any("%.4x,", category);
		base::put_any("%.1x,", severity);
		base::put_any("%llx,", tag);
	}


	// --------------------------------------------------------------


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


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline void log<LineSize, Container, View, Filter>::push_back_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) {
		if (_filter.is_enabled(category, severity)) {
			char line[LineSize];
			std::size_t buffer_offset = 0;
			while (_view.format_binary(line, LineSize, category, severity, tag, buffer, buffer_size, buffer_offset) != 0) {
				_container.push_back(line);
			}
		}
	}


	namespace log_container {
		inline ostream::ostream() noexcept
			: ostream(std::cout.rdbuf()) {
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
				throw exception<std::logic_error>("std::strlen(path)", 0x10002);
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
			int fixed_size = format_fixed(line, line_size, category, severity, tag);
			if (0 <= fixed_size && fixed_size < line_size) {
				std::vsnprintf(line + fixed_size, line_size - fixed_size, format, vlist);
			}
		}


		template <typename Clock>
		inline int debug<Clock>::format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) {
			int fixed_size = format_fixed(line, line_size, category, severity, tag);
			if (0 <= fixed_size && fixed_size < line_size) {
				int binary_size = log_view::format_binary(line + fixed_size, line_size - fixed_size, buffer, buffer_size, buffer_offset);
				if (0 < binary_size) {
					return fixed_size + binary_size;
				}
			}

			return 0;
		}


		template <typename Clock>
		inline int debug<Clock>::format_fixed(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag) {
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

			return std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::friendly, buf_thread_id, format::separator::friendly, buf_category, format::separator::friendly, buf_severity, format::separator::friendly, buf_tag, format::separator::friendly);
		}


		template <typename Clock>
		inline void diag<Clock>::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			int fixed_size = format_fixed(line, line_size, category, severity, tag);
			if (0 <= fixed_size && fixed_size < line_size) {
				std::vsnprintf(line + fixed_size, line_size - fixed_size, format, vlist);
			}
		}


		template <typename Clock>
		inline int diag<Clock>::format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) {
			int fixed_size = format_fixed(line, line_size, category, severity, tag);
			if (0 <= fixed_size && fixed_size < line_size) {
				int binary_size = log_view::format_binary(line + fixed_size, line_size - fixed_size, buffer, buffer_size, buffer_offset);
				if (0 < binary_size) {
					return fixed_size + binary_size;
				}
			}

			return 0;
		}


		template <typename Clock>
		inline int diag<Clock>::format_fixed(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag) {
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

			return std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::compact, buf_thread_id, format::separator::compact, buf_category, format::separator::compact, buf_severity, format::separator::compact, buf_tag, format::separator::compact);
		}


		template <typename Clock>
		inline test<Clock>::test(severity_t severity) noexcept
			: _severity(severity) {
		}


		template <typename Clock>
		inline void test<Clock>::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			int fixed_size = format_fixed(line, line_size, category, severity, tag);
			if (0 <= fixed_size && fixed_size < line_size) {
				std::vsnprintf(line + fixed_size, line_size - fixed_size, format, vlist);
			}
		}


		template <typename Clock>
		inline int test<Clock>::format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) {
			int fixed_size = format_fixed(line, line_size, category, severity, tag);
			if (0 <= fixed_size && fixed_size < line_size) {
				int binary_size = log_view::format_binary(line + fixed_size, line_size - fixed_size, buffer, buffer_size, buffer_offset);
				if (0 < binary_size) {
					return fixed_size + binary_size;
				}
			}

			return 0;
		}


		template <typename Clock>
		inline int test<Clock>::format_fixed(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag) {
			char buf_timestamp[31];
			if (_severity >= severity::important) {
				format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp<Clock>(), format::datetime::friendly);
			}
			else {
				buf_timestamp[0] = '\0';
			}

			char buf_thread_id[17];
			if (_severity >= severity::debug) {
				format_thread_id(buf_thread_id, sizeof(buf_thread_id), std::this_thread::get_id());
			}
			else {
				buf_thread_id[0] = '\0';
			}

			char buf_tag[17];
			if (_severity >= severity::debug) {
				format_tag(buf_tag, sizeof(buf_tag), tag, format::tag::friendly);
			}
			else {
				buf_tag[0] = '\0';
			}

			char buf_severity[2 * severity::abc + 1];
			severity = severity <= severity::abc ? severity : severity::abc;
			std::memset(buf_severity, ' ', 2 * severity);
			buf_severity[2 * (severity - 1)] = '\0';

			return std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::space, buf_thread_id, format::separator::space, buf_tag, format::separator::space, buf_severity, format::separator::space);
		}


		inline void blank::format(char* line, std::size_t line_size, category_t /*category*/, severity_t /*severity*/, tag_t /*tag*/, const char* format, va_list vlist) {
			std::vsnprintf(line, line_size, format, vlist);
		}


		inline int blank::format_binary(char* line, std::size_t line_size, category_t /*category*/, severity_t /*severity*/, tag_t /*tag*/, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) {
			return log_view::format_binary(line, line_size, buffer, buffer_size, buffer_offset);
		}


		template <typename Clock>
		inline int format_timestamp(char* line, std::size_t line_size, const timestamp<Clock>& ts, const char* format) {
			return std::snprintf(line, line_size, format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
		}


		inline int format_thread_id(char* line, std::size_t line_size, std::thread::id thread_id) {
			if (line_size < 17) {
				return 0;
			}

			buffer_streambuf sb(nullptr, 0, 0, line, 0, line_size - 1);
			std::ostream stream(&sb);
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

		inline int format_binary(char* line, std::size_t line_size, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) {
			// 0000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  abcdefghijklmnop
			constexpr std::size_t half_chunk_size = 8;
			constexpr std::size_t chunk_size = half_chunk_size * 2;
			constexpr std::size_t local_size = 5 + (chunk_size * 3) + 1 + 2 + chunk_size + 1;
			constexpr char hex[] = "0123456789abcdef";

			if (line_size < local_size) {
				return 0;
			}

			if (buffer_size <= buffer_offset) {
				return 0;
			}

			if ((buffer_offset % chunk_size) != 0) {
				return 0;
			}

			const std::uint8_t* chunk = static_cast<const std::uint8_t*>(buffer) + buffer_offset;
			std::size_t local_offset = 0;

			// 0000:
			line[local_offset++] = hex[(buffer_offset >> 12) & 0xf];
			line[local_offset++] = hex[(buffer_offset >>  8) & 0xf];
			line[local_offset++] = hex[(buffer_offset >>  4) & 0xf];
			line[local_offset++] = hex[buffer_offset & 0xf];
			line[local_offset++] = ':';

			// 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f
			for (int half = 0; half < 2; half++) {
				line[local_offset++] = ' ';

				for (std::size_t chunk_offset = 0; chunk_offset < half_chunk_size; chunk_offset++) {
					std::size_t source_offset = half * half_chunk_size + chunk_offset;

					if (buffer_offset + source_offset < buffer_size) {
						line[local_offset++] = hex[(chunk[source_offset] >> 4) & 0xf];
						line[local_offset++] = hex[chunk[source_offset] & 0xf];
					}
					else {
						line[local_offset++] = ' ';
						line[local_offset++] = ' ';
					}
		
					line[local_offset++] = ' ';
				}
			}

			line[local_offset++] = ' ';

			//  abcdefghijklmnop
			for (std::size_t chunk_offset = 0; chunk_offset < chunk_size; chunk_offset++) {
				if (buffer_offset + chunk_offset < buffer_size) {
					if (std::isprint(chunk[chunk_offset])) {
						line[local_offset++] = chunk[chunk_offset];
					}
					else {
						line[local_offset++] = '.';
					}
				}
				else {
					line[local_offset++] = ' ';
				}
			}

			line[local_offset++] = '\0';

			buffer_offset += chunk_size;
			return static_cast<int>(chunk_size);
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
