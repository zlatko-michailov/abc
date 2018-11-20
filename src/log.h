#pragma once

#include <cstdint>
#include <string>
#include <iostream>
#include <fstream>
#include <chrono>
#include "status.h"


namespace abc {

	typedef std::uint16_t	category_t;
	typedef std::uint32_t	tag_t;


	template <typename Char>
	class basic_log {
	public:
		static const Char* const default_separator;

		static basic_log<Char>& global;
		static basic_log<Char>& critical;

	public:
		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept = 0;
	};


	template <typename Char>
	class basic_slog : public basic_log<Char> {
	public:
		basic_slog(std::basic_streambuf<Char>* streambuf, const Char* separator = basic_log<Char>::default_separator) noexcept
			: _ostream(streambuf)
			, _separator(separator)
		{
		}

	public:
		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept override {
			_ostream
				<< std::right << std::fixed << std::hex << std::showbase
				<< _separator.c_str() << category
				<< _separator.c_str() << tag
				<< _separator.c_str() << status;

			_ostream << _separator.c_str();
			if (message != nullptr) {
				_ostream << message;
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
		basic_flog(const char* path, const Char* separator = basic_log<Char>::default_separator) noexcept
			: _ofstream(path)
			, _slog(_ofstream.rdbuf(), separator)
		{
		}

		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept override {
			_slog.push(category, tag, status, message);
		}

	private:
		std::basic_ofstream<Char>	_ofstream;
		basic_slog<Char>			_slog;
	};


	template <typename Char>
	class basic_rflog : public basic_log<Char> {
	public:
		basic_rflog(const char* path, std::chrono::minutes rminutes, const Char* separator = basic_log<Char>::default_separator) noexcept
			: _path(path)
			, _rminutes(rminutes)
			, _separator(separator)
		{
		}

		virtual void push(category_t category, tag_t tag, status_t status, const Char* message = nullptr) noexcept override {
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

	static slog default_global			= slog(std::cout.rdbuf());
	static slog default_critical		= slog(std::cerr.rdbuf());

	static wslog wdefault_global		= wslog(std::wcout.rdbuf());
	static wslog wdefault_critical		= wslog(std::wcerr.rdbuf());

	template <> log& log::global		= default_global;
	template <> log& log::critical		= default_critical;

	template <> wlog& wlog::global		= wdefault_global;
	template <> wlog& wlog::critical	= wdefault_critical;

} // namespce abc
