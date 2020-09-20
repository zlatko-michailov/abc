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


namespace abc {

	template <typename Stream>
	class _stream : protected Stream {
		using base = Stream;

	protected:
		_stream(std::streambuf* sb);
		_stream(_stream&& other) = default;

	public:
		std::streambuf*	rdbuf() const;

		bool			eof() const;
		bool			good() const;
		bool			bad() const;
		bool			fail() const;
		bool			operator!() const;
						operator bool() const;

	protected:
		void			reset();
		bool			is_good() const;
		void			set_bad();
		void			set_bad_if(bool condition);
		void			set_fail();
		void			set_fail_if(bool condition);
	};


	// --------------------------------------------------------------


	class _istream : public _stream<std::istream> {
		using base = _stream<std::istream>;

	protected:
		_istream(std::streambuf* sb);
		_istream(_istream&& other) = default;

	public:
		std::size_t		gcount() const noexcept;

	protected:
		void			reset();
		void			set_gcount(std::size_t gcount) noexcept;
	private:
		std::size_t		_gcount;
	};


	// --------------------------------------------------------------


	class _ostream : public _stream<std::ostream> {
		using base = _stream<std::ostream>;

	protected:
		_ostream(std::streambuf* sb);
		_ostream(_ostream&& other) = default;

	public:
		void			flush();
	};

}

