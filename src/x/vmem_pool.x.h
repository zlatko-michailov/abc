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
	inline vmem_pool<MaxMappedPages, Log>::vmem_pool(const char* file_path, Log* log)
		: _ready(false)
		, _mapped_page_count(0)
		, _mapped_pages{ 0 }
		, _mapped_page_totals{ 0 }
		, _log(log) {

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1037c, "vmem_pool::vmem_pool() Open path='%s'", file_path);
		}

		_fd = open(file_path, O_CREAT | O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1037d, "vmem_pool::vmem_pool() Open fd=%d, errno=%d", _fd, errno);
		}

		if (_fd < 0) {
			throw exception<std::runtime_error, Log>("Not found vmem file", 0x1037e, _log);
		}

		vmem_page_pos_t file_size = lseek(_fd, 0, SEEK_END);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1037f, "vmem_pool::vmem_pool() size=0x%llx", (long long)file_size);
		}

		if ((file_size & (vmem_page_size - 1)) != 0) {
			throw exception<std::runtime_error, Log>("Corrupt vmem file - size", 0x10380, _log);
		}

		// The file is empty...
		if (file_size == 0) {
			// Create and init the root page (0).
			{
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x10381, "vmem_pool::vmem_pool() Creating root page");
				}

				vmem_page<Pool, Log> page(this, _log);

				if (page.ptr() == nullptr) {
					throw exception<std::runtime_error, Log>("Insufficient capacity", 0x10382, _log);
				}

				std::memset(page.ptr(), 0, vmem_page_size);

				_vmem_root_page init;
				std::memmove(page.ptr(), &init, sizeof(init));

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x10383, "vmem_pool::vmem_pool() Root page created");
				}
			}

			// Create and init the start page (1).
			{
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x10384, "vmem_pool::vmem_pool() Creating start page");
				}

				vmem_page<Pool, Log> page(this, _log);

				if (page.ptr() == nullptr) {
					throw exception<std::runtime_error, Log>("Insufficient capacity", 0x10385, _log);
				}

				std::memset(page.ptr(), 0, vmem_page_size);

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x10386, "vmem_pool::vmem_pool() Start page created");
				}
			}
		}

		// Verify integrity of the root page (0).
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x10387, "vmem_pool::vmem_pool() Verifying root page integrity");
			}

			vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

			if (page.ptr() == nullptr) {
				throw exception<std::runtime_error, Log>("Cannot verify root page", 0x10388, _log);
			}

			_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(page.ptr());

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x10389, "vmem_pool::vmem_pool() Root page integrity pos=0x%llx, ptr=%p, version=%u, signature='%s', page_size=%u",
					(long long)page.pos(), page.ptr(), root_page->version, root_page->signature, root_page->page_size);
			}

			_vmem_root_page init;

			if (root_page->version != init.version) {
				throw exception<std::runtime_error, Log>("vmem file integrity - version", 0x1038a, _log);
			}

			if (std::strcmp(root_page->signature, init.signature) != 0) {
				throw exception<std::runtime_error, Log>("vmem file integrity - signature", 0x1038b, _log);
			}

			if (root_page->page_size != vmem_page_size) {
				throw exception<std::runtime_error, Log>("vmem file integrity - page_size", 0x1038c, _log);
			}
		}

		// Verify the start page is loadable.
		{
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x1038d, "vmem_pool::vmem_pool() Verifying start page integrity");
			}

			vmem_page<Pool, Log> page(this, vmem_page_pos_start, _log);

			if (page.ptr() == nullptr) {
				throw exception<std::runtime_error, Log>("Cannot verify start page", 0x1038e, _log);
			}

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1038f, "vmem_pool::vmem_pool() Start page integrity pos=0x%llx, ptr=%p",
					(long long)page.pos(), page.ptr());
			}
		}

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10390, "vmem_pool::vmem_pool() Verified");
		}

		_ready = true;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::alloc_page() noexcept {
		vmem_page_pos_t page_pos = vmem_page_pos_nil;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10391, "vmem_pool::alloc_page() Start. ready=%d", _ready);
		}

		if (_ready) {
			vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

			if (page.ptr() == nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x10392, "vmem_pool::alloc_page() Could not check the free_pages list");
			}
			else {
				_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(page.ptr());

				vmem_list<vmem_page_pos_t, Pool, Log> free_pages_list(&root_page->free_pages, this, _log);

				if (!free_pages_list.empty()) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::debug, 0x10393, "vmem_pool::alloc_page() Free page. size=%zu", free_pages_list.size());
					}

					page_pos = free_pages_list.back();
					free_pages_list.pop_back();

					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::abc::optional, 0x10394, "vmem_pool::alloc_page() Found free page. pos=0x%llx", (long long)page_pos);
					}
				}
			}
		}

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x10395, "vmem_pool::alloc_page() No free pages. Creating...");
			}

			page_pos = create_page();
		}

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::important, 0x10396, "vmem_pool::alloc_page() Could not create a page on the file.");
			}
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::create_page() noexcept {
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

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::free_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::optional, 0x10399, "vmem_pool::free_page() pos=0x%llx", (long long)page_pos);
		}

		if (page_pos != vmem_page_pos_nil && _ready) {
			vmem_page<Pool, Log> page(this, vmem_page_pos_root, _log);

			if (page.ptr() == nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, 0x1039a, "vmem_pool::free_page() Could not add to the free_pages list");
			}
			else {
				_vmem_root_page* root_page = reinterpret_cast<_vmem_root_page*>(page.ptr());

				vmem_list<vmem_page_pos_t, Pool, Log> free_pages_list(&root_page->free_pages, this, _log);

				free_pages_list.push_back(page_pos);
			}
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::lock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x1039b, "vmem_pool::lock_page() Start pos=0x%llx", (long long)page_pos);
		}

		// Try to find the page among the mapped pages.
		std::size_t i;
		for (i = 0; i < _mapped_page_count; i++) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::debug, 0x1039c, "vmem_pool::lock_page() Examine i=%zu pos=0x%llx, lock_count=%u, keep_count=%u, ptr=%p",
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
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x1039d, "vmem_pool::lock_page() Not found. No capacity.");
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
								_log->put_any(category::abc::vmem, severity::abc::debug, 0x1039e, "vmem_pool::lock_page() Keeping page i=%zu, pos=0x%llx, keep_count=%u, avg_keep_count=%u",
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
									_log->put_any(category::abc::vmem, severity::abc::optional, 0x1039f, "vmem_pool::lock_page() Moving page empty_pos=%zu, i=%zu", empty_pos, i);
								}

								_mapped_pages[empty_pos] = _mapped_pages[i];

								_mapped_pages[i] = { 0 };
								empty_pos = i;
							}
						}
						else {
							// This page will be unmapped.
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::optional, 0x103a0, "vmem_pool::lock_page() Unmapping page i=%zu, pos=0x%llx, keep_count=%u, avg_keep_count=%u",
									i, (long long)_mapped_pages[i].pos, (unsigned)_mapped_pages[i].keep_count, (unsigned)avg_keep_count);
							}

							int um = munmap(_mapped_pages[i].ptr, vmem_page_size);
							void* ptr = _mapped_pages[i].ptr;
							_mapped_pages[i] = { 0 };

							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::abc::important, 0x103a1, "vmem_pool::lock_page() Unmap i=%zu, ptr=%p, um=%d, errno=%d", i, ptr, um, errno);
							}

							if (unmapped_count++ == 0) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::abc::optional, 0x103a2, "vmem_pool::lock_page() First empty slot i=%zu", i);
								}

								empty_pos = i;
							}
						}
					} // for (i)

					if (unmapped_count > 0) {
						// The first attempt was successful - we were able to free up some capacity.
						_mapped_page_count -= unmapped_count;

						if (_log != nullptr) {
							_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a3, "vmem_pool::lock_page() Compacted. _mapped_page_count=%zu", _mapped_page_count);
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
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x103a4, "vmem_pool::lock_page() Capacity _mapped_page_count=%zu", _mapped_page_count);
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

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::abc::optional, 0x103a6, "vmem_pool::lock_page() Found at i=%zu", i);
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
				_log->put_any(category::abc::vmem, severity::abc::important, 0x103a7, "vmem_pool::lock_page() mmap i=%zu, pos=0x%llx, ptr=%p, errno=%d",
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
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a8, "vmem_pool::lock_page() Swapping j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
						j, (long long)_mapped_pages[j].pos, i, (long long)_mapped_pages[i].pos);
				}

				// Swap.
				_vmem_mapped_page temp = _mapped_pages[j];
				_mapped_pages[j] = _mapped_pages[i];
				_mapped_pages[i] = temp;

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::debug, 0x103a9, "vmem_pool::lock_page() Swapped  j=%zu (pos=0x%llx), i=%zu (pos=0x%llx)",
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
			_log->put_any(category::abc::vmem, severity::abc::debug, 0x103aa, "vmem_pool::unlock_page() pos=0x%llx", (long long)page_pos);
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
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x103ab, "vmem_pool::unlock_page() msync i=%zu, ptr=%p, sn=%d, errno=%d",
						i, _mapped_pages[i].ptr, sn, errno);
				}
			}
			else {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::abc::optional, 0x103ac, "vmem_pool::unlock_page() Found at i=%zu, ptr=%p, locks=%u",
						i, _mapped_pages[i].ptr, (unsigned)_mapped_pages[i].lock_count);
				}
			}
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
			throw exception<std::logic_error, Log>("pool", 0x103af);
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
			throw exception<std::runtime_error, Log>("Dereferencing invalid vmem_ptr", 0x103b5);
		}

		return *ptr();
	}


	// --------------------------------------------------------------

}
