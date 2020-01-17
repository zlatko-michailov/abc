#pragma once

#include <string>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <future>

#include "legacy_base.h"
#include "timestamp.h"
#include "mutex.h"


namespace abc {

	typedef std::uint64_t field_t;

	namespace field {
		constexpr field_t all			= -1;

		constexpr field_t timestamp		= 0x01;
		constexpr field_t category		= 0x02;
		constexpr field_t tag			= 0x04;
		constexpr field_t status		= 0x08;
		constexpr field_t thread		= 0x10;
		constexpr field_t request		= 0x20;
	}


	class basic_log {
	public:
		static constexpr const char*				default_separator	= " | ";
		static constexpr std::chrono::minutes::rep	no_rotation			= 0;
		static constexpr std::size_t				max_path			= 4 * 1024;

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
		field_t		filed_mask		= field::all;
		severity_t	min_severity	= severity::warning;

		std::future<status_t> push_async(severity_t severity, category_t category, tag_t tag, status_t status) noexcept;
		std::future<status_t> push_async(severity_t severity, category_t category, tag_t tag, status_t status, std::string&& formatted) noexcept;
		status_t push(severity_t severity, category_t category, tag_t tag, status_t status, const char* format = nullptr, ...) noexcept;

	private:
		status_t push(severity_t severity, category_t category, tag_t tag, status_t status, std::thread::id thread_id, const char* custom, bool use_vlist, va_list vlist) noexcept;
		status_t prepare_push(severity_t severity, int fwide_sign) noexcept;

	private:
		std::FILE*					_f;
		const char*					_separator;
		const char*					_path;
		std::chrono::minutes::rep	_rotation_minutes;
		timestamp<>					_rotation_timestamp;
		spin_mutex<spin_for::disk>	_mutex;
	};


	class legacy_log : public basic_log {
	public:
		legacy_log(std::FILE* f, const char* separator = default_separator) /*throws*/
			: basic_log()
			, _path()
			, _separator(separator) {
			reset(f, nullptr, 0, _separator.c_str());
		}

		legacy_log(const char* path, const char* separator = default_separator) /*throws*/
			: basic_log()
			, _path(path)
			, _separator(separator) {
			reset(nullptr, _path.c_str(), 0, _separator.c_str());
		}

		legacy_log(const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator = default_separator) /*throws*/
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
