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

#include <cstdint>
#include <fstream>
#include <chrono>
#include <ios>

#include "log.i.h"
#include "timestamp.i.h"


namespace abc {

	/**
	 * @brief					`streambuf` specialization that is backed by files whose names are made of timestamps.
	 * @tparam MaxPath			Maximum length of the file path.
	 * @tparam Clock			Clock to generate a timestamp.
	 * @tparam Log				Logging facility.
	 */
	template <std::size_t MaxPath = size::k2, typename Clock = std::chrono::system_clock, typename Log = null_log>
	class multifile_streambuf : public std::filebuf {
		using base = std::filebuf;

	public:
		/**
		 * @brief				Constructor.
		 * @param path			Path to an existing folder 
		 * @param mode			`std::ios_base::openmode`
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		multifile_streambuf(const char* path, std::ios_base::openmode mode = std::ios_base::out, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		multifile_streambuf(multifile_streambuf&& other) noexcept;

		/**
		 * @brief				Deleted.
		 */
		multifile_streambuf(const multifile_streambuf& other) = delete;

	public:
		/**
		 * @brief				Closes the current file, and opens a new file.
		 */
		void reopen();

		/**
		 * @brief				Returns the full path of the current file.
		 */
		const char* path() const noexcept;

	private:
		/**
		 * @brief				Buffer to store the file path.
		 */
		char _path[MaxPath + 1];

		/**
		 * @brief				Length of the path to the parent folder.
		 */
		std::size_t _path_length;

		/**
		 * @brief				`std::ios_base::openmode`.
		 */
		std::ios_base::openmode _mode;

		/**
		 * @brief				The log passed in to the constructor.
		 */
		Log* _log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`multifile_streambuf` specialization that is backed by files whose names are made of timestamps.
	 * @details					Automatically closes and reopens a new file when the given time duration has passed. 
	 * @tparam MaxPath			Maximum length of the file path.
	 * @tparam Clock			Clock to generate a timestamp.
	 * @tparam Log				Logging facility.
	 */
	template <std::size_t MaxPath = size::k2, typename Clock = std::chrono::system_clock, typename Log = null_log>
	class duration_multifile_streambuf : public multifile_streambuf<MaxPath, Clock, Log> {
		using base = multifile_streambuf<MaxPath, Clock, Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param duration		Duration limit of the file.
		 * @param path			Path to an existing folder 
		 * @param mode			`std::ios_base::openmode`
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		duration_multifile_streambuf(typename Clock::duration duration, const char* path, std::ios_base::openmode mode = std::ios_base::out, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		duration_multifile_streambuf(duration_multifile_streambuf&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		duration_multifile_streambuf(const duration_multifile_streambuf& other) = delete;

	public:
		/**
		 * @brief				Closes the current file, and opens a new file.
		 */
		void reopen();

	protected:
		/**
		 * @brief				Handler that checks the duration, and calls `reopen()` if needed.
		 */
		virtual int sync() override;

	private:
		/**
		 * @brief				The duration passed in to the constructor.
		 */
		const typename Clock::duration _duration;

		/**
		 * @brief				The creation timestamp of the current file.
		 */
		timestamp<Clock> _ts;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					`multifile_streambuf` specialization that is backed by files whose names are made of timestamps.
	 * @details					Automatically closes and reopens a new file when the given time duration has passed. 
	 * @tparam MaxPath			Maximum length of the file path.
	 * @tparam Clock			Clock to generate a timestamp.
	 * @tparam Log				Logging facility.
	 */
	template <std::size_t MaxPath = size::k2, typename Clock = std::chrono::system_clock, typename Log = null_log>
	class size_multifile_streambuf : public multifile_streambuf<MaxPath, Clock, Log> {
		using base = multifile_streambuf<MaxPath, Clock, Log>;

	public:
		/**
		 * @brief				Constructor.
		 * @param size			Size limit of the file.
		 * @param path			Path to an existing folder 
		 * @param mode			`std::ios_base::openmode`
		 * @param log			Pointer to a `Log` instance. May be `nullptr`.
		 */
		size_multifile_streambuf(std::size_t size, const char* path, std::ios_base::openmode mode = std::ios_base::out, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		size_multifile_streambuf(size_multifile_streambuf&& other) noexcept = default;

		/**
		 * @brief				Deleted.
		 */
		size_multifile_streambuf(const size_multifile_streambuf& other) = delete;

	public:
		/**
		 * @brief				Closes the current file, and opens a new file.
		 */
		void reopen();

	protected:
		/**
		 * @brief				Handler that counts the chars put out at once.
		 * @param s				Char sequence to put out. 
		 * @param count			Count of chars.
		 */
		virtual std::streamsize xsputn(const char* s, std::streamsize count) override;

		/**
		 * @brief				Handler that checks the size, and calls `reopen()` if needed.
		 */
		virtual int sync() override;

	protected:
		/**
		 * @brief				Overrides the default pcount calculation.
		 */
		std::size_t pcount() const noexcept;

	private:
		/**
		 * @brief				The size passed in to the constructor.
		 */
		const std::size_t _size;

		/**
		 * @brief				Current file size.
		 */
		std::size_t _current_size;
	};

}
