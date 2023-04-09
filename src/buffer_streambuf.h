/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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

	/**
	 * @brief					`std::streambuf` implementation over a char buffer.
	 * @details					The buffer is neither contained nor kept alive by this instance.
	 * @tparam Char				Char type, e.g. `char` or `wchar_t`.
	 */
	template <typename Char>
	class basic_buffer_streambuf : public std::basic_streambuf<Char> {
		using base = std::basic_streambuf<Char>;

	public:
		/**
		 * @brief				Constructor, using positions.
		 * @param get_buffer	"Get" buffer.
		 * @param get_begin_pos	"Begin" position on the "get" buffer.
		 * @param get_end_pos	"End" position on the "get" buffer.
		 * @param put_buffer	"Put" buffer.
		 * @param put_begin_pos	"Begin" position on the "put" buffer.
		 * @param put_end_pos	"Enc" position on the "put" buffer.
		 */
		basic_buffer_streambuf(Char* get_buffer, std::size_t get_begin_pos, std::size_t get_end_pos, Char* put_buffer, std::size_t put_begin_pos, std::size_t put_end_pos) noexcept;

		/**
		 * @brief				Constructor, using pointers.
		 * @param get_begin_ptr	Pointer to the "begin" item on the "get" buffer.
		 * @param get_end_ptr	Pointer to the "end" item on the "get" buffer.
		 * @param put_begin_ptr	Pointer to the "begin" item on the "put" buffer.
		 * @param put_end_ptr	Pointer to the "end" item on the "put" buffer.
		 */
		basic_buffer_streambuf(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept;

		/**
		 * @brief				Move constructor.
		 */
		basic_buffer_streambuf(basic_buffer_streambuf<Char>&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		basic_buffer_streambuf(const basic_buffer_streambuf<Char>& other) = delete;

	private:
		void reset(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept;

	private:
		Char* _get_begin_ptr;
		Char* _get_end_ptr;
		Char* _put_begin_ptr;
		Char* _put_end_ptr;
	};


	/**
	 * @brief					`std::streambuf` implementation over a `char` buffer.
	 */
	using buffer_streambuf = basic_buffer_streambuf<char>;

	/**
	 * @brief					`std::streambuf` implementation over a `wchar_t` buffer.
	 */
	using wbuffer_streambuf = basic_buffer_streambuf<wchar_t>;


	// --------------------------------------------------------------


	template <typename Char>
	inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_buffer, std::size_t get_begin_pos, std::size_t get_end_pos, Char* put_buffer, std::size_t put_begin_pos, std::size_t put_end_pos) noexcept
		: basic_buffer_streambuf<Char>(&get_buffer[get_begin_pos], &get_buffer[get_end_pos], &put_buffer[put_begin_pos], &put_buffer[put_end_pos]) {
	}


	template <typename Char>
	inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept
		: std::basic_streambuf<Char>() {
		reset(get_begin_ptr, get_end_ptr, put_begin_ptr, put_end_ptr);
	}


	template <typename Char>
	inline basic_buffer_streambuf<Char>::basic_buffer_streambuf(basic_buffer_streambuf<Char>&& other) noexcept
		: basic_buffer_streambuf<Char>(other._get_begin_ptr, other._get_end_ptr, other._put_begin_ptr, other._put_end_ptr) {
	}


	template <typename Char>
	inline void basic_buffer_streambuf<Char>::reset(Char* get_begin_ptr, Char* get_end_ptr, Char* put_begin_ptr, Char* put_end_ptr) noexcept {
		_put_begin_ptr = put_begin_ptr;
		_put_end_ptr   = put_end_ptr;
		_get_begin_ptr = get_begin_ptr;
		_get_end_ptr   = get_end_ptr;

		base::setg(get_begin_ptr, get_begin_ptr, get_end_ptr);
		base::setp(put_begin_ptr, put_end_ptr);
	}

}

