#pragma once

#include <iostream>
#include <chrono>
#include <cstdarg>
#include <thread>

#include "tag.h"
#include "timestamp.i.h"
#include "mutex.i.h"


namespace abc {

	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	class log;


	namespace size {
		constexpr std::size_t k1 = 1024;
		constexpr std::size_t k2 = 2 * k1;
		constexpr std::size_t k4 = 4 * k1;
		constexpr std::size_t k8 = 8 * k1;
	}


	typedef std::uint8_t	severity_t;

	namespace severity {
		constexpr severity_t off		= 0x0;
		constexpr severity_t critical	= 0x1;
		constexpr severity_t warning	= 0x2;
		constexpr severity_t important	= 0x3;
		constexpr severity_t optional	= 0x4;
		constexpr severity_t debug		= 0x5;
		constexpr severity_t abc		= 0x6;

		bool is_higher(severity_t severity, severity_t other);
		bool is_higher_or_equal(severity_t severity, severity_t other);
	}


	typedef std::uint16_t	category_t;

	namespace category {
		constexpr category_t any	= 0xFFFF;

		namespace abc {
			constexpr category_t base	= 0x8000;
		}
	}


	namespace log_container {
		class ostream;
		
		template <typename Clock>
		class file;
	}


	namespace log_view {
		class diag;
		class test;
		class blank;
	
		namespace format {
			namespace datetime {
				static constexpr const char* friendly	= "%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u";
				static constexpr const char* iso		= "%4.4u-%2.2u-%2.2uT%2.2u:%2.2u:%2.2u.%3.3uZ";
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
	}


	namespace log_filter {
		class none;
		class severity;
	}


	// --------------------------------------------------------------


	template <std::size_t LineSize = size::k4, typename Container = log_container::ostream, typename View = log_view::diag, typename Filter = log_filter::none>
	class log {
	public:
		log(Container&& container, View&& view, Filter&& filter) noexcept;
		log(log&& other) noexcept = default;

	public:
		void push_back(category_t category, severity_t severity, tag_t tag, const char* format, ...);
		void vpush_back(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);

	private:
		Container		_container;
		View			_view;
		Filter			_filter;
	};


	namespace log_container {
		class ostream {
		public:
			ostream(std::streambuf* sb = std::clog.rdbuf()) noexcept;
			ostream(ostream&& other) noexcept;

		public:
			void push_back(const char* line);

		protected:
			void set_stream(std::streambuf* sb);

		private:
			spin_mutex<spin_for::disk>	_mutex;
			std::ostream				_stream;
		};


		template <typename Clock = std::chrono::system_clock>
		class file : public ostream {
		public:
			static constexpr std::chrono::minutes::rep	no_rotation	= 0;

		public:
			file(const char* path);
			file(const char* path, std::chrono::minutes::rep rotation_minutes);
			file(file&& other) noexcept = default;

		public:
			void push_back(const char* line);

		protected:
			void ensure_file_stream();

		private:
			std::string					_path;
			std::chrono::minutes::rep	_rotation_minutes;
			timestamp<Clock>			_rotation_timestamp;
		};
	}


	namespace log_view {
		class debug {
		public:
			debug() noexcept = default;
			debug(debug&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};


		class diag {
		public:
			diag() noexcept = default;
			diag(diag&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};


		class test {
		public:
			test() noexcept = default;
			test(test&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};


		class blank {
		public:
			blank() noexcept = default;
			blank(blank&& other) noexcept = default;

		public:
			void format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};
	}


	namespace log_filter {
		class none {
		public:
			none() noexcept = default;
			none(none&& other) noexcept = default;

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;
		};


		class severity {
		public:
			severity() noexcept = default;
			severity(severity&& other) noexcept = default;

		public:
			severity(severity_t min_severity) noexcept;

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;

		private:
			severity_t	_min_severity;
		};
	}


	// --------------------------------------------------------------



}
