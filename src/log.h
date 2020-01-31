#include <cstring>
#include <fstream>

#include "log.i.h"
#include "timestamp.h"
#include "mutex.h"
#include "exception.h"


namespace abc {

	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline log<LineSize, Container, View, Filter>::log(Container&& container, View&& view, Filter&& filter) noexcept
		: _container(std::move(container))
		, _view(std::move(view))
		, _filter(std::move(filter)) {
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline log<LineSize, Container, View, Filter>::log(log<LineSize, Container, View, Filter>&& other) noexcept {
		_container = std::move(other._container);
		_view = std::move(other._view);
		_filter = std::move(other.filter);
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline void log<LineSize, Container, View, Filter>::push_back(category_t category, severity_t severity, tag_t tag, const char* format, ...) {
		va_list vlist;
		va_start(vlist, format);

		vpush_back(category, severity, tag, format, vlist);

		va_end(vlist);
	}


	template <std::size_t LineSize, typename Container, typename View, typename Filter>
	inline void log<LineSize, Container, View, Filter>::vpush_back(category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
		if (_filter.is_enabled(category, severity)) {
			char line[LineSize];
			_view.format(line, LineSize, category, severity, tag, format, vlist);

			_container.push_back(line);
		}
	}


	namespace log_container {
		inline ostream::ostream(std::streambuf* sb) noexcept
			: _mutex()
			, _stream(sb) {
		}


		inline ostream::ostream(ostream&& other) noexcept
			: _mutex()
			, _stream(other._stream.rdbuf()) {
			other._stream.rdbuf(nullptr);
		}


		inline void ostream::push_back(const char* line) {
			std::lock_guard lock(_mutex);
			_stream << line << std::endl;
		}


		inline void ostream::set_streambuf(std::streambuf* sb) {
			std::lock_guard lock(_mutex);
			_stream.flush();
			_stream.rdbuf(sb);
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::file(const char* path)
			: file<MaxPath, Clock>(path, no_rotation) {
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::file(const char* path, std::chrono::minutes::rep rotation_minutes)
			: ostream(&_filebuf)
			, _rotation_minutes(rotation_minutes) {
			std::cout << "construct  path='" << path << "'" << std::endl;

			set_streambuf(&_filebuf);

			std::size_t path_length = std::strlen(path);
			if (path_length + 16 > MaxPath) {
				throw unexpected("std::strlen(path)", __TAG__);
			}

			_path_length = path_length;
			std::strcpy(_path, path);

			ensure_filebuf();
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::file(file<MaxPath, Clock>&& other) noexcept
			: ostream(&_filebuf) {
			std::memmove(_path, other._path, sizeof(_path));
			_path_length = other._path_length;
			_filebuf = std::move(other._filebuf);
			_rotation_minutes = other._rotation_minutes;
			_rotation_timestamp = other._rotation_timestamp;
		}

		template <std::size_t MaxPath, typename Clock>
		inline void file<MaxPath, Clock>::push_back(const char* line) {
			if (_rotation_minutes > no_rotation) {
				ensure_filebuf();
			}
	
			ostream::push_back(line);
		}


		template <std::size_t MaxPath, typename Clock>
		inline file<MaxPath, Clock>::~file() {
			std::cout << "destroy _path='" << _path << "', is_open=" << _filebuf.is_open() << std::endl;
			if (_filebuf.is_open()) {
				_filebuf.close();
			}
		}


		template <std::size_t MaxPath, typename Clock>
		inline void file<MaxPath, Clock>::ensure_filebuf() {
			timestamp<Clock> expected_rotation_timestamp;
			if (_rotation_minutes > no_rotation) {
				expected_rotation_timestamp = expected_rotation_timestamp.coerse_minutes(_rotation_minutes);
			}

			if (_rotation_timestamp != expected_rotation_timestamp || !_filebuf.is_open()) {
				log_view::format_timestamp(_path + _path_length, MaxPath + 1 - _path_length, expected_rotation_timestamp, log_view::format::datetime::file);
				_rotation_timestamp = expected_rotation_timestamp;

				if (_filebuf.is_open()) {
					std::cout << "close _path='" << _path << "'" << std::endl;
					_filebuf.close();
				}

				std::cout << "open  _path='" << _path << "'" << std::endl;
				_filebuf.open(_path, std::ios_base::out);
			}
		}
	}


	namespace log_view {
		// TODO: template <Clock>
		inline void debug::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			char buf_timestamp[31];
			format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp(), format::datetime::friendly);

			char buf_category[5];
			format_category(buf_category, sizeof(buf_category), category, format::category::friendly);

			char buf_severity[2];
			format_severity(buf_severity, sizeof(buf_severity), severity, format::severity::friendly);

			char buf_tag[17];
			format_tag(buf_tag, sizeof(buf_tag), tag, format::tag::friendly);

			int char_count = std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::friendly, buf_category, format::separator::friendly, buf_severity, format::separator::friendly, buf_tag, format::separator::friendly);
			if (0 <= char_count && char_count < line_size) {
				std::vsnprintf(line + char_count, line_size - char_count, format, vlist);
			}
		}


		inline void diag::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t tag, const char* format, va_list vlist) {
			char buf_timestamp[31];
			format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp(), format::datetime::iso);

			char buf_category[5];
			format_category(buf_category, sizeof(buf_category), category, format::category::compact);

			char buf_severity[2];
			format_severity(buf_severity, sizeof(buf_severity), severity, format::severity::compact);

			char buf_tag[17];
			format_tag(buf_tag, sizeof(buf_tag), tag, format::tag::compact);

			int char_count = std::snprintf(line, line_size, "%s%s%s%s%s%s%s%s", buf_timestamp, format::separator::compact, buf_category, format::separator::compact, buf_severity, format::separator::compact, buf_tag, format::separator::compact);
			if (0 <= char_count && char_count < line_size) {
				std::vsnprintf(line + char_count, line_size - char_count, format, vlist);
			}
		}


		inline void test::format(char* line, std::size_t line_size, category_t category, severity_t severity, tag_t /*tag*/, const char* format, va_list vlist) {
			char buf_timestamp[31];
			format_timestamp(buf_timestamp, sizeof(buf_timestamp), timestamp(), format::datetime::friendly);

			char buf_severity[2 * severity::abc + 1];
			severity = severity <= severity::abc ? severity : severity::abc;
			std::memset(buf_severity, ' ', 2 * severity);
			buf_severity[2 * (severity - 1)] = '\0';

			int char_count = std::snprintf(line, line_size, "%s%s%s%s", buf_timestamp, format::separator::space, buf_severity, format::separator::space);
			if (0 <= char_count && char_count < line_size) {
				std::vsnprintf(line + char_count, line_size - char_count, format, vlist);
			}
		}


		inline void blank::format(char* line, std::size_t line_size, category_t /*category*/, severity_t /*severity*/, tag_t /*tag*/, const char* format, va_list vlist) {
			std::vsnprintf(line, line_size, format, vlist);
		}


		template <typename Clock = std::chrono::system_clock>
		inline int format_timestamp(char* line, std::size_t line_size, const timestamp<Clock>& ts, const char* format) {
			return std::snprintf(line, line_size, format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
		}


		inline int format_thread_id(char* line, std::size_t line_size, std::thread::id thread_id) {
			// TODO:
			return -1;
		}


		inline int format_category(char* line, std::size_t line_size, category_t category, const char* format) {
			return std::snprintf(line, line_size, format, category);
		}


		inline int format_severity(char* line, std::size_t line_size, severity_t severity, const char* format) {
			return std::snprintf(line, line_size, format, severity);
		}


		inline int format_tag(char* line, std::size_t line_size, tag_t tag, const char* format) {
			return std::snprintf(line, line_size, format, tag);
		}
	}


	namespace log_filter {
		inline bool none::is_enabled(category_t category, severity_t severity) const noexcept {
			return true;
		}


		inline severity::severity(severity_t min_severity) noexcept
			: _min_severity(min_severity) {
		}

		inline bool severity::is_enabled(category_t /*category*/, severity_t severity) const noexcept {
			return abc::severity::is_higher_or_equal(severity, _min_severity);
		}
	}

}














#ifdef REMOVE
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
				throw unexpected("_path == nullptr", __TAG__);
			}

			char path[max_path + 1];
			std::snprintf(path, sizeof(path) / sizeof(char), "%s_%4.4u%2.2u%2.2u_%2.2u%2.2u.log", _path.c_str(),
				_rotation_timestamp.year(), _rotation_timestamp.month(), _rotation_timestamp.day(), _rotation_timestamp.hours(), _rotation_timestamp.minutes());

			_f = std::fopen(path, "w+");
			if (_f == nullptr) {
				throw failed("fopen()", __TAG__);
			}
		}

		// Check wide-ness of the file.
		if (std::fwide(_f, 0) * fwide_sign < 0) {
			throw failed("fwide()", __TAG__);
		}
	}


}
#endif