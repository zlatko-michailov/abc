#pragma once

#include <streambuf>


namespace abc {

	template <typename Char>
	class basic_streambuf_adapter : public std::basic_streambuf<Char> {
	public:
		basic_streambuf_adapter(Char* array, std::size_t begin_pos, std::size_t end_pos) noexcept;
		basic_streambuf_adapter(Char* begin_ptr, Char* end_ptr) noexcept;
	};


	using streambuf_adapter = basic_streambuf_adapter<char>;
	using wstreambuf_adapter = basic_streambuf_adapter<wchar_t>;


	// --------------------------------------------------------------


	template <typename Char>
	inline basic_streambuf_adapter<Char>::basic_streambuf_adapter(Char* array, std::size_t begin_pos, std::size_t end_pos) noexcept
		: basic_streambuf_adapter<Char>(&array[begin_pos], &array[end_pos]) {
	}


	template <typename Char>
	inline basic_streambuf_adapter<Char>::basic_streambuf_adapter(Char* begin_ptr, Char* end_ptr) noexcept
		: std::basic_streambuf<Char>() {
		std::basic_streambuf<Char>::setp(begin_ptr, end_ptr);
	}

}

