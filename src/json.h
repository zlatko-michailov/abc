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
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_istream::skip_value() <<< item=%4.4x", item);
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
							log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected=':' ", ch, ch);
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
							log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected='}' ", ch, ch);
						}

						this->set_bad();
					}
				}
				else {
					if (ch != ']') {
						if (log_ptr_local != nullptr) {
							log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_or_skip_token() ch='%c' (\\u4.4x). Expected=']' ", ch, ch);
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
						log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_escaped_char() gcount=%lu", gcount);
					}

					this->set_bad();
					ch = '\0';
				}
				else if (buffer[0] == '0' && buffer[1] == '0') {
					ch = (ascii::hex(buffer[2]) << 4) | ascii::hex(buffer[3]);
				}
				else {
					if (log_ptr_local != nullptr) {
						log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_istream::get_escaped_char() Wide chars not supported.");
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


	template <typename LogPtr, std::size_t MaxLevels>
	inline json_ostream<LogPtr, MaxLevels>::json_ostream(std::streambuf* sb, const LogPtr& log_ptr)
		: _json_stream<std::ostream, LogPtr, MaxLevels>(sb, log_ptr) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_ostream::json_ostream()");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_token(const json::token_t* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_ostream::put_token() item='%4.4x' >>>", buffer->item);
		}

		switch (buffer->item)
		{
		case json::item::null:
			this->put_null();
			break;

		case json::item::boolean:
			this->put_boolean(buffer->value.boolean);
			this->set_gcount(sizeof(bool));
			break;

		case json::item::number:
			this->put_number(buffer->value.number);
			this->set_gcount(sizeof(double));
			break;

		case json::item::string:
			this->put_string(buffer->value.string, size);
			break;

		case json::item::property:
			this->put_property(buffer->value.property, size);
			break;

		case json::item::begin_array:
			this->put_begin_array();
			break;

		case json::item::end_array:
			this->put_end_array();
			break;

		case json::item::begin_object:
			this->put_begin_object();
			break;

		case json::item::end_object:
			this->put_end_object();
			break;

		default:
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_token() Unexpected item=%4.4x <<<", buffer->item);
			}

			this->set_bad();
			break;
		}

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_ostream::put_token() <<<");
		}
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_space() {
		this->put_chars(" ", 1);
		this->set_gcount(0);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_tab() {
		this->put_chars("\t", 1);
		this->set_gcount(0);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_cr() {
		this->put_chars("\r", 1);
		this->set_gcount(0);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_lf() {
		this->put_chars("\n", 1);
		this->set_gcount(0);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_null() {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_null() Expected a property.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("null", 4);
		this->set_gcount(0);

		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_boolean(bool value) {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_boolean() Expected a property.");
			}

			this->set_bad();
			return;
		}

		if (value) {
			this->put_chars("true", 4);
		}
		else {
			this->put_chars("false", 5);
		}

		this->set_gcount(0);

		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_number(double value) {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_number() Expected a property.");
			}

			this->set_bad();
			return;
		}

		char literal[19 + 6 + 1];
		std::size_t size = std::snprintf(literal, sizeof(literal), "%.16lg", value);

		this->put_chars(literal, size);
		this->set_gcount(0);

		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_string(const char* buffer, std::size_t size) {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_string() Expected a property.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("\"", 1);
		std::size_t gcount = this->put_chars(buffer, size);
		this->put_chars("\"", 1);

		this->set_gcount(gcount);

		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_property(const char* buffer, std::size_t size) {
		if (!this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_property() Expected a value.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("\"", 1);
		std::size_t gcount = this->put_chars(buffer, size);
		this->put_chars("\"", 1);

		this->set_gcount(gcount);

		this->set_expect_property(false);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_begin_array() {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_begin_array() Expected a property.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("[", 1);

		this->set_gcount(0);

		this->push_level(json::level::array);
		this->set_expect_property(false);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_end_array() {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_end_array() Expected a property.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("]", 1);

		this->set_gcount(0);

		this->pop_level(json::level::array);
		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_begin_object() {
		if (this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_begin_object() Expected a property.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("{", 1);

		this->set_gcount(0);

		this->push_level(json::level::object);
		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline void json_ostream<LogPtr, MaxLevels>::put_end_object() {
		if (!this->expect_property()) {
			LogPtr log_ptr_local = this->log_ptr();
			if (log_ptr_local != nullptr) {
				log_ptr_local->push_back(category::abc::json, severity::important, __TAG__, "json_ostream::put_null() Expected a value.");
			}

			this->set_bad();
			return;
		}

		this->put_chars("}", 1);

		this->set_gcount(0);

		this->pop_level(json::level::object);
		this->set_expect_property(true);
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_ostream<LogPtr, MaxLevels>::put_chars(const char* buffer, std::size_t size) {
		LogPtr log_ptr_local = this->log_ptr();
		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_ostream::put_chars() buffer='%s' >>>", buffer);
		}

		std::size_t gcount = 0;

		while (this->is_good() && gcount < size) {
			this->put(buffer[gcount++]);
		}

		if (gcount < size) {
			this->set_fail();
		}

		this->flush();

		if (log_ptr_local != nullptr) {
			log_ptr_local->push_back(category::abc::json, severity::abc, __TAG__, "json_ostream::put_chars() gcount=%lu <<<", gcount);
		}

		return gcount;
	}


	template <typename LogPtr, std::size_t MaxLevels>
	inline std::size_t json_ostream<LogPtr, MaxLevels>::put_char(char ch) {
		if (this->is_good()) {
			this->put(ch);
		}

		return this->is_good() ? 1 : 0;
	}


	// --------------------------------------------------------------

}

