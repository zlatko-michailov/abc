/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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

#include <string>

#include "../i/vmem.i.h"


namespace abc {

    template <typename Char, typename Pool, typename Log>
	inline vmem_basic_string_streambuf<Char, Pool, Log>::vmem_basic_string_streambuf(String* string, Log* log)
		: base()
		, _string(string)
		, _log(log)
		, _get_itr(nullptr)
		, _get_ch(0)
		, _put_ch(0) {
		if (string == nullptr) {
			throw exception<std::logic_error, Log>("vmem_basic_string_streambuf::vmem_basic_string_streambuf(string)", 0x107b3, _log);
		}

		base::setg(&_get_ch, &_get_ch + 1, &_get_ch + 1);
		base::setp(&_put_ch, &_put_ch + 1);
	}


    template <typename Char, typename Pool, typename Log>
	inline vmem_basic_string_streambuf<Char, Pool, Log>::vmem_basic_string_streambuf(vmem_basic_string_streambuf&& other) noexcept
		: base()
		, _string(other._string)
		, _log(other._log)
		, _get_itr(std::move(other._get_itr))
		, _get_ch(other._get_ch)
		, _put_ch(other._put_ch) {
		base::setg(&_get_ch, &_get_ch + 1, &_get_ch + 1);
		base::setp(&_put_ch, &_put_ch + 1);

		other._string = nullptr;
		other._log = nullptr;
		other._get_itr = nullptr;
		other.setg(nullptr, nullptr, nullptr);
		other.setp(nullptr, nullptr);
	}


    template <typename Char, typename Pool, typename Log>
	inline typename std::basic_streambuf<Char>::int_type vmem_basic_string_streambuf<Char, Pool, Log>::underflow() {
		if (_get_itr == nullptr) {
			_get_itr = _string->begin();
		}

		if (!_get_itr.can_deref())
		{
			return std::char_traits<Char>::eof();
		}

		_get_ch = *_get_itr++;

		base::setg(&_get_ch, &_get_ch, &_get_ch + 1);

		return _get_ch;
	}

    template <typename Char, typename Pool, typename Log>
	inline typename std::basic_streambuf<Char>::int_type vmem_basic_string_streambuf<Char, Pool, Log>::overflow(typename base::int_type ch) {
		_string->push_back(_put_ch);
		_string->push_back(ch);

		base::setp(&_put_ch, &_put_ch + 1);

		return ch;
	}

    template <typename Char, typename Pool, typename Log>
	inline int vmem_basic_string_streambuf<Char, Pool, Log>::sync() {
		if (base::pptr() != &_put_ch) {
			_string->push_back(_put_ch);
		}

		base::setp(&_put_ch, &_put_ch + 1);

		return 0;
	}

}
