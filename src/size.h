/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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

#include <cstddef>
#include <cstdint>


namespace abc {

	namespace size {
		constexpr std::size_t invalid = SIZE_MAX;
		constexpr std::size_t strlen = invalid;

		constexpr std::size_t _16  =  16;
		constexpr std::size_t _32  =  32;
		constexpr std::size_t _64  =  64;
		constexpr std::size_t _128 = 128;
		constexpr std::size_t _256 = 256;
		constexpr std::size_t _512 = 512;

		constexpr std::size_t k1   = 1024;
		constexpr std::size_t k2   =  2 * k1;
		constexpr std::size_t k4   =  4 * k1;
		constexpr std::size_t k8   =  8 * k1;
		constexpr std::size_t k16  = 16 * k1;
		constexpr std::size_t k32  = 32 * k1;
		constexpr std::size_t k64  = 64 * k1;
	}

}
