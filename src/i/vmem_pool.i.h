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


	/**
	 * @brief					Information about a mapped vmem page.
	 */
	struct vmem_mapped_page {
		vmem_page_pos_t			pos;
		void*					ptr;
		vmem_page_hit_count_t	lock_count;
		vmem_page_hit_count_t	keep_count;
	};


	/**
	 * @brief					Performance counters of a vmem pool.
	 */
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

	template <typename Pool, typename Log>
	class vmem_linked;


	/**
	 * @brief					Virtual memory (vmem) pool.
	 * @details					Every pool is persisted to a file.
	 * @tparam MaxMappedPages	Maximum number of pages that could be mapped in memory at the same time.
	 * 							Once this limit is reached, the pool fails to lock any more pages until one or more locked pages get unlocked. 
	 * @tparam Log				Logging facility.
	 */
	template <std::size_t MaxMappedPages, typename Log = null_log>
	class vmem_pool {
		using Pool = vmem_pool<MaxMappedPages, Log>;

	public:
		/**
		 * @brief				Returns the `MaxMappedPages` limit.
		 */
		static constexpr std::size_t max_mapped_pages() noexcept;

	public:
		/**
		 * @brief				Constructor.
		 * @param file_path		Path to the pool file.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_pool<MaxMappedPages, Log>(const char* file_path, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		vmem_pool<MaxMappedPages, Log>(vmem_pool<MaxMappedPages, Log>&& other) noexcept;

		/**
		 * @brief				Destructor.
		 */
		~vmem_pool<MaxMappedPages, Log>() noexcept;

	private:
		friend vmem_page<Pool, Log>;

		/**
		 * @brief				Allocates a page.
		 * @details				Tries to reuse a free page if there are any. Otherwise, adds a new page to the pool. 
		 * @return				The position of the allocated page. 
		 */
		vmem_page_pos_t alloc_page() noexcept;

		/**
		 * @brief				Frees a page by adding it to the list of free pages.
		 * @param page_pos		Page position.
		 */
		void free_page(vmem_page_pos_t page_pos) noexcept;

		/**
		 * @brief				Locks a page in memory
		 * @details				Once a page is locked, its contents can be addressed by regular pointers.
		 * @param page_pos		Page position.
		 * @return				Pointer to the first byte of the page.
		 */
		void* lock_page(vmem_page_pos_t page_pos) noexcept;

		/**
		 * @brief				Unlocks a page in memory.
		 * @details				A page may be locked multiple times.
		 * 						Once the last lock is removed, the contents of the page can no longer be addressed by regular pointers.
		 * @param page_pos		Page position.
		 * @return				`true` = success; `false` = error.
		 */
		bool unlock_page(vmem_page_pos_t page_pos) noexcept;

	private:
		friend vmem_linked<Pool, Log>;

		/**
		 * @brief				Frees up all pages of the `vmem_linked` struct at once.
		 * @param linked		Reference to a `vmem_linked` instance.
		 */
		void clear_linked(vmem_linked<Pool, Log>& linked);

	// Constructor helpers
	private:
		/**
		 * @brief				Verifies arguments before opening the pool file. Throws on error.
		 * @param file_path		Pool file path.
		 */
		void verify_args_or_throw(const char* file_path);

		/**
		 * @brief				Opens the pool file, and verifies its essential pages. Throws on error. An empty file is acceptable.
		 * @param file_path		Pool file path.
		 * @return				`true` = the pool file is already initialized; `false` = the pool file needs initialization.
		 */
		bool open_pool_or_throw(const char* file_path);

		/**
		 * @brief				Initializes an empty pool file. Throws on error.
		 */
		void init_pool_or_throw();

		/**
		 * @brief				Creates the root page on an empty pool file. Throws on error.
		 */
		void create_root_page_or_throw();

		/**
		 * @brief				Creates the start page on an empty pool file. Throws on error.
		 */
		void create_start_page_or_throw();

		/**
		 * @brief				Verifies the essential pages on an existing pool file. Throws on error.
		 */
		void verify_pool_or_throw();

		/**
		 * @brief				Verifies the root page on an existing pool file. Throws on error.
		 */
		void verify_root_page_or_throw();

		/**
		 * @brief				Verifies the start page on an existing pool file. Throws on error.
		 */
		void verify_start_page_or_throw();

	// alloc_page() / free_page() helpers
	private:
		/**
		 * @brief				Pops a free page from the pool's list of free pages, and returns its position.
		 * @return				The position of the popped page.
		 */
		vmem_page_pos_t pop_free_page_pos() noexcept;

		/**
		 * @brief				Pushes a page to the pool's list of free pages.
		 * @param page_pos		Page position.
		 */
		void push_free_page_pos(vmem_page_pos_t page_pos) noexcept;

		/**
		 * @brief				Unconditionally creates a new page on the pool file, and returns its position.
		 * @return				The position of the new page.
		 */
		vmem_page_pos_t create_page() noexcept;

	// lock_page() / unlock_page() helpers
	private:
		/**
		 * @brief				Tries to find a mapped page.
		 * @param page_pos		Page position.
		 * @param i				Output. The position of the page on the array of mapped pages.
		 * @return				`true` = found; `false` = not found.
		 */
		bool find_mapped_page(vmem_page_pos_t page_pos, std::size_t& i) noexcept;

		/**
		 * @brief				Checks whether there is capacity for at least one more page on the array of mapped pages.
		 */
		bool has_mapping_capacity() noexcept;

		/**
		 * @brief				Frees unlocked mapped pages.
		 * @details				First tries to free unlocked mapped pages with a keep count below the current average.
		 * 						If no page gets freed, it tries to free all unlocked mapped pages.
		 * @return				The count of unmapped pages.
		 */
		std::size_t make_mapping_capacity() noexcept;

		/**
		 * @brief				Frees unlocked mapped pages with a keep count below the given minimum.
		 * @param min_keep_count Minimum keep count for an unlocked mapped page to be kept.
		 * @return				The count of unmapped pages.
		 */
		std::size_t make_mapping_capacity(vmem_page_hit_count_t min_keep_count) noexcept;

		/**
		 * @brief				Checks weather an unlock mapped page meets the required minimum keep count.
		 * @param i				Position on the array of mapped pages.
		 * @param min_keep_count Minimum keep count.
		 */
		bool should_keep_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count) noexcept;

		/**
		 * @brief				Keeps an unlocked mapped page.
		 * @details				Subtracts the minimum keep count from the mapped page's keep count, so that eventually the page would get unmapped after a few rounds.
		 * 						Separately, if a valid position of an empty slot is provided, the kept page is moved to that slot, and this slot becomes the new empty one.
		 * 						This process bubbles empty slots to the end of the array of mapped pages.
		 * @param i				Position on the array of mapped pages.
		 * @param min_keep_count Minimum keep count.
		 * @param empty_i		Input/output. The lowest position of an empty item on the array of mapped pages.
		 */
		void keep_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count, std::size_t& empty_i) noexcept;

		/**
		 * @brief				Unconditionally unmaps a  mapped page.
		 * @param i				Position on the array of mapped pages.
		 * @param min_keep_count Minimum keep count.
		 * @param empty_i		Output. If this is the first unmapped page, this argument is set to `i`.
		 * @param unmapped_count Input/output. Current count of unmapped pages. This counter gets incremented once.
		 */
		void unmap_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count, std::size_t& empty_i, std::size_t& unmapped_count) noexcept;

		/**
		 * @brief				Locks a mapped page, and returns a pointer to the page's content.
		 * @details				Increments the lock count and the keep count of the mapped page.
		 * @param i				Position on the array of mapped pages.
		 * @return				Pointer to the beginning of the page.
		 */
		void* lock_mapped_page(std::size_t i) noexcept;

		/**
		 * @brief				Unlocks a mapped page.
		 * @details				Decrements the lock count of the mapped page.
		 * 						If the lock count becomes zero, i.e. if all locks have been released, the page is synced to the file.
		 * @param i				Position on the array of mapped pages.
		 */
		void unlock_mapped_page(std::size_t i) noexcept;

		/**
		 * @brief				Maps and locks a page in memory.
		 * @param i				Position on the array of mapped pages.
		 * @param page_pos		Page position.
		 * @return				Pointer to the beginning of the page.
		 */
		void* map_new_page(std::size_t i, vmem_page_pos_t page_pos) noexcept;

		/**
		 * @brief				Swaps a mapped page with another one that has a lower position on the array of mapped pages and a lower keep count.
		 * @details				This process keeps mapped pages with high keep counts, i.e. frequently used pages, closer to the beginning of the array,
		 * 						so that they can be found using fewer comparisons.
		 * @param i				Position on the array of mapped pages.
		 */
		void optimize_mapped_page(std::size_t i) noexcept;

		/**
		 * @brief				Tries to find an empty slot on the array of mapped pages before a given position `i`.
		 * @param i				Position on the array of mapped pages.
		 * @param empty_i		Lower bound (exclusive) for the sought empty position.
		 * @return				The lowest position of an empty slot on the array of mapped pages, or `i` if no such empty slot exists.
		 */
		std::size_t next_empty_i(std::size_t i, std::size_t empty_i) noexcept;

		/**
		 * @brief				Logs performance stats.
		 */
		void log_totals() noexcept;

	private:
		bool						_ready;
		int							_fd;
		std::size_t					_mapped_page_count;
		vmem_mapped_page			_mapped_pages[MaxMappedPages];
		_vmem_mapped_page_totals	_mapped_page_totals;
		Log*						_log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Virtual memory (vmem) page.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename Pool, typename Log = null_log>
	class vmem_page {
	public:
		/**
		 * @brief				Constructor.
		 * @details				Maps a free page, if there are any. Otherwise, allocates a new page at the end of the pool. Locks the page.
		 * @param pool			Pointer to a Pool instance.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_page<Pool, Log>(Pool* pool, Log* log = nullptr);

		/**
		 * @brief				Constructor.
		 * @details				Maps and locks a specific page.
		 * @param pool			Pointer to a Pool instance.
		 * @param page_pos		Page position. If `vmem_page_pos_nil`, a free/new page is mapped.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_page<Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		vmem_page<Pool, Log>(vmem_page<Pool, Log>&& other) noexcept;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_page<Pool, Log>(const vmem_page<Pool, Log>& other) noexcept;

		/**
		 * @brief				Constructor.
		 * @details				Constructs an invalid page. Does not map any page.
		 */
		vmem_page<Pool, Log>(std::nullptr_t) noexcept;

		/**
		 * @brief				Destructor.
		 */
		~vmem_page<Pool, Log>() noexcept;

	public:
		vmem_page<Pool, Log>&		operator =(const vmem_page<Pool, Log>& other) noexcept;
		vmem_page<Pool, Log>&		operator =(vmem_page<Pool, Log>&& other) noexcept;

	public:
		/**
		 * @brief				Returns the pointer to the Pool instance passed in to the constructor.
		 */
		Pool* pool() const noexcept;

		/**
		 * @brief				Returns the page's position in the pool.
		 */
		vmem_page_pos_t pos() const noexcept;

		/**
		 * @brief				Returns a pointer the page's mapped area in memory.
		 */
		void* ptr() noexcept;

		/**
		 * @brief				Returns a `const` pointer the page's mapped area in memory.
		 */
		const void* ptr() const noexcept;

	public:
		/**
		 * @brief				Frees the page
		 * @details				Adds the page to the list of free pages.
		 */
		void free() noexcept;

	private:
		/**
		 * @brief				Allocates a pool page for this instance.
		 * @return				`true` = success; `false` = error.
		 */
		bool alloc() noexcept;

		/**
		 * @brief				Locks this page in memory.
		 * @details				A page's pointer may be used only after the page has been locked.
		 * 						A page may be called multiple times. It gets unlocked after the lock count goes down to `0`.
		 * @return				`true` = success; `false` = error.
		 */
		bool lock() noexcept;

		/**
		 * @brief				Unlocks this page.
		 * @details				Decrements the page's lock count. When the lock count drops down to `0`, the page's mapped are is sync'd to the disk, and is no longer valid.
		 */
		void unlock() noexcept;

		/**
		 * @brief				Unconditionally invalidates the page.
		 * @details				If the instance had associated resources, they will remain orphan.
		 */
		void invalidate() noexcept;

	protected:
		Pool*						_pool;
		vmem_page_pos_t				_pos;
		void*						_ptr;
		Log*						_log;
	};


	// --------------------------------------------------------------


	/**
	 * @brief					Virtual memory (vmem) pointer.
	 * @tparam T				Type of pointed item.
	 * @tparam Pool				Pool.
	 * @tparam Log				Logging facility.
	 */
	template <typename T, typename Pool, typename Log = null_log>
	class vmem_ptr {
	public:
		/**
		 * @brief				Constructor.
		 * @details				Contains a `vmem_page` instance for the referenced page to keep it locked. 
		 * @param pool			Pointer to a Pool instance.
		 * @param page_pos		Page position.
		 * @param byte_pos		Byte position on the page.
		 * @param log			Pointer to a `log_ostream` instance.
		 */
		vmem_ptr<T, Pool, Log>(Pool* pool, vmem_page_pos_t page_pos, vmem_item_pos_t byte_pos, Log* log = nullptr);

		/**
		 * @brief				Move constructor.
		 */
		vmem_ptr<T, Pool, Log>(vmem_ptr<T, Pool, Log>&& other) noexcept = default;

		/**
		 * @brief				Copy constructor.
		 */
		vmem_ptr<T, Pool, Log>(const vmem_ptr<T, Pool, Log>& other) noexcept = default;

		/**
		 * @brief				Constructor.
		 * @details				Constructs an invalid/null pointer.
		 */
		vmem_ptr<T, Pool, Log>(std::nullptr_t) noexcept;

		/**
		 * @brief				Destructor.
		 */
		~vmem_ptr<T, Pool, Log>() = default;

	public:
		vmem_ptr<T, Pool, Log>&		operator =(const vmem_ptr<T, Pool, Log>& other) noexcept = default;
		vmem_ptr<T, Pool, Log>&		operator =(vmem_ptr<T, Pool, Log>&& other) noexcept = default;

	public:
		/**
		 * @brief				Returns the pointer to the Pool instance passed in to the constructor.
		 */
		Pool* pool() const noexcept;

		/**
		 * @brief				Returns the pointer's page position in the pool.
		 */
		vmem_page_pos_t page_pos() const noexcept;

		/**
		 * @brief				Returns the pointer's byte position on the page.
		 */
		vmem_item_pos_t byte_pos() const noexcept;

		/**
		 * @brief				Returns a typed pointer.
		 */
		operator T*() noexcept;

		/**
		 * @brief				Returns a typed pointer.
		 */
		operator const T*() const noexcept;

		/**
		 * @brief				Returns a typed pointer.
		 */
		T* operator ->() noexcept;

		/**
		 * @brief				Returns a typed pointer.
		 */
		const T* operator ->() const noexcept;

		/**
		 * @brief				Returns a typed reference.
		 */
		T& operator *();

		/**
		 * @brief				Returns a typed reference.
		 */
		const T& operator *() const;

	private:
		/**
		 * @brief				Returns a typed pointer.
		 */
		T* ptr() const noexcept;

		/**
		 * @brief				Returns a typed reference.
		 */
		T& deref() const;

	protected:
		vmem_page<Pool, Log>		_page;
		vmem_item_pos_t				_byte_pos;
		Log*						_log;
	};


	// --------------------------------------------------------------

}
