#pragma once

#include <cstdint>
#include <array>
#include <streambuf>

#include "legacy_base.h"


namespace abc {

	template <typename Char>
	class basic_arraybuf_adapter : public std::basic_streambuf<Char> {
	public:
		void reset(Char* array, std::size_t begin_pos, std::size_t end_pos) noexcept {
			reset(&array[begin_pos], &array[end_pos]);
		}

		void reset(Char* begin_ptr, Char* end_ptr) noexcept {
			std::basic_streambuf<Char>::setp(begin_ptr, end_ptr);
		}
	};


	template <typename Char, std::size_t Size>
	class basic_arraybuf : public basic_arraybuf_adapter<Char> {
	public:
		basic_arraybuf() noexcept {
			_array.fill(0);
			basic_arraybuf_adapter<Char>::reset(_array.data(), 0, Size - 1);
		}

		const Char* c_str() const noexcept {
			return _array.data();
		}

	private:
		std::array<Char, Size> _array;
	};


	template <std::size_t Size>
	using arraybuf = basic_arraybuf<char, Size>;

	template <std::size_t Size>
	using warraybuf = basic_arraybuf<wchar_t, Size>;

}
