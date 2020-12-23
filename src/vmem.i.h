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
#include <climits>

#include "size.h"
#include "log.h"


namespace abc {

	using vmem_page_pos_t		= std::uint64_t;
	using vmem_item_pos_t		= std::uint16_t;
	using vmem_version_t		= std::uint16_t;
	using vmem_page_hit_count_t	= std::uint32_t;


	constexpr std::size_t		vmem_page_size		= size::k4;
	constexpr vmem_page_pos_t	vmem_page_pos_root	= 0;
	constexpr vmem_page_pos_t	vmem_page_pos_start	= 1;
	constexpr vmem_page_pos_t	vmem_page_pos_nil	= static_cast<vmem_page_pos_t>(ULLONG_MAX);
	constexpr vmem_item_pos_t	vmem_item_pos_nil	= static_cast<vmem_item_pos_t>(USHRT_MAX);


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
		vmem_page<Pool, Log>(vmem_page<Pool, Log>&& other) noexcept;

		~vmem_page<Pool, Log>() noexcept;

	public:
		Pool*						pool() const noexcept;
		vmem_page_pos_t				pos() const noexcept;
		void*						ptr() noexcept;
		const void*					ptr() const noexcept;

	public:
		void						free() noexcept;

	private:
		bool						alloc() noexcept;
		bool						lock() noexcept;
		void						unlock() noexcept;
		void						invalidate() noexcept;

	protected:
		Pool*						_pool;
		vmem_page_pos_t				_pos;
		void*						_ptr;
		Log*						_log;
	};


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	class vmem_ptr {
	public:
		vmem_ptr<T, Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, Log* log = nullptr);
		vmem_ptr<T, Pool, Log>(const vmem_ptr<T, Pool, Log>& other) noexcept = default;
		vmem_ptr<T, Pool, Log>(vmem_ptr<T, Pool, Log>&& other) noexcept = default;
		~vmem_ptr<T, Pool, Log>() = default;

	public:
		Pool*						pool() const noexcept;
		vmem_page_pos_t				page_pos() const noexcept;
		vmem_item_pos_t				item_pos() const noexcept;
									operator T*() noexcept;
									operator const T*() const noexcept;
		T*							operator ->() noexcept;
		const T*					operator ->() const noexcept;
		T&							operator *();
		const T&					operator *() const;

	private:
		T*							ptr() const noexcept;
		T&							deref() const;

	protected:
		vmem_page<Pool, Log>		_page;
		vmem_item_pos_t				_item_pos;
		Log*						_log;
	};


	// --------------------------------------------------------------


	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)


	struct vmem_list_state {
		vmem_page_pos_t		front_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		back_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		item_size			= 0;
		vmem_page_pos_t		total_item_count	= 0;
	};


	template <typename T>
	struct _vmem_list_page {
		vmem_page_pos_t		prev_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		item_count			= 0;
		T					items[1]			= { 0 };
	};


	struct _vmem_root_page {
		vmem_version_t		version				= 1;
		char				signature[10]		= "abc::vmem";
		vmem_item_pos_t		page_size			= vmem_page_size;
		std::uint16_t		unused1				= 0xcccc;
		vmem_list_state		free_pages;
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


	using vmem_iterator_edge_t = std::uint8_t;

	namespace vmem_iterator_edge {
		constexpr vmem_iterator_edge_t	none	= 0;
		constexpr vmem_iterator_edge_t	rbegin	= 1; // before front
		constexpr vmem_iterator_edge_t	end		= 2; // after back
	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	class vmem_list;


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_list_iterator {
	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= const pointer;
		using reference					= T&;
		using const_reference			= const T&;

	private:
		friend class vmem_list<T, Pool, Log>;

		vmem_list_iterator<T, Pool, Log>(const vmem_list<T, Pool, Log>* list, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log);

	public:
		vmem_list_iterator<T, Pool, Log>(const vmem_list_iterator<T, Pool, Log>& other) = default;
		vmem_list_iterator<T, Pool, Log>(vmem_list_iterator<T, Pool, Log>&& other) noexcept = default;

	public:
		const vmem_list_iterator<T, Pool, Log>&	operator =(const vmem_list_iterator<T, Pool, Log>& other) noexcept;

	public:
		bool								operator !=(const vmem_list_iterator<T, Pool, Log>& other) const noexcept;
		vmem_list_iterator<T, Pool, Log>&	operator ++() noexcept;
		vmem_list_iterator<T, Pool, Log>&	operator ++(int) noexcept;
		vmem_list_iterator<T, Pool, Log>&	operator --() noexcept;
		vmem_list_iterator<T, Pool, Log>&	operator --(int) noexcept;
		pointer								operator ->() noexcept;
		const_pointer						operator ->() const noexcept;
		reference							operator *();
		const_reference						operator *() const;

		bool								can_deref() const noexcept;

	private:
		friend class vmem_list<T, Pool, Log>;

		pointer								ptr() const noexcept;
		reference							deref() const;

	private:
		const vmem_list<T, Pool, Log>*		_list;
		vmem_page_pos_t						_page_pos;
		vmem_item_pos_t						_item_pos;
		vmem_iterator_edge_t				_edge;
		Log*								_log;
	};


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_list {
	public:
		using value_type				= T;
		using pointer					= vmem_ptr<T, Pool, Log>;
		using const_pointer				= const pointer;
		using reference					= T&;
		using const_reference			= const T&;
		using iterator					= vmem_list_iterator<T, Pool, Log>;
		using const_iterator			= const iterator;
		using reverse_iterator			= iterator;
		using const_reverse_iterator	= const_iterator;

	public:
		static constexpr std::size_t	items_pos() noexcept;
		static constexpr std::size_t	max_item_size() noexcept;
		static constexpr std::size_t	page_capacity() noexcept;
		static constexpr bool			is_uninit(const vmem_list_state* state) noexcept;

	public:
		vmem_list<T, Pool, Log>(vmem_list_state* state, Pool* pool, Log* log);

		vmem_list<T, Pool, Log>(const vmem_list<T, Pool, Log>& other) noexcept = default;
		vmem_list<T, Pool, Log>(vmem_list<T, Pool, Log>&& other) noexcept = default;

	public:
		iterator				begin() noexcept;
		const_iterator			begin() const noexcept;
		const_iterator			cbegin() const noexcept;

		iterator				end() noexcept;
		const_iterator			end() const noexcept;
		const_iterator			cend() const noexcept;

		reverse_iterator		rend() noexcept;
		const_reverse_iterator	rend() const noexcept;
		const_reverse_iterator	crend() const noexcept;

		reverse_iterator		rbegin() noexcept;
		const_reverse_iterator	rbegin() const noexcept;
		const_reverse_iterator	crbegin() const noexcept;

	public:
		bool					empty() const noexcept;
		std::size_t				size() const noexcept;

		pointer					frontptr() noexcept;
		const_pointer			frontptr() const noexcept;

		reference				front();
		const_reference			front() const;

		pointer					backptr() noexcept;
		const_pointer			backptr() const noexcept;

		reference				back();
		const_reference			back() const;

		void					push_back(const_reference item);
		void					pop_back();

		void					push_front(const_reference item);
		void					pop_front();

		iterator				insert(const_iterator itr, const_reference item);
		template <typename InputItr>
		iterator				insert(const_iterator itr, InputItr first, InputItr last);
		iterator				erase(const_iterator itr);
		iterator				erase(const_iterator first, const_iterator last);
		void					clear() noexcept;

	private:
		friend class vmem_list_iterator<T, Pool, Log>;

		void					move_next(iterator& itr) const noexcept;
		void					move_prev(iterator& itr) const noexcept;
		pointer					at(const_iterator& itr) const noexcept;

	private:
		void					begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;
		void					rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept;

	private:
		vmem_list_state*		_state;
		Pool*					_pool;
		Log*					_log;
	};

}
