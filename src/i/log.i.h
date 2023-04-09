/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov

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

#include "../tag.h"
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
		constexpr const char* yellow		= "1;33m";
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
			constexpr category_t exception	= base +  1;
			constexpr category_t stream		= base +  2;
			constexpr category_t socket		= base +  3;
			constexpr category_t http		= base +  4;
			constexpr category_t json		= base +  5;
			constexpr category_t multifile	= base +  6;
			constexpr category_t endpoint	= base +  7;
			constexpr category_t vmem		= base +  8;
			constexpr category_t samples	= base +  9;
			constexpr category_t gpio		= base + 10;
		}
	}


	// --------------------------------------------------------------


	/**
	 * @brief					`line_ostream` specialization for debug logging.
	 * @tparam Size				Maximum line size.
	 * @tparam Clock			Clock for obtaining a timestamp.
	 */
	template <std::size_t Size = size::k2, typename Clock = std::chrono::system_clock>
	class debug_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		/**
		 * @brief				Default constructor.
		 */
		debug_line_ostream();

		/**
		 * @brief				Constructor.
		 * @param table			Pointer to a `table_ostream` instance to write the line to.
		 */
		debug_line_ostream(table_ostream* table);

		/**
		 * @brief				Move constructor.
		 */
		debug_line_ostream(debug_line_ostream&& other);

		/**
		 * @brief				Deleted.
		 */
		debug_line_ostream(const debug_line_ostream& other) = delete;

	public:
		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param ...			Message arguments.
		 */
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;

		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param vlist			Message arguments.
		 */
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;

		/**
		 * @brief				Write binary buffer as a sequence of hexadecimal bytes.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param buffer		Data buffer.
		 * @param buffer_size	Content size.
		 */
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	protected:
		/**
		 * @brief				Writes the static properties of a long entry.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 */
		void put_props(category_t category, severity_t severity, tag_t tag) noexcept;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`line_ostream` specialization for diagnostic logging.
	 * @tparam Size				Maximum line size.
	 * @tparam Clock			Clock for obtaining a timestamp.
	 */
	template <std::size_t Size = size::k2, typename Clock = std::chrono::system_clock>
	class diag_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		/**
		 * @brief				Default constructor.
		 */
		diag_line_ostream();

		/**
		 * @brief				Constructor.
		 * @param table			Pointer to a `table_ostream` instance to write the line to.
		 */
		diag_line_ostream(table_ostream* table);

		/**
		 * @brief				Move constructor.
		 */
		diag_line_ostream(diag_line_ostream&& other);

		/**
		 * @brief				Deleted.
		 */
		diag_line_ostream(const diag_line_ostream& other) = delete;

	public:
		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param ...			Message arguments.
		 */
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;

		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param vlist			Message arguments.
		 */
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;

		/**
		 * @brief				Write binary buffer as a sequence of hexadecimal bytes.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param buffer		Data buffer.
		 * @param buffer_size	Content size.
		 */
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	protected:
		/**
		 * @brief				Writes the static properties of a long entry.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 */
		void put_props(category_t category, severity_t severity, tag_t tag) noexcept;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`line_ostream` specialization for test logging.
	 * @tparam Size				Maximum line size.
	 * @tparam Clock			Clock for obtaining a timestamp.
	 */
	template <std::size_t Size = size::k2, typename Clock = std::chrono::system_clock>
	class test_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		/**
		 * @brief				Default constructor.
		 */
		test_line_ostream();

		/**
		 * @brief				Constructor.
		 * @param table			Pointer to a `table_ostream` instance to write the line to.
		 */
		test_line_ostream(table_ostream* table);

		/**
		 * @brief				Move constructor.
		 */
		test_line_ostream(test_line_ostream&& other);

		/**
		 * @brief				Deleted.
		 */
		test_line_ostream(const test_line_ostream& other) = delete;

	public:
		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param ...			Message arguments.
		 */
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;

		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param vlist			Message arguments.
		 */
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;

		/**
		 * @brief				Write binary buffer as a sequence of hexadecimal bytes.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param buffer		Data buffer.
		 * @param buffer_size	Content size.
		 */
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	protected:
		/**
		 * @brief				Writes the static properties of a long entry.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 */
		void put_props(category_t category, severity_t severity, tag_t tag) noexcept;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`table_ostream` specialization for logging.
	 * @tparam Line				`line_ostream` specialization.
	 * @tparam Filter			Line filter.
	 */
	template <typename Line, typename Filter>
	class log_ostream : public table_ostream {
		using base = table_ostream;

	public:
		/**
		 * @brief				Constructor.
		 * @param sb			Pointer to a `std::streambuf` instance to write to.
		 * @param filter		Pointer to a `Filter` instance.
		 */
		log_ostream(std::streambuf* sb, Filter* filter);

		/**
		 * @brief				Move constructor.
		 */
		log_ostream(log_ostream&& other);

		/**
		 * @brief				Deleted.
		 */
		log_ostream(const log_ostream& other) = delete;

	public:
		/**
		 * @brief				Returns the pointer to the `Filter` instance.
		 */
		Filter* filter() noexcept;

	public:
		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param ...			Message arguments.
		 */
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...) noexcept;

		/**
		 * @brief				Write a formatted message.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param format		Message format.
		 * @param vlist			Message arguments.
		 */
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) noexcept;

		/**
		 * @brief				Write binary buffer as a sequence of hexadecimal bytes.
		 * @param category		Entry category.
		 * @param severity		Entry severity.
		 * @param tag			Entry tag.
		 * @param buffer		Data buffer.
		 * @param buffer_size	Content size.
		 */
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size) noexcept;

	private:
		/**
		 * @brief				Pointer to the `Filter` instance passed in to the constructor.
		 */
		Filter* _filter;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Log line filter.
	 */
	class log_filter {
	public:
		/**
		 * @brief				Default constructor.
		 */
		log_filter() noexcept = default;

		/**
		 * @brief				Move constructor.
		 */
		log_filter(log_filter&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		log_filter(const log_filter& other) noexcept = default;

	public:
		/**
		 * @brief				Constructor.
		 * @param min_severity	Minimum severity for a line to be written.
		 */
		log_filter(severity_t min_severity) noexcept;

	public:
		/**
		 * @brief				Returns the minimum severity.
		 */
		severity_t min_severity() const noexcept;

		/**
		 * @brief				Sets the minimum severity.
		 */
		severity_t min_severity(severity_t min_severity) noexcept;

	public:
		/**
		 * @brief				Returns whether an entry with the given `category` and `severity` passes the filter.
		 * @param category		Category.
		 * @param severity		Severity.
		 * @return				true = passes. false = filtered out. 
		 */
		bool is_enabled(category_t category, severity_t severity) const noexcept;

	private:
		/**
		 * @brief				Minimum severity for a line to be written.
		 */
		severity_t _min_severity;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`log_ostream` specialization that doesn't log anything.
	 */
	using null_log = log_ostream<diag_line_ostream<0>, log_filter>;

}
