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

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "vmem.i.h"
#include "exception.h"


namespace abc {

	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_pool<MaxMappedPages, Log>::vmem_pool(const char* file_path, Log* log)
		: _mapped_page_count(0)
		, _mapped_pages{ 0 }
		, _mapped_page_totals{ 0 }
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() Open path='%s'", file_path);
		}

		_fd = open(file_path, O_CREAT | O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() Open fd=%d, errno=%d", _fd, errno);
		}

		if (_fd < 0) {
			throw exception<std::runtime_error, Log>("Not found vmem file", __TAG__, _log);
		}

		vmem_page_pos_t file_size = lseek(_fd, 0, SEEK_END);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() size=%llu", (unsigned long long)file_size);
		}

		if ((file_size & (vmem_page_size - 1)) != 0) {
			throw exception<std::runtime_error, Log>("Corrupt vmem file - size", __TAG__, _log);
		}

		// The file is empty...
		if (file_size == 0) {
			// Create and init the root page (0).
			{
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Creating root page");
				}

				vmem_page<vmem_pool<MaxMappedPages, Log>, Log> page(this, _log);

				std::memset(page.ptr(), 0, vmem_page_size);

				_vmem_root_page init;
				std::memmove(page.ptr(), &init, sizeof(init));

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Root page created");
				}
			}

			// Create and init the start page (1).
			{
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Creating start page");
				}

				vmem_page<vmem_pool<MaxMappedPages, Log>, Log> page(this, _log);

				std::memset(page.ptr(), 0, vmem_page_size);

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Start page created");
				}
			}
		}

		// Verify integrity of the root page (0).
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Verifying root page integrity");
			}

			vmem_page<vmem_pool<MaxMappedPages, Log>, Log> page(this, vmem_page_pos_root, _log);

			_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() Root page integrity pos=%llu, ptr=%p, version=%u, signature='%s', page_size=%u",
					(unsigned long long)page.pos(), page.ptr(), root_page->version, root_page->signature, root_page->page_size);
			}

			_vmem_root_page init;

			if (root_page->version != init.version) {
				throw exception<std::runtime_error, Log>("vmem file integrity - version", __TAG__, _log);
			}

			if (std::strcmp(root_page->signature, init.signature) != 0) {
				throw exception<std::runtime_error, Log>("vmem file integrity - signature", __TAG__, _log);
			}

			if (root_page->page_size != vmem_page_size) {
				throw exception<std::runtime_error, Log>("vmem file integrity - page_size", __TAG__, _log);
			}
		}

		// Verify the start page is loadable.
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Verifying start page integrity");
			}

			vmem_page<vmem_pool<MaxMappedPages, Log>, Log> page(this, vmem_page_pos_start, _log);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() Start page integrity pos=%llu, ptr=%p", (unsigned long long)page.pos(), page.ptr());
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Verified");
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::alloc_page() noexcept {
		vmem_page_pos_t page_pos;

		//// TODO: First, check the free pages.
		page_pos = vmem_page_pos_nil;

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::alloc_page() No free pages. Creating...");
			}

			page_pos = create_page();
		}

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_pool::alloc_page() Could not create a page on the file.");
			}
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::create_page() noexcept {
		vmem_page_pos_t page_off = lseek(_fd, 0, SEEK_END);
		vmem_page_pos_t page_pos = page_off / vmem_page_size;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::create_page() pos=%llu off=%llu", (unsigned long long)page_pos, (unsigned long long)page_off);
		}

		std::uint8_t blank_page[vmem_page_size] = { 0 };
		ssize_t wb = write(_fd, blank_page, vmem_page_size);

		if (wb != vmem_page_size) {
			page_pos = vmem_page_pos_nil;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::create_page() wb=%l, errno=%d", (long)wb, errno);
			}
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::free_page(vmem_page_pos_t page_pos) noexcept {
		//// TODO: Add to the list of free pages.
		return true;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::lock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Start pos=%llu", (unsigned long long)page_pos);
		}

		// Try to find the page among the mapped pages.
		std::size_t i;
		for (i = 0; i < _mapped_page_count; i++) {
			if (_mapped_pages[i].pos == page_pos) {
				break;
			}
		}

		_mapped_page_totals.check_count += i + 1;

		if (i >= _mapped_page_count) {
			// The page was not found.

			// Check if there is capacity for one more mapped page.
			if (_mapped_page_count >= MaxMappedPages) {
				// There is no more capacity for mapped pages.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Not found. No capacity.");
				}

				_mapped_page_totals.unmap_count++;

				// Try to unmap some mapped pages that aren't locked.
				// We unmap all the mapped pages whose keep_count is below the average.
				// This is to to avoid doing this process too frequently while still keeping frequently used pages mapped.
				// To enforce some fairness, we subtract the avg_keep_count from the keep_count of each page that will be kept.
				vmem_page_hit_count_t avg_keep_count = _mapped_page_totals.keep_count / _mapped_page_count;
				for (std::size_t attempt = 0; attempt < 2; attempt++) {
					bool has_empty_pos = false;
					std::size_t empty_pos = MaxMappedPages;
					for (i = 0; i < _mapped_page_count; i++) {
						if (_mapped_pages[i].lock_count > 0 || _mapped_pages[i].keep_count > avg_keep_count) {
							// This page will be kept.
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Keeping page i=%zu, keep_count=%u, avg_keep_count=%u", i, (unsigned)_mapped_pages[i].keep_count, (unsigned)avg_keep_count);
							}

							// Reduce the keep_count for fairness.
							if (attempt == 0 && _mapped_pages[i].keep_count > avg_keep_count) {
								_mapped_page_totals.keep_count -= avg_keep_count;
								_mapped_pages[i].keep_count -= avg_keep_count;
							}
							else {
								_mapped_page_totals.keep_count -= _mapped_pages[i].keep_count;
								_mapped_pages[i].keep_count = 0;
							}

							// If there is an empty slot, we move this element there.
							if (has_empty_pos) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Moving page empty_pos=%zu, i=%zu", empty_pos, i);
								}

								_mapped_pages[empty_pos++] = _mapped_pages[i];
							}
						}
						else {
							// This page will be unmapped.
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Unmapping page i=%zu, keep_count=%u, avg_keep_count=%u", i, _mapped_pages[i].keep_count, avg_keep_count);
							}

							int um = munmap(_mapped_pages[i].ptr, vmem_page_size);

							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::lock_page() Unmap i=%u, ptr=%p, um=%d, errno=%d", i, _mapped_pages[i].ptr, um, errno);
							}

							if (!has_empty_pos) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() First empty slot i=%zu", i);
								}

								has_empty_pos = true;
								empty_pos = i;
							}
						}
					} // for (i)

					// empty_pos is the new _mapped_page_count.
					if (has_empty_pos) {
						// The first attempt was successful - we were able to free up some capacity.
						_mapped_page_count = empty_pos;

						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Compacted. _mapped_page_count=%zu", _mapped_page_count);
						}

						break;
					}
					else {
						// The first attempt failed.
						// In the second attempt, we'll try to free all pages that aren't locked.
						avg_keep_count = _mapped_page_totals.keep_count + 1;
					}
				} // for (attempts)
			} // No capacity

			if (_mapped_page_count < MaxMappedPages) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Capacity _mapped_page_count=%zu", _mapped_page_count);
				}

				i = _mapped_page_count;
			}
			else {
				// All the maped pages are locked. We cannot find a slot for the new page.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_pool::lock_page() Insufficient capacity. MaxedMappedPages=%zu", MaxMappedPages);
				}

				return nullptr;
			}
		} // Not found

		// We have a slot where we should lock the page.
		_vmem_mapped_page& mapped_page = _mapped_pages[i];

		if (i < _mapped_page_count) {
			// The page is already mapped. Only re-lock it.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Found at i=%zu", i);
			}

			mapped_page.lock_count++;
			mapped_page.keep_count++;

			_mapped_page_totals.keep_count++;
			_mapped_page_totals.hit_count++;
		}
		else {
			// The page is not mapped. Map it. Then lock it.

			off_t page_off = static_cast<off_t>(page_pos * vmem_page_size);
			void* ptr = mmap(NULL, vmem_page_size, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_LOCKED, _fd, page_off);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::lock_page() Map i=%zu, pos=%zu, ptr=%p, errno=%d", i, page_pos, ptr, errno);
			}

			_mapped_page_count++;

			mapped_page.pos = page_pos;
			mapped_page.ptr = ptr;
			mapped_page.lock_count = 1;
			mapped_page.keep_count = 1;

			_mapped_page_totals.keep_count++;
			_mapped_page_totals.miss_count++;
		}

		// Optimization: Swap this page forward (once) to keep them sorted by keep_count.
		for (std::size_t j = 0; j < i; j++) {
			if (_mapped_pages[j].keep_count < _mapped_pages[i].keep_count) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Swapping j=%zu, i=%zu", j, i);
				}

				// Swap.
				_vmem_mapped_page temp = _mapped_pages[j];
				_mapped_pages[j] = _mapped_pages[i];
				_mapped_pages[i] = temp;

				break;
			}
		}

		log_totals();

		return mapped_page.ptr;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::unlock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::unlock_page() pos=%llu", (unsigned long long)page_pos);
		}

		_mapped_page_totals.unlock_count++;

		// Try to find the page among the mapped pages.
		std::size_t i;
		for (i = 0; i < _mapped_page_count; i++) {
			if (_mapped_pages[i].pos == page_pos) {
				break;
			}
		}

		_mapped_page_totals.check_count += i + 1;

		if (i < _mapped_page_count) {
			// The page was found.
			_vmem_mapped_page& mapped_page = _mapped_pages[i];
			mapped_page.lock_count--;

			if (mapped_page.lock_count == 0) {
				int sn = msync(mapped_page.ptr, vmem_page_size, MS_ASYNC);

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::unlock_page() Sync i=%zu, ptr=%p, sn=%d, errno=%d", i, mapped_page.ptr, sn, errno);
				}
			}
			else {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::unlock_page() Found at i=%zu, ptr=%p, locks=%u", i, mapped_page.ptr, (unsigned)mapped_page.lock_count);
				}
			}
		}
		else {
			// The page was not found. This is a logic error.
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_pool::unlock_page() Trying to unlock a page that is not locked. page_pos=%llu", (unsigned long long)page_pos);
			}

			return false;
		}

		log_totals();

		return true;
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

			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::log_totals() Pool Totals hits=%u (%u%%), misses=%u (%u%%), checks=%u (%u.%u, %u%%)", 
				(unsigned)_mapped_page_totals.hit_count, (unsigned)hit_percent,
				(unsigned)_mapped_page_totals.miss_count, (unsigned)miss_percent,
				(unsigned)_mapped_page_totals.check_count, (unsigned)(check_factor_x10 / 10), (unsigned)(check_factor_x10 % 10), (unsigned)check_factor_percent);
		}
	}


	// --------------------------------------------------------------


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(Pool* pool, Log* log)
		: _pool(pool)
		, _pos(vmem_page_pos_nil)
		, _ptr(nullptr)
		, _log(log) {

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("pool", __TAG__);
		}

		if (!alloc()) {
			return;
		}

		lock();
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(Pool* pool, vmem_page_pos_t page_pos, Log* log)
		: _pool(pool)
		, _pos(page_pos)
		, _ptr(nullptr)
		, _log(log) {

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("pool", __TAG__);
		}

		if (page_pos != vmem_page_pos_nil) {
			lock();
		}
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
	inline vmem_page<Pool, Log>::~vmem_page() noexcept {
		unlock();
		invalidate();
	}


	template <typename Pool, typename Log>
	inline bool vmem_page<Pool, Log>::alloc() noexcept {
		_pos = _pool->alloc_page();

		if (_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_page::alloc() _pos=nil");
			}

			return false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::alloc() _pos=%llu", (unsigned long long)_pos);
		}

		return true;
	}


	template <typename Pool, typename Log>
	inline void vmem_page<Pool, Log>::free() noexcept {
		unlock();

		if (_pos == vmem_page_pos_nil) {
			_pos = _pool->free_page();

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::free() _pos=%llu", (unsigned long long)_pos);
			}
		}

		invalidate();
	}


	template <typename Pool, typename Log>
	inline bool vmem_page<Pool, Log>::lock() noexcept {
		_ptr = _pool->lock_page(_pos);

		if (_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_page::lock() _pos=%llu, _ptr=nullptr", (unsigned long long)_pos);
			}

			return false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::lock() _pos=%llu, _ptr=%p", (unsigned long long)_pos, _ptr);
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
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::unlock() _pos=%llu", (unsigned long long)_pos);
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
		: _page(pool, page_pos)
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
		char* page_ptr = reinterpret_cast<char*>(_page.ptr());

		if (page_ptr == nullptr || _item_pos == vmem_item_pos_nil) {
			return nullptr;
		}

		return reinterpret_cast<T*>(page_ptr + _item_pos);
	}


	template <typename T, typename Pool, typename Log>
	T& vmem_ptr<T, Pool, Log>::deref() const {
		T* ptr = ptr();

		if (ptr == nullptr) {
			throw exception<std::runtime_error, Log>("Dereferencing invalid vmem_ptr", __TAG__);
		}

		return *ptr();
	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>::vmem_list_iterator(const vmem_list<T, Pool, Log>* list, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, Log* log)
		: _list(list)
		, _page_pos(page_pos)
		, _item_pos(item_pos)
		, _log(log) {

		if (list == nullptr) {
			throw exception<std::logic_error, Log>("list", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list_iterator::vmem_list_iterator() _page_pos=%llu, _item_pos=%u", (unsigned long long)_page_pos, (unsigned)_item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline const vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator =(const vmem_list_iterator<T, Pool, Log>& other) noexcept {
		_list = other._list;
		_page_pos = other._page_pos;
		_item_pos = other._item_pos;
		_log = other._log;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list_iterator::operator =() _page_pos=%llu, _item_pos=%u", (unsigned long long)_page_pos, (unsigned)_item_pos);
		}

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list_iterator<T, Pool, Log>::operator !=(const vmem_list_iterator<T, Pool, Log>& other) const noexcept {
		return _list != other._list
			|| _page_pos != other._page_pos
			|| _item_pos != other._item_pos;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator ++() noexcept {
		_list->move_next(this);

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator --() noexcept {
		_list->move_prev(this);

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_list_iterator<T, Pool, Log>::operator ->() noexcept {
		return ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline const vmem_ptr<T, Pool, Log> vmem_list_iterator<T, Pool, Log>::operator ->() const noexcept {
		return ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline T& vmem_list_iterator<T, Pool, Log>::operator *() {
		return deref();
	}


	template <typename T, typename Pool, typename Log>
	inline const T& vmem_list_iterator<T, Pool, Log>::operator *() const {
		return deref();
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_list_iterator<T, Pool, Log>::ptr() noexcept {
		return _list->at(this);
	}


	template <typename T, typename Pool, typename Log>
	inline T& vmem_list_iterator<T, Pool, Log>::deref() {
		T* t = ptr();

		if (t == nullptr) {
			throw exception<std::runtime_error, Log>("Dereferencing invalid iterator", __TAG__);
		}

		return *t;
	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	inline vmem_list<T, Pool, Log>::vmem_list(vmem_list_state* state, Pool* pool, Log* log)
		: _state(state)
		, _front_item_pos(vmem_item_pos_nil)
		, _back_item_pos(vmem_item_pos_nil)
		, _pool(pool)
		, _log(log) {

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("state", __TAG__);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("pool", __TAG__);
		}

		if (_state->front_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, _state->front_page_pos, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());
			if (list_page->item_count > 0) {
				_front_item_pos = 0;
			}
		}

		if (_state->back_page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());
			_back_item_pos = list_page->item_count - 1;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::vmem_list() front_page_pos=%llu, front_item_pos=%u, front_page_pos=%llu, front_item_pos=%u",
				(unsigned long long)_state->front_page_pos, (unsigned)_front_item_pos, (unsigned long long)_state->back_page_pos, (unsigned)_back_item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::move_next(iterator* itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_next() Before _page_pos=%llu, _item_pos=%u", (unsigned long long)itr->_page_pos, (unsigned)itr->_item_pos);
		}

		//// TODO:

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_next() After _page_pos=%llu, _item_pos=%u", (unsigned long long)itr->_page_pos, (unsigned)itr->_item_pos);
		}

		return false; //// TODO:
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::move_prev(iterator* itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_next() Before _page_pos=%llu, _item_pos=%u", (unsigned long long)itr->_page_pos, (unsigned)itr->_item_pos);
		}

		//// TODO:

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_next() After _page_pos=%llu, _item_pos=%u", (unsigned long long)itr->_page_pos, (unsigned)itr->_item_pos);
		}

		return false; //// TODO:
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_list<T, Pool, Log>::at(const iterator* itr) const noexcept {
		return vmem_ptr<T, Pool, Log>(_pool, itr->_page_pos, itr->_item_pos, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = _front_item_pos;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::begin_pos() page_pos=%llu, item_pos=%u", (unsigned long long)page_pos, (unsigned)item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::rbegin_pos() page_pos=%llu, item_pos=%u", (unsigned long long)page_pos, (unsigned)item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;
		item_pos = _back_item_pos + 1;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::end_pos() page_pos=%llu, item_pos=%u", (unsigned long long)page_pos, (unsigned)item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;
		item_pos = _back_item_pos;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::rend_pos() page_pos=%llu, item_pos=%u", (unsigned long long)page_pos, (unsigned)item_pos);
		}
	}





///////////////////////////////////////

	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::empty() const noexcept {
		return _state->total_item_count == 0;
	}


	template <typename T, typename Pool, typename Log>
	inline std::size_t vmem_list<T, Pool, Log>::size() const noexcept {
		return _state->total_item_count;
	}


#ifdef TEMP ////
	template <typename T, typename Pool, typename Log>
	inline T vmem_list<T, Pool, Log>::front() const noexcept {
		if (empty()) {
			return nullptr;
		}

		vmem_page<Pool, Log> page(_pool, _state->front_page_pos, _log);
		_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());
		return &list_page->items[0];
	}


	template <typename T, typename Pool, typename Log>
	inline T vmem_list<T, Pool, Log>::back() const noexcept {
		if (empty()) {
			return nullptr;
		}

		vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);
		_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());
		return &list_page->items[list_page->item_count - 1];
	}
#endif




}
