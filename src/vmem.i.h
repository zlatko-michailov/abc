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

#include "log.h"


namespace abc {

	using vmem_page_pos_t		= std::uint64_t;
	using vmem_item_pos_t		= std::uint16_t;
	using vmem_version_t		= std::uint16_t;
	using vmem_page_hit_count_t	= std::uint32_t;


	constexpr std::size_t		vmem_page_size		= size::k4;
	constexpr vmem_page_pos_t	vmem_page_pos_root	= 0;
	constexpr vmem_page_pos_t	vmem_page_pos_start	= 1;
	constexpr vmem_page_pos_t	vmem_page_pos_nil	= static_cast<vmem_page_pos_t>(-1);
	constexpr vmem_item_pos_t	vmem_item_pos_nil	= static_cast<vmem_item_pos_t>(-1);


	// --------------------------------------------------------------


	struct _vmem_mapped_page {
		vmem_page_pos_t			pos;
		void*					ptr;
		vmem_page_hit_count_t	lock_count;
		vmem_page_hit_count_t	keep_count;
	};


	struct _vmem_mapped_page_totals {
		vmem_page_hit_count_t	keep_count;
		vmem_page_hit_count_t	hit_count;
		vmem_page_hit_count_t	miss_count;
		vmem_page_hit_count_t	unlock_count;
		vmem_page_hit_count_t	check_count;
		vmem_page_hit_count_t	unmap_count;
	};


	// --------------------------------------------------------------


	template <typename Pool, typename Log>
	class vmem_page;


	template <std::size_t MaxMappedPages, typename Log = null_log>
	class vmem_pool {
	public:
		vmem_pool<MaxMappedPages, Log>(const char* file_path, Log* log = nullptr);

	private:
		friend class vmem_page<vmem_pool<MaxMappedPages, Log>, Log>;

		vmem_page_pos_t				alloc_page() noexcept;
		bool						free_page(vmem_page_pos_t page_pos) noexcept;

		void*						lock_page(vmem_page_pos_t page_pos) noexcept;
		bool						unlock_page(vmem_page_pos_t page_pos) noexcept;

	private:
		vmem_page_pos_t				create_page() noexcept;
		void						log_totals() noexcept;

	private:
		int							_fd;
		std::size_t					_mapped_page_count;
		_vmem_mapped_page			_mapped_pages[MaxMappedPages];
		_vmem_mapped_page_totals	_mapped_page_totals;
		Log*						_log;
	};


	// --------------------------------------------------------------


	template <typename Pool, typename Log>
	class vmem_page {
	public:
		vmem_page<Pool, Log>(Pool* pool, Log* log = nullptr);
		vmem_page<Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, Log* log = nullptr);
		vmem_page<Pool, Log>(const vmem_page<Pool, Log>& other);
		vmem_page<Pool, Log>(vmem_page<Pool, Log>&& other);

		~vmem_page<Pool, Log>() noexcept;

	public:
		Pool*								pool() const noexcept;
		vmem_page_pos_t						pos() const noexcept;
		void*								ptr() noexcept;
		const void*							ptr() const noexcept;

	public:
		void								free() noexcept;

	private:
		bool								alloc() noexcept;
		bool								lock() noexcept;
		void								unlock() noexcept;
		void								invalidate() noexcept;

	protected:
		Pool*								_pool;
		vmem_page_pos_t						_pos;
		void*								_ptr;
		Log*								_log;
	};


	// --------------------------------------------------------------


#ifdef REMOVE ////
	template <typename Pool, typename Log>
	class _vmem_ptr {
	public:
		_vmem_ptr<Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos);
		_vmem_ptr<Pool, Log>(const _vmem_ptr<Pool, Log>& other) = default;
		_vmem_ptr<Pool, Log>(_vmem_ptr<Pool, Log>&& other) = default;
		~_vmem_ptr<Pool, Log>();

	public:
		Pool*						pool() const noexcept;
		vmem_page_pos_t				page_pos() const noexcept;
		vmem_item_pos_t				item_pos() const noexcept;

	protected:
		Pool*						_pool;
		vmem_page_pos_t				_page_pos;
		vmem_item_pos_t				_item_pos;
		Log*						_log;
	};


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_ptr : public _vmem_ptr<Pool, Log> {
		using base = _vmem_ptr<Pool, Log>;

	private:
		friend class vmem_page<Pool, Log>;

		vmem_ptr<T, Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, Log* log = nullptr);

	public:
		vmem_ptr<T, Pool, Log>(const vmem_ptr<T, Pool, Log>& other) = default;
		vmem_ptr<T, Pool, Log>(vmem_ptr<T, Pool, Log>&& other) = default;
		~vmem_ptr<T, Pool, Log>();

	public:
		T*							get();
		const T*					get() const;
	};
#endif


	// --------------------------------------------------------------


	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)


	template <typename T>
	struct _vmem_deque_page {
		vmem_page_pos_t		prev_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		item_count			= 0;
		T					items[1]			= { 0 };
	};


	struct vmem_deque_state {
		vmem_page_pos_t		front_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		back_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		total_item_count	= 0;
	};


	struct _vmem_root_page {
		vmem_version_t		version				= 1;
		char				signature[10]		= "abc::vmem";
		vmem_item_pos_t		page_size			= vmem_page_size;
		std::uint16_t		unused1				= 0xcccc;
		vmem_deque_state	free_pages;
		std::uint8_t		unused2				= 0xcc;
	};


	template <typename Key>
	struct _vmem_key_item {
		Key					key;
		vmem_page_pos_t		child_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		value_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		value_item_pos		= vmem_page_pos_nil;
	};


	template <typename Key>
	struct _vmem_key_page {
		vmem_item_pos_t		item_count			= 0;
		vmem_page_pos_t		else_page_pos		= vmem_page_pos_nil;
		_vmem_key_item<Key>	items[1]			= { 0 };
	};


	template <typename Value>
	struct _vmem_value_page {
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		item_count			= 0;
		Value				items[1]			= { 0 };
	};


	using _vmem_free_page = _vmem_value_page<vmem_page_pos_t>;


	#pragma pack(pop)


	// --------------------------------------------------------------


	using vmem_deque_iterator_flag_t = std::uint8_t;

	namespace vmem_deque_iterator_flag {
		constexpr vmem_deque_iterator_flag_t	none			= 0x00;
		constexpr vmem_deque_iterator_flag_t	read_data		= 0x01;
		constexpr vmem_deque_iterator_flag_t	write_date		= 0x02;
		constexpr vmem_deque_iterator_flag_t	move_forward	= 0x04;
		constexpr vmem_deque_iterator_flag_t	move_backward	= 0x08;
	}


	template <typename T, typename Pool, typename Log>
	class _vmem_deque;


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_deque_iterator {
	private:
		friend class _vmem_deque<T, Pool, Log>;

		vmem_deque_iterator<T, Pool, Log>(Pool* pool, Log* log, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_deque_iterator_flag_t falgs);

	public:
		vmem_deque_iterator<T, Pool, Log>(const vmem_deque_iterator<T, Pool, Log>& other);

	public:
		const vmem_deque_iterator<T, Pool, Log>&	operator =(const vmem_deque_iterator<T, Pool, Log>& other) noexcept;

	public:
		vmem_deque_iterator<T, Pool, Log>&			operator ++() noexcept;
		vmem_deque_iterator<T, Pool, Log>&			operator --() noexcept;
		T*											operator ->() noexcept;
		T											operator *() noexcept;
		bool										operator !=(const vmem_deque_iterator<T, Pool, Log>& other) noexcept;

	private:
		bool										push(const T& item) noexcept;
		T											pop() noexcept;

	private:
		Pool*										_pool;
		Log*										_log;

		vmem_page_pos_t								_page_pos;
		vmem_item_pos_t								_item_pos;
		vmem_deque_iterator_flag_t					_flags;
	};


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	class _vmem_deque {
	protected:
		_vmem_deque(vmem_deque_state* state, Pool* pool, Log* log);

	public:
		bool				empty() const noexcept;
		std::size_t			size() const noexcept;

	protected:
		T					front() const noexcept;
		T					back() const noexcept;

		bool				push_back(const T& item) noexcept;
		T					pop_back() noexcept;

		bool				push_front(const T& item) noexcept;
		T					pop_front() noexcept;


	protected:
		const vmem_deque_iterator<T, Pool, Log>&	begin() noexcept;
		const vmem_deque_iterator<T, Pool, Log>&	end() noexcept;

		const vmem_deque_iterator<T, Pool, Log>&	rbegin() noexcept;
		const vmem_deque_iterator<T, Pool, Log>&	rend() noexcept;

	protected:
		vmem_deque_state*					_state;
		Pool*								_pool;
		Log*								_log;

		vmem_deque_iterator<T, Pool, Log>	_begin;
		vmem_deque_iterator<T, Pool, Log>	_end;
		vmem_deque_iterator<T, Pool, Log>	_rbegin;
		vmem_deque_iterator<T, Pool, Log>	_rend;
	};


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_deque : public _vmem_deque<T, Pool, Log> {
		using base = _vmem_deque<T, Pool, Log>;

	public:
		vmem_deque<T, Pool, Log>(vmem_deque_state* state, Pool* pool, Log* log);

	public:
		const vmem_deque_iterator<T, Pool, Log>&	begin() noexcept;
		const vmem_deque_iterator<T, Pool, Log>&	end() noexcept;

		const vmem_deque_iterator<T, Pool, Log>&	rbegin() noexcept;
		const vmem_deque_iterator<T, Pool, Log>&	rend() noexcept;

	public:
		T			front() const noexcept;
		T			back() const noexcept;

		bool		push_back(const T& item) noexcept;
		bool		pop_back() noexcept;

		bool		push_front(const T& item) noexcept;
		bool		pop_front() noexcept;
	};


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_stack : public _vmem_deque<T, Pool, Log> {
		using base = _vmem_deque<T, Pool, Log>;

	public:
		vmem_stack<T, Pool, Log>(vmem_deque_state* state, Pool* pool, Log* log);

	public:
		T*			top() noexcept;

		bool		push(const T& item) noexcept;
		bool		pop() noexcept;
	};


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_queue : public _vmem_deque<T, Pool, Log> {
		using base = _vmem_deque<T, Pool, Log>;

	public:
		vmem_queue<T, Pool, Log>(vmem_deque_state* state, Pool* pool, Log* log);

	public:
		T*			front() noexcept;
		T*			back() noexcept;

		bool		push(const T& item) noexcept;
		bool		pop() noexcept;
	};


}
