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

#include <iostream>
#include <fstream>
#include <chrono>
#include <cstdarg>
#include <thread>
#include <mutex>

#include "tag.h"
#include "timestamp.i.h"
#include "table.i.h"


namespace abc {

	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	class log;


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
		constexpr severity_t off		= 0x0;
		constexpr severity_t critical	= 0x1;
		constexpr severity_t warning	= 0x2;
		constexpr severity_t important	= 0x3;
		constexpr severity_t optional	= 0x4;
		constexpr severity_t debug		= 0x5;
		constexpr severity_t abc		= 0x6;

		bool is_higher(severity_t severity, severity_t other) noexcept;
		bool is_higher_or_equal(severity_t severity, severity_t other) noexcept;
	}


	using category_t = std::uint16_t;

	namespace category {
		constexpr category_t any	= 0xffff;

		namespace abc {
			constexpr category_t base		= 0x8000;
			constexpr category_t exception	= base | 0x0001;
			constexpr category_t socket		= base | 0x0002;
			constexpr category_t http		= base | 0x0003;
			constexpr category_t json		= base | 0x0004;
		}
	}


	//// begin delete

	namespace log_container {
		class ostream;
		
		template <std::size_t MaxPath, typename Clock>
		class file;
	}


	namespace log_view {
		template <typename Clock = std::chrono::system_clock>
		class debug;

		template <typename Clock = std::chrono::system_clock>
		class diag;

		template <typename Clock = std::chrono::system_clock>
		class test;

		class blank;
	
		namespace format {
			namespace datetime {
				static constexpr const char* friendly	= "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u";
				static constexpr const char* iso		= "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ";
				static constexpr const char* file		= "%4.4u%2.2u%2.2u_%2.2u%2.2u";
				static constexpr const char* date_only	= "%4.4u-%2.2u-%2.2u";
				static constexpr const char* time_only	= "%2.2u:%2.2u:%2.2u.%3.3u";
			}

			namespace category {
				static constexpr const char* friendly	= "%4.4x";
				static constexpr const char* compact	= "%.4x";
			}

			namespace severity {
				static constexpr const char* friendly	= "%1.1x";
				static constexpr const char* compact	= "%1.1x";
			}

			namespace tag {
				static constexpr const char* friendly	= "%16llx";
				static constexpr const char* compact	= "%llx";
			}

			namespace separator {
				static constexpr const char* friendly	= " | ";
				static constexpr const char* compact	= "|";
				static constexpr const char* space		= " ";
			}

			static constexpr const char* none			= "";
		}

		template <typename Clock = std::chrono::system_clock>
		int format_timestamp(char* line, std::size_t line_size, const timestamp<Clock>& ts, const char* format = format::datetime::friendly);

		int format_thread_id(char* line, std::size_t line_size, std::thread::id thread_id);
		int format_category(char* line, std::size_t line_size, category_t category, const char* format = format::category::friendly);
		int format_severity(char* line, std::size_t line_size, severity_t severity, const char* format = format::severity::friendly);
		int format_tag(char* line, std::size_t line_size, tag_t tag, const char* format = format::tag::friendly);
		int format_binary(char* line, std::size_t line_size, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset);
	}


	namespace log_filter {
		class none;
		class severity;
	}

	//// end delete


	// --------------------------------------------------------------


	template <std::size_t Size = size::k2>
	class debug_line_ostream : public line_ostream<Size> {
		using base = line_ostream<Size>;

	public:
		debug_line_ostream();
		debug_line_ostream(table_ostream* table);
		debug_line_ostream(debug_line_ostream&& other) = default;

	public:
		void put_any(category_t category, severity_t severity, tag_t tag, const char* format, ...);
		void put_anyv(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		void put_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size);

	protected:
		void put_props(category_t category, severity_t severity, tag_t tag);
	};


	// --------------------------------------------------------------


	template <std::size_t LineSize = size::k4, typename Container = log_container::ostream, typename View = log_view::diag<>, typename Filter = log_filter::none>
	class log {
	public:
		log() noexcept = default;
		log(Container&& container, View&& view, Filter&& filter) noexcept;
		log(log&& other) noexcept;

		log& operator=(log&& other) noexcept;

	public:
		void push_back(category_t category, severity_t severity, tag_t tag, const char* format, ...);
		void vpush_back(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		void push_back_blank(category_t category, severity_t severity);
		void push_back_binary(category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size);

	private:
		Container		_container;
		View			_view;
		Filter			_filter;
	};


	namespace log_container {
		class ostream {
		public:
			ostream() noexcept;
			ostream(std::streambuf* sb) noexcept;
			ostream(ostream&& other) noexcept;

			ostream& operator=(ostream&& other) noexcept;

		public:
			void push_back(const char* line);

		private:
			std::mutex		_mutex;
			std::ostream	_stream;
		};


		template <std::size_t MaxPath = size::k2, typename Clock = std::chrono::system_clock>
		class file {
		public:
			static constexpr std::chrono::minutes::rep	no_rotation	= 0;

		public:
			file(const char* path);
			file(const char* path, std::chrono::minutes::rep rotation_minutes);
			file(file&& other) noexcept;

		public:
			void push_back(const char* line);

		protected:
			void ensure_filebuf();

		private:
			char						_path[MaxPath + 1];
			std::size_t					_path_length;
			std::chrono::minutes::rep	_rotation_minutes;
			timestamp<Clock>			_rotation_timestamp;
			std::filebuf				_filebuf;
			ostream						_ostream;
		};
	}


	namespace log_view {
		template <typename Clock>
		class debug {
		public:
			debug() noexcept = default;
			debug(debug&& other) noexcept = default;

			debug& operator=(debug&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
			int format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset);

		protected:
			int format_fixed(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag);
		};


		template <typename Clock>
		class diag {
		public:
			diag() noexcept = default;
			diag(diag&& other) noexcept = default;

			diag& operator=(diag&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
			int format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset);

		protected:
			int format_fixed(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag);
		};


		template <typename Clock>
		class test {
		public:
			test(severity_t severity = severity::critical) noexcept;
			test(test&& other) noexcept = default;

			test& operator=(test&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
			int format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset);

		protected:
			int format_fixed(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag);

		private:
			severity_t	_severity;
		};


		class blank {
		public:
			blank() noexcept = default;
			blank(blank&& other) noexcept = default;

			blank& operator=(blank&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
			int format_binary(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset);
		};
	}


	namespace log_filter {
		class none {
		public:
			none() noexcept = default;
			none(none&& other) noexcept = default;

			none& operator=(none&& other) noexcept = default;

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;
		};


		class off {
		public:
			off() noexcept = default;
			off(off&& other) noexcept = default;

			off& operator=(off&& other) noexcept = default;

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;
		};


		class severity {
		public:
			severity() noexcept = default;
			severity(severity&& other) noexcept = default;

			severity& operator=(severity&& other) noexcept = default;

		public:
			severity(severity_t min_severity) noexcept;

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;

		private:
			severity_t	_min_severity;
		};
	}


	// --------------------------------------------------------------


	using null_log = log<1, log_container::ostream, abc::log_view::blank, abc::log_filter::off>;
	using null_log_ptr = null_log*;

}
