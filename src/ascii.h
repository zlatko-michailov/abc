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

#include "size.h"


namespace abc {
	namespace ascii {

		inline bool is_between(char ch, char low, char high) noexcept {
			return low <= ch && ch <= high;
		}


		inline bool is_ascii(char ch) noexcept {
			return is_between(ch, 0x00, 0x7f);
		}


		inline bool is_digit(char ch) noexcept {
			return is_between(ch, '0', '9');
		}


		inline bool is_hex(char ch) noexcept {
			return is_digit(ch) || is_between(ch, 'A', 'F') || is_between(ch, 'a', 'f');
		}


		inline bool is_upperalpha(char ch) noexcept {
			return is_between(ch, 'A', 'Z');
		}


		inline bool is_loweralpha(char ch) noexcept {
			return is_between(ch, 'a', 'z');
		}


		inline bool is_alpha(char ch) noexcept {
			return is_upperalpha(ch) || is_loweralpha(ch);
		}


		inline char to_upper(char ch) noexcept {
			return is_loweralpha(ch) ? 'A' + (ch - 'a') : ch;
		}


		inline char to_lower(char ch) noexcept {
			return is_upperalpha(ch) ? 'a' + (ch - 'A') : ch;
		}


		inline bool is_space(char ch) noexcept {
			return ch == ' ' || ch == '\t';
		}


		inline bool is_control(char ch) noexcept {
			return is_between(ch, 0x00, 0x1f) || ch == 0x7f;
		}


		inline bool is_stdprint(char ch) noexcept {
			return is_between(ch, 0x20, 0x7e);
		}


		inline bool is_abcprint(char ch) noexcept {
			return is_between(ch, 0x21, 0x7e);
		}


		inline bool is_abcprint_or_space(char ch) noexcept {
			return is_abcprint(ch) || is_space(ch);
		}


		inline std::uint8_t hex(char ch) noexcept {
			if (is_digit(ch)) {
				return ch - '0';
			}
			else if (is_between(ch, 'A', 'F')) {
				return 10 + (ch - 'A');
			}
			else if (is_between(ch, 'a', 'f')) {
				return 10 + (ch - 'a');
			}

			return 0;
		}


		inline bool are_equal(const char* s1, const char* s2, bool case_sensitive, std::size_t max_chars) {
			if (s1 == nullptr) {
				return s2 == nullptr;
			}
			
			if (s2 == nullptr) {
				return false;
			}

			for (std::size_t i = 0; i < max_chars; i++) {
				const char& ch1 = s1[i];
				const char& ch2 = s2[i];

				bool are_ch_equal = case_sensitive ? ch1 == ch2 : to_lower(ch1) == to_lower(ch2);

				if (!are_ch_equal) {
					return false;
				}

				if (ch1 == '\0' && ch2 == '\0') {
					return true;
				}
			}

			return true;
		}


		inline bool are_equal(const char* s1, const char* s2) {
			return are_equal(s1, s2, true, size::strlen);
		}


		inline bool are_equal_i(const char* s1, const char* s2) {
			return are_equal(s1, s2, false, size::strlen);
		}


		inline bool are_equal_n(const char* s1, const char* s2, std::size_t max_chars) {
			return are_equal(s1, s2, true, max_chars);
		}


		inline bool are_equal_i_n(const char* s1, const char* s2, std::size_t max_chars) {
			return are_equal(s1, s2, false, max_chars);
		}


		namespace http {
			inline bool is_separator(char ch) noexcept {
				return
					is_space(ch) ||
					ch == '(' || ch == ')' || ch == '<' || ch == '>' || ch == '[' || ch == ']' ||  ch == '{' || ch == '}' ||
					ch == '@' || ch == ',' || ch == ';' || ch == ':' || ch == '\\' || ch == '/' || ch == '"' || ch == '?' || ch == '=';
			}


			inline bool is_token(char ch) noexcept {
				return is_abcprint(ch) && !is_separator(ch);
			}
		}


		namespace json {
			inline bool is_valid(char /*ch*/) noexcept {
				return true;
			}


			inline bool is_space(char ch) noexcept {
				return ascii::is_space(ch) || ch == '\r' || ch == '\n';
			}


			inline bool is_string_content(char ch) noexcept {
				return is_valid(ch) && ch != '"' && ch != '\\';
			}
		}
	}
}

