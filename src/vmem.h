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
#include <cstring>

#include "vmem.i.h"
#include "exception.h"


namespace abc {

	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_pool<MaxMappedPages, Log>::vmem_pool(const char* file_path, Log* log)
		: _ready(false)
		, _mapped_page_count(0)
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
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() size=0x%llx", (long long)file_size);
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

				vmem_page<Pool, Log> page(this, _log);

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

				vmem_page<Pool, Log> page(this, _log);

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

			vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);
			_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() Root page integrity pos=0x%llx, ptr=%p, version=%u, signature='%s', page_size=%u",
					(long long)page.pos(), page.ptr(), root_page->version, root_page->signature, root_page->page_size);
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

			vmem_page<Pool, Log> page(this, vmem_page_pos_start, _log);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::vmem_pool() Start page integrity pos=0x%llx, ptr=%p",
					(long long)page.pos(), page.ptr());
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::vmem_pool() Verified");
		}

		_ready = true;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::alloc_page() noexcept {
		vmem_page_pos_t page_pos = vmem_page_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::alloc_page() Start. ready=%d", _ready);
		}

		if (_ready) {
			vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);
			if (page) {
				_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(page.ptr());

				vmem_list<vmem_page_pos_t, Pool, Log> free_pages_list(&root_page->free_pages, this, _log);

				if (!free_pages_list.empty()) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::alloc_page() Free page. size=%zu", free_pages_list.size());
					}

					page_pos = free_pages_list.back();
					free_pages_list.pop_back();

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::alloc_page() Found free page. pos=0x%llx", (long long)page_pos);
					}
				}
			}
		}

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
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::create_page() pos=0x%llx off=0x%llx",
				(long long)page_pos, (long long)page_off);
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
	inline void vmem_pool<MaxMappedPages, Log>::free_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::free_page() pos=0x%llx", (long long)page_pos);
		}

		if (page_pos != vmem_page_pos_nil && _ready) {
			void* ptr = lock_page(vmem_page_pos_root);
			if (ptr == nullptr) {
				_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(ptr);

				vmem_list<vmem_page_pos_t, Pool, Log> free_pages_list(&root_page->free_pages, this, _log);
				free_pages_list.push_back(page_pos);

				unlock_page(vmem_page_pos_root);
			}
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::lock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Start pos=0x%llx", (long long)page_pos);
		}

		// Try to find the page among the mapped pages.
		std::size_t i;
		for (i = 0; i < _mapped_page_count; i++) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Examine i=%zu pos=0x%llx, lock_count=%u, keep_count=%u, ptr=%p",
					i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].lock_count, (unsigned)_mapped_pages[i].keep_count, _mapped_pages[i].ptr);
			}

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
					std::size_t unmapped_count = 0;
					std::size_t empty_pos = MaxMappedPages;
					for (i = 0; i < _mapped_page_count; i++) {
						if (_mapped_pages[i].lock_count > 0 || _mapped_pages[i].keep_count > avg_keep_count) {
							// This page will be kept.
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Keeping page i=%zu, pos=0x%llx, keep_count=%u, avg_keep_count=%u",
									i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)avg_keep_count);
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
							if (unmapped_count > 0) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Moving page empty_pos=%zu, i=%zu", empty_pos, i);
								}

								_mapped_pages[empty_pos] = _mapped_pages[i];

								_mapped_pages[i] = { 0 };
								empty_pos = i;
							}
						}
						else {
							// This page will be unmapped.
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Unmapping page i=%zu, pos=0x%llx, keep_count=%u, avg_keep_count=%u",
									i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)avg_keep_count);
							}

							int um = munmap(_mapped_pages[i].ptr, vmem_page_size);
							void* ptr = _mapped_pages[i].ptr;
							_mapped_pages[i] = { 0 };

							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::lock_page() Unmap i=%zu, ptr=%p, um=%d, errno=%d", i, ptr, um, errno);
							}

							if (unmapped_count++ == 0) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() First empty slot i=%zu", i);
								}

								empty_pos = i;
							}
						}
					} // for (i)

					if (unmapped_count > 0) {
						// The first attempt was successful - we were able to free up some capacity.
						_mapped_page_count -= unmapped_count;

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

		void* page_ptr = nullptr; 

		if (i < _mapped_page_count) {
			// The page is already mapped. Only re-lock it.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::lock_page() Found at i=%zu", i);
			}

			_mapped_pages[i].lock_count++;
			_mapped_pages[i].keep_count++;

			page_ptr = _mapped_pages[i].ptr;

			_mapped_page_totals.keep_count++;
			_mapped_page_totals.hit_count++;
		}
		else {
			// The page is not mapped. Map it. Then lock it.

			off_t page_off = static_cast<off_t>(page_pos * vmem_page_size);
			void* ptr = mmap(NULL, vmem_page_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, page_off);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::important, __TAG__, "vmem_pool::lock_page() mmap i=%zu, pos=0x%llx, ptr=%p, errno=%d",
					i, (long long)page_pos, ptr, errno);
			}

			_mapped_page_count++;

			_mapped_pages[i].pos = page_pos;
			_mapped_pages[i].ptr = ptr;
			_mapped_pages[i].lock_count = 1;
			_mapped_pages[i].keep_count = 1;

			page_ptr = ptr;

			_mapped_page_totals.keep_count++;
			_mapped_page_totals.miss_count++;
		}

		// Optimization: Swap this page forward (once) to keep them sorted by keep_count.
		for (std::size_t j = 0; j < i; j++) {
			if (_mapped_pages[j].keep_count < _mapped_pages[i].keep_count) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Swapping j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
						j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
				}

				// Swap.
				_vmem_mapped_page temp = _mapped_pages[j];
				_mapped_pages[j] = _mapped_pages[i];
				_mapped_pages[i] = temp;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::lock_page() Swapped  j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
						j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
				}

				break;
			}
		}

		log_totals();

		return page_ptr;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::unlock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_pool::unlock_page() pos=0x%llx", (long long)page_pos);
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

			_mapped_pages[i].lock_count--;

			if (_mapped_pages[i].lock_count == 0) {
				int sn = msync(_mapped_pages[i].ptr, vmem_page_size, MS_ASYNC);

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::unlock_page() msync i=%zu, ptr=%p, sn=%d, errno=%d",
						i, _mapped_pages[i].ptr, sn, errno);
				}
			}
			else {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_pool::unlock_page() Found at i=%zu, ptr=%p, locks=%u",
						i, _mapped_pages[i].ptr, (unsigned)_mapped_pages[i].lock_count);
				}
			}
		}
		else {
			// The page was not found. This is a logic error.
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_pool::unlock_page() Trying to unlock a page that is not locked. page_pos=0x%llx",
					(long long)page_pos);
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
		: vmem_page<Pool, Log>(pool, vmem_page_pos_nil, log) {
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

		if (page_pos == vmem_page_pos_nil) {
			if (!alloc()) {
				return;
			}
		}

		lock();
	}


	template <typename Pool, typename Log>
	inline vmem_page<Pool, Log>::vmem_page(const vmem_page<Pool, Log>& other)
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
	inline vmem_page<Pool, Log>::operator bool() const noexcept {
		return _ptr != nullptr;
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
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::alloc() _pos=0x%llx", (long long)_pos);
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
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_page::lock() _pos=0x%llx, _ptr=nullptr", (long long)_pos);
			}

			return false;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::lock() _pos=0x%llx, _ptr=%p", (long long)_pos, _ptr);
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
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_page::unlock() _pos=0x%llx", (long long)_pos);
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
		char* page_ptr = reinterpret_cast<char*>(const_cast<void*>(_page.ptr()));

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
	inline vmem_list_iterator<T, Pool, Log>::vmem_list_iterator(const vmem_list<T, Pool, Log>* list, vmem_page_pos_t page_pos, vmem_item_pos_t item_pos, vmem_iterator_edge_t edge, Log* log)
		: _list(list)
		, _page_pos(page_pos)
		, _item_pos(item_pos)
		, _edge(edge)
		, _log(log) {

		if (list == nullptr) {
			throw exception<std::logic_error, Log>("list", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list_iterator::vmem_list_iterator() _page_pos=%lld, _item_pos=%d", (long long)_page_pos, (short)_item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline const vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator =(const vmem_list_iterator<T, Pool, Log>& other) noexcept {
		_list = other._list;
		_page_pos = other._page_pos;
		_item_pos = other._item_pos;
		_edge = other._edge;
		_log = other._log;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list_iterator::operator =() _page_pos=%lld, _item_pos=%d", (long long)_page_pos, (short)_item_pos);
		}

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list_iterator<T, Pool, Log>::operator ==(const vmem_list_iterator<T, Pool, Log>& other) const noexcept {
		return _list == other._list
			&& _page_pos == other._page_pos
			&& _item_pos == other._item_pos;
			// Do not include _edge. Otherwise begin() != end() on an empty list.
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list_iterator<T, Pool, Log>::operator !=(const vmem_list_iterator<T, Pool, Log>& other) const noexcept {
		return !operator ==(other);
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator ++() noexcept {
		_list->move_next(*this);

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator ++(int) noexcept {
		_list->move_next(*this);

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator --() noexcept {
		_list->move_prev(*this);

		return *this;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list_iterator<T, Pool, Log>& vmem_list_iterator<T, Pool, Log>::operator --(int) noexcept {
		_list->move_prev(*this);

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
	inline bool vmem_list_iterator<T, Pool, Log>::can_deref() const noexcept {
		return _page_pos != vmem_page_pos_nil
			&& _item_pos != vmem_item_pos_nil;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_list_iterator<T, Pool, Log>::ptr() const noexcept {
		return _list->at(*this);
	}


	template <typename T, typename Pool, typename Log>
	inline T& vmem_list_iterator<T, Pool, Log>::deref() const {
		T* t = ptr();

		if (t == nullptr) {
			throw exception<std::runtime_error, Log>("Dereferencing invalid iterator", __TAG__);
		}

		return *t;
	}


	// --------------------------------------------------------------


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_list<T, Pool, Log>::items_pos() noexcept {
		return sizeof(_vmem_list_page<std::uint8_t>) - sizeof(std::uint8_t);
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_list<T, Pool, Log>::max_item_size() noexcept {
		return vmem_page_size - items_pos();
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr std::size_t vmem_list<T, Pool, Log>::page_capacity() noexcept {
		return max_item_size() / sizeof(T);
	}


	template <typename T, typename Pool, typename Log>
	inline constexpr bool vmem_list<T, Pool, Log>::is_uninit(const vmem_list_state* state) noexcept {
		return state != nullptr
			&& state->front_page_pos == vmem_page_pos_nil
			&& state->back_page_pos == vmem_page_pos_nil
			&& state->item_size == 0;
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_list<T, Pool, Log>::vmem_list(vmem_list_state* state, Pool* pool, Log* log)
		: _state(state)
		, _pool(pool)
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::vmem_list() state=%p, pool=%p", state, pool);
		}

		if (state == nullptr) {
			throw exception<std::logic_error, Log>("state", __TAG__);
		}

		if (pool == nullptr) {
			throw exception<std::logic_error, Log>("pool", __TAG__);
		}

		if (sizeof(T) > max_item_size()) {
			throw exception<std::logic_error, Log>("size excess", __TAG__);
		}

		if (is_uninit(state)) {
			state->item_size = sizeof(T);
		}

		if (sizeof(T) != _state->item_size) {
			throw exception<std::logic_error, Log>("size mismatch", __TAG__);
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::vmem_list() front_page_pos=0x%llx, back_page_pos=0x%llx", 
				(long long)_state->front_page_pos, (long long)_state->back_page_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::begin() noexcept {
		return cbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::begin() const noexcept {
		return cbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::cbegin() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		begin_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::end() noexcept {
		return cend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::end() const noexcept {
		return cend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::cend() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		end_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::end, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::rend() noexcept {
		return crend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::rend() const noexcept {
		return crend();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::crend() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		rend_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::rbegin() noexcept {
		return crbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::rbegin() const noexcept {
		return crbegin();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_iterator vmem_list<T, Pool, Log>::crbegin() const noexcept {
		vmem_page_pos_t page_pos;
		vmem_item_pos_t item_pos;
		rbegin_pos(page_pos, item_pos);

		return vmem_list_iterator<T, Pool, Log>(this, page_pos, item_pos, vmem_iterator_edge::rbegin, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline bool vmem_list<T, Pool, Log>::empty() const noexcept {
		return _state->front_page_pos == vmem_page_pos_nil
			|| _state->back_page_pos == vmem_page_pos_nil;
	}


	template <typename T, typename Pool, typename Log>
	inline std::size_t vmem_list<T, Pool, Log>::size() const noexcept {
		return _state->total_item_count;
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::pointer vmem_list<T, Pool, Log>::frontptr() noexcept {
		return begin().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_pointer vmem_list<T, Pool, Log>::frontptr() const noexcept {
		return begin().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::reference vmem_list<T, Pool, Log>::front() {
		return begin().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_reference vmem_list<T, Pool, Log>::front() const {
		return begin().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::pointer vmem_list<T, Pool, Log>::backptr() noexcept {
		return rend().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_pointer vmem_list<T, Pool, Log>::backptr() const noexcept {
		return rend().ptr();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::reference vmem_list<T, Pool, Log>::back() {
		return rend().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::const_reference vmem_list<T, Pool, Log>::back() const {
		return rend().deref();
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::push_back(const_reference item) {
		insert(end(), item);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::pop_back() {
		erase(rend());
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::push_front(const_reference item) {
		insert(begin(), item);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::pop_front() {
		erase(begin());
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::insert(const_iterator itr, const_reference item) {
		if (itr._page_pos == vmem_page_pos_nil && (itr._item_pos != vmem_item_pos_nil || !empty())) {
			throw exception<std::logic_error, Log>("itr.page", __TAG__);
		}

		if (itr._item_pos == vmem_item_pos_nil && (itr._page_pos != _state->back_page_pos && itr._edge != vmem_iterator_edge::end)) {
			throw exception<std::logic_error, Log>("itr.item", __TAG__);
		}

		vmem_page_pos_t page_pos = vmem_page_pos_nil;
		vmem_item_pos_t item_pos = vmem_item_pos_nil;
		vmem_item_pos_t page_item_count = 0;

		// Copy the item to a local variable to make sure the reference is valid and copyable before we change any state.
		T item_copy(item);

		// IMPORTANT: There must be no exceptions from here to the end of the method!

		if (itr._page_pos == vmem_page_pos_nil) {
			// Empty list.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() Empty");
			}

			vmem_page<Pool, Log> page(_pool, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			// Insert page - only one.
			list_page->next_page_pos = vmem_page_pos_nil;
			list_page->prev_page_pos = vmem_page_pos_nil;
			list_page->item_count = 0;

			_state->front_page_pos = page.pos();
			_state->back_page_pos = page.pos();

			// Insert item - first/only one.
			page_pos = page.pos();
			item_pos = 0;
			page_item_count = ++list_page->item_count;
			std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

			if (_log != nullptr) {
				_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
			}
		}
		else {
			// Non-empty list.

			vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() item_count=%u, page_capacity=%zu", list_page->item_count, (std::size_t)page_capacity());
			}

			if (list_page->item_count == page_capacity()) {
				// The page has no capacity.

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() No capacity");
				}

				vmem_page<Pool, Log> new_page(_pool, _log);
				_vmem_list_page<T>* new_list_page = reinterpret_cast<_vmem_list_page<T>*>(new_page.ptr());

				// Insert page - after.
				new_list_page->next_page_pos = list_page->next_page_pos;
				new_list_page->prev_page_pos = itr._page_pos;
				new_list_page->item_count = 0;

				if (list_page->next_page_pos != vmem_page_pos_nil) {
					vmem_page<Pool, Log> next_page(_pool, list_page->next_page_pos, _log);
					_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

					next_list_page->prev_page_pos = new_page.pos();
				}

				list_page->next_page_pos = new_page.pos();

				if (_state->back_page_pos == page.pos()) {
					_state->back_page_pos = new_page.pos();
				}

				if (itr._item_pos != vmem_item_pos_nil) {
					// Inserting to the middle of a full page.

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() No capacity. Middle.");
					}

					// Split it at the insertion position.
					std::size_t move_item_count = list_page->item_count - itr._item_pos;
					std::memmove(&new_list_page->items[0], &list_page->items[itr._item_pos], move_item_count * sizeof(T));

					new_list_page->item_count = move_item_count;
					list_page->item_count -= move_item_count;

					// Insert item - middle.
					page_pos = itr._page_pos;
					item_pos = itr._item_pos;
					page_item_count = ++list_page->item_count;
					std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

					if (_log != nullptr) {
						_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
					}
				}
				else {
					// Inserting to the end of a full page.
					// Insert at the beginning of the new page.

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() No capacity. End.");
					}

					// Insert item - first/only one.
					page_pos = new_page.pos();
					item_pos = 0;
					page_item_count = ++new_list_page->item_count;
					std::memmove(&new_list_page->items[item_pos], &item_copy, sizeof(T));

					if (_log != nullptr) {
						_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &new_list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
					}
				}
			}
			else {
				// The page has capacity.

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() Capacity");
				}

				if (itr._item_pos != vmem_item_pos_nil) {
					// Inserting to the middle of a page with capacity.

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() Capacity. Middle.");
					}

					// Shift the items from the insertion position to free up a slot.
					std::size_t move_item_count = list_page->item_count - itr._item_pos;
					std::memmove(&list_page->items[itr._item_pos + 1], &list_page->items[itr._item_pos], move_item_count * sizeof(T));

					// Insert item - middle.
					page_pos = itr._page_pos;
					item_pos = itr._item_pos;
					page_item_count = ++list_page->item_count;
					std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

					if (_log != nullptr) {
						_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
					}
				}
				else {
					// Inserting to the end of a page with capacity.

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::insert() Capacity. End.");
					}

					// Insert item - last one.
					page_pos = itr._page_pos;
					item_pos = list_page->item_count;
					page_item_count = ++list_page->item_count;
					std::memmove(&list_page->items[item_pos], &item_copy, sizeof(T));

					if (_log != nullptr) {
						_log->put_binary(category::abc::vmem, severity::abc::debug, __TAG__, &list_page->items[item_pos], std::min(sizeof(T), (std::size_t)16));
					}
				}
			}
		}

		// Update the total item count.
		_state->total_item_count++;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::insert() Done. page_pos=%zu, item_pos=%u, page_item_count=%u, total_item_count=%zu",
				(std::size_t)page_pos, item_pos, page_item_count, (std::size_t)_state->total_item_count);
		}

		return iterator(this, page_pos, item_pos, vmem_iterator_edge::none, _log);
	}


	template <typename T, typename Pool, typename Log>
	template <typename InputItr>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::insert(const_iterator itr, InputItr first, InputItr last) {
		iterator ret(itr);

		for (InputItr item = first; item != last; item++) {
			if (!insert(itr++, *item).can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_list::insert() Breaking from the loop.");
				}
				break;
			}
		}

		return ret;
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::erase(const_iterator itr) {
		if (!itr.can_deref()) {
			throw exception<std::logic_error, Log>("itr", __TAG__);
		}

		// IMPORTANT: There must be no exceptions from here to the end of the method!

		vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);
		_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

		vmem_page_pos_t page_pos = itr._page_pos;
		vmem_item_pos_t item_pos = itr._item_pos;
		vmem_iterator_edge_t edge = vmem_iterator_edge::none;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase() Start. page_pos=%zu, item_pos=%u, page_item_count=%u, total_item_count=%zu",
				(std::size_t)page_pos, item_pos, list_page->item_count, (std::size_t)_state->total_item_count);
		}

		if (list_page->item_count > 1) {
			// The page has multiple items.

			// To delete an item, bring up the remaining elements, if there are any.
			if (itr._item_pos < list_page->item_count - 1) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::erase() Multiple. Middle.");
				}

				std::size_t move_item_count = list_page->item_count - itr._item_pos - 1;
				std::memmove(&list_page->items[itr._item_pos], &list_page->items[itr._item_pos + 1], move_item_count * sizeof(T));
			}
			else {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::erase() Multiple. Last.");
				}

				// If we are deleting the last item on a page, the next item is item 0 on the next page or end().
				if (list_page->next_page_pos != vmem_page_pos_nil) {
					page_pos = list_page->next_page_pos;
					item_pos = 0;
				}
				else {
					end_pos(page_pos, item_pos);
					edge = vmem_iterator_edge::end;
				}
			}

			list_page->item_count--;
		}
		else {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::erase() Only.");
			}

			// The page has no other items.

			// We can free the current page now.
			vmem_page_pos_t prev_page_pos = list_page->prev_page_pos;
			vmem_page_pos_t next_page_pos = list_page->next_page_pos;
			list_page = nullptr;
			page.free();

			// Connect the two adjaceent pages, and free this page.
			// The next item is item 0 on the next page or end().
			if (prev_page_pos != vmem_page_pos_nil) {
				vmem_page<Pool, Log> prev_page(_pool, prev_page_pos, _log);
				_vmem_list_page<T>* prev_list_page = reinterpret_cast<_vmem_list_page<T>*>(prev_page.ptr());

				prev_list_page->next_page_pos = next_page_pos;
			}
			else {
				_state->front_page_pos = next_page_pos;
			}

			if (next_page_pos != vmem_page_pos_nil) {
				vmem_page<Pool, Log> next_page(_pool, next_page_pos, _log);
				_vmem_list_page<T>* next_list_page = reinterpret_cast<_vmem_list_page<T>*>(next_page.ptr());

				next_list_page->prev_page_pos = prev_page_pos;

				page_pos = next_page_pos;
				item_pos = 0;
			}
			else {
				_state->back_page_pos = prev_page_pos;

				end_pos(page_pos, item_pos);
				edge = vmem_iterator_edge::end;
			}
		}

		// Update the total item count.
		_state->total_item_count--;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, __TAG__, "vmem_list::erase() Done. page_pos=%zu, item_pos=%u, total_item_count=%zu",
				(std::size_t)page_pos, item_pos, (std::size_t)_state->total_item_count);
		}

		return iterator(this, page_pos, item_pos, edge, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline typename vmem_list<T, Pool, Log>::iterator vmem_list<T, Pool, Log>::erase(const_iterator first, const_iterator last) {
		iterator item = first;

		while (item != last) {
			if (!item.can_deref()) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::important, __TAG__, "vmem_list::erase() Breaking from the loop.");
				}

				break;
			}

			item = erase(item);
		}

		return item;
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::clear() noexcept {
		erase(begin(), end());
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::move_next(iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_next() Before _page_pos=%lld, _item_pos=%d, _edge=%d",
				(long long)itr._page_pos, (short)itr._item_pos, itr._edge);
		}

		if (itr._item_pos == vmem_item_pos_nil && itr._edge == vmem_iterator_edge::rbegin) {
			begin_pos(itr._page_pos, itr._item_pos);
			itr._edge = vmem_iterator_edge::none;
		}
		else if (itr._page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			if (itr._item_pos < list_page->item_count - 1) {
				itr._item_pos++;
			}
			else {
				if (list_page->next_page_pos == vmem_page_pos_nil) {
					end_pos(itr._page_pos, itr._item_pos);
					itr._edge = vmem_iterator_edge::end;
				}
				else {
					itr._page_pos = list_page->next_page_pos;
					itr._item_pos = 0;
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_next() After _page_pos=%lld, _item_pos=%d, _edge=%d",
				(long long)itr._page_pos, (short)itr._item_pos, itr._edge);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::move_prev(iterator& itr) const noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_prev() Before _page_pos=%lld, _item_pos=%d, _edge=%d",
				(long long)itr._page_pos, (short)itr._item_pos, itr._edge);
		}

		if (itr._item_pos == vmem_item_pos_nil && itr._edge == vmem_iterator_edge::end) {
			rend_pos(itr._page_pos, itr._item_pos);
			itr._edge = vmem_iterator_edge::none;
		}
		else if (itr._page_pos != vmem_page_pos_nil) {
			vmem_page<Pool, Log> page(_pool, itr._page_pos, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());

			if (itr._item_pos > 0) {
				itr._item_pos--;
			}
			else {
				if (list_page->prev_page_pos == vmem_page_pos_nil) {
					rbegin_pos(itr._page_pos, itr._item_pos);
					itr._edge = vmem_iterator_edge::rbegin;
				}
				else {
					vmem_page<Pool, Log> prev_page(_pool, list_page->prev_page_pos, _log);
					_vmem_list_page<T>* prev_list_page = reinterpret_cast<_vmem_list_page<T>*>(prev_page.ptr());

					itr._page_pos = list_page->prev_page_pos;
					itr._item_pos = prev_list_page->item_count - 1;
				}
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::move_prev() After _page_pos=%lld, _item_pos=%d, _edge=%d",
				(long long)itr._page_pos, (short)itr._item_pos, itr._edge);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline vmem_ptr<T, Pool, Log> vmem_list<T, Pool, Log>::at(const_iterator& itr) const noexcept {
		vmem_item_pos_t item_pos =
			itr._item_pos == vmem_item_pos_nil ?
				vmem_item_pos_nil :
				items_pos() + (itr._item_pos * sizeof(T));

		return vmem_ptr<T, Pool, Log>(_pool, itr._page_pos, item_pos, _log);
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::begin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = _state->front_page_pos == vmem_page_pos_nil ? vmem_item_pos_nil : 0; 

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::begin_pos() page_pos=%lld, item_pos=%d", (long long)page_pos, (short)item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::rbegin_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->front_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::rbegin_pos() page_pos=%lld, item_pos=%d", (long long)page_pos, (short)item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::end_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;
		item_pos = vmem_item_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::end_pos() page_pos=%lld, item_pos=%d", (long long)page_pos, (short)item_pos);
		}
	}


	template <typename T, typename Pool, typename Log>
	inline void vmem_list<T, Pool, Log>::rend_pos(vmem_page_pos_t& page_pos, vmem_item_pos_t& item_pos) const noexcept {
		page_pos = _state->back_page_pos;

		if (_state->back_page_pos == vmem_page_pos_nil) {
			item_pos = vmem_item_pos_nil;
		}
		else {
			vmem_page<Pool, Log> page(_pool, _state->back_page_pos, _log);
			_vmem_list_page<T>* list_page = reinterpret_cast<_vmem_list_page<T>*>(page.ptr());
			item_pos = list_page->item_count - 1;
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, __TAG__, "vmem_list::rend_pos() page_pos=%lld, item_pos=%d", (long long)page_pos, (short)item_pos);
		}
	}

}
