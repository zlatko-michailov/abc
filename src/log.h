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

#include "log.i.h"
#include "timestamp.h"
#include "buffer_streambuf.h"
#include "table.h"

namespace abc {

	template <std::size_t Size, typename Clock>
	inline debug_line_ostream<Size, Clock>::debug_line_ostream()
		: base() {
	}


	template <std::size_t Size, typename Clock>
	inline debug_line_ostream<Size, Clock>::debug_line_ostream(table_ostream* table)
		: base(table) {
	}


	template <std::size_t Size, typename Clock>
	inline void debug_line_ostream<Size, Clock>::put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t Size, typename Clock>
	inline void debug_line_ostream<Size, Clock>::put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
		put_props(category, severity, tag);

		base::put_anyv(format, vlist);
	}


	template <std::size_t Size, typename Clock>
	inline void debug_line_ostream<Size, Clock>::put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
		std::size_t buffer_offset = 0;
		bool hasMore = true;

		while (hasMore) {
			if (buffer_offset != 0) {
				base::flush();
			}

			put_props(category, severity, tag);

			hasMore = base::put_binary(buffer, buffer_size, buffer_offset);
		}
	}

	template <std::size_t Size, typename Clock>
	inline void debug_line_ostream<Size, Clock>::put_props(category_t category, severity_t severity, tag_t tag) noexcept {
		base::put_timestamp(timestamp<Clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u |");
		base::put_thread_id(std::this_thread::get_id(), " %16s |");
		base::put_any(" %4.4x |", category);
		base::put_any(" %1.1x |", severity);
		base::put_any(" %16llx | ", tag);
	}


	// --------------------------------------------------------------


	template <std::size_t Size, typename Clock>
	inline diag_line_ostream<Size, Clock>::diag_line_ostream()
		: base() {
	}


	template <std::size_t Size, typename Clock>
	inline diag_line_ostream<Size, Clock>::diag_line_ostream(table_ostream* table)
		: base(table) {
	}


	template <std::size_t Size, typename Clock>
	inline void diag_line_ostream<Size, Clock>::put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t Size, typename Clock>
	inline void diag_line_ostream<Size, Clock>::put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
		put_props(category, severity, tag);

		base::put_anyv(format, vlist);
	}


	template <std::size_t Size, typename Clock>
	inline void diag_line_ostream<Size, Clock>::put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
		std::size_t buffer_offset = 0;
		bool hasMore = true;

		while (hasMore) {
			if (buffer_offset != 0) {
				base::flush();
			}

			put_props(category, severity, tag);

			hasMore = base::put_binary(buffer, buffer_size, buffer_offset);
		}
	}

	template <std::size_t Size, typename Clock>
	inline void diag_line_ostream<Size, Clock>::put_props(category_t category, severity_t severity, tag_t tag) noexcept {
		base::put_timestamp(timestamp<Clock>(), "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ,");
		base::put_thread_id(std::this_thread::get_id(), "%s,");
		base::put_any("%.4x,", category);
		base::put_any("%.1x,", severity);
		base::put_any("%llx,", tag);
	}


	// --------------------------------------------------------------


	template <std::size_t Size, typename Clock>
	inline test_line_ostream<Size, Clock>::test_line_ostream()
		: base() {
	}


	template <std::size_t Size, typename Clock>
	inline test_line_ostream<Size, Clock>::test_line_ostream(table_ostream* table)
		: base(table) {
	}


	template <std::size_t Size, typename Clock>
	inline void test_line_ostream<Size, Clock>::put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t Size, typename Clock>
	inline void test_line_ostream<Size, Clock>::put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
		put_props(category, severity, tag);

		base::put_anyv(format, vlist);
	}


	template <std::size_t Size, typename Clock>
	inline void test_line_ostream<Size, Clock>::put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
		std::size_t buffer_offset = 0;
		bool hasMore = true;

		while (hasMore) {
			if (buffer_offset != 0) {
				base::flush();
			}

			put_props(category, severity, tag);

			hasMore = base::put_binary(buffer, buffer_size, buffer_offset);
		}
	}

	template <std::size_t Size, typename Clock>
	inline void test_line_ostream<Size, Clock>::put_props(category_t category, severity_t severity, tag_t tag) noexcept {
		base::put_timestamp(timestamp<Clock>(), "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u ");

		char buf_severity[2 * severity::abc::debug + 1];
		severity = severity <= severity::abc::debug ? severity : severity::abc::debug;
		std::memset(buf_severity, ' ', 2 * severity);
		buf_severity[2 * (severity - 1)] = '\0';
		base::put_any(buf_severity);
	}


	// --------------------------------------------------------------


	template <typename Line, typename Filter>
	inline log_ostream<Line, Filter>::log_ostream(std::streambuf* sb, const Filter* filter)
		: base(sb)
		, _filter(filter) {
	}


	template <typename Line, typename Filter>
	inline void log_ostream<Line, Filter>::put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <typename Line, typename Filter>
	inline void log_ostream<Line, Filter>::put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept {
		if (_filter == nullptr || _filter->is_enabled(category, severity)) {
			Line line(this);
			line.put_anyv(category, severity, tag, format, vlist);
		}
	}


	template <typename Line, typename Filter>
	inline void log_ostream<Line, Filter>::put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept {
		if (_filter == nullptr || _filter->is_enabled(category, severity)) {
			Line line(this);
			line.put_binary(category, severity, tag, buffer, buffer_size);
		}
	}


	// --------------------------------------------------------------


	inline log_filter::log_filter(severity_t min_severity) noexcept
		: _min_severity(min_severity) {
	}

	inline bool log_filter::is_enabled(category_t /*category*/, severity_t severity) const noexcept {
		return abc::severity::is_higher_or_equal(severity, _min_severity);
	}


	// --------------------------------------------------------------


	inline bool severity::is_higher(severity_t severity, severity_t other) noexcept {
		return severity < other;
	}


	inline bool severity::is_higher_or_equal(severity_t severity, severity_t other) noexcept {
		return severity <= other;
	}

}
