#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <cstdarg>
#include <cstdio>
#include "status.h"


namespace abc {

	typedef std::uint16_t	category_t;
	typedef std::uint32_t	tag_t;

	namespace category {
		static const category_t log		= 0x0001;

		static const category_t custom	= 0x8000;
	}


	template <typename Char>
	class basic_log {
	public:
		static const Char* const default_separator;

		static basic_log<Char>& diag;
		static basic_log<Char>& critical;

	public:
		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr, ...) = 0;
	};


	template <typename Char>
	class basic_slog : public basic_log<Char> {
	private:
		static constexpr size_t buffer_capacity = 4 * 1024 + 1;

	public:
		basic_slog(std::basic_streambuf<Char>* streambuf, const Char* separator = basic_log<Char>::default_separator)
			: _ostream(streambuf)
			, _separator(separator)
		{
		}

	public:
		virtual void push(category_t category, tag_t tag, status_t status, const Char* format = nullptr, ...) override {
			va_list vlist;
			va_start(vlist, format);

			push(category, tag, status, format, vlist);
		}

	protected:
		void push(category_t category, tag_t tag, status_t status, const char* format, va_list vlist) {
			char buffer[buffer_capacity];

			snprintf(buffer, sizeof(buffer) / sizeof(char), "%s0x%4.4x%s0x%8.8x%s0x%4.4x%s", _separator.c_str(), category, _separator.c_str(), tag, _separator.c_str(), status, _separator.c_str());
			_ostream << buffer;

			if (format != nullptr) {
				vsnprintf(buffer, sizeof(buffer) / sizeof(char), format, vlist);
				_ostream << buffer;
			}

			_ostream
				<< _separator.c_str() << std::endl;
		}

		void push(category_t category, tag_t tag, status_t status, const wchar_t* format, va_list vlist) {
			wchar_t buffer[buffer_capacity];

			swprintf(buffer, sizeof(buffer) / sizeof(wchar_t), L"%ls0x%4.4x%ls0x%8.8x%ls0x%4.4x%ls", _separator.c_str(), category, _separator.c_str(), tag, _separator.c_str(), status, _separator.c_str());
			_ostream << buffer;

			if (format != nullptr) {
				vswprintf(buffer, sizeof(buffer) / sizeof(wchar_t), format, vlist);
				_ostream << buffer;
			}

			_ostream
				<< _separator.c_str() << std::endl;
		}

	private:
		std::basic_ostream<Char>	_ostream;
		std::basic_string<Char>		_separator;
	};


	template <typename Char>
	class basic_flog : public basic_log<Char> {
	public:
		basic_flog(const char* path, const Char* separator = basic_log<Char>::default_separator)
			: _ofstream(path)
			, _slog(_ofstream.rdbuf(), separator)
		{
			_ofstream
				 << separator << "categ."
				 << separator << "   tag    "
				 << separator << "status"
				 << separator << "message"
				 << separator << std::endl

				 << separator << "------"
				 << separator << "----------"
				 << separator << "------"
				 << separator << "-------"
				 << separator << std::endl;
		}

		virtual void push(category_t category, tag_t tag, status_t status, const Char* format = nullptr, ...) override {
			_slog.push(category, tag, status, format);
		}

	private:
		std::basic_ofstream<Char>	_ofstream;
		basic_slog<Char>			_slog;
	};


	template <typename Char>
	class basic_rflog : public basic_log<Char> {
	public:
		basic_rflog(const char* path, std::chrono::minutes rminutes, const Char* separator = basic_log<Char>::default_separator)
			: _path(path)
			, _rminutes(rminutes)
			, _separator(separator)
		{
		}

		virtual void push(category_t category, tag_t tag, status_t status, const Char* format = nullptr, ...) override {
			typedef int32_t minutes_t;
			constexpr minutes_t hours_per_day = 24;
			constexpr minutes_t minutes_per_hour = 60;
			constexpr minutes_t minutes_per_day = hours_per_day * minutes_per_hour;

			std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
			std::chrono::minutes minutes_since_epoch = std::chrono::duration_cast<std::chrono::minutes>(now.time_since_epoch());

			minutes_t minutes_of_day = static_cast<minutes_t>(minutes_since_epoch.count() % minutes_per_day);
			minutes_t hours_of_day = minutes_of_day / minutes_per_hour;
			minutes_t minutes_of_hour = minutes_of_day % minutes_per_hour;

			std::ostringstream hh_mm;
			hh_mm
				<< std::setfill('0')
        		<< std::setw(2) << hours_of_day
				<< '_'
        		<< std::setw(2) << minutes_of_hour;

			std::string path = _path + hh_mm.str();

			// TODO:
		}

	private:
		std::string				_path;
		std::chrono::minutes	_rminutes;
		std::basic_string<Char>	_separator;
	};


	typedef basic_log<char>			log;
	typedef basic_log<wchar_t>		wlog;

	typedef basic_slog<char>		slog;
	typedef basic_slog<wchar_t>		wslog;

	typedef basic_flog<char>		flog;
	typedef basic_flog<wchar_t>		wflog;

	typedef basic_rflog<char>		rflog;
	typedef basic_rflog<wchar_t>	wrflog;


	template <> const char* const		log::default_separator		= " | ";
	template <> const wchar_t* const	wlog::default_separator		= L" | ";

	static slog default_diag			= slog(std::cout.rdbuf());
	static slog default_critical		= slog(std::cerr.rdbuf());

	static wslog wdefault_diag			= wslog(std::wcout.rdbuf());
	static wslog wdefault_critical		= wslog(std::wcerr.rdbuf());

	template <> log& log::diag			= default_diag;
	template <> log& log::critical		= default_critical;

	template <> wlog& wlog::diag		= wdefault_diag;
	template <> wlog& wlog::critical	= wdefault_critical;

} // namespce abc
