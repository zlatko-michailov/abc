#include <cstdio>
#include <cstdarg>
#include "base.h"
#include "timestamp.h"
#include "log.h"
#include "macros.h"


namespace abc {

	basic_log::~basic_log() noexcept {
		if (_f != nullptr) {
			std::fflush(_f);
			std::fclose(_f);
			_f = nullptr;
		}
	}


	status_t basic_log::push(severity_t severity, category_t category, tag_t tag, status_t status) noexcept {
		return push(severity, category, tag, status, static_cast<const char*>(nullptr));
	}


	status_t basic_log::push(severity_t severity, category_t category, tag_t tag, status_t status, const char* format, ...) noexcept {
		// TODO: Filter by severity

		status_t st = ensure_f();
		if (status::failed(st)) {
			return st;
		}

		if (std::fwide(_f, 0) > 0)
		{
			return status::bad_state;
		}

		std::fprintf(_f, "%s0x%4.4x%s0x%8.8x%s0x%4.4x%s", _separator, category, _separator, tag, _separator, status, _separator);

		if (format != nullptr) {
			va_list vlist;
			va_start(vlist, format);

			std::vfprintf(_f, format, vlist);
		}

		std::fprintf(_f, "\n");

		return status::success;
	}


	status_t basic_log::push(severity_t severity, category_t category, tag_t tag, status_t status, const wchar_t* format, ...) noexcept {
		// TODO: Filter by severity
		
		status_t st = ensure_f();
		if (status::failed(st)) {
			return st;
		}

		if (std::fwide(_f, 0) < 0)
		{
			return status::bad_state;
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


	status_t basic_log::ensure_f() noexcept {
		// TODO: rotation

		if (_f == nullptr) {
			if (_path == nullptr) {
				return status::assert_failed;
			}

			_f = std::fopen(_path, "w+");
			if (_f == nullptr) {
				return status::bad_state;
			}
		}

		return status::success;
	}


	static basic_log default_diag = basic_log(stdout);
	/*static*/ basic_log& basic_log::diag = default_diag;

}
