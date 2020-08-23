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

#include <cstdint>
#include <fstream>
#include <chrono>
#include <ios>

#include "log.i.h"


namespace abc {

	template <typename Log = null_log, std::size_t MaxPath = size::k2, typename Clock = std::chrono::system_clock>
	class multifile_streambuf : public std::filebuf {
		using base = std::filebuf;

		static constexpr const char*	filename_format = "%4.4u%2.2u%2.2u_%2.2u%2.2u%2.2u";
		static constexpr std::size_t	filename_length = 15;
		static constexpr char			path_separator  = '/';

	public:
		multifile_streambuf(const char* path, std::ios_base::openmode mode = std::ios_base::out, Log* log = nullptr);
		multifile_streambuf(multifile_streambuf&& other) noexcept;

	public:
		void reopen();

#ifdef REMOVE ////
	protected:
		virtual int_type	underflow() override;
		virtual int_type	overflow(int_type ch) override;
		virtual int			sync() override;
#endif ////

	private:
		char					_path[MaxPath + 1];
		std::size_t				_path_length;
		std::ios_base::openmode	_mode;
		Log*					_log;
	};


	// --------------------------------------------------------------

}
