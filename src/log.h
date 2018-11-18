#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include "status.h"


namespace abc {

	typedef std::uint16_t	category_t;
	typedef std::uint32_t	tag_t;


	template <typename Char>
	struct basic_log {
		static basic_log<Char>&	global;
		static basic_log<Char>&	critical;

		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept = 0;
	}; // class basic_log


	template <typename Char>
	class basic_slog : public basic_log<Char> {
	public:
		basic_slog(std::basic_streambuf<Char>* streambuf, const Char* separator) noexcept
			: _ostream(streambuf)
			, _separator(separator)
		{
		}

	public:
		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept override {
			_ostream
				<< std::right << std::fixed << std::hex << std::showbase
				<< _separator << category
				<< _separator << tag
				<< _separator << status;

			if (message != nullptr) {
				_ostream
					<< _separator << message;
			}

			_ostream
				<< _separator << std::endl;
		}

	private:
		std::basic_ostream<Char> _ostream;
		const Char* _separator;
	}; // class basic_slog


	template <typename Char>
	class basic_flog : public basic_log<Char> {
	public:
		basic_flog(std::string&& path, const Char* separator) noexcept
			: _path(std::move(path))
			, _separator(separator)
		{
		}

		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept override {
			// TODO:
		}

	private:
		std::string _path;
		const Char* _separator;
	}; // class basic_flog


	template <typename Char>
	class basic_rflog : public basic_log<Char> {
	public:
		typedef std::uint16_t	minutes;

		basic_rflog(std::string&& path, minutes rminutes, const Char* separator) noexcept
			: _path(std::move(path))
			, _rminutes(rminutes)
			, _separator(separator)
		{
		}

		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept override {
			// TODO:
		}

	private:
		std::string _path;
		minutes _rminutes;
		const Char* _separator;
	}; // class basic_rflog


	typedef basic_log<char>			log;
	typedef basic_log<wchar_t>		wlog;

	typedef basic_slog<char>		slog;
	typedef basic_slog<wchar_t>		wslog;

	typedef basic_flog<char>		flog;
	typedef basic_flog<wchar_t>		wflog;

	typedef basic_rflog<char>		rflog;
	typedef basic_rflog<wchar_t>	wrflog;


	constexpr const char*		default_log_separator	= " | ";
	constexpr const wchar_t*	wdefault_log_separator	= L" | ";

	static slog default_global			= slog(std::cout.rdbuf(), default_log_separator);
	static slog default_critical		= slog(std::cerr.rdbuf(), default_log_separator);

	static wslog wdefault_global		= wslog(std::wcout.rdbuf(), wdefault_log_separator);
	static wslog wdefault_critical		= wslog(std::wcerr.rdbuf(), wdefault_log_separator);

	template <> log& log::global		= default_global;
	template <> log& log::critical		= default_critical;

	template <> wlog& wlog::global		= wdefault_global;
	template <> wlog& wlog::critical	= wdefault_critical;

} // namespce abc
