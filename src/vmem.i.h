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
		vmem_page_hit_count_t	check_count;
		vmem_page_hit_count_t	unmap_count;
	};


	// --------------------------------------------------------------


	template <typename Pool, typename Log>
	class _vmem_page;

	template <typename Pool, typename Log>
	class _vmem_ptr;


	template <std::size_t MaxMappedPages, typename Log = null_log>
	class vmem_pool {
	public:
		vmem_pool<MaxMappedPages, Log>(const char* file_path, Log* log = nullptr);

	private:
		friend class _vmem_page<vmem_pool<MaxMappedPages, Log>, Log>;
		friend class _vmem_ptr<vmem_pool<MaxMappedPages, Log>, Log>;

		vmem_page_pos_t				get_free_page();
		vmem_page_pos_t				create_page();
		void						delete_page(vmem_page_pos_t page_pos);

		void*						lock_page(vmem_page_pos_t page_pos);
		void						unlock_page(vmem_page_pos_t page_pos);

	private:
		int							_fd;
		std::size_t					_mapped_page_count;
		_vmem_mapped_page			_mapped_pages[MaxMappedPages];
		_vmem_mapped_page_totals	_mapped_page_totals;
		Log*						_log;
	};


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	class vmem_ptr;


	template <typename Pool, typename Log>
	class _vmem_page {
	public:
		_vmem_page<Pool, Log>(Pool* pool, Log* = nullptr);
		_vmem_page<Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, Log* log = nullptr);
		_vmem_page<Pool, Log>(const _vmem_page<Pool, Log>& other) = default;
		_vmem_page<Pool, Log>(_vmem_page<Pool, Log>&& other) = default;

	public:
		Pool*								pool() const noexcept;
		vmem_page_pos_t						pos() const noexcept;

		vmem_ptr<void, Pool, Log>&&			front() noexcept;
		const vmem_ptr<void, Pool, Log>&&	front() const noexcept;
		vmem_ptr<void, Pool, Log>&&			back() noexcept;
		const vmem_ptr<void, Pool, Log>&&	back() const noexcept;

		void								erase();

	protected:
		Pool*								_pool;
		vmem_page_pos_t						_pos;
		Log*								_log;
	};


	template <typename T, typename Pool, typename Log = null_log>
	class vmem_page : public _vmem_page<Pool, Log> {
		using base = _vmem_page<Pool, Log>;

	public:
		vmem_page<T, Pool, Log>(Pool* pool, Log* log = nullptr);
		vmem_page<T, Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, Log* log = nullptr);
		vmem_page<T, Pool, Log>(const vmem_page<T, Pool, Log>& other) = default;
		vmem_page<T, Pool, Log>(vmem_page<T, Pool, Log>&& other) = default;

	public:
		vmem_ptr<T, Pool, Log>&&			operator[](vmem_item_pos_t item_pos);
		const vmem_ptr<T, Pool, Log>&&		operator[](vmem_item_pos_t item_pos) const;
	};


	// --------------------------------------------------------------


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
		friend class vmem_page<T, Pool, Log>;

		vmem_ptr<T, Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, Log* log = nullptr);

	public:
		vmem_ptr<T, Pool, Log>(const vmem_ptr<T, Pool, Log>& other) = default;
		vmem_ptr<T, Pool, Log>(vmem_ptr<T, Pool, Log>&& other) = default;
		~vmem_ptr<T, Pool, Log>();

	public:
		T*							get();
		const T*					get() const;
	};


	// --------------------------------------------------------------


	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)


	struct _vmem_root_page {
		vmem_version_t		version				= 1;
		char				signature[10]		= "abc::vmem";
		vmem_item_pos_t		page_size			= vmem_page_size;
		vmem_page_pos_t		total_page_count	= 1;
		vmem_page_pos_t		free_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		value_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		key_page_pos		= vmem_page_pos_nil;
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

}
