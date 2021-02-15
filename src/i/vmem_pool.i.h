/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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

#include "../size.h"
#include "log.i.h"


namespace abc {

	using vmem_page_pos_t		= std::uint64_t;
	using vmem_item_pos_t		= std::uint16_t;
	using vmem_version_t		= std::uint16_t;
	using vmem_page_hit_count_t	= std::uint32_t;


	constexpr std::size_t		vmem_page_size			= size::k4;
	constexpr vmem_page_pos_t	vmem_page_pos_root		= 0;
	constexpr vmem_page_pos_t	vmem_page_pos_start		= 1;
	constexpr vmem_page_pos_t	vmem_page_pos_nil		= static_cast<vmem_page_pos_t>(ULLONG_MAX);
	constexpr vmem_item_pos_t	vmem_item_pos_nil		= static_cast<vmem_item_pos_t>(USHRT_MAX);
	constexpr std::size_t		vmem_min_mapped_pages	= 3;


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
		using Pool = vmem_pool<MaxMappedPages, Log>;
		using Page = vmem_page<vmem_pool<MaxMappedPages, Log>, Log>;

	public:
		static constexpr std::size_t	max_mapped_pages() noexcept;

	public:
		vmem_pool<MaxMappedPages, Log>(const char* file_path, Log* log = nullptr);

	private:
		friend Page;

		vmem_page_pos_t				alloc_page() noexcept;
		void						free_page(vmem_page_pos_t page_pos) noexcept;

		void*						lock_page(vmem_page_pos_t page_pos) noexcept;
		bool						unlock_page(vmem_page_pos_t page_pos) noexcept;

	// Constructor helpers
	private:
		void						verify_args_or_throw(const char* file_path);
		void						open_pool_or_throw(const char* file_path, /*out*/ bool& is_empty);
		void						init_pool_or_throw();
		void						create_root_page_or_throw();
		void						create_start_page_or_throw();
		void						verify_pool_or_throw();
		void						verify_root_page_or_throw();
		void						verify_start_page_or_throw();

	// alloc_page() / free_page() helpers
	private:
		vmem_page_pos_t				pop_free_page_pos() noexcept;
		void						push_free_page_pos(vmem_page_pos_t page_pos) noexcept;
		vmem_page_pos_t				create_page() noexcept;

	// lock_page() / unlock_page() helpers
	private:
		bool						find_mapped_page(vmem_page_pos_t page_pos, /*out*/ std::size_t& i) noexcept;
		bool						has_mapping_capacity() noexcept;
		std::size_t					make_mapping_capacity() noexcept;
		std::size_t					make_mapping_capacity(vmem_page_hit_count_t min_keep_count) noexcept;
		bool						should_keep_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count) noexcept;
		void						keep_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count, /*inout*/ std::size_t& empty_i) noexcept;
		void						unmap_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count, /*out*/ std::size_t& empty_i, /*inout*/ std::size_t& unmapped_count) noexcept;
		void*						lock_mapped_page(std::size_t i) noexcept;
		void						unlock_mapped_page(std::size_t i) noexcept;
		void*						map_new_page(std::size_t i, vmem_page_pos_t page_pos) noexcept;
		void						optimize_mapped_page(std::size_t i) noexcept;
		std::size_t					next_empty_i(std::size_t i, std::size_t empty_i) noexcept;
		void						log_totals() noexcept;

	private:
		bool						_ready;
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
		vmem_page<Pool, Log>(const vmem_page<Pool, Log>& other) noexcept;
		vmem_page<Pool, Log>(vmem_page<Pool, Log>&& other) noexcept;
		vmem_page<Pool, Log>(nullptr_t) noexcept;

		~vmem_page<Pool, Log>() noexcept;

	public:
		vmem_page<Pool, Log>&		operator =(const vmem_page<Pool, Log>& other) noexcept;

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
		vmem_ptr<T, Pool, Log>&		operator =(const vmem_ptr<T, Pool, Log>& other) noexcept = default;

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


	struct vmem_linked_state {
		vmem_page_pos_t		front_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		back_page_pos		= vmem_page_pos_nil;
	};


	struct vmem_container_state : public vmem_linked_state {
		vmem_item_pos_t		item_size			= 0;
		vmem_page_pos_t		total_item_count	= 0;
	};


	struct vmem_list_state : public vmem_container_state {
	};


	struct vmem_linked_page {
		vmem_page_pos_t		prev_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
	};

	template <typename T>
	struct vmem_container_page : public vmem_linked_page {
		vmem_item_pos_t		item_count			= 0;
		T					items[1]			= { 0 };
	};

	template <typename T>
	struct vmem_list_page : public vmem_container_page<T> {
	};


	struct vmem_root_page {
		const vmem_version_t	version			= 1;
		const char				signature[10]	= "abc::vmem";
		const vmem_item_pos_t	page_size		= vmem_page_size;
		const std::uint16_t		unused1			= 0xcccc;
		vmem_list_state			free_pages;
		const std::uint8_t		unused2			= 0xcc;
	};


	template <typename Key>
	struct vmem_key_item {
		Key					key;
		vmem_page_pos_t		child_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		value_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		value_item_pos		= vmem_page_pos_nil;
	};


	template <typename Key>
	struct vmem_key_page {
		vmem_item_pos_t		item_count			= 0;
		vmem_page_pos_t		else_page_pos		= vmem_page_pos_nil;
		vmem_key_item<Key>	items[1]			= { 0 };
	};


	template <typename Value>
	struct vmem_value_page {
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
		vmem_item_pos_t		item_count			= 0;
		Value				items[1]			= { 0 };
	};


	#pragma pack(pop)


	// --------------------------------------------------------------

}
