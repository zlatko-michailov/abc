#pragma once

#include <string>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <thread>
#include <future>

#include "mutex.itf.h"
#include "timestamp.h"
#include "process.itf.h"


namespace abc {

	class log;


	typedef std::uint8_t	severity_t;

	namespace severity {
		constexpr severity_t debug_abc	= 0x1;
		constexpr severity_t debug		= 0x4;
		constexpr severity_t trace		= 0x7;
		constexpr severity_t warning	= 0xA;
		constexpr severity_t critical	= 0xC;
	}


	typedef std::uint16_t	category_t;

	namespace category {
		constexpr category_t log		= 0x0001;
		constexpr category_t timestamp	= 0x0002;
		constexpr category_t mutex		= 0x0003;
		constexpr category_t pool		= 0x0004;
		constexpr category_t process	= 0x0005;
		constexpr category_t thread		= 0x0006;
		constexpr category_t os			= 0x0007;
		constexpr category_t host		= 0x0008;
		constexpr category_t async		= 0x0009;
		constexpr category_t custom		= 0x8000;
		// constexpr category_t sample_custom_category	= custom + 0x0001;
	}


	typedef std::uint32_t	tag_t;
	// TODO:: Remove when tagger gets implemented
	constexpr tag_t __TAG__ = 0;


	typedef std::uint64_t field_t;

	namespace field {
		constexpr field_t all			= -1;

		constexpr field_t timestamp		= 0x01;
		constexpr field_t process		= 0x02;
		constexpr field_t thread		= 0x04;
		constexpr field_t request		= 0x08;
		constexpr field_t category		= 0x10;
		constexpr field_t tag			= 0x20;
		constexpr field_t severity		= 0x40;
	}


	// --------------------------------------------------------------


	class log {
	public:
		static constexpr const char*				default_separator		= " | ";
		static constexpr field_t					default_field_mask		= field::all;
		static constexpr severity_t					default_min_severity	= severity::warning;
		static constexpr std::chrono::minutes::rep	no_rotation				= 0;
		static constexpr std::size_t				max_path				= 4 * 1024;

	public:
		log(std::FILE* f, const char* separator = default_separator, field_t field_mask = default_field_mask, severity_t min_severity = default_min_severity);
		log(const char* path, const char* separator = default_separator, field_t field_mask = default_field_mask, severity_t min_severity = default_min_severity);
		log(const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator = default_separator, field_t field_mask = default_field_mask, severity_t min_severity = default_min_severity);

		virtual ~log() noexcept;

	public:
		void push(severity_t severity, category_t category, tag_t tag, const char* format = nullptr, ...);

	private:
		void push(severity_t severity, category_t category, tag_t tag, process_id_t process_id, thread_id_t thread_id, const char* format, bool use_vlist, va_list vlist);
		void prepare_push(severity_t severity, int fwide_sign);

	private:
		std::FILE*					_f;
		std::string					_separator;
		field_t						_field_mask;
		severity_t					_min_severity;
		std::string					_path;
		std::chrono::minutes::rep	_rotation_minutes;
		timestamp<>					_rotation_timestamp;
	};

}
