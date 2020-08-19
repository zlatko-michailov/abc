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
#include <ostream>
#include <thread>

#include "stream.i.h"
#include "buffer_streambuf.h"
#include "timestamp.i.h"


namespace abc {

	namespace size {
		constexpr std::size_t strlen = -1;

		constexpr std::size_t k1 = 1024;
		constexpr std::size_t k2 = 2 * k1;
		constexpr std::size_t k4 = 4 * k1;
		constexpr std::size_t k8 = 8 * k1;
	}


	// --------------------------------------------------------------


	class table_ostream : public _ostream {
		using base = _ostream;

	public:
		static constexpr char endl = '\n';

	public:
		table_ostream(std::streambuf* sb);
		table_ostream(table_ostream&& other) = default;

	public:
		void put_line(const char* line, std::size_t line_size = size::strlen) noexcept;
		void put_blank_line() noexcept;
	};


	// --------------------------------------------------------------


	template <std::size_t Size = size::k2>
	class line_ostream : public _ostream {
		using base = _ostream;

	public:
		static constexpr char endl = '\n';
		static constexpr char ends = '\0';

	public:
		line_ostream();
		line_ostream(table_ostream* table);
		line_ostream(line_ostream&& other) = default;
		~line_ostream() noexcept;

	public:
		const char* get() noexcept;
		void		flush() noexcept;

		void 		put_any(const char* format, ...) noexcept;
		void 		put_anyv(const char* format, va_list vlist) noexcept;

		template <typename Clock>
		void		put_timestamp(const timestamp<Clock>& ts, const char* format) noexcept;

		void		put_thread_id(std::thread::id thread_id, const char* format = "%s") noexcept;
		bool		put_binary(const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) noexcept;

	private:
		table_ostream*		_table;
		char				_buffer[Size + 2];
		buffer_streambuf	_sb;
		std::size_t			_pcount;
	};

}
