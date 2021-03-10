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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>

#include "../exception.h"
#include "../i/vmem.i.h"


namespace abc {

	template <std::size_t MaxMappedPages, typename Log>
	inline constexpr std::size_t vmem_pool<MaxMappedPages, Log>::max_mapped_pages() noexcept {
		return MaxMappedPages;
	}


	// ..............................................................


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_pool<MaxMappedPages, Log>::vmem_pool(const char* file_path, Log* log)
		: _ready(false)
		, _mapped_page_count(0)
		, _mapped_pages{ 0 }
		, _mapped_page_totals{ 0 }
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::vmem_pool() Start");
		}

		verify_args_or_throw(file_path);

		bool is_empty;
		open_pool_or_throw(file_path, /*out*/ is_empty);

		if (is_empty) {
			init_pool_or_throw();
		}

		verify_pool_or_throw();

		_ready = true;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10390, "vmem_pool::vmem_pool() Done");
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::verify_args_or_throw(const char* file_path) {
		if (Pool::max_mapped_pages() < vmem_min_mapped_pages) {
			throw exception<std::logic_error, Log>("vmem_pool::verify_args_or_throw<MaxMappedPages>", __TAG__);
		}

		if (file_path == nullptr) {
			throw exception<std::logic_error, Log>("vmem_pool::verify_args_or_throw(file_path)", __TAG__);
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::open_pool_or_throw(const char* file_path, /*out*/ bool& is_empty) {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1037c, "vmem_pool::open_pool_or_throw() Start path='%s'", file_path);
		}

		_fd = open(file_path, O_CREAT | O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1037d, "vmem_pool::open_pool_or_throw() Open fd=%d, errno=%d", _fd, errno);
		}

		if (_fd < 0) {
			throw exception<std::runtime_error, Log>("vmem_pool::open_pool_or_throw() Not found vmem file", 0x1037e, _log);
		}

		vmem_page_pos_t file_size = lseek(_fd, 0, SEEK_END);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1037f, "vmem_pool::open_pool_or_throw() size=0x%llx", (long long)file_size);
		}

		if ((file_size & (vmem_page_size - 1)) != 0) {
			throw exception<std::runtime_error, Log>("vmem_pool::open_pool_or_throw() Corrupt vmem file - size", 0x10380, _log);
		}

		is_empty = (file_size == 0);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::open_pool_or_throw() Done");
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::init_pool_or_throw() {
		// IMPORTANT! Keep this order:
		// root  (0)
		// start (1)
		create_root_page_or_throw();
		create_start_page_or_throw();
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::create_root_page_or_throw() {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10381, "vmem_pool::create_root_page_or_throw() Start");
		}

		vmem_page<Pool, Log> page(this, _log);

		if (page.ptr() == nullptr) {
			throw exception<std::runtime_error, Log>("vmem_pool::create_root_page_or_throw() Insufficient capacity", 0x10382, _log);
		}

		std::memset(page.ptr(), 0, vmem_page_size);

		vmem_root_page init;
		std::memmove(page.ptr(), &init, sizeof(init));

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10383, "vmem_pool::create_root_page_or_throw() Done");
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::create_start_page_or_throw() {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10384, "vmem_pool::create_start_page_or_throw() Start");
		}

		vmem_page<Pool, Log> page(this, _log);

		if (page.ptr() == nullptr) {
			throw exception<std::runtime_error, Log>("vmem_pool::create_start_page_or_throw() Insufficient capacity", 0x10385, _log);
		}

		std::memset(page.ptr(), 0, vmem_page_size);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10386, "vmem_pool::create_start_page_or_throw() Done");
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::verify_pool_or_throw() {
		verify_root_page_or_throw();
		verify_start_page_or_throw();
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::verify_root_page_or_throw() {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10387, "vmem_pool::verify_root_page_or_throw() Start");
		}

		vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

		if (page.ptr() == nullptr) {
			throw exception<std::runtime_error, Log>("vmem_pool::verify_root_page_or_throw() Cannot verify root page", 0x10388, _log);
		}

		vmem_root_page* root_page = reinterpret_cast<vmem_root_page*>(page.ptr());

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10389, "vmem_pool::verify_root_page_or_throw() Root page integrity pos=0x%llx, ptr=%p, version=%u, signature='%s', page_size=%u",
				(long long)page.pos(), page.ptr(), root_page->version, root_page->signature, root_page->page_size);
		}

		vmem_root_page init;

		if (root_page->version != init.version) {
			throw exception<std::runtime_error, Log>("vmem_pool::verify_root_page_or_throw() vmem file integrity - version", 0x1038a, _log);
		}

		if (std::strcmp(root_page->signature, init.signature) != 0) {
			throw exception<std::runtime_error, Log>("vmem_pool::verify_root_page_or_throw() vmem file integrity - signature", 0x1038b, _log);
		}

		if (root_page->page_size != vmem_page_size) {
			throw exception<std::runtime_error, Log>("vmem_pool::verify_root_page_or_throw() vmem file integrity - page_size", 0x1038c, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::verify_root_page_or_throw() Done");
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::verify_start_page_or_throw() {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1038d, "vmem_pool::verify_start_page_or_throw() Start");
		}

		vmem_page<Pool, Log> page(this, vmem_page_pos_start, _log);

		if (page.ptr() == nullptr) {
			throw exception<std::runtime_error, Log>("vmem_pool::verify_start_page_or_throw() Cannot verify start page", 0x1038e, _log);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1038f, "vmem_pool::verify_start_page_or_throw() Start page integrity pos=0x%llx, ptr=%p",
				(long long)page.pos(), page.ptr());
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::verify_start_page_or_throw() Done");
		}
	}


	// ..............................................................


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::alloc_page() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10391, "vmem_pool::alloc_page() Start. ready=%d", _ready);
		}

		vmem_page_pos_t page_pos = vmem_page_pos_nil;

		if (_ready) {
			page_pos = pop_free_page_pos();
		}

		if (page_pos == vmem_page_pos_nil) {
			page_pos = create_page();
		}

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::important, 0x10396, "vmem_pool::alloc_page() Could not create a page on the file.");
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::alloc_page() Done. ready=%d, page_pos=0x%llx", _ready, (long long)page_pos);
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::free_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x10399, "vmem_pool::free_page() ready=%d, page_pos=0x%llx", _ready, (long long)page_pos);
		}

		if (page_pos != vmem_page_pos_nil && _ready) {
			push_free_page_pos(page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::free_page() ready=%d, page_pos=0x%llx", _ready, (long long)page_pos);
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::pop_free_page_pos() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::pop_free_page_pos() Start");
		}

		vmem_page_pos_t page_pos = vmem_page_pos_nil;
		vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10392, "vmem_pool::pop_free_page_pos() Could not check free_pages");
			}
		}
		else {
			vmem_root_page* root_page = reinterpret_cast<vmem_root_page*>(page.ptr());

			vmem_linked<Pool, Log> free_pages_linked(&root_page->free_pages, this, _log);

			if (!free_pages_linked.empty()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x10393, "vmem_pool::pop_free_page_pos() Non-empty");
				}

				page_pos = free_pages_linked.back();
				free_pages_linked.pop_back();

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x10394, "vmem_pool::pop_free_page_pos() Found free page. page_pos=0x%llx", (long long)page_pos);
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::pop_free_page_pos() Done. page_pos=0x%llx", (long long)page_pos);
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::push_free_page_pos(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::push_free_page_pos() Start. page_pos=0x%llx", (long long)page_pos);
		}

		vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x1039a, "vmem_pool::push_free_page_pos() Could not add to free_pages");
			}
		}
		else {
			vmem_root_page* root_page = reinterpret_cast<vmem_root_page*>(page.ptr());

			vmem_linked<Pool, Log> free_pages_linked(&root_page->free_pages, this, _log);

			free_pages_linked.push_back(page_pos);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::push_free_page_pos() Done. page_pos=0x%llx", (long long)page_pos);
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::create_page() noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::create_page() Start");
		}

		vmem_page_pos_t page_off = lseek(_fd, 0, SEEK_END);
		vmem_page_pos_t page_pos = page_off / vmem_page_size;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x10397, "vmem_pool::create_page() pos=0x%llx off=0x%llx",
				(long long)page_pos, (long long)page_off);
		}

		std::uint8_t blank_page[vmem_page_size] = { 0 };
		ssize_t wb = write(_fd, blank_page, vmem_page_size);

		if (wb != vmem_page_size) {
			page_pos = vmem_page_pos_nil;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10398, "vmem_pool::create_page() wb=%l, errno=%d", (long)wb, errno);
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::create_page() Done. page_pos=0x%llx", (long long)page_pos);
		}

		return page_pos;
	}


	// ..............................................................


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::lock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, 0x1039b, "vmem_pool::lock_page() Start. page_pos=0x%llx", (long long)page_pos);
		}

		std::size_t i;
		bool is_found = find_mapped_page(page_pos, /*out*/ i);

		if (!is_found) {
			// The page is not mapped. We'll have to map it. We need capacity for that.
			if (!has_mapping_capacity()) {
				make_mapping_capacity();
			}

			if (has_mapping_capacity()) {
				// There is capacity to map one more page.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a4, "vmem_pool::lock_page() Capacity _mapped_page_count=%zu", _mapped_page_count);
				}

				i = _mapped_page_count;
			}
			else {
				// All the maped pages are locked. We cannot find a slot for the new page.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, 0x103a5, "vmem_pool::lock_page() Insufficient capacity. MaxedMappedPages=%zu", MaxMappedPages);
				}

				return nullptr;
			}
		} // Not found

		void* page_ptr = nullptr; 

		if (i < _mapped_page_count) {
			// The page is already mapped. Only re-lock it.
			page_ptr = lock_mapped_page(i);
		}
		else {
			// The page is not mapped. Map it. Then lock it.
			page_ptr = map_new_page(i, page_pos);
		}

		// Optimization: Swap this page forward (once) to keep pages sorted by keep_count.
		optimize_mapped_page(i);

		log_totals();

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::lock_page() Done. page_pos=0x%llx, ptr=%p", (long long)page_pos, page_ptr);
		}

		return page_ptr;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::unlock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103aa, "vmem_pool::unlock_page() pos=0x%llx", (long long)page_pos);
		}

		_mapped_page_totals.unlock_count++;

		std::size_t i;
		bool is_found = find_mapped_page(page_pos, /*out*/ i);

		if (is_found) {
			unlock_mapped_page(i);
		}
		else {
			// The page was not found. This is a logic error.
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x103ad, "vmem_pool::unlock_page() Trying to unlock a page that is not locked. page_pos=0x%llx",
					(long long)page_pos);
			}

			return false;
		}

		log_totals();

		return true;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::find_mapped_page(vmem_page_pos_t page_pos, /*out*/ std::size_t& i) noexcept {
		bool is_found = false;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::find_mapped_page() Start. page_pos=0x%llx", (long long)page_pos);
		}

		for (i = 0; i < _mapped_page_count; i++) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::find_mapped_page() Examine i=%zu pos=0x%llx, lock_count=%d, keep_count=%d, ptr=%p",
					i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count, (unsigned)_mapped_pages[i].keep_count, _mapped_pages[i].ptr);
			}

			if (_mapped_pages[i].pos == page_pos) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::find_mapped_page() Found i=%zu pos=0x%llx, lock_count=%d, keep_count=%d, ptr=%p",
						i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count, (unsigned)_mapped_pages[i].keep_count, _mapped_pages[i].ptr);
				}

				is_found = true;
				break;
			}
		}

		// Update the totals with the cost of this attempt.
		_mapped_page_totals.check_count += i + 1;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::find_mapped_page() Done. page_pos=0x%llx, i=%zu", (long long)page_pos, i);
		}

		return is_found;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::has_mapping_capacity() noexcept {
		return _mapped_page_count < MaxMappedPages;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline std::size_t vmem_pool<MaxMappedPages, Log>::make_mapping_capacity() noexcept {
		// Record this run in the totals.
		_mapped_page_totals.unmap_count++;

		// Since this process is not cheap - it requires a scan of all the pages - we don't unmap a single page per run.
		// Instead, we unmap as many pages that match a certain condition.  
		// This is to to avoid doing this process too frequently while still keeping frequently used pages mapped.
		// To enforce some fairness, we subtract the min_keep_count from the keep_count of each page that will be kept.

		// First try to unmap the pages with a keep_count below the average.
		vmem_page_hit_count_t avg_keep_count = _mapped_page_totals.keep_count / _mapped_page_count;
		std::size_t unmapped_count = make_mapping_capacity(avg_keep_count);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::make_mapping_capacity() First attempt. unmapped_count=%zu", unmapped_count);
		}

		// If the attempt makes some capacity, we are done.
		if (unmapped_count > 0) {
			return unmapped_count;
		}

		// If the first attempt, doesn't make any capacity, we try to unmap all pages that are not locked.
		avg_keep_count = _mapped_page_totals.keep_count  + 1;
		unmapped_count = make_mapping_capacity(avg_keep_count);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::make_mapping_capacity() Second attempt. unmapped_count=%zu", unmapped_count);
		}

		return unmapped_count;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline std::size_t vmem_pool<MaxMappedPages, Log>::make_mapping_capacity(vmem_page_hit_count_t min_keep_count) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x1039d, "vmem_pool::make_mapping_capacity() Start. min_keep_count=%u, mapped_page_count=%zu",
				(unsigned)min_keep_count, _mapped_page_count);
		}

		std::size_t unmapped_count = 0;
		std::size_t empty_i = MaxMappedPages;

		for (std::size_t i = 0; i < _mapped_page_count; i++) {
			if (should_keep_mapped_page(i, min_keep_count)) {
				keep_mapped_page(i, min_keep_count, /*out*/ empty_i);
			}
			else {
				unmap_mapped_page(i, min_keep_count, /*out*/ empty_i, /*inout*/ unmapped_count);
			}
		}

		// Update the mapped page count.
		_mapped_page_count -= unmapped_count;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::make_mapping_capacity() Done. min_keep_count=%u, mapped_page_count=%zu, unmapped_count=%zu",
				(unsigned)min_keep_count, _mapped_page_count, unmapped_count);
		}

		return unmapped_count;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::should_keep_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count) noexcept {
		return _mapped_pages[i].lock_count > 0 || _mapped_pages[i].keep_count > min_keep_count;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::keep_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count, /*inout*/ std::size_t& empty_i) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1039e, "vmem_pool::keep_mapped_page() Start. i=%zu, pos=0x%llx, keep_count=%u, min_keep_count=%u",
				i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)min_keep_count);
		}

		// Reduce the keep_count for fairness.
		if (_mapped_pages[i].keep_count > min_keep_count) {
			_mapped_page_totals.keep_count -= min_keep_count;
			_mapped_pages[i].keep_count -= min_keep_count;
		}
		else {
			_mapped_page_totals.keep_count -= _mapped_pages[i].keep_count;
			_mapped_pages[i].keep_count = 0;
		}

		// If there is already an empty slot, we move this element there, and zero out this slot.
		if (empty_i < _mapped_page_count) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1039f, "vmem_pool::keep_mapped_page() Moving page empty_i=%zu, i=%zu, pos=0x%llx",
					empty_i, i, (long long)_mapped_pages[i].pos);
			}

			// Move the element to the empty slot.
			_mapped_pages[empty_i] = _mapped_pages[i];

			// Zero out this slot.
			_mapped_pages[i] = { 0 };
			_mapped_pages[i].ptr = nullptr;

			// Find the next empty slot.
			empty_i = next_empty_i(i, empty_i);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::keep_mapped_page() Done. i=%zu, pos=0x%llx, keep_count=%u, min_keep_count=%u",
				i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)min_keep_count);
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::unmap_mapped_page(std::size_t i, vmem_page_hit_count_t min_keep_count, /*out*/ std::size_t& empty_i, /*inout*/ std::size_t& unmapped_count) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a0, "vmem_pool::unmap_mapped_page() Start. i=%zu, pos=0x%llx, keep_count=%u, min_keep_count=%u",
				i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)min_keep_count);
		}

		// Unmap the OS page.
		int um = munmap(_mapped_pages[i].ptr, vmem_page_size);
		void* ptr = _mapped_pages[i].ptr;

		// Zero out the slot.
		_mapped_pages[i] = { 0 };
		_mapped_pages[i].ptr = nullptr;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a1, "vmem_pool::unmap_mapped_page() Unmap. i=%zu, ptr=%p, um=%d, errno=%d", i, ptr, um, errno);
		}

		// If this is the first unmapped page, set empty_i.
		if (unmapped_count++ == 0) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a2, "vmem_pool::unmap_mapped_page() First empty slot i=%zu", i);
			}

			empty_i = i;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::unmap_mapped_page() Done. i=%zu", i);
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::lock_mapped_page(std::size_t i) noexcept {
		void* ptr = _mapped_pages[i].ptr;

		// Update the slot.
		_mapped_pages[i].lock_count++;
		_mapped_pages[i].keep_count++;

		// Update the stats.
		_mapped_page_totals.keep_count++;
		_mapped_page_totals.hit_count++;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a6, "vmem_pool::lock_mapped_page() i=%zu, pos=0x%llx, lock_count=%d",
				i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count);
		}

		return ptr;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::unlock_mapped_page(std::size_t i) noexcept {
		_mapped_pages[i].lock_count--;

		if (_mapped_pages[i].lock_count == 0) {
			// When all locks on a page are released, we sync the OS page. 
			int sn = msync(_mapped_pages[i].ptr, vmem_page_size, MS_ASYNC);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x103ab, "vmem_pool::unlock_mapped_page() msync i=%zu pos=0x%llx, ptr=%p, lock_count=%d, sn=%d, errno=%d",
					i, (long long)_mapped_pages[i].pos, _mapped_pages[i].ptr, (unsigned)_mapped_pages[i].lock_count, sn, errno);
			}
		}
		else {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x103ac, "vmem_pool::unlock_mapped_page() Used. i=%zu pos=0x%llx, ptr=%p, lock_count=%d",
					i, (long long)_mapped_pages[i].pos, _mapped_pages[i].ptr, (unsigned)_mapped_pages[i].lock_count);
			}
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::map_new_page(std::size_t i, vmem_page_pos_t page_pos) noexcept {
		// Map the OS page.
		off_t page_off = static_cast<off_t>(page_pos * vmem_page_size);
		void* ptr = mmap(NULL, vmem_page_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, page_off);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a7, "vmem_pool::map_new_page() mmap i=%zu, pos=0x%llx, lock_count=1, ptr=%p, errno=%d",
				i, (long long)page_pos, ptr, errno);
		}

		_mapped_page_count++;

		// Set the slot.
		_mapped_pages[i].pos = page_pos;
		_mapped_pages[i].ptr = ptr;
		_mapped_pages[i].lock_count = 1;
		_mapped_pages[i].keep_count = 1;

		// Update the stats.
		_mapped_page_totals.keep_count++;
		_mapped_page_totals.miss_count++;

		return ptr;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::optimize_mapped_page(std::size_t i) noexcept {
		// The goal is to keep pages with a high keep_count close to the front.
		for (std::size_t j = 0; j < i; j++) {
			if (_mapped_pages[j].keep_count < _mapped_pages[i].keep_count) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a8, "vmem_pool::optimize_mapped_page() Swapping j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
						j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
				}

				// Swap.
				_vmem_mapped_page temp = _mapped_pages[j];
				_mapped_pages[j] = _mapped_pages[i];
				_mapped_pages[i] = temp;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a9, "vmem_pool::optimize_mapped_page() Swapped  j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
						j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
				}

				break;
			}
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline std::size_t vmem_pool<MaxMappedPages, Log>::next_empty_i(std::size_t i, std::size_t empty_i) noexcept {
		std::size_t next_empty_i = empty_i + 1;

		while (next_empty_i < i && _mapped_pages[next_empty_i].ptr != nullptr) {
			next_empty_i++;
		}

		return next_empty_i;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::log_totals() noexcept {
		if (_log != nullptr) {
			vmem_page_hit_count_t total_lock_count = _mapped_page_totals.hit_count + _mapped_page_totals.miss_count;
			vmem_page_hit_count_t hit_percent = (_mapped_page_totals.hit_count * 100) / total_lock_count;
			vmem_page_hit_count_t miss_percent = (_mapped_page_totals.miss_count * 100) / total_lock_count;

			vmem_page_hit_count_t total_lookup_count = _mapped_page_totals.hit_count + _mapped_page_totals.miss_count + _mapped_page_totals.unlock_count;
			vmem_page_hit_count_t check_factor_x10 = (_mapped_page_totals.check_count * 10) / total_lookup_count;
			vmem_page_hit_count_t check_factor_percent = (check_factor_x10 * 10) / MaxMappedPages;

			_log->put_any(category::abc::vmem, severity::abc::optional, 0x103ae, "vmem_pool::log_totals() Pool Totals hits=%u (%u%%), misses=%u (%u%%), checks=%u (%u.%u, %u%%)", 
				(unsigned)_mapped_page_totals.hit_count, (unsigned)hit_percent,
				(unsigned)_mapped_page_totals.miss_count, (unsigned)miss_percent,
				(unsigned)_mapped_page_totals.check_count, (unsigned)(check_factor_x10 / 10), (unsigned)(check_factor_x10 % 10), (unsigned)check_factor_percent);
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::clear_linked(/*inout*/ vmem_linked<Pool, Log>& linked) {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::clear_linked() Start");
		}

		vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

		if (page.ptr() == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_pool::clear_linked() Could not check free_pages");
			}
		}
		else {
			vmem_root_page* root_page = reinterpret_cast<vmem_root_page*>(page.ptr());

			vmem_linked<Pool, Log> free_pages_linked(&root_page->free_pages, this, _log);
			free_pages_linked.splice(/*inout*/linked);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::clear_linked() Done.");
		}
	}


	// --------------------------------------------------------------


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(Pool* pool, Log* log)
		: vmem_page<Pool, Log>(pool, vmem_page_pos_nil, log) {
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(Pool* pool, vmem_page_pos_t page_pos, Log* log)
		: _pool(pool)
		, _pos(page_pos)
		, _ptr(nullptr)
		, _log(log) {

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("vmem_page::vmem_page(pool)", 0x103af);
		}

		if (page_pos == vmem_page_pos_nil) {
			if (!alloc()) {
				return;
			}
		}

		lock();
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(const vmem_page<Pool, Log>& other) noexcept
		: _pool(other._pool)
		, _pos(other._pos)
		, _ptr(other._ptr)
		, _log(other._log) {

		if (_pool != nullptr && _pos != vmem_page_pos_nil) {
			lock();
		}
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(vmem_page<Pool, Log>&& other) noexcept
		: _pool(other._pool)
		, _pos(other._pos)
		, _ptr(other._ptr)
		, _log(other._log) {

		other.invalidate();
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(nullptr_t) noexcept
		: _pool(nullptr)
		, _pos(vmem_page_pos_nil)
		, _ptr(nullptr)
		, _log(nullptr) {
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::~vmem_page() noexcept {
		unlock();
		invalidate();
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>& vmem_page<Pool, Log>::operator =(const vmem_page<Pool, Log>& other) noexcept {
		unlock();

		_pool = other._pool;
		_pos = other._pos;
		_ptr = other._ptr;
		_log = other._log;

		if (_pool != nullptr && _pos != vmem_page_pos_nil) {
			lock();
		}

		return *this;
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>& vmem_page<Pool, Log>::operator =(vmem_page<Pool, Log>&& other) noexcept {
		unlock();

		_pool = other._pool;
		_pos = other._pos;
		_ptr = other._ptr;
		_log = other._log;

		other.invalidate();

		return *this;
	}


	template <typename Pool, typename Log>
	inline bool vmem_page<Pool, Log>::alloc() noexcept {
		_pos = _pool->alloc_page();

		if (_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x103b0, "vmem_page::alloc() _pos=nil");
			}

			return false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103b1, "vmem_page::alloc() _pos=0x%llx", (long long)_pos);
		}

		return true;
	}


	template <typename Pool, typename Log>
	inline void vmem_page<Pool, Log>::free() noexcept {
		unlock();

		if (_pos != vmem_page_pos_nil) {
			_pool->free_page(_pos);
		}

		invalidate();
	}


	template <typename Pool, typename Log>
	inline bool vmem_page<Pool, Log>::lock() noexcept {
		_ptr = _pool->lock_page(_pos);

		if (_ptr == nullptr) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x103b2, "vmem_page::lock() _pos=0x%llx, _ptr=nullptr", (long long)_pos);
			}

			return false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103b3, "vmem_page::lock() _pos=0x%llx, _ptr=%p", (long long)_pos, _ptr);
		}

		return  true;
	}


	template <typename Pool, typename Log>
	inline void vmem_page<Pool, Log>::unlock() noexcept {
		if (_pool != nullptr && _pos != vmem_page_pos_nil && _ptr != nullptr)
		{
			_pool->unlock_page(_pos);
			_ptr = nullptr;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x103b4, "vmem_page::unlock() _pos=0x%llx", (long long)_pos);
			}
		}
	}


	template <typename Pool, typename Log>
	inline void vmem_page<Pool, Log>::invalidate() noexcept {
		_pool = nullptr;
		_pos = vmem_page_pos_nil;
		_ptr = nullptr;
		_log = nullptr;
	}


	template <typename Pool, typename Log>
	inline Pool* vmem_page<Pool, Log>::pool() const noexcept {
		return _pool;
	}


	template <typename Pool, typename Log>
	inline vmem_page_pos_t vmem_page<Pool, Log>::pos() const noexcept {
		return _pos;
	}


	template <typename Pool, typename Log>
	inline void* vmem_page<Pool, Log>::ptr() noexcept {
		return _ptr;
	}


	template <typename Pool, typename Log>
	inline const void* vmem_page<Pool, Log>::ptr() const noexcept {
		return _ptr;
	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log>::vmem_ptr(Pool* pool, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, Log* log)
		: _page(page_pos != vmem_page_pos_nil ? vmem_page<Pool, Log>(pool, page_pos, log) : vmem_page<Pool, Log>(nullptr))
		, _item_pos(item_pos)
		, _log(log) {
	}


	template <typename T, typename Pool, typename Log>
	inline Pool* vmem_ptr<T, Pool, Log>::pool() const noexcept {
		return _page.pool();
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_page_pos_t vmem_ptr<T, Pool, Log>::page_pos() const noexcept {
		return _page.pos();
	}


	template <typename T, typename Pool, typename Log>
	vmem_item_pos_t vmem_ptr<T, Pool, Log>::item_pos() const noexcept {
		return _item_pos;
	}


	template <typename T, typename Pool, typename Log>
	vmem_ptr<T, Pool, Log>::operator T*() noexcept {
		return ptr();
	}


	template <typename T, typename Pool, typename Log>
	vmem_ptr<T, Pool, Log>::operator const T*() const noexcept {
		return ptr();
	}


	template <typename T, typename Pool, typename Log>
	T* vmem_ptr<T, Pool, Log>::operator ->() noexcept {
		return ptr();
	}


	template <typename T, typename Pool, typename Log>
	const T* vmem_ptr<T, Pool, Log>::operator ->() const noexcept {
		return ptr();
	}


	template <typename T, typename Pool, typename Log>
	T& vmem_ptr<T, Pool, Log>::operator *() {
		return deref();
	}


	template <typename T, typename Pool, typename Log>
	const T& vmem_ptr<T, Pool, Log>::operator *() const {
		return deref();
	}


	template <typename T, typename Pool, typename Log>
	T* vmem_ptr<T, Pool, Log>::ptr() const noexcept {
		char* page_ptr = reinterpret_cast<char*>(const_cast<void*>(_page.ptr()));

		if (page_ptr == nullptr || _item_pos == vmem_item_pos_nil) {
			return nullptr;
		}

		return reinterpret_cast<T*>(page_ptr + _item_pos);
	}


	template <typename T, typename Pool, typename Log>
	T& vmem_ptr<T, Pool, Log>::deref() const {
		T* p = ptr();

		if (p == nullptr) {
			throw exception<std::runtime_error, Log>("vmem_ptr::deref() Dereferencing invalid vmem_ptr", 0x103b5);
		}

		return *p;
	}


	// --------------------------------------------------------------

}
