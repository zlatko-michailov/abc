/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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

#include <cstddef>
#include <chrono>
#include <iostream>
#include <fstream>
#include <cstdarg>
#include <thread>

#include "tag.h"
#include "timestamp.i.h"
#include "table.i.h"


namespace abc {

	namespace color {
		constexpr const char* begin			= "\x1b[";
		constexpr const char* end			= "\x1b[0m";
		constexpr const char* black			= "30m";
		constexpr const char* red			= "31m";
		constexpr const char* green			= "32m";
		constexpr const char* blue			= "34m";
		constexpr const char* purple		= "35m";
		constexpr const char* cyan			= "36m";
		constexpr const char* light_gray	= "37m";
		constexpr const char* dark_gray		= "1;30m";
		constexpr const char* light_red		= "1;31m";
		constexpr const char* yello			= "1;33m";
		constexpr const char* light_cyan	= "1;36m";
	}


	using severity_t = std::uint8_t;

	namespace severity {
		constexpr severity_t off			= 0x0;
		constexpr severity_t critical		= 0x1;
		constexpr severity_t warning		= 0x2;
		constexpr severity_t important		= 0x3;
		constexpr severity_t optional		= 0x4;
		constexpr severity_t debug			= 0x5;

		namespace abc {
			constexpr severity_t important	= 0x6;
			constexpr severity_t optional	= 0x7;
			constexpr severity_t debug		= 0x8;
		}

		bool is_higher(severity_t severity, severity_t other) noexcept;
		bool is_higher_or_equal(severity_t severity, severity_t other) noexcept;
	}


	using category_t = std::uint16_t;

	namespace category {
		constexpr category_t any	= 0xffff;

		namespace abc {
			constexpr category_t base		= 0x8000;
			constexpr category_t exception	= base + 1;
			constexpr category_t stream		= base + 2;
			constexpr category_t socket		= base + 3;
			constexpr category_t http		= base + 4;
			constexpr category_t json		= base + 5;
			constexpr category_t multifile	= base + 6;
			constexpr category_t endpoint	= base + 7;
			constexpr category_t vmem		= base + 8;
			constexpr category_t samples	= base + 9;
		}
	}


	// --------------------------------------------------------------


	template <std::size_t Size = size::k2, typename Clock = std::chrono::system_clock>
	class debug_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		debug_line_ostream();
		debug_line_ostream(table_ostream* table);
		debug_line_ostream(debug_line_ostream&& other) = default;

	public:
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	protected:
		void put_props(category_t category, severity_t severity, tag_t tag) noexcept;
	};


	// --------------------------------------------------------------


	template <std::size_t Size = size::k2, typename Clock = std::chrono::system_clock>
	class diag_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		diag_line_ostream();
		diag_line_ostream(table_ostream* table);
		diag_line_ostream(diag_line_ostream&& other) = default;

	public:
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	protected:
		void put_props(category_t category, severity_t severity, tag_t tag) noexcept;
	};


	// --------------------------------------------------------------


	template <std::size_t Size = size::k2, typename Clock = std::chrono::system_clock>
	class test_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		test_line_ostream();
		test_line_ostream(table_ostream* table);
		test_line_ostream(test_line_ostream&& other) = default;

	public:
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	protected:
		void put_props(category_t category, severity_t severity, tag_t tag) noexcept;
	};


	// --------------------------------------------------------------


	template <typename Line, typename Filter>
	class log_ostream : public table_ostream {
		using base = table_ostream;

	public:
		log_ostream(std::streambuf* sb, Filter* filter);
		log_ostream(log_ostream&& other) = default;

	public:
		Filter*	filter() noexcept;

	public:
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	private:
		Filter*	_filter;
	};


	// --------------------------------------------------------------


	class log_filter {
	public:
		log_filter() noexcept = default;
		log_filter(log_filter&& other) noexcept = default;

	public:
		log_filter(severity_t min_severity) noexcept;

	public:
		severity_t	min_severity() const noexcept;
		severity_t	min_severity(severity_t min_severity) noexcept;

	public:
		bool is_enabled(category_t category, severity_t severity) const noexcept;

	private:
		severity_t	_min_severity;
	};


	// --------------------------------------------------------------


	using null_log = log_ostream<diag_line_ostream<0>, log_filter>;

}
