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

#include <cstdarg>
#include <cstring>

#include "table.i.h"


namespace abc {

	inline table_ostream::table_ostream(std::streambuf* sb)
		: base(sb) {
	}


	inline void table_ostream::put_line(const char* line, std::size_t line_size) {
		if (line_size == size::strlen) {
			line_size = std::strlen(line);
		}

		base::write(line, line_size);
	}


	inline void table_ostream::put_blank_line() {
		*this << endl;
	}


	// --------------------------------------------------------------


	template <std::size_t Size>
	inline line_ostream<Size>::line_ostream()
		: line_ostream(nullptr) {
	}


	template <std::size_t Size>
	inline line_ostream<Size>::line_ostream(table_ostream* table)
		: base(nullptr)
		, _table(table)
		, _pcount(0) {
	}


	template <std::size_t Size>
	inline line_ostream<Size>::~line_ostream() {
		flush();
	}


	template <std::size_t Size>
	inline const char* line_ostream<Size>::get() {
		if (0 <= _pcount && _pcount <= Size) {
			_buffer[_pcount] = ends;
		}

		return _buffer;
	}


	template <std::size_t Size>
	inline void line_ostream<Size>::flush() {
		if (0 <= _pcount && _pcount <= Size) {
			_buffer[_pcount++] = endl;
			_buffer[_pcount] = ends;
		}

		if (_table != nullptr) {
			_table->put_line(_buffer, _pcount);
		}

		_pcount = 0;
	}


	template <std::size_t Size>
	inline void line_ostream<Size>::put_any(const char* format, ...) {
		va_list vlist;
		va_start(vlist, format);

		put_anyv(format, vlist);

		va_end(vlist);
	}


	template <std::size_t Size>
	inline void line_ostream<Size>::put_anyv(const char* format, va_list vlist) {
		int pc = std::vsnprintf(_buffer + _pcount, Size - _pcount, format, vlist);
		_pcount += pc;
	}


	template <std::size_t Size>
	template <typename Clock>
	inline void line_ostream<Size>::put_timestamp(const timestamp<Clock>& ts, const char* format) {
		put_any(format, ts.year(), ts.month(), ts.day(), ts.hours(), ts.minutes(), ts.seconds(), ts.milliseconds());
	}


	template <std::size_t Size>
	inline void line_ostream<Size>::put_thread_id(std::thread::id thread_id, const char* format) {
		char buf[17];
		buffer_streambuf sb(nullptr, 0, 0, buf, 0, sizeof(buf) - 1);
		std::ostream stream(&sb);
		stream << std::hex << thread_id << ends;

		put_any(format, buf);
	}


	template <std::size_t Size>
	inline std::size_t line_ostream<Size>::put_binary(const void* buffer, std::size_t buffer_size, std::size_t& buffer_offset) {
		// 0000: 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f  abcdefghijklmnop
		constexpr std::size_t half_chunk_size = 8;
		constexpr std::size_t chunk_size = half_chunk_size * 2;
		constexpr std::size_t local_size = 5 + (chunk_size * 3) + 1 + 2 + chunk_size + 1;
		constexpr char hex[] = "0123456789abcdef";
		constexpr char blank = ' ';
		constexpr char nonprint = '.';
		constexpr char head = ':';

		if (Size - _pcount < local_size) {
			return 0;
		}

		if (buffer_size <= buffer_offset) {
			return 0;
		}

		if ((buffer_offset % chunk_size) != 0) {
			return 0;
		}

		const std::uint8_t* chunk = static_cast<const std::uint8_t*>(buffer) + buffer_offset;
		std::size_t local_offset = 0;
		char* line = _buffer + _pcount;

		// 0000:
		line[local_offset++] = hex[(buffer_offset >> 12) & 0xf];
		line[local_offset++] = hex[(buffer_offset >>  8) & 0xf];
		line[local_offset++] = hex[(buffer_offset >>  4) & 0xf];
		line[local_offset++] = hex[buffer_offset & 0xf];
		line[local_offset++] = head;

		// 00 01 02 03 04 05 06 07  08 09 0a 0b 0c 0d 0e 0f
		for (int half = 0; half < 2; half++) {
			line[local_offset++] = blank;

			for (std::size_t chunk_offset = 0; chunk_offset < half_chunk_size; chunk_offset++) {
				std::size_t source_offset = half * half_chunk_size + chunk_offset;

				if (buffer_offset + source_offset < buffer_size) {
					line[local_offset++] = hex[(chunk[source_offset] >> 4) & 0xf];
					line[local_offset++] = hex[chunk[source_offset] & 0xf];
				}
				else {
					line[local_offset++] = blank;
					line[local_offset++] = blank;
				}
	
				line[local_offset++] = blank;
			}
		}

		line[local_offset++] = blank;

		//  abcdefghijklmnop
		for (std::size_t chunk_offset = 0; chunk_offset < chunk_size; chunk_offset++) {
			if (buffer_offset + chunk_offset < buffer_size) {
				if (std::isprint(chunk[chunk_offset])) {
					line[local_offset++] = chunk[chunk_offset];
				}
				else {
					line[local_offset++] = nonprint;
				}
			}
			else {
				line[local_offset++] = blank;
			}
		}

		line[local_offset++] = ends;

		_pcount += local_size;
		buffer_offset += chunk_size;
		return static_cast<int>(chunk_size);
	}

}
