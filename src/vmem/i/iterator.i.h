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

#include "../../diag/i/diag_ready.i.h"
#include "layout.i.h"
#include "ptr.i.h"


namespace abc { namespace vmem {

    /**
     * @brief Iterator edge - special positions.
     */
    enum class iterator_edge : std::uint8_t {
        none   = 0,
        rbegin = 1,
        end    = 2,
    };


    // --------------------------------------------------------------


    /**
     * @brief            Generic iterator state. For internal use.
     * @details          This class does the heavy lifting for iterators.
     * @tparam Container Container type.
     */
    template <typename Container>
    class basic_iterator_state
        : public diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

    private:
        static constexpr const char* origin() noexcept;

    public:
        using container_type = Container;

    public:
        /**
         * @brief           Constructor.
         * @param container Container.
         * @param page_pos  Page position.
         * @param item_pos  Item position.
         * @param edge      Edge.
         * @param log       Pointer to a `log_ostream` instance.
         */
        basic_iterator_state<Container>(const Container* container, page_pos_t page_pos, item_pos_t item_pos, iterator_edge edge, diag::log_ostream* log = nullptr) noexcept;

        /**
         * @brief Move constructor.
         */
        basic_iterator_state<Container>(basic_iterator_state<Container>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        basic_iterator_state<Container>(const basic_iterator_state<Container>& other) = default;

        /**
         * @brief Default-like constructor.
         */
        basic_iterator_state<Container>(std::nullptr_t, diag::log_ostream* log = nullptr) noexcept;
    
    public:
        basic_iterator_state<Container>& operator =(const basic_iterator_state<Container>& other) = default;
        basic_iterator_state<Container>& operator =(basic_iterator_state<Container>&& other) noexcept = default;

    public:
        bool operator ==(const basic_iterator_state<Container>& other) const noexcept;
        bool operator !=(const basic_iterator_state<Container>& other) const noexcept;

    public:
        /**
         * @brief Checks whether this iterator state is associated with a container.
         */
        bool is_valid() const noexcept;

        /**
         * @brief           Checks whether this iterator state is associated with the given container.
         * @param container Container.
         */
        bool is_valid(const Container* container) const noexcept;

        /**
         * @brief Checks whether this iterator state can be dereferenced.
         */
        bool can_deref() const noexcept;

    public:
        /**
         * @brief Checks whether this iterator state represents rbegin.
         */
        bool is_rbegin() const noexcept;

        /**
         * @brief Checks whether this iterator state represents end.
         */
        bool is_end() const noexcept;

    public:
        /**
         * @brief Returns the container.
         */
        const Container* container() const noexcept;

        /**
         * @brief Returns the page position.
         */
        page_pos_t page_pos() const noexcept;

        /**
         * @brief Returns the item position.
         */
        item_pos_t item_pos() const noexcept;

        /**
         * @brief Returns the edge.
         */
        iterator_edge edge() const noexcept;

    protected:
        const Container* _container;
        page_pos_t       _page_pos;
        item_pos_t       _item_pos;
        iterator_edge    _edge;
    };


    /**
     * @brief            Generic iterator. For internal use.
     * @details          This class is a stateless wrapper around `basic_iterator_state`.
     * @tparam Base      Base class - a `basic_iterator_state` specialization.
     * @tparam Container Container.
     * @tparam T         Item type.
     */
    template <typename Base, typename Container, typename T>
    class basic_iterator
        : public Base {

        using base = Base;
        using diag_base = diag::diag_ready<const char*>;

    public:
        using value_type      = T;
        using pointer         = vmem::ptr<T>;
        using const_pointer   = vmem::ptr<const T>;
        using reference       = T&;
        using const_reference = const T&;

    public:
        /**
         * @brief           Constructor.
         * @param container Container
         * @param page_pos  Page position.
         * @param item_pos  Item position.
         * @param edge      Edge.
         * @param log       Pointer to a `log_ostream` instance.
         */
        basic_iterator<Base, Container, T>(const Container* container, page_pos_t page_pos, item_pos_t item_pos, iterator_edge edge, diag::log_ostream* log = nullptr) noexcept;

        /**
         * @brief Move constructor.
         */
        basic_iterator<Base, Container, T>(basic_iterator<Base, Container, T>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        basic_iterator<Base, Container, T>(const basic_iterator<Base, Container, T>& other) = default;

        /**
         * @brief Copy constructor from const_iterator.
         */
        template <typename OtherIterator>
        basic_iterator<Base, Container, T>(const OtherIterator& other) noexcept;

        /**
         * @brief Default-like constructor.
         */
        basic_iterator<Base, Container, T>(std::nullptr_t, diag::log_ostream* log = nullptr) noexcept;

    public:
        basic_iterator<Base, Container, T>& operator =(const basic_iterator<Base, Container, T>& other) = default;
        basic_iterator<Base, Container, T>& operator =(basic_iterator<Base, Container, T>&& other) noexcept = default;

    public:
        bool operator ==(const basic_iterator<Base, Container, T>& other) const noexcept;
        bool operator !=(const basic_iterator<Base, Container, T>& other) const noexcept;
        basic_iterator<Base, Container, T>& operator ++() noexcept;
        basic_iterator<Base, Container, T>  operator ++(int) noexcept;
        basic_iterator<Base, Container, T>& operator --() noexcept;
        basic_iterator<Base, Container, T>  operator --(int) noexcept;

    public:
        pointer         operator ->() noexcept;
        const_pointer   operator ->() const noexcept;
        reference       operator *();
        const_reference operator *() const;

    private:
        friend Container;

        /**
         * @brief Returns a `ptr` pointing at the item in memory, if the iterator is valid.
         */
        pointer ptr() const noexcept;

        /**
         * @brief Returns a reference to the item in memory, if the iterator is valid. Otherwise, it throws.
         */
        reference deref() const;
    };


    /**
     * @brief            Generic const iterator.
     * @tparam Container Container.
     * @tparam T         Item type.
     */
    template <typename Container, typename T>
    using const_iterator = basic_iterator<basic_iterator_state<Container>, Container, const T>;


    /**
     * @brief            Generic iterator.
     * @tparam Container Container.
     * @tparam T         Item type.
     */
    template <typename Container, typename T>
    using iterator = basic_iterator<const_iterator<Container, T>, Container, T>;


    // --------------------------------------------------------------

} }
