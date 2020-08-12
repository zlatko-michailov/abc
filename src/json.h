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
#include <cstdio>

#include "ascii.h"
#include "exception.h"
#include "json.i.h"
#include "stream.h"


namespace abc {

	template <typename LogPtr, std::size_t MaxLevels>
	inline _json_state<LogPtr, MaxLevels>::_json_state(const LogPtr& log_ptr)
		: _expect_property(false)
		, _level_top(-1)
		, _log_ptr(log_ptr) {
		if (_log_ptr != nullptr) {
			_log_ptr->put_any(category::abc::json, severity::abc, 0x100f9, "_json_state::_json_state()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_state<LogPtr, MaxLevels>::reset() noexcept {
		if (_log_ptr != nullptr) {
			_log_ptr->put_any(category::abc::json, severity::abc, 0x100fa, "_json_state::reset()");
		}

		_expect_property = false;
		_level_top = -1;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t _json_state<LogPtr, MaxLevels>::levels() const noexcept {
		return _level_top + 1;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline json::level_t _json_state<LogPtr, MaxLevels>::top_level() const noexcept {
		return 0 <= _level_top ? _level_stack[_level_top] : json::level::array;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline bool _json_state<LogPtr, MaxLevels>::expect_property() const noexcept {
		return
			_expect_property
			&& _level_top >= 0
			&& _level_stack[_level_top] == json::level::object;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void _json_state<LogPtr, MaxLevels>::set_expect_property(bool expect) noexcept {
		_expect_property =
			expect
			&& _level_top >= 0
			&& _level_stack[_level_top] == json::level::object;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline bool _json_state<LogPtr, MaxLevels>::push_level(json::level_t level) noexcept {
		if (_level_top + 1 >= MaxLevels) {
			if (_log_ptr != nullptr) {
				_log_ptr->put_any(category::abc::json, severity::important, 0x100fb, "_json_state::push_level() levels='%lu', MaxLevels=%lu", (std::uint32_t)_level_top + 1, (std::uint32_t)MaxLevels);
			}

			return false;
		}

		_level_stack[++_level_top] = level;
		return true;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline bool _json_state<LogPtr, MaxLevels>::pop_level(json::level_t level) noexcept {
		if (_level_top + 1 <= 0) {
			if (_log_ptr != nullptr) {
				_log_ptr->put_any(category::abc::json, severity::important, 0x100fc, "_json_state::pop_level() levels='%lu'", (std::uint32_t)_level_top + 1);
			}

			return false;
		}

		if (_level_stack[_level_top] != level) {
			if (_log_ptr != nullptr) {
				_log_ptr->put_any(category::abc::json, severity::important, 0x100fd, "_json_state::pop_level() levels='%lu', top=%lu, pop=%lu", (std::uint32_t)_level_top + 1, (std::uint32_t)_level_stack[_level_top], (std::uint32_t)level);
			}

			return false;
		}

		_level_top--;
		return true;
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_istream<LogPtr, MaxLevels>::json_istream(std::streambuf* sb, const LogPtr& log_ptr)
		: base(sb, log_ptr)
		, state(log_ptr) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x100fe, "json_istream::json_istream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_istream<LogPtr, MaxLevels>::get_token(json::token_t* buffer, std::size_t size) {
		LogPtr log_ptr_local = base::log_ptr();

		if (buffer == nullptr) {
			throw exception<std::logic_error, LogPtr>("json_istream::get_token() buffer=nullptr", 0x100ff, log_ptr_local);
		}

		if (size < sizeof(json::token_t)) {
			char buffer[100];
			std::snprintf(buffer, sizeof(buffer), "json_istream::get_token() size=%ld (< %ld) ", (std::int32_t)size, (std::int32_t)sizeof(json::token_t));

			throw exception<std::logic_error, LogPtr>(buffer, 0x10100, log_ptr_local);
		}

		get_or_skip_token(buffer, size);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline json::item_t json_istream<LogPtr, MaxLevels>::skip_value() {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10101, "json_istream::skip_value() >>>");
		}

		std::size_t base_levels = state::levels();
		json::item_t item = json::item::none;
		do {
			item = get_or_skip_token(nullptr, 0);
		}
		while (state::levels() > base_levels);

		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10102, "json_istream::skip_value() <<< item=%4.4x", item);
		}

		return item;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline json::item_t json_istream<LogPtr, MaxLevels>::get_or_skip_token(json::token_t* buffer, std::size_t size) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10103, "json_istream::get_or_skip_token() >>>");
		}

		json::item_t item = json::item::none;

		std::size_t gcount = sizeof(json::item_t);
		bool trail_comma = true;

		skip_spaces();

		char ch = peek_char();

		if (state::expect_property()) {
			if (ch == '"') {
				gcount += get_or_skip_string(buffer != nullptr ? buffer->value.property : nullptr, size - gcount);
				if (base::is_good()) {
					item = json::item::property;

					skip_spaces();

					ch = peek_char();
					if (ch == ':') {
						base::get();
					}
					else {
						if (log_ptr_local != nullptr) {
							log_ptr_local->put_any(category::abc::json, severity::important, 0x10104, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected=':' ", ch, ch);
						}

						base::set_bad();
					}
				}

				state::set_expect_property(false);
				trail_comma = false;
			}
			else if (ch == '}') {
				base::get();

				item = json::item::end_object;
				base::set_bad_if(!state::pop_level(json::level::object));

				state::set_expect_property(true);
			}
			else {
				if (log_ptr_local != nullptr) {
					log_ptr_local->put_any(category::abc::json, severity::important, 0x10105, "json_istream::get_or_skip_token() ch='%c' (\\u%4.4x). Expected='\"' or '}'.", ch, ch);
				}

				base::set_bad();
			}
		}
		else {
			if (ch == 'n') {
				get_literal("null");
				if (base::is_good()) {
					item = json::item::null;
				}
			}
			else if (ch == 'f') {
				get_literal("false");
				if (base::is_good()) {
					item = json::item::boolean;
					if (buffer != nullptr) {
						buffer->value.boolean = false;
					}
					gcount += sizeof(bool);
				}
			}
			else if (ch == 't') {
				get_literal("true");
				if (base::is_good()) {
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
				if (base::is_good()) {
					item = json::item::string;
				}
			}
			else if (ch == '[') {
				base::get();

				item = json::item::begin_array;
				base::set_bad_if(!state::push_level(json::level::array));
				trail_comma = false;
			}
			else if (ch == ']') {
				base::get();

				item = json::item::end_array;
				base::set_bad_if(!state::pop_level(json::level::array));
			}
			else if (ch == '{') {
				base::get();

				item = json::item::begin_object;
				base::set_bad_if(!state::push_level(json::level::object));
				trail_comma = false;
			}
			else {
				if (log_ptr_local != nullptr) {
					log_ptr_local->put_any(category::abc::json, severity::important, 0x10106, "json_istream::get_or_skip_token() ch=%c (\\u%4.4x)", ch, ch);
				}
				base::set_bad();
			}

			state::set_expect_property(true);
		}


		if (trail_comma && state::levels() > 0) {
			skip_spaces();

			ch = peek_char();
			if (ch == ',') {
				base::get();
			}
			else {
				if (state::expect_property()) {
					if (ch != '}') {
						if (log_ptr_local != nullptr) {
							log_ptr_local->put_any(category::abc::json, severity::important, 0x10107, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected='}' ", ch, ch);
						}

						base::set_bad();
					}
				}
				else {
					if (ch != ']') {
						if (log_ptr_local != nullptr) {
							log_ptr_local->put_any(category::abc::json, severity::important, 0x10108, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected=']' ", ch, ch);
						}

						base::set_bad();
					}
				}
			}
		}

		base::set_gcount(gcount);

		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10109, "json_istream::get_or_skip_token() ch=%c (\\u%4.4x) <<<", ch, ch);
		}

		if (buffer != nullptr) {
			buffer->item = item;
		}

		return item;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_istream<LogPtr, MaxLevels>::get_or_skip_string(char* buffer, std::size_t size) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x1010a, "json_istream::get_or_skip_string() >>>");
		}

		std::size_t gcount = 0;

		char ch = peek_char();
		if (ch == '"') {
			base::get();

			for (;;) {
				gcount += get_or_skip_string_content(buffer != nullptr ? buffer + gcount : nullptr, size - gcount);

				ch = peek_char();
				if (ch == '"') {
					base::get();
					break;
				}
				else if (ch == '\\') {
					char ech = get_escaped_char();
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
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x1010b, "json_istream::get_or_skip_string() string='%s' <<<", buffer);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_istream<LogPtr, MaxLevels>::get_or_skip_number(double* buffer) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x1010c, "json_istream::get_or_skip_number() >>>");
		}

		std::size_t gcount = 0;
		char digits[19 + 6 + 1];

		char ch = peek_char();
		if (ch == '+' || ch == '-') {
			digits[gcount++] = base::get();
		}

		gcount += get_digits(digits + gcount, sizeof(digits) - gcount);

		ch = peek_char();
		if (ch == '.') {
			digits[gcount++] = base::get();

			gcount += get_digits(digits + gcount, sizeof(digits) - gcount);
		}

		ch = peek_char();
		if (ch == 'e' || ch == 'E') {
			digits[gcount++] = base::get();

			ch = peek_char();
			if (ch == '+' || ch == '-') {
				digits[gcount++] = base::get();
			}

			gcount += get_digits(digits + gcount, sizeof(digits) - gcount);
		}

		digits[gcount] = '\0';

		double number = 0;
		if (buffer != nullptr) {
			number = std::atof(digits);
			*buffer = number;
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x1010d, "json_istream::get_or_skip_number() number=%lf (%s) <<<", number, digits);
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_istream<LogPtr, MaxLevels>::get_literal(const char* literal) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x1010e, "json_istream::get_literal() literal='%s' >>>", literal);
		}

		for (const char* it = literal; *it != '\0'; it++) {
			char ch = get_char();
			if (ch != *it) {
				if (log_ptr_local != nullptr) {
					log_ptr_local->put_any(category::abc::json, severity::important, 0x1010f, "json_istream::get_literal() ch='%c' (\\u%4.4x). Expected='%c' (\\u%4.4x)", ch, ch, *it, *it);
				}

				base::set_bad();
				break;
			}
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10110, "json_istream::get_literal() <<<");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline char json_istream<LogPtr, MaxLevels>::get_escaped_char() {
		LogPtr log_ptr_local = base::log_ptr();

		char ch = peek_char();

		if (ch == '\\') {
			base::get();

			ch = peek_char();
			if (ch == '"' || ch == '\\' || ch == '/') {
				base::get();
			}
			else if (ch == 'b') {
				base::get();
				ch = '\b';
			}
			else if (ch == 'f') {
				base::get();
				ch = '\f';
			}
			else if (ch == 'n') {
				base::get();
				ch = '\n';
			}
			else if (ch == 'r') {
				base::get();
				ch = '\r';
			}
			else if (ch == 't') {
				base::get();
				ch = '\t';
			}
			else if (ch == 'u') {
				base::get();

				char buffer[4 + 1];

				std::size_t gcount = get_hex(buffer, sizeof(buffer));

				if (gcount != 4) {
					if (log_ptr_local != nullptr) {
						log_ptr_local->put_any(category::abc::json, severity::important, 0x10111, "json_istream::get_escaped_char() gcount=%lu", gcount);
					}

					base::set_bad();
					ch = '\0';
				}
				else if (buffer[0] == '0' && buffer[1] == '0') {
					ch = (ascii::hex(buffer[2]) << 4) | ascii::hex(buffer[3]);
				}
				else {
					if (log_ptr_local != nullptr) {
						log_ptr_local->put_any(category::abc::json, severity::important, 0x10112, "json_istream::get_escaped_char() Wide chars not supported.");
					}

					base::set_bad();
					ch = '\0';
				}
			}
		}
		else {
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x10113, "json_istream::get_escaped_char() ch='%c' (\\u%4.4x). Unexpected.", ch, ch);
			}

			base::set_bad();
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

		while (base::is_good() && predicate(peek_char())) {
			if (gcount == size - 1) {
				base::set_fail();
				break;
			}

			buffer[gcount++] = base::get();
		}
		buffer[gcount] = '\0';

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	template <typename Predicate>
	inline std::size_t json_istream<LogPtr, MaxLevels>::skip_chars(Predicate&& predicate) {
		std::size_t gcount = 0;
		
		while (base::is_good() && predicate(peek_char())) {
			base::get();
			gcount++;
		}

		return gcount;
	}
	
	
	template <typename LogPtr, std::size_t MaxLevels>
	inline char json_istream<LogPtr, MaxLevels>::get_char() {
		char ch = peek_char();

		if (base::is_good()) {
			base::get();
		}

		return ch;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline char json_istream<LogPtr, MaxLevels>::peek_char() {
		char ch = base::peek();

		if (!ascii::json::is_valid(ch)) {
			base::set_bad();
			ch = '\0';
		}

		return ch;
	}


	// --------------------------------------------------------------


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_ostream<LogPtr, MaxLevels>::json_ostream(std::streambuf* sb, const LogPtr& log_ptr)
		: base(sb, log_ptr)
		, state(log_ptr) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10114, "json_ostream::json_ostream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_token(const json::token_t* buffer, std::size_t size) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10115, "json_ostream::put_token() item='%4.4x' >>>", buffer->item);
		}

		switch (buffer->item)
		{
		case json::item::null:
			put_null();
			break;

		case json::item::boolean:
			put_boolean(buffer->value.boolean);
			break;

		case json::item::number:
			put_number(buffer->value.number);
			break;

		case json::item::string:
			put_string(buffer->value.string, size);
			break;

		case json::item::property:
			put_property(buffer->value.property, size);
			break;

		case json::item::begin_array:
			put_begin_array();
			break;

		case json::item::end_array:
			put_end_array();
			break;

		case json::item::begin_object:
			put_begin_object();
			break;

		case json::item::end_object:
			put_end_object();
			break;

		default:
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x10116, "json_ostream::put_token() Unexpected item=%4.4x <<<", buffer->item);
			}

			base::set_bad();
			break;
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10117, "json_ostream::put_token() <<<");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_space() {
		put_chars(" ", 1);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_tab() {
		put_chars("\t", 1);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_cr() {
		put_chars("\r", 1);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_lf() {
		put_chars("\n", 1);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_null() {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x10118, "json_ostream::put_null() Expected a property.");
			}

			base::set_bad();
			return;
		}

		if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
			put_chars(",", 1);
		}

		put_chars("null", 4);

		_skip_comma = false;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_boolean(bool value) {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x10119, "json_ostream::put_boolean() Expected a property.");
			}

			base::set_bad();
			return;
		}

		if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
			put_chars(",", 1);
		}

		if (value) {
			put_chars("true", 4);
		}
		else {
			put_chars("false", 5);
		}

		_skip_comma = false;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_number(double value) {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x1011a, "json_ostream::put_number() Expected a property.");
			}

			base::set_bad();
			return;
		}

		if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
			put_chars(",", 1);
		}

		char literal[19 + 6 + 1];
		std::size_t size = std::snprintf(literal, sizeof(literal), "%.16lg", value);

		put_chars(literal, size);

		_skip_comma = false;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_string(const char* buffer, std::size_t size) {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x1011b, "json_ostream::put_string() Expected a property.");
			}

			base::set_bad();
			return;
		}

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
			put_chars(",", 1);
		}

		put_chars("\"", 1);
		put_chars(buffer, size);
		put_chars("\"", 1);

		_skip_comma = false;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_property(const char* buffer, std::size_t size) {
		if (!state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x1011c, "json_ostream::put_property() Expected a value.");
			}

			base::set_bad();
			return;
		}

		if (size == size::strlen) {
			size = std::strlen(buffer);
		}

		if (state::levels() > 0 && state::top_level() == json::level::object && !_skip_comma) {
			put_chars(",", 1);
		}

		put_chars("\"", 1);
		put_chars(buffer, size);
		put_chars("\":", 2);

		_skip_comma = true;
		state::set_expect_property(false);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_begin_array() {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x1011d, "json_ostream::put_begin_array() Expected a property.");
			}

			base::set_bad();
			return;
		}

		if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
			put_chars(",", 1);
		}

		put_chars("[", 1);

		base::set_bad_if(!state::push_level(json::level::array));

		_skip_comma = true;
		state::set_expect_property(false);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_end_array() {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x1011e, "json_ostream::put_end_array() Expected a property.");
			}

			base::set_bad();
			return;
		}

		put_chars("]", 1);

		base::set_bad_if(!state::pop_level(json::level::array));

		_skip_comma = false;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_begin_object() {
		if (state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x1011f, "json_ostream::put_begin_object() Expected a property.");
			}

			base::set_bad();
			return;
		}

		if (state::levels() > 0 && state::top_level() == json::level::array && !_skip_comma) {
			put_chars(",", 1);
		}

		put_chars("{", 1);

		base::set_bad_if(!state::push_level(json::level::object));

		_skip_comma = true;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_end_object() {
		if (!state::expect_property()) {
			LogPtr log_ptr_local = base::log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->put_any(category::abc::json, severity::important, 0x10120, "json_ostream::put_null() Expected a value.");
			}

			base::set_bad();
			return;
		}

		put_chars("}", 1);

		base::set_bad_if(!state::pop_level(json::level::object));

		_skip_comma = false;
		state::set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_ostream<LogPtr, MaxLevels>::put_chars(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = base::log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10121, "json_ostream::put_chars() buffer='%s' >>>", buffer);
		}

		std::size_t pcount = 0;

		while (base::is_good() && pcount < size) {
			base::put(buffer[pcount++]);
		}

		if (pcount < size) {
			base::set_fail();
		}

		base::flush();

		if (log_ptr_local != nullptr) {
			log_ptr_local->put_any(category::abc::json, severity::abc, 0x10122, "json_ostream::put_chars() pcount=%lu <<<", pcount);
		}

		return pcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_ostream<LogPtr, MaxLevels>::put_char(char ch) {
		if (base::is_good()) {
			base::put(ch);
		}

		return base::is_good() ? 1 : 0;
	}


	// --------------------------------------------------------------

}

