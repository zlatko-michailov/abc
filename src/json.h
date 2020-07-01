/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/


#pragma once

#include <cstdlib>

#include "ascii.h"
#include "exception.h"
#include "json.i.h"


namespace abc {

	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline _json_stream<StdStream, LogPtr, MaxLevels>::_json_stream(std::streambuf* sb, const LogPtr& log_ptr)
		: StdStream(sb)
		, _expect_property(false)
		, _level_top(-1)
		, _gcount(0)
		, _log_ptr(log_ptr) {
		if (_log_ptr != nullptr) {
			_log_ptr->push_back(category::abc::json, severity::abc, __TAG__, "_json_stream::_json_stream()");
		}
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::reset() {
		if (_log_ptr != nullptr) {
			_log_ptr->push_back(category::abc::json, severity::abc, __TAG__, "_json_stream::reset()");
		}

		StdStream::clear(StdStream::goodbit);
		_expect_property = false;
		_level_top = -1;
		_gcount = 0;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_stream<StdStream, LogPtr, MaxLevels>::levels() const noexcept {
		return _level_top + 1;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_stream<StdStream, LogPtr, MaxLevels>::gcount() const {
		return _gcount;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::eof() const {
		return StdStream::eof();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::good() const {
		return StdStream::good();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::bad() const {
		return StdStream::bad();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::fail() const {
		return StdStream::fail();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::operator!() const {
		return StdStream::operator!();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline _json_stream<StdStream, LogPtr, MaxLevels>::operator bool() const {
		return StdStream::operator bool();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::expect_property() const noexcept {
		return
			_expect_property
			&& _level_top >= 0
			&& _level_stack[_level_top] == json::level::object;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::set_expect_property(bool expect) noexcept {
		_expect_property =
			expect
			&& _level_top >= 0
			&& _level_stack[_level_top] == json::level::object;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::push_level(json::level_t level) noexcept {
		if (_level_top + 1 >= MaxLevels) {
			if (_log_ptr != nullptr) {
				_log_ptr->push_back(category::abc::json, severity::important, __TAG__, "_json_stream::push_level() levels='%lu', MaxLevels=%lu", (std::uint32_t)_level_top + 1, (std::uint32_t)MaxLevels);
			}

			set_bad();
			return;
		}

		_level_stack[++_level_top] = level;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::pop_level(json::level_t level) noexcept {
		if (_level_top + 1 <= 0) {
			if (_log_ptr != nullptr) {
				_log_ptr->push_back(category::abc::json, severity::important, __TAG__, "_json_stream::pop_level() levels='%lu'", (std::uint32_t)_level_top + 1);
			}

			set_bad();
			return;
		}

		if (_level_stack[_level_top] != level) {
			if (_log_ptr != nullptr) {
				_log_ptr->push_back(category::abc::json, severity::important, __TAG__, "_json_stream::pop_level() levels='%lu', top=%lu, pop=%lu", (std::uint32_t)_level_top + 1, (std::uint32_t)_level_stack[_level_top], (std::uint32_t)level);
			}

			set_bad();
			return;
		}

		_level_top--;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::set_gcount(std::size_t gcount) noexcept {
		_gcount = gcount;
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline bool _json_stream<StdStream, LogPtr, MaxLevels>::is_good() const {
		return StdStream::good() && !StdStream::eof();
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::set_bad() {
		StdStream::clear(StdStream::badbit | StdStream::failbit);
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline void _json_stream<StdStream, LogPtr, MaxLevels>::set_fail() {
		StdStream::setstate(StdStream::failbit);
	}


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	inline const LogPtr& _json_stream<StdStream, LogPtr, MaxLevels>::log_ptr() const noexcept {
		return _log_ptr;
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_istream<LogPtr, MaxLevels>::json_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: _json_stream<std::istream, LogPtr, MaxLevels>(sb, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::json_istream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_istream<LogPtr, MaxLevels>::get_token(json::token_t* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();

		if (buffer == nullptr) {
			throw exception<std::logic_error, LogPtr>("json_istream::get_token() buffer=nullptr", __TAG__, log_ptr_local);
		}

		if (size < sizeof(json::token_t)) {
			char buffer[100];
			std::snprintf(buffer, sizeof(buffer), "json_istream::get_token() size=%ld (< %ld) ", (std::int32_t)size, (std::int32_t)sizeof(json::token_t));

			throw exception<std::logic_error, LogPtr>(buffer, __TAG__, log_ptr_local);
		}

		get_or_skip_token(buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline json::item_t json_istream<LogPtr, MaxLevels>::skip_value() {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::skip_value() >>>");
		}

		std::size_t base_levels = this->levels();
		json::item_t item = json::item::none;
		do {
			item = get_or_skip_token(nullptr, 0);
		}
		while (this->levels() > base_levels);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::skip_value() <<< item=%4.4x", (std::uint32_t)item);
		}

		return item;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline json::item_t json_istream<LogPtr, MaxLevels>::get_or_skip_token(json::token_t* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_token() >>>");
		}

		json::item_t item = json::item::none;

		std::size_t gcount = sizeof(json::item_t);
		bool trail_comma = true;

		this->skip_spaces();

		char ch = this->peek_char();

		if (this->expect_property()) {
			if (ch == '"') {
				gcount += get_or_skip_string(buffer != nullptr ? buffer->value.property : nullptr, size - gcount);
				if (this->is_good()) {
					item = json::item::property;

					this->skip_spaces();

					ch = this->peek_char();
					if (ch == ':') {
						this->get();
					}
					else {
						if (log_ptr_local != nullptr) {
							log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected=':' ", ch, ch);
						}

						this->set_bad();
					}
				}

				this->set_expect_property(false);
				trail_comma = false;
			}
			else if (ch == '}') {
				this->get();

				item = json::item::end_object;
				this->pop_level(json::level::object);

				this->set_expect_property(true);
			}
			else {
				if (log_ptr_local != nullptr) {
					log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u%4.4x). Expected='\"' or '}'.", ch, ch);
				}

				this->set_bad();
			}
		}
		else {
			if (ch == 'n') {
				get_literal("null");
				if (this->is_good()) {
					item = json::item::null;
				}
			}
			else if (ch == 'f') {
				get_literal("false");
				if (this->is_good()) {
					item = json::item::boolean;
					if (buffer != nullptr) {
						buffer->value.boolean = false;
					}
					gcount += sizeof(bool);
				}
			}
			else if (ch == 't') {
				get_literal("true");
				if (this->is_good()) {
					item = json::item::boolean;
					if (buffer != nullptr) {
						buffer->value.boolean = true;
					}
					gcount += sizeof(bool);
				}
			}
			else if (ascii::is_digit(ch) || ch == '+' || ch == '-') {
				item = json::item::number;
				get_or_skip_number(buffer != nullptr ? &buffer->value.number : nullptr);
				gcount += sizeof(double);
			}
			else if (ch == '"') {
				gcount += get_or_skip_string(buffer != nullptr ? buffer->value.string : nullptr, size - gcount);
				if (this->is_good()) {
					item = json::item::string;
				}
			}
			else if (ch == '[') {
				this->get();

				item = json::item::begin_array;
				this->push_level(json::level::array);
				trail_comma = false;
			}
			else if (ch == ']') {
				this->get();

				item = json::item::end_array;
				this->pop_level(json::level::array);
			}
			else if (ch == '{') {
				this->get();

				item = json::item::begin_object;
				this->push_level(json::level::object);
				trail_comma = false;
			}
			else {
				if (log_ptr_local != nullptr) {
					log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_or_skip_token() ch=%c (\\u%4.4x)", ch, ch);
				}
				this->set_bad();
			}

			this->set_expect_property(true);
		}


		if (trail_comma && this->levels() > 0) {
			this->skip_spaces();

			ch = this->peek_char();
			if (ch == ',') {
				this->get();
			}
			else {
				if (this->expect_property()) {
					if (ch != '}') {
						if (log_ptr_local != nullptr) {
							log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected='}' ", ch, ch);
						}

						this->set_bad();
					}
				}
				else {
					if (ch != ']') {
						if (log_ptr_local != nullptr) {
							log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected=']' ", ch, ch);
						}

						this->set_bad();
					}
				}
			}
		}

		this->set_gcount(gcount);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_token() ch=%c (\\u%4.4x) <<<", ch, ch);
		}

		if (buffer != nullptr) {
			buffer->item = item;
		}

		return item;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_istream<LogPtr, MaxLevels>::get_or_skip_string(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_string() >>>");
		}

		std::size_t gcount = 0;

		char ch = this->peek_char();
		if (ch == '"') {
			this->get();

			for (;;) {
				gcount += this->get_or_skip_string_content(buffer != nullptr ? buffer + gcount : nullptr, size - gcount);

				ch = this->peek_char();
				if (ch == '"') {
					this->get();
					break;
				}
				else if (ch == '\\') {
					char ech = this->get_escaped_char();
					if (buffer != nullptr) {
						buffer[gcount++] = ech;
					}
				}
			}
		}

		if (buffer != nullptr) {
			buffer[gcount] = '\0';
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_string() string='%s' <<<", buffer);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_istream<LogPtr, MaxLevels>::get_or_skip_number(double* buffer) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_number() >>>");
		}

		std::size_t gcount = 0;
		char digits[19 + 6 + 1];

		char ch = this->peek_char();
		if (ch == '+' || ch == '-') {
			digits[gcount++] = this->get();
		}

		gcount += this->get_digits(digits + gcount, sizeof(digits) - gcount);

		ch = this->peek_char();
		if (ch == '.') {
			digits[gcount++] = this->get();

			gcount += this->get_digits(digits + gcount, sizeof(digits) - gcount);
		}

		ch = this->peek_char();
		if (ch == 'e' || ch == 'E') {
			digits[gcount++] = this->get();

			ch = this->peek_char();
			if (ch == '+' || ch == '-') {
				digits[gcount++] = this->get();
			}

			gcount += this->get_digits(digits + gcount, sizeof(digits) - gcount);
		}

		digits[gcount] = '\0';

		double number = 0;
		if (buffer != nullptr) {
			number = std::atof(digits);
			*buffer = number;
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_or_skip_number() number=%lf (%s) <<<", number, digits);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_istream<LogPtr, MaxLevels>::get_literal(const char* literal) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_literal() literal='%s' >>>", literal);
		}

		for (const char* it = literal; *it != '\0'; it++) {
			char ch = this->get_char();
			if (ch != *it) {
				if (log_ptr_local != nullptr) {
					log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_literal() ch='%c' (\\u%4.4x). Expected='%c' (\\u%4.4x)", ch, ch, *it, *it);
				}

				this->set_bad();
				break;
			}
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_literal() <<<");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline char json_istream<LogPtr, MaxLevels>::get_escaped_char() {
		LogPtr log_ptr_local = this->log_ptr();

		char ch = peek_char();

		if (ch == '\\') {
			this->get();

			ch = this->peek_char();
			if (ch == '"' || ch == '\\' || ch == '/') {
				this->get();
			}
			else if (ch == 'b') {
				this->get();
				ch = '\b';
			}
			else if (ch == 'f') {
				this->get();
				ch = '\f';
			}
			else if (ch == 'n') {
				this->get();
				ch = '\n';
			}
			else if (ch == 'r') {
				this->get();
				ch = '\r';
			}
			else if (ch == 't') {
				this->get();
				ch = '\t';
			}
			else if (ch == 'u') {
				this->get();

				char buffer[4 + 1];

				std::size_t gcount = this->get_hex(buffer, sizeof(buffer));

				if (gcount != 4) {
					if (log_ptr_local != nullptr) {
						log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_escaped_char() gcount=%d ", gcount);
					}

					this->set_bad();
					ch = '\0';
				}
				else if (buffer[0] == '0' && buffer[1] == '0') {
					ch = (ascii::hex(buffer[2]) << 4) | ascii::hex(buffer[3]);
				}
				else {
					if (log_ptr_local != nullptr) {
						log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::get_escaped_char() Wide chars not supported.");
					}

					this->set_bad();
					ch = '\0';
				}
			}
		}
		else {
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_escaped_char() ch='%c' (\\u%4.4x). Unexpected.", ch, ch);
			}

			this->set_bad();
			ch = '\0';
		}

		return ch;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_istream<LogPtr, MaxLevels>::get_or_skip_string_content(char* buffer, std::size_t size) {
		std::size_t gcount = 0;

		if (buffer != nullptr) {
			gcount = get_chars(ascii::json::is_string_content, buffer, size);
		}
		else {
			gcount = skip_chars(ascii::json::is_string_content);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_istream<LogPtr, MaxLevels>::get_hex(char* buffer, std::size_t size) {
		return get_chars(ascii::is_hex, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_istream<LogPtr, MaxLevels>::get_digits(char* buffer, std::size_t size) {
		return get_chars(ascii::is_digit, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_istream<LogPtr, MaxLevels>::skip_spaces() {
		return skip_chars(ascii::json::is_space);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	template <typename Predicate>
	inline std::size_t json_istream<LogPtr, MaxLevels>::get_chars(Predicate&& predicate, char* buffer, std::size_t size) {
		std::size_t gcount = 0;

		while (this->is_good() && predicate(peek_char())) {
			if (gcount == size - 1) {
				this->set_fail();
				break;
			}

			buffer[gcount++] = this->get();
		}
		buffer[gcount] = '\0';

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	template <typename Predicate>
	inline std::size_t json_istream<LogPtr, MaxLevels>::skip_chars(Predicate&& predicate) {
		std::size_t gcount = 0;
		
		while (this->is_good() && predicate(peek_char())) {
			this->get();
			gcount++;
		}

		return gcount;
	}
	
	
	template <typename LogPtr, std::size_t MaxLevels>
	inline char json_istream<LogPtr, MaxLevels>::get_char() {
		char ch = peek_char();

		if (this->is_good()) {
			this->get();
		}

		return ch;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline char json_istream<LogPtr, MaxLevels>::peek_char() {
		char ch = this->peek();

		if (!ascii::json::is_valid(ch)) {
			this->set_bad();
			ch = '\0';
		}

		return ch;
	}


	// --------------------------------------------------------------

#ifdef TEMP
	template <typename LogPtr, std::size_t MaxLevels>
	inline _json_ostream<LogPtr>::_json_ostream(std::streambuf* sb, json::item_t next, const LogPtr& log_ptr)
		: _json_stream<std::ostream, LogPtr>(sb, next, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10047, "_json_ostream::_json_ostream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_ostream<LogPtr>::set_pstate(std::size_t gcount, json::item_t next) {
		this->flush();
		this->set_state(gcount, next);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_protocol(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10048, "_json_ostream::put_protocol() >>>");
		}

		this->assert_next(json::item::protocol);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = 0;	

		if (size < 5 || std::strncmp(buffer, "json/", 5) != 0) {
			this->set_bad();
		}
		else {
			gcount = this->put_bytes("json/", 5);
		}

		if (this->is_good() && gcount < size) {
			std::size_t gcount_local = this->put_digits(buffer + gcount, size - gcount);
			if (gcount_local == 0) {
				this->set_bad();
			}
			else {
				gcount += gcount_local;
			}
		}

		if (this->is_good() && gcount < size) {
			if (buffer[gcount] != '.') {
				this->set_bad();
			}
			else {
				this->put('.');
				gcount++;
			}
		}

		if (this->is_good() && gcount < size) {
			std::size_t gcount_local = this->put_digits(buffer + gcount, size - gcount);
			if (gcount_local == 0) {
				this->set_bad();
			}
			else {
				gcount += gcount_local;
			}
		}

		if (this->is_good() && gcount < size) {
			this->set_bad();
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10049, "_json_ostream::put_protocol() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_ostream<LogPtr>::put_header_name(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1004a, "_json_ostream::put_header_name() >>>");
		}

		this->assert_next(json::item::header_name);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = this->put_token(buffer, size);

		if (this->is_good()) {
			if (gcount < size) {
				this->set_bad();
			}
		}

		if (this->is_good()) {
			this->put(':');
			this->put_space();
		}

		this->set_pstate(gcount, json::item::header_value);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1004b, "_json_ostream::put_header_name() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_ostream<LogPtr>::put_header_value(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1004c, "_json_ostream::put_header_value() >>>");
		}

		this->assert_next(json::item::header_value);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = 0;
		do {
			std::size_t sp = this->skip_spaces_in_header_value(buffer + gcount, size - gcount);

			if (gcount > 0 && sp > 0 && gcount + sp < size) {
				this->put_space();
			}

			gcount += sp;

			if (gcount < size) {
				if (ascii::is_abcprint(buffer[gcount])) {
					gcount += this->put_prints(buffer + gcount, size - gcount);
				}
				else {
					this->set_bad();
				}
			}
		}
		while (this->is_good() && gcount < size);

		this->put_crlf();

		this->set_pstate(gcount, json::item::header_name);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1004d, "_json_ostream::put_header_value() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_ostream<LogPtr>::end_headers() {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1004e, "_json_ostream::end_headers() >>>");
		}

		this->assert_next(json::item::header_name);

		std::size_t gcount = 0;
		this->put_crlf();

		this->set_pstate(gcount, json::item::body);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1004f, "_json_ostream::end_headers() <<< gcount=%lu", (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_ostream<LogPtr>::put_body(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10050, "_json_ostream::put_body() >>>");
		}

		this->assert_next(json::item::body);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = this->put_bytes(buffer, size);

		this->set_pstate(gcount, json::item::body);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10051, "_json_ostream::put_body() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_crlf() {
		return this->put_char('\r') + this->put_char('\n');
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_space() {
		return this->put_char(' ');
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_token(const char* buffer, std::size_t size) {
		return this->put_chars(ascii::json::is_token, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_prints(const char* buffer, std::size_t size) {
		return this->put_chars(ascii::is_abcprint, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_prints_and_spaces(const char* buffer, std::size_t size) {
		return this->put_chars(ascii::is_abcprint_or_space, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_alphas(const char* buffer, std::size_t size) {
		return this->put_chars(ascii::is_alpha, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_digits(const char* buffer, std::size_t size) {
		return this->put_chars(ascii::is_digit, buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_bytes(const char* buffer, std::size_t size) {
		std::size_t gcount = 0;

		while (this->is_good() && gcount < size) {
			this->put(buffer[gcount++]);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	template <typename Predicate>
	inline std::size_t _json_ostream<LogPtr>::put_chars(Predicate&& predicate, const char* buffer, std::size_t size) {
		std::size_t gcount = 0;

		while (this->is_good() && gcount < size && predicate(buffer[gcount])) {
			this->put(buffer[gcount++]);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::put_char(char ch) {
		if (this->is_good()) {
			this->put(ch);
		}

		return this->is_good() ? 1 : 0;;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::skip_spaces_in_header_value(const char* buffer, std::size_t size) {
		std::size_t sp = 0;

		while (sp < size) {
			if (ascii::is_space(buffer[sp])) {
				sp++;
			}
			else if (sp + 3 < size && buffer[sp] == '\r' && buffer[sp + 1] == '\n' && ascii::is_space(buffer[sp + 2])) {
				sp += 3;
			}
			else {
				break;
			}
		}

		return sp;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_ostream<LogPtr>::skip_spaces(const char* buffer, std::size_t size) {
		std::size_t sp = 0;

		while (sp < size && ascii::is_space(buffer[sp])) {
			sp++;
		}

		return sp;
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_request_istream<LogPtr>::json_request_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: json_istream<LogPtr, MaxLevels>(sb, json::item::method, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10052, "json_request_istream::json_request_istream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_istream<LogPtr>::reset() {
		_json_stream<std::istream, LogPtr>::reset(json::item::method);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_istream<LogPtr>::get_method(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10053, "json_request_istream::get_method() >>>");
		}

		this->assert_next(json::item::method);

		std::size_t gcount = this->get_token(buffer, size);

		this->skip_spaces();

		this->set_gstate(gcount, json::item::resource);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10054, "json_request_istream::get_method() <<< method='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_istream<LogPtr>::get_resource(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10055, "json_request_istream::get_resource() >>>");
		}

		this->assert_next(json::item::resource);

		std::size_t gcount = this->get_prints(buffer, size);

		this->skip_spaces();

		this->set_gstate(gcount, json::item::protocol);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10056, "json_request_istream::get_resource() <<< resource='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_istream<LogPtr>::get_protocol(char* buffer, std::size_t size) {
		std::size_t gcount = json_istream<LogPtr, MaxLevels>::get_protocol(buffer, size);

		this->skip_crlf();

		this->set_gstate(gcount, json::item::header_name);
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_request_ostream<LogPtr>::json_request_ostream(std::streambuf* sb, const LogPtr& log_ptr)
		: _json_ostream<LogPtr>(sb, json::item::method, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10057, "json_request_ostream::json_request_ostream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_ostream<LogPtr>::reset() {
		_json_stream<std::ostream, LogPtr>::reset(json::item::method);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_ostream<LogPtr>::put_method(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10058, "_json_ostream::put_method() >>>");
		}

		this->assert_next(json::item::method);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = this->put_token(buffer, size);

		this->put_space();

		this->set_pstate(gcount, json::item::resource);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10059, "_json_ostream::put_method() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_ostream<LogPtr>::put_resource(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1005a, "_json_ostream::put_resource() >>>");
		}

		this->assert_next(json::item::resource);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = this->put_prints(buffer, size);

		this->put_space();

		this->set_pstate(gcount, json::item::protocol);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1005b, "_json_ostream::put_resource() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_request_ostream<LogPtr>::put_protocol(const char* buffer, std::size_t size) {
		std::size_t gcount = _json_ostream<LogPtr>::put_protocol(buffer, size);

		this->put_crlf();

		this->set_pstate(gcount, json::item::header_name);
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_response_istream<LogPtr>::json_response_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: json_istream<LogPtr, MaxLevels>(sb, json::item::protocol, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1005c, "json_response_istream::json_response_istream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_istream<LogPtr>::reset() {
		_json_stream<std::istream, LogPtr>::reset(json::item::protocol);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_istream<LogPtr>::get_protocol(char* buffer, std::size_t size) {
		std::size_t gcount = json_istream<LogPtr, MaxLevels>::get_protocol(buffer, size);

		this->set_gstate(gcount, json::item::status_code);
	}

	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_istream<LogPtr>::get_status_code(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1005d, "json_response_istream::get_status_code() >>>");
		}

		this->assert_next(json::item::status_code);

		std::size_t gcount = this->get_digits(buffer, size);

		this->skip_spaces();

		this->set_gstate(gcount, json::item::reason_phrase);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1005e, "json_response_istream::get_status_code() <<< status_code='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_istream<LogPtr>::get_reason_phrase(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x1005f, "json_response_istream::get_reason_phrase() >>>");
		}

		this->assert_next(json::item::reason_phrase);

		std::size_t gcount = this->get_prints_and_spaces(buffer, size);

		this->skip_spaces();
		this->skip_crlf();

		this->set_gstate(gcount, json::item::header_name);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10060, "json_response_istream::get_reson_phrase() <<< reason_phrase='%s', gcount=%lu", buffer, (std::uint32_t)gcount);
		}
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_response_ostream<LogPtr>::json_response_ostream(std::streambuf* sb, const LogPtr& log_ptr)
		: _json_ostream<LogPtr>(sb, json::item::protocol, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10061, "json_response_ostream::json_response_ostream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_ostream<LogPtr>::reset() {
		_json_stream<std::ostream, LogPtr>::reset(json::item::protocol);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_ostream<LogPtr>::put_protocol(const char* buffer, std::size_t size) {
		std::size_t gcount = _json_ostream<LogPtr>::put_protocol(buffer, size);

		this->put_space();

		this->set_pstate(gcount, json::item::status_code);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_ostream<LogPtr>::put_status_code(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10062, "json_response_ostream::put_status_code() >>>");
		}

		this->assert_next(json::item::status_code);

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		std::size_t gcount = this->put_digits(buffer, size);

		this->put_space();

		this->set_pstate(gcount, json::item::reason_phrase);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10063, "json_response_ostream::put_status_code() <<< buffer='%s', size=%lu, gcount=%lu", buffer, (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_response_ostream<LogPtr>::put_reason_phrase(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10064, "json_response_ostream::put_reason_phrase() >>>");
		}

		this->assert_next(json::item::reason_phrase);

		std::size_t gcount = 0;
		if (buffer != nullptr) {
			if (size == size::strlen) {
				size = std::strlen(buffer);
			}

			gcount = this->put_prints_and_spaces(buffer, size);
		}

		this->put_crlf();

		this->set_pstate(gcount, json::item::header_name);

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, 0x10065, "json_response_ostream::put_reason_phrase() <<< buffer='%s', size=%lu, gcount=%lu", buffer != nullptr ? buffer : "<nullptr>", (std::uint32_t)size, (std::uint32_t)gcount);
		}
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_client_stream<LogPtr>::json_client_stream(std::streambuf* sb, const LogPtr& log_ptr)
		: json_request_ostream<LogPtr>(sb, log_ptr)
		, json_response_istream<LogPtr>(sb, log_ptr) {
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_server_stream<LogPtr>::json_server_stream(std::streambuf* sb, const LogPtr& log_ptr)
		: json_request_istream<LogPtr>(sb, log_ptr)
		, json_response_ostream<LogPtr>(sb, log_ptr) {
	}
#endif

}

