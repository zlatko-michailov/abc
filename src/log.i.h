#pragma once

//#include <string>
#include <iostream>
#include <chrono>
#include <cstdarg>
//#include <cstdio>
#include <thread>
//#include <future>

#include "tag.h"
#include "timestamp.i.h"
#include "mutex.i.h"


namespace abc {

	template <typename Container, typename View, typename Filter, std::size_t LineSize>
	class log;


	namespace log_container {
		class ostream;
		
		template <typename Clock>
		class file;
	}


	namespace log_view {
		class diag;
		class test;
		class blank;
	}


	namespace log_filter {
		class none;
		class severity;
	}


	namespace size {
		constexpr std::size_t k1 = 1024;
		constexpr std::size_t k2 = 2 * k1;
		constexpr std::size_t k4 = 4 * k1;
		constexpr std::size_t k8 = 8 * k1;
	}


	typedef std::uint8_t	severity_t;

	namespace severity {
		constexpr severity_t critical	= 0x1; // C
		constexpr severity_t warning	= 0x2; // W
		constexpr severity_t important	= 0x3; // I
		constexpr severity_t optional	= 0x4; // O
		constexpr severity_t debug		= 0x5; // D
		constexpr severity_t abc		= 0x6; // A

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


	template <typename Clock = std::chrono::system_clock>
	void format_timestamp(char* line, std::size_t line_size, const timestamp<Clock>& ts);

	template <typename Clock = std::chrono::system_clock>
	void format_current_timestamp(char* line, std::size_t line_size);

	void format_thread_id(char* line, std::size_t line_size, std::thread::id thread_id);
	void format_current_thread_id(char* line, std::size_t line_size);

	void format_category(char* line, std::size_t line_size, category_t category);
	void format_severity(char* line, std::size_t line_size, severity_t severity);
	void format_tag(char* line, std::size_t line_size, tag_t tag);


	// --------------------------------------------------------------


	template <typename Container = log_container::ostream, typename View = log_view::diag, typename Filter = log_filter::none, std::size_t LineSize = size::k4>
	class log {
	public:
		log(Container&& container, View&& view, Filter&& filter, std::size_t line_size);

	public:
		void push_back(category_t category, severity_t severity, tag_t tag, const char* format, ...);
		void push_back_v(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);

	private:
		Container		_container;
		View			_view;
		Filter			_filter;
		std::size_t		_line_size;
	};


	namespace log_container {
		class ostream {
		public:
			ostream(std::streambuf* sb) noexcept;

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
		class diag {
		public:
			static constexpr const char* separator	= " | ";

		public:
			diag();

		public:
			void format_v(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};


		class test {
		public:
			test();

		public:
			void format_v(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};


		class blank {
		public:
			blank();

		public:
			void format_v(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist);
		};
	}


	namespace log_filter {
		class none {
		public:
			none();

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;
		};


		class severity {
		public:
			severity(severity_t min_severity);

		public:
			bool is_enabled(category_t category, severity_t severity) const noexcept;

		private:
			severity_t	_min_severity;
		};
	}


	// --------------------------------------------------------------



}
