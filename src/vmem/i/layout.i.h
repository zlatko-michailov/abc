/*
MIT License

Copyright (c) 2018-2024 Zlatko Michailov 

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

#include "base.i.h"


namespace abc { namespace vmem {

    // IMPORTANT: Ensure a predictable layout of the data on disk!
    #pragma pack(push, 1)


    // ..............................................................


    /**
     * @brief Empty struct to represent no page header.
     */
    struct noheader {
    };


    // ..............................................................


    /**
     * @brief Base linked page.
     */
    struct linked_page {
        page_pos_t page_pos      = page_pos_nil;
        page_pos_t prev_page_pos = page_pos_nil;
        page_pos_t next_page_pos = page_pos_nil;
    };


    /**
     * @brief         Common container page.
     * @details       Includes a `linked_page` at the beginning.
     * @tparam T      Item type.
     * @tparam Header Custom page header.
     */
    template <typename T, typename Header = noheader>
    struct container_page
        : public linked_page {

        Header     header     = { };
        item_pos_t item_count = 0;
        T          items[1]   = { };
    };


    /**
     * @brief    List page.
     * @details  Same as `container_page`.
     * @tparam T Item type.
     */
    template <typename T>
    struct list_page
        : public container_page<T, noheader> {
    };


    /**
     * @brief      Item on a map key page.
     * @tparam Key Key type.
     */
    template <typename Key>
    struct map_key {
        Key        key      = { };
        page_pos_t page_pos = { };
    };


    /**
     * @brief      Item on a map value page.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    struct map_value {
        Key key   = { };
        T   value = { };
    };


    /**
     * @brief      Map key page.
     * @tparam Key Key type.
     */
    template <typename Key>
    struct map_key_page
        : public container_page<map_key<Key>, noheader> {
    };


    /**
     * @brief      Map value page.
     * @tparam Key Key type.
     * @tparam T   Value type.
     */
    template <typename Key, typename T>
    struct map_value_page
        : public container_page<map_value<Key, T>, noheader> {
    };


    // ..............................................................


    /**
     * @brief Linked state - the linked pages of a container.
     */
    struct linked_state {
        page_pos_t front_page_pos = page_pos_nil;
        page_pos_t back_page_pos  = page_pos_nil;
    };


    /**
     * @brief   Common container state.
     * @details Includes a `linked_state` at the beginning.
     */
    struct container_state
        : public linked_state {

        item_pos_t  item_size        = 0;
        std::size_t total_item_count = 0;
        //// TODO: Keep the count of the back page in the state, so we don't have to map the back page to create an rend() iterator.
    };


    /**
     * @brief   List state.
     * @details Same as `container_state`.
     */
    struct list_state
        : public container_state {
    };


    /**
     * @brief   Stack state.
     * @details Same as `container_state`.
     */
    struct stack_state
        : public container_state {
    };


    /**
     * @brief   Map state.
     * @details Consists of a stack of key lists, and a value list.
     */
    struct map_state {
        stack_state     keys;
        container_state values;
    };


    /**
     * @brief   String state.
     * @details Same as `list_state`.
     */
    using string_state = list_state;


    // ..............................................................


    /**
     * @brief   Root page.
     * @details Not linked. Always at position 0.
     */
    struct root_page {
        const version_t     version       = 3;
        const char          signature[10] = "abc::vmem";
        const item_pos_t    page_size     = vmem::page_size;
        const std::uint16_t unused1       = 0xcccc;
        linked_state        free_pages;
        const std::uint8_t  unused2       = 0xcc;
    };


    #pragma pack(pop)


    // --------------------------------------------------------------

} }
