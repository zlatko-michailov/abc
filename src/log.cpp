#include <cstdio>
#include <cstdarg>
#include <thread>
#include <ostream>
#include "base.h"
#include "timestamp.h"
#include "mutex.h"
#include "streambuf.h"
#include "log.h"
#include "macros.h"


namespace abc {

	static constexpr int fwide_char = -1;
	static constexpr int fwide_wide = +1;


	basic_log::~basic_log() noexcept {
		if (_f != nullptr) {
			std::fclose(_f);
			_f = nullptr;
		}
	}


	template <>
	status_t basic_log::push<char>(severity_t severity, category_t category, tag_t tag, status_t status) noexcept {
		return push(severity, category, tag, status, static_cast<const char*>(nullptr));
	}


	template <>
	status_t basic_log::push<wchar_t>(severity_t severity, category_t category, tag_t tag, status_t status) noexcept {
		return push(severity, category, tag, status, static_cast<const wchar_t*>(nullptr));
	}


	status_t basic_log::push(severity_t severity, category_t category, tag_t tag, status_t status, const char* format, ...) noexcept {
		status_lock lock(_mutex);
		if (status::failed(lock.status())) {
			return lock.status();
		}

		status_t st = prepare_push(severity, fwide_char);
		if (st != status::success) {
			return st;
		}

		if ((filed_mask & field::timestamp) != 0 && min_severity > severity::debug_abc) {
			timestamp ts;
			std::fprintf(_f, "%s%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u", _separator, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
		}

		if ((filed_mask & field::category) != 0) {
			std::fprintf(_f, "%s0x%4.4x", _separator, category);
		}

		if ((filed_mask & field::thread) != 0) {
			abc::arraybuf<20> buf;
			std::ostream stream(&buf);
			stream << std::hex << std::this_thread::get_id();

			std::fputs(_separator, _f);
			std::fputs(buf.c_str(), _f);
		}

		if ((filed_mask & field::tag) != 0) {
			std::fprintf(_f, "%s0x%8.8x", _separator, tag);
		}

		if ((filed_mask & field::status) != 0) {
			std::fprintf(_f, "%s0x%4.4x", _separator, status);
		}

		if (format != nullptr) {
			if (filed_mask != 0) {
				std::fputs(_separator, _f);
			}

			va_list vlist;
			va_start(vlist, format);

			std::vfprintf(_f, format, vlist);
		}

		if (filed_mask != 0) {
			std::fputs(_separator, _f);
		}

		std::fputs("\n", _f);

		return status::success;
	}


	status_t basic_log::push(severity_t severity, category_t category, tag_t tag, status_t status, const wchar_t* format, ...) noexcept {
		status_lock lock(_mutex);
		if (status::failed(lock.status())) {
			return lock.status();
		}

		status_t st = prepare_push(severity, fwide_wide);
		if (st != status::success) {
			return st;
		}

		if (min_severity > severity::debug_abc) {
			timestamp ts;
			std::fwprintf(_f, L"%hs%4.4u-%2.2u-%2.2u %2.2u:%2.2u:%2.2u.%3.3u",
				_separator, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
		}

		std::fwprintf(_f, L"%hs0x%4.4x%hs0x%8.8x%hs0x%4.4x%hs", _separator, category, _separator, tag, _separator, status, _separator);

		if (format != nullptr) {
			va_list vlist;
			va_start(vlist, format);

			std::vfwprintf(_f, format, vlist);
		}

		std::fwprintf(_f, L"\n");

		return status::success;
	}


	status_t basic_log::prepare_push(severity_t severity, int fwide_sign) noexcept {
		// Filter by severity.
		if (severity < min_severity) {
			return status::ignored;
		}

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
			if (_path == nullptr) {
				return status::assert_failed;
			}

			char path[max_path + 1];
			std::snprintf(path, sizeof(path) / sizeof(char), "%s_%4.4u%2.2u%2.2u_%2.2u%2.2u.log", _path,
				_rotation_timestamp.year(), _rotation_timestamp.month(), _rotation_timestamp.day(), _rotation_timestamp.hours(), _rotation_timestamp.minutes());

			_f = std::fopen(path, "w+");
			if (_f == nullptr) {
				return status::bad_state;
			}
		}

		// Check wide-ness of the file.
		if (std::fwide(_f, 0) * fwide_sign < 0) {
			return status::bad_state;
		}

		return status::success;
	}


	static basic_log default_diag = basic_log(stdout);
	/*static*/ basic_log& basic_log::diag = default_diag;

}
