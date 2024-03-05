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

#include "vmem_list.i.h"
#include "log.i.h"


namespace abc {

	/**
	 * @brief					Virtually contiguous generic string.
	 * @tparam Char			    Character.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
    template <typename Char, typename Pool, typename Log = null_log>
    using vmem_basic_string = vmem_list<Char, Pool, Log>;


	/**
	 * @brief					Virtually contiguous `char` string.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
    template <typename Pool, typename Log = null_log>
    using vmem_string = vmem_basic_string<char, Pool, Log>;


	// --------------------------------------------------------------


	/**
	 * @brief					Generic string iterator.
	 * @tparam Char			    Character.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Char, typename Pool, typename Log = null_log>
	using vmem_basic_string_iterator = vmem_list_iterator<Char, Pool, Log>;


	/**
	 * @brief					Generic string const iterator.
	 * @tparam Char			    Character.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Char, typename Pool, typename Log = null_log>
	using vmem_basic_string_const_iterator = vmem_list_const_iterator<Char, Pool, Log>;


	/**
	 * @brief					String iterator.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Pool, typename Log = null_log>
	using vmem_string_iterator = vmem_basic_string_iterator<char, Pool, Log>;


	/**
	 * @brief					String const iterator.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Pool, typename Log = null_log>
	using vmem_string_const_iterator = vmem_basic_string_const_iterator<char, Pool, Log>;


	// --------------------------------------------------------------


	/**
	 * @brief					`std::streambuf` specialization that is backed by a generic string.
	 * @tparam Char			    Character.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
    template <typename Char, typename Pool, typename Log = null_log>
	class vmem_basic_string_streambuf : public std::basic_streambuf<Char> {
		using base = std::basic_streambuf<Char>;
        using String = vmem_basic_string<Char, Pool, Log>;
        using Iterator = vmem_basic_string_iterator<Char, Pool, Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param string		Pointer to a `String` instance.
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		vmem_basic_string_streambuf(String* string, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		vmem_basic_string_streambuf(vmem_basic_string_streambuf&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		vmem_basic_string_streambuf(const vmem_basic_string_streambuf& other) = delete;

	protected:
		/**
		 * @brief				Handler that reads a char from the string.
		 * @return				The char received.
		 */
		virtual typename std::basic_streambuf<Char>::int_type underflow() override;

		/**
		 * @brief				Handler that appends a char to the string.
		 * @param ch			Char to be sent.
		 * @return				`ch`
		 */
		virtual typename std::basic_streambuf<Char>::int_type overflow(typename base::int_type ch) override;

		/**
		 * @brief				Flushes.
		 * @return				`0`
		 */
		virtual int sync() override;

	private:
		/**
		 * @brief				The String pointer passed in to the constructor.
		 */
		String* _string;

		/**
		 * @brief				The Log pointer passed in to the constructor.
		 */
		Log* _log;

		/**
		 * @brief				'get' iterator.
		 */
		Iterator _get_itr;

		/**
		 * @brief				Cached 'get' char.
		 */
		Char _get_ch;

		/**
		 * @brief				Cached 'put' char.
		 */
		Char _put_ch;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`std::streambuf` specialization that is backed by a `char` string.
	 * @tparam Pool			    Pool.
	 * @tparam Log				Logging facility.
	 */
    template <typename Pool, typename Log = null_log>
	using vmem_string_streambuf = vmem_basic_string_streambuf<char, Pool, Log>;


}
