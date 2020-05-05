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


namespace abc {

	template <typename Char>
	class basic_buffer_streambuf : public std::basic_streambuf<Char> {
	public:
		basic_buffer_streambuf(Char* get_buffer, std::size_t get_begin_pos, std::size_t get_end_pos, Char* put_buffer, std::size_t put_begin_pos, std::size_t put_end_pos) noexcept;
		basic_buffer_streambuf(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept;
	};


	using buffer_streambuf = basic_buffer_streambuf<char>;
	using wbuffer_streambuf = basic_buffer_streambuf<wchar_t>;


	// --------------------------------------------------------------


	template <typename Char>
	inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_buffer, std::size_t get_begin_pos, std::size_t get_end_pos, Char* put_buffer, std::size_t put_begin_pos, std::size_t put_end_pos) noexcept
		: basic_buffer_streambuf<Char>(&get_buffer[get_begin_pos], &get_buffer[get_end_pos], &put_buffer[put_begin_pos], &put_buffer[put_end_pos]) {
	}


	template <typename Char>
	inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept
		: std::basic_streambuf<Char>() {
		std::basic_streambuf<Char>::setg(get_begin_ptr, get_begin_ptr, get_end_ptr);
		std::basic_streambuf<Char>::setp(put_begin_ptr, put_end_ptr);
	}

}

