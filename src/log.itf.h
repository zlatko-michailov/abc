#pragma once

#include <string>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <future>

#include "timestamp.h"
#include "mutex.h"


namespace abc {

	class log;


	typedef std::uint64_t field_t;

	namespace field {
		constexpr field_t all			= -1;

		constexpr field_t timestamp		= 0x01;
		constexpr field_t process		= 0x02;
		constexpr field_t thread		= 0x04;
		constexpr field_t request		= 0x08;
		constexpr field_t category		= 0x10;
		constexpr field_t tag			= 0x20;
		constexpr field_t status		= 0x40;
	}


	// --------------------------------------------------------------


	class log {
	public:
		static constexpr const char*				default_separator	= " | ";
		static constexpr std::chrono::minutes::rep	no_rotation			= 0;
		static constexpr std::size_t				max_path			= 4 * 1024;

	public:
		log(std::FILE* f, const char* separator = default_separator);
		log(const char* path, const char* separator = default_separator);
		log(const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator = default_separator);

		virtual ~log() noexcept;

	public:
		field_t		filed_mask		= field::all;
		severity_t	min_severity	= severity::warning;

	public:
		std::future<void> push_async(severity_t severity, category_t category, tag_t tag, status_t status);
		std::future<void> push_async(severity_t severity, category_t category, tag_t tag, status_t status, std::string&& formatted);
		void push(severity_t severity, category_t category, tag_t tag, status_t status, const char* format = nullptr, ...);

	private:
		void push(severity_t severity, category_t category, tag_t tag, status_t status, std::thread::id thread_id, const char* custom, bool use_vlist, va_list vlist);
		void prepare_push(severity_t severity, int fwide_sign);

	private:
		std::FILE*					_f;
		std::string					_separator;
		std::string					_path;
		std::chrono::minutes::rep	_rotation_minutes;
		timestamp<>					_rotation_timestamp;
		spin_mutex<spin_for::disk>	_mutex;
	};

}
