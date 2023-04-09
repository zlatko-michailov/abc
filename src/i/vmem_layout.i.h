/*
MIT License

Copyright (c) 2018-2023 Zlatko Michailov 

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

#include "vmem_pool.i.h"


namespace abc {

	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)


	// ..............................................................


	/**
	 * @brief			Empty struct to represent no page header.
	 */
	struct vmem_noheader {
	};


	// ..............................................................


	/**
	 * @brief			Base linked page.
	 */
	struct vmem_linked_page {
		vmem_page_pos_t		page_pos			= vmem_page_pos_nil;
		vmem_page_pos_t		prev_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
	};


	/**
	 * @brief			Common container page.
	 * @details			Includes a `vmem_linked_page` at the beginning.
	 * @tparam T		Item type.
	 * @tparam Header	Custom page header.
	 */
	template <typename T, typename Header = vmem_noheader>
	struct vmem_container_page : public vmem_linked_page {
		Header				header				= { };
		vmem_item_pos_t		item_count			= 0;
		T					items[1]			= { };
	};


	/**
	 * @brief			List page.
	 * @details			Same as `vmem_container_page`.
	 * @tparam T		Item type.
	 */
	template <typename T>
	struct vmem_list_page : public vmem_container_page<T, vmem_noheader> {
	};


	/**
	 * @brief			Item on a map key page.
	 * @tparam Key		Key type.
	 */
	template <typename Key>
	struct vmem_map_key {
		Key					key					= { };
		vmem_page_pos_t		page_pos			= { };
	};


	/**
	 * @brief			Item on a map value page.
	 * @tparam Key		Key type.
	 * @tparam T		Value type.
	 */
	template <typename Key, typename T>
	struct vmem_map_value {
		Key					key					= { };
		T					value				= { };
	};


	/**
	 * @brief			Map key page.
	 * @tparam Key		Key type.
	 */
	template <typename Key>
	struct vmem_map_key_page : public vmem_container_page<vmem_map_key<Key>, vmem_noheader> {
	};


	/**
	 * @brief			Map value page.
	 * @tparam Key		Key type.
	 * @tparam T		Value type.
	 */
	template <typename Key, typename T>
	struct vmem_map_value_page : public vmem_container_page<vmem_map_value<Key, T>, vmem_noheader> {
	};


	// ..............................................................


	/**
	 * @brief			Linked pages state.
	 */
	struct vmem_linked_state {
		vmem_page_pos_t		front_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		back_page_pos		= vmem_page_pos_nil;
	};


	/**
	 * @brief			Common container state.
	 * @details			Includes a `vmem_linked_state` at the beginning.
	 */
	struct vmem_container_state : public vmem_linked_state {
		vmem_item_pos_t		item_size			= 0;
		vmem_page_pos_t		total_item_count	= 0;
	};


	/**
	 * @brief			List state.
	 * @details			Same as `vmem_container_state`.
	 */
	struct vmem_list_state : public vmem_container_state {
	};


	/**
	 * @brief			Stack state.
	 * @details			Same as `vmem_container_state`.
	 */
	struct vmem_stack_state : public vmem_container_state {
	};


	/**
	 * @brief			Map state.
	 * @details			Consists of a stack of key lists, and a value list.
	 */
	struct vmem_map_state {
		vmem_stack_state		keys;
		vmem_container_state	values;
	};


	// ..............................................................


	/**
	 * @brief			Root page.
	 * @details			Not linked. Always at position 0.
	 */
	struct vmem_root_page {
		const vmem_version_t	version			= 2;
		const char				signature[10]	= "abc::vmem";
		const vmem_item_pos_t	page_size		= vmem_page_size;
		const std::uint16_t		unused1			= 0xcccc;
		vmem_linked_state		free_pages;
		const std::uint8_t		unused2			= 0xcc;
	};


	#pragma pack(pop)


	// --------------------------------------------------------------

}
