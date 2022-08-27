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

#include <streambuf>
#include <istream>
#include <ostream>


namespace abc {

	using CharPredicate = bool (*) (char);


	// --------------------------------------------------------------


	/**
	 * @brief				Common stream functionality.
	 * @tparam Stream		Base stream class.
	 */
	template <typename Stream>
	class stream : protected Stream {
		using base = Stream;

	protected:
		/**
		 * @brief			Constructor.
		 * @param sb		Pointer to a `std::streambuf` implementation. 
		 */
		stream(std::streambuf* sb);

		/**
		 * @brief			Move constructor.
		 */
		stream(stream&& other);

		/**
		 * @brief			Deleted.
		 */
		stream(const stream& other) = delete;

	public:
		/**
		 * @brief			Returns the pointer to `std::streambuf` passed in to the constructor.
		 */
		std::streambuf* rdbuf() const;

		/**
		 * @brief			Delegates to the corresponding getter on the base stream.
		 */
		bool eof() const;

		/**
		 * @brief			Delegates to the corresponding getter on the base stream.
		 */
		bool good() const;

		/**
		 * @brief			Delegates to the corresponding getter on the base stream.
		 */
		bool bad() const;

		/**
		 * @brief			Delegates to the corresponding getter on the base stream.
		 */
		bool fail() const;

		/**
		 * @brief			Delegates to the corresponding getter on the base stream.
		 */
		bool operator!() const;

		/**
		 * @brief			Delegates to the corresponding getter on the base stream.
		 */
		operator bool() const;

	protected:
		/**
		 * @brief			Resets all state bits on the base stream.
		 */
		void reset();

		/**
		 * @brief			Returns if reading can continue from this stream.
		 */
		bool is_good() const;

		/**
		 * @brief			Sets the bad bits on the base stream.
		 */
		void set_bad();

		/**
		 * @brief			Sets the bad bits on the base stream if the condition is `true`.
		 * @param condition	Condition.
		 */
		void set_bad_if(bool condition);

		/**
		 * @brief			Sets the fail bit on the base stream.
		 */
		void set_fail();

		/**
		 * @brief			Sets the fail bit on the base stream if the condition is `true`.
		 * @param condition	Condition.
		 */
		void set_fail_if(bool condition);
	};


	// --------------------------------------------------------------


	/**
	 * @brief				Common input stream functionality.
	 */
	class _istream : public stream<std::istream> {
		using base = stream<std::istream>;

	protected:
		/**
		 * @brief			Constructor.
		 * @param sb		Pointer to a `std::streambuf` implementation. 
		 */
		_istream(std::streambuf* sb);

		/**
		 * @brief			Move constructor.
		 */
		_istream(_istream&& other);

		/**
		 * @brief			Deleted.
		 */
		_istream(const _istream& other) = delete;

	public:
		/**
		 * @brief			Returns the number of chars read by the last read operation.
		 */
		std::size_t gcount() const noexcept;

	protected:
		/**
		 * @brief			Resets the `gcount`.
		 */
		void reset();

		/**
		 * @brief			Sets the `gcount` to the specified value.
		 * @param gcount	New value.
		 */
		void set_gcount(std::size_t gcount) noexcept;

	private:
		/**
		 * @brief			The number of chars read by the last read operation.
		 */
		std::size_t _gcount;
	};


	// --------------------------------------------------------------


	/**
	 * @brief				Common output stream functionality.
	 */
	class _ostream : public stream<std::ostream> {
		using base = stream<std::ostream>;

	protected:
		/**
		 * @brief			Constructor.
		 * @param sb		Pointer to a `std::streambuf` implementation. 
		 */
		_ostream(std::streambuf* sb);

		/**
		 * @brief			Move constructor.
		 */
		_ostream(_ostream&& other);

		/**
		 * @brief			Deleted.
		 */
		_ostream(const _ostream& other) = delete;

	public:
		/**
		 * @brief			Delegates to the `flush()` of the base stream.
		 */
		void flush();
	};

}

