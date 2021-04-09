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

#include "vmem_pool.i.h"


namespace abc {

	// IMPORTANT: Ensure a predictable layout of the data on disk!
	#pragma pack(push, 1)


	// ..............................................................


	struct vmem_noheader {
	};


	struct vmem_map_header {
		vmem_page_pos_t			parent_page_pos	= vmem_page_pos_nil;
	};


	struct vmem_map_key_header: public vmem_map_header { ////
	};


	struct vmem_map_value_header: public vmem_map_header { ////
	};


	// ..............................................................


	struct vmem_linked_page {
		vmem_page_pos_t		page_pos			= vmem_page_pos_nil;
		vmem_page_pos_t		prev_page_pos		= vmem_page_pos_nil;
		vmem_page_pos_t		next_page_pos		= vmem_page_pos_nil;
	};


	template <typename T, typename Header = vmem_noheader>
	struct vmem_container_page : public vmem_linked_page {
		Header				header				= { 0 };
		vmem_item_pos_t		item_count			= 0;
		T					items[1]			= { 0 };
	};


	template <typename T>
	struct vmem_list_page : public vmem_container_page<T, vmem_noheader> {
	};


	template <typename Key>
	struct vmem_map_key {
		Key						key				= { 0 };
		vmem_page_pos_t			page_pos		= { 0 };
	};


	template <typename Key, typename T>
	struct vmem_map_value {
		Key						key				= { 0 };
		T						value			= { 0 };
	};


	template <typename Key>
	struct vmem_map_key_page : public vmem_container_page<vmem_map_key<Key>, vmem_map_key_header> {
	};


	template <typename Key, typename T>
	struct vmem_map_value_page : public vmem_container_page<vmem_map_value<Key, T>, vmem_map_value_header> {
	};


	// ..............................................................


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


	struct vmem_stack_state : public vmem_container_state {
	};


	struct vmem_map_state {
		vmem_stack_state		keys;
		vmem_container_state	values;
	};


	// ..............................................................


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
