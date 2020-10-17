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
			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::vmem_pool() Open path='%s'", file_path);
		}

		_fd = open(file_path, O_CREAT | O_RDWR | O_LARGEFILE, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::vmem_pool() Open fd=%d, errno=%d", _fd, errno);
		}

		vmem_page_pos_t file_size = lseek(_fd, 0, SEEK_END);

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::vmem_pool() size=%llu", file_size);
		}

		if ((file_size & (vmem_page_size - 1)) != 0) {
			throw exception<std::runtime_error, Log>("Corrupt vmem file - size", __TAG__, _log);
		}

		// Create and init the root and the start pages if the file is empty.
		if (file_size == 0) {
			for (vmem_page_pos_t page_pos = vmem_page_pos_root; page_pos <= vmem_page_pos_start; page_pos++) {
				vmem_page_pos_t created_page_pos = create_page();

				if (created_page_pos != page_pos) {
					if (_log != nullptr) {
						_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::vmem_pool() Created page mismatch: actual=%llu, expected=%llu", created_page_pos, page_pos);
					}

					throw exception<std::runtime_error, Log>("Couldn't init vmem file", __TAG__, _log);
				}

				// Materialize the page in memory.
				void* page_ptr = lock_page(page_pos);

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::vmem_pool() New page pos=%llu, ptr=%p", page_pos, page_ptr);
				}

				// Init the content of the page.
				memset(page_ptr, 0, vmem_page_size);

				if (page_pos == vmem_page_pos_root) {
					_vmem_root_page init;
					memmove(page_ptr, &init, sizeof(init));
				}
			}
		}
		else {
			// Load the root and start pages.
			for (vmem_page_pos_t page_pos = vmem_page_pos_root; page_pos <= vmem_page_pos_start; page_pos++) {
				void* page_ptr = lock_page(page_pos);

				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::vmem_pool() Loaded page pos=%llu, ptr=%p", page_pos, page_ptr);
				}
			}
		}
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::alloc_page() noexcept {
		vmem_page_pos_t page_pos;

		// TODO: First, check the free pages.
		page_pos = vmem_page_pos_nil;

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::alloc_page() No free pages. Creating...");
			}

			page_pos = create_page();
		}

		if (page_pos == vmem_page_pos_nil) {
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::alloc_page() Could not create a page on the file.");
			}
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline vmem_page_pos_t vmem_pool<MaxMappedPages, Log>::create_page() noexcept {
		off_t page_off = lseek(_fd, 0, SEEK_END);
		vmem_page_pos_t page_pos = page_off / vmem_page_size;

		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::create_page() pos=%llu off=%llu", page_pos, page_off);
		}

		std::uint8_t blank_page[vmem_page_size] = { 0 };
		ssize_t wb = write(_fd, blank_page, vmem_page_size);

		if (wb != vmem_page_size) {
			page_pos = vmem_page_pos_nil;

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::create_page() wb=%ld, errno=%d", wb, errno);
			}
		}

		return page_pos;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline bool vmem_pool<MaxMappedPages, Log>::free_page(vmem_page_pos_t page_pos) noexcept {
		// TODO: Add to the list of free pages.
		return true;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void* vmem_pool<MaxMappedPages, Log>::lock_page(vmem_page_pos_t page_pos) noexcept {
		if (_log != nullptr) {
			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Start pos=%llu", page_pos);
		}

		// Try to find the page among the mapped pages.
		std::size_t i;
		for (i = 0; i < _mapped_page_count; i++) {
			if (_mapped_pages[i].pos == page_pos) {
				break;
			}
		}

		if (i >= _mapped_page_count) {
			// The page was not found.

			// Check if there is capacity for one more mapped page.
			if (_mapped_page_count >= MaxMappedPages) {
				// There is no more capacity for mapped pages.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Not found. No capacity.");
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
								_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Keeping page i=%lu, keep_count=%lu, avg_keep_count=%lu", i, _mapped_pages[i].keep_count, avg_keep_count);
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

							// If there is an empty slot, we move this elemet there.
							if (has_empty_pos) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Moving page empty_pos=%lu, i=%lu", empty_pos, i);
								}

								_mapped_pages[empty_pos++] = _mapped_pages[i];
							}
						}
						else {
							// This page will be unmapped.
							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Unmapping page i=%lu, keep_count=%lu, avg_keep_count=%lu", i, _mapped_pages[i].keep_count, avg_keep_count);
							}

							int um = munmap(_mapped_pages[i].ptr, vmem_page_size);

							if (_log != nullptr) {
								_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Unmap i=%lu, ptr=%p, um=%d, errno=%d", i, _mapped_pages[i].ptr, um, errno);
							}

							if (!has_empty_pos) {
								if (_log != nullptr) {
									_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() First empty slot i=%lu", i);
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
							_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Compacted. _mapped_page_count=%lu", _mapped_page_count);
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
					_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Capacity _mapped_page_count=%lu", _mapped_page_count);
				}

				i = _mapped_page_count;
			}
			else {
				// All the maped pages are locked. We cannot find a slot for the new page.
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_pool::lock_page() Insufficient capacity. MaxedMappedPages=%lu", MaxMappedPages);
				}

				return nullptr;
			}
		} // Not found

		// We have a slot where we should lock the page.
		_vmem_mapped_page& mapped_page = _mapped_pages[i];

		if (i < _mapped_page_count) {
			// The page is already mapped. Only re-lock it.

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Found at i=%lu", i);
			}

			mapped_page.lock_count++;
			mapped_page.keep_count++;

			_mapped_page_totals.keep_count++;
			_mapped_page_totals.hit_count++;
			_mapped_page_totals.check_count += i;
		}
		else {
			// The page is not mapped. Map it. Then lock it.

			off_t page_off = static_cast<off_t>(page_pos * vmem_page_size);
			void* ptr = mmap(NULL, vmem_page_size, PROT_READ | PROT_WRITE, MAP_SHARED, _fd, page_off);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Map i=%lu, pos=%llu, ptr=%p, errno=%d", i, page_pos, ptr, errno);
			}

			_mapped_page_count++;

			mapped_page.pos = page_pos;
			mapped_page.ptr = ptr;
			mapped_page.lock_count = 1;
			mapped_page.keep_count = 1;

			_mapped_page_totals.keep_count++;
			_mapped_page_totals.miss_count++;
			_mapped_page_totals.check_count += i;
		}

		// Optimization: Swap this page forward (once) to keep them sorted by keep_count.
		for (std::size_t j = 0; j < i; j++) {
			if (_mapped_pages[j].keep_count < _mapped_pages[i].keep_count) {
				if (_log != nullptr) {
					_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Swapping j=%lu, i=%lu", j, i);
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
			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::unlock_page() pos=%llu", page_pos);
		}

		// Try to find the page among the mapped pages.
		std::size_t i;
		for (i = 0; i < _mapped_page_count; i++) {
			if (_mapped_pages[i].pos == page_pos) {
				break;
			}
		}

		if (i < _mapped_page_count) {
			// The page was found.
			_vmem_mapped_page& mapped_page = _mapped_pages[i];
			mapped_page.lock_count--;

			_mapped_page_totals.check_count += i;

			int um = munmap(mapped_page.ptr, vmem_page_size);

			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::unlock_page() Found at i=%lu, ptr=%p, um=%d, errno=%d", i, mapped_page.ptr, um, errno);
			}
		}
		else {
			// The page was not found. This is a logic error.
			if (_log != nullptr) {
				_log->put_any(category::abc::vmem, severity::warning, __TAG__, "vmem_pool::unlock_page() Trying to unlock a page that is not locked. page_pos=%llu", page_pos);
			}

			return false;
		}

		log_totals();

		return true;
	}


	template <std::size_t MaxMappedPages, typename Log>
	inline void vmem_pool<MaxMappedPages, Log>::log_totals() noexcept {
		if (_log != nullptr) {
			vmem_page_hit_count_t total = _mapped_page_totals.hit_count + _mapped_page_totals.miss_count;
			vmem_page_hit_count_t hit_percent = (_mapped_page_totals.hit_count * 100) / total;
			vmem_page_hit_count_t miss_percent = (_mapped_page_totals.miss_count * 100) / total;
			vmem_page_hit_count_t check_factor = _mapped_page_totals.check_count / total;
			vmem_page_hit_count_t check_factor_percent = (check_factor * 100) / MaxMappedPages;

			_log->put_any(category::abc::vmem, severity::critical, __TAG__, "vmem_pool::lock_page() Totals hits=%lu (%lu%%), misses=%lu (%lu%%), checks=%lu (%lu, %lu%%)", 
				_mapped_page_totals.hit_count, hit_percent, _mapped_page_totals.miss_count, miss_percent, _mapped_page_totals.check_count, check_factor, check_factor_percent);
		}
	}


	// --------------------------------------------------------------

}
