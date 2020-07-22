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

#include <streambuf>
#include <istream>
#include <ostream>
#include <bitset>

#include "log.i.h"


namespace abc {

	template <typename LogPtr, std::size_t MaxLevels>
	class json_istream;

	template <typename LogPtr, std::size_t MaxLevels>
	class json_ostream;


	namespace json {
		using item_t = std::uint16_t;

		namespace item {
			constexpr item_t none			= 0x0000;

			constexpr item_t null			= 0x0001;
			constexpr item_t boolean		= 0x0002;
			constexpr item_t number			= 0x0004;
			constexpr item_t string			= 0x0008;
			constexpr item_t begin_array	= 0x0010;
			constexpr item_t end_array		= 0x0020;
			constexpr item_t begin_object	= 0x0040;
			constexpr item_t end_object		= 0x0080;
			constexpr item_t property		= 0x0100;
		}


		union value_t {
			bool	boolean;
			double	number;
			char	string[1];
			char	property[1];
		};
		
		struct token_t {
			item_t	item;
			value_t	value;
		};


		using level_t = bool;

		namespace level {
			constexpr level_t	array	= false;
			constexpr level_t	object	= true;
		}
	}

	// --------------------------------------------------------------


	template <typename StdStream, typename LogPtr, std::size_t MaxLevels>
	class _json_stream : protected StdStream {
	protected:
		_json_stream(std::streambuf* sb, const LogPtr& log_ptr);
		_json_stream(_json_stream&& other) = default;

	public:
		std::size_t			levels() const noexcept;
		json::level_t		top_level() const noexcept;

		bool				eof() const;
		bool				good() const;
		bool				bad() const;
		bool				fail() const;
		bool				operator!() const;
							operator bool() const;

	protected:
		std::size_t			gcount() const noexcept;
		void				reset();
		bool				expect_property() const noexcept;
		void				set_expect_property(bool expect) noexcept;
		void				push_level(json::level_t level) noexcept;
		void				pop_level(json::level_t level) noexcept;
		void				set_gcount(std::size_t gcount) noexcept;
		bool				is_good() const;
		void				set_bad();
		void				set_fail();
		const LogPtr&		log_ptr() const noexcept;

	private:
		bool					_expect_property;
		std::size_t				_level_top;
		std::bitset<MaxLevels>	_level_stack;
		std::size_t				_gcount;
		LogPtr					_log_ptr;
	};


	template <typename LogPtr, std::size_t MaxLevels = 64>
	class json_istream : public _json_stream<std::istream, LogPtr, MaxLevels> {
	public:
		json_istream(std::streambuf* sb, const LogPtr& log_ptr);
		json_istream(json_istream&& other) = default;

	public:
		std::size_t			gcount() const noexcept;

	public:
		void				get_token(json::token_t* buffer, std::size_t size);
		json::item_t		skip_value();

	protected:
		json::item_t		get_or_skip_token(json::token_t* buffer, std::size_t size);
		void				get_or_skip_number(double* buffer);
		std::size_t			get_or_skip_string(char* buffer, std::size_t size);

	protected:
		void				get_literal(const char* literal);
		char				get_escaped_char();
		std::size_t			get_or_skip_string_content(char* buffer, std::size_t size);
		std::size_t			get_hex(char* buffer, std::size_t size);
		std::size_t			get_digits(char* buffer, std::size_t size);
		std::size_t			skip_spaces();

		template <typename Predicate>
		std::size_t			get_chars(Predicate&& predicate, char* buffer, std::size_t size);
		template <typename Predicate>
		std::size_t			skip_chars(Predicate&& predicate);
		char				get_char();
		char				peek_char();
	};


	template <typename LogPtr, std::size_t MaxLevels = 64>
	class json_ostream : public _json_stream<std::ostream, LogPtr, MaxLevels> {
	public:
		json_ostream(std::streambuf* sb, const LogPtr& log_ptr);
		json_ostream(json_ostream&& other) = default;

	public:
		void				put_token(const json::token_t* buffer, std::size_t size = size::strlen);

		void				put_space();
		void				put_tab();
		void				put_cr();
		void				put_lf();

		void				put_null();
		void				put_boolean(bool value);
		void				put_number(double value);
		void				put_string(const char* buffer, std::size_t size = size::strlen);
		void				put_property(const char* buffer, std::size_t size = size::strlen);
		void				put_begin_array();
		void				put_end_array();
		void				put_begin_object();
		void				put_end_object();

	protected:
		std::size_t			put_chars(const char* buffer, std::size_t size);
		std::size_t			put_char(char ch);

	private:
		bool				_skip_comma;
	};


	// --------------------------------------------------------------

}

