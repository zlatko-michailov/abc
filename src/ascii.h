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
	}
}

