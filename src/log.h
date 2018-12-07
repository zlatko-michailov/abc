#pragma once

//#include <cstdint>
//#include <iostream>
//#include <fstream>
//#include <sstream>
//#include <iomanip>
#include <string>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include "base.h"
#include "timestamp.h"


namespace abc {

	class basic_log {
	public:
		static constexpr const char*				default_separator	= " | ";
		static constexpr std::chrono::minutes::rep	no_rotation			= 0;
		static constexpr size_t						max_path			= 4 * 1024;

	public:
		static basic_log& diag;

	public:
		basic_log(std::FILE* f, const char* separator = default_separator) noexcept {
			reset(f, nullptr, 0, separator);
		}

		basic_log(const char* path, const char* separator = default_separator) noexcept {
			reset(nullptr, path, 0, separator);
		}

		basic_log(const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator = default_separator) noexcept {
			reset(nullptr, path, rotation_minutes, separator);
		}

		virtual ~basic_log() noexcept;

	protected:
		basic_log() noexcept {
			reset(nullptr, nullptr, 0, nullptr);
		}

		void reset(std::FILE* f, const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator = default_separator) noexcept {
			_f = f;
			_separator = separator;
			_path = path;
			_rotation_minutes = rotation_minutes;
		}

	public:
		severity_t min_severity = severity::warning;

		status_t push(severity_t severity, category_t category, tag_t tag, status_t status) noexcept;
		status_t push(severity_t severity, category_t category, tag_t tag, status_t status, const char* format, ...) noexcept;
		status_t push(severity_t severity, category_t category, tag_t tag, status_t status, const wchar_t* format, ...) noexcept;

	private:
		status_t prepare_push(severity_t severity, int fwide_sign) noexcept;

	private:
		std::FILE*					_f;
		const char*					_separator;
		const char*					_path;
		std::chrono::minutes::rep	_rotation_minutes;
		timestamp<>					_rotation_timestamp;
	};


	class log : public basic_log {
	public:
		log(std::FILE* f, const char* separator = default_separator) /*throws*/
			: basic_log()
			, _path()
			, _separator(separator) {
			reset(f, nullptr, 0, _separator.c_str());
		}

		log(const char* path, const char* separator = default_separator) /*throws*/
			: basic_log()
			, _path(path)
			, _separator(separator) {
			reset(nullptr, _path.c_str(), 0, _separator.c_str());
		}

		log(const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator = default_separator) /*throws*/
			: basic_log()
			, _path(path)
			, _separator(separator) {
			reset(nullptr, _path.c_str(), rotation_minutes, _separator.c_str());
		}

	private:
		std::string		_path;
		std::string		_separator;
	};

}
