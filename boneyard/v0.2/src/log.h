#include <cstdio>
#include <cstdarg>

#include "log.itf.h"
#include "exception.h"
#include "process.h"
#include "timestamp.h"


namespace abc {

	static constexpr int fwide_char = -1;
	static constexpr int fwide_wide = +1;


	inline log::log(std::FILE* f, const char* separator, field_t field_mask, severity_t min_severity)
		: _f(f)
		, _separator(separator)
		, _field_mask(field_mask)
		, _min_severity(min_severity)
		, _rotation_minutes(no_rotation) {
	}


	inline log::log(const char* path, const char* separator, field_t field_mask, severity_t min_severity)
		: log(path, no_rotation, separator, field_mask, min_severity) {
	}


	inline log::log(const char* path, std::chrono::minutes::rep rotation_minutes, const char* separator, field_t field_mask, severity_t min_severity)
		: _f(nullptr)
		, _separator(separator)
		, _field_mask(field_mask)
		, _min_severity(min_severity)
		, _rotation_minutes(rotation_minutes)
		, _rotation_timestamp() {
		if (path == nullptr) {
			throw std::invalid_argument("path");
		}
	}


	inline log::~log() noexcept {
		if (_f != nullptr) {
			std::fclose(_f);
			_f = nullptr;
		}
	}


	inline void log::push(severity_t severity, category_t category, tag_t tag, const char* format, ...) {
		va_list vlist;
		va_start(vlist, format);

		push(severity, category, tag, process::current_process_id(), thread::current_thread_id(), format, true, vlist);

		va_end(vlist);
	}


	inline void log::push(severity_t severity, category_t category, tag_t tag, process_id_t process_id, thread_id_t thread_id, const char* format, bool use_vlist, va_list vlist) {
		// Filter by severity.
		if (severity < _min_severity) {
			return;
		}

		prepare_push(severity, fwide_char);

		if ((_field_mask & field::timestamp) != 0 && _min_severity > severity::debug_abc) {
			timestamp ts;
			std::fprintf(_f, "%s%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u", _separator.c_str(), ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
		}

		if ((_field_mask & field::category) != 0) {
			std::fprintf(_f, "%s0x%4.4x", _separator.c_str(), category);
		}

		if ((_field_mask & field::tag) != 0) {
			std::fprintf(_f, "%s0x%8.8x", _separator.c_str(), tag);
		}

		if ((_field_mask & field::process) != 0) {
			std::fprintf(_f, "%s0x%8.8x", _separator.c_str(), process_id);
		}

		if ((_field_mask & field::thread) != 0) {
			std::fprintf(_f, "%s0x%8.8x", _separator.c_str(), thread_id);
		}

		if (format != nullptr) {
			if (_field_mask != 0) {
				std::fputs(_separator.c_str(), _f);
			}

			if (use_vlist) {
				std::vfprintf(_f, format, vlist);
			}
			else {
				std::fputs(format, _f);
			}
		}

		if (_field_mask != 0) {
			std::fputs(_separator.c_str(), _f);
		}

		std::fputs("\n", _f);
	}


	inline void log::prepare_push(severity_t severity, int fwide_sign) {
		// Adjust rotation, if needed.
		if (_rotation_minutes > no_rotation) {
			timestamp current_timestamp;
			timestamp expected_rotation_timestamp = current_timestamp.coerse_minutes(_rotation_minutes);
			if (_rotation_timestamp != expected_rotation_timestamp) {
				_rotation_timestamp = expected_rotation_timestamp;

				if (_f != nullptr) {
					std::fclose(_f);
					_f = nullptr;
				}
			}
		}

		// Re-open the file, if needed.
		if (_f == nullptr) {
			if (_path.empty()) {
				throw unexpected("_path == nullptr", 0x1);
			}

			char path[max_path + 1];
			std::snprintf(path, sizeof(path) / sizeof(char), "%s_%4.4u%2.2u%2.2u_%2.2u%2.2u.log", _path.c_str(),
				_rotation_timestamp.year(), _rotation_timestamp.month(), _rotation_timestamp.day(), _rotation_timestamp.hours(), _rotation_timestamp.minutes());

			_f = std::fopen(path, "w+");
			if (_f == nullptr) {
				throw failed("fopen()", 0x2);
			}
		}

		// Check wide-ness of the file.
		if (std::fwide(_f, 0) * fwide_sign < 0) {
			throw failed("fwide()", 0x3);
		}
	}


}
