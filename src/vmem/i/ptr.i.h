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
#include "page.i.h"


namespace abc { namespace vmem {

    // --------------------------------------------------------------


    class pool;


    // --------------------------------------------------------------


    /**
     * @brief    Virtual memory (vmem) pointer.
     * @tparam T Type of pointed item.
     */
    template <typename T>
    class ptr
        : protected diag::diag_ready<const char*> {

        using diag_base = diag::diag_ready<const char*>;

        static constexpr const char* _origin = "abc::vmem::ptr";

    public:
        /**
         * @brief          Constructor.
         * @details        Contains a `page` instance for the referenced page to keep it locked. 
         * @param pool     Pointer to a `pool` instance.
         * @param page_pos Page position.
         * @param byte_pos Byte position on the page. //// TODO: Can this be item position?
         * @param log      Pointer to a `log_ostream` instance.
         */
        ptr<T>(pool* pool, page_pos_t page_pos, item_pos_t byte_pos, diag::log_ostream* log = nullptr);

        /**
         * @brief Move constructor.
         */
        ptr<T>(ptr<T>&& other) noexcept = default;

        /**
         * @brief Copy constructor.
         */
        ptr<T>(const ptr<T>& other) noexcept = default;

        /**
         * @brief     Constructor.
         * @details   Constructs an invalid/null pointer.
         * @param log Pointer to a `log_ostream` instance.
         */
        ptr<T>(std::nullptr_t, diag::log_ostream* log = nullptr) noexcept;

        /**
         * @brief Destructor.
         */
        ~ptr<T>() = default;

    public:
        ptr<T>& operator =(const ptr<T>& other) noexcept = default;
        ptr<T>& operator =(ptr<T>&& other) noexcept = default;

    public:
        /**
         * @brief Returns the pointer to the Pool instance passed in to the constructor.
         */
        vmem::pool* pool() const noexcept;

        /**
         * @brief Returns the pointer's page position in the pool.
         */
        page_pos_t page_pos() const noexcept;

        /**
         * @brief Returns the pointer's byte position on the page.
         */
        item_pos_t byte_pos() const noexcept;

        /**
         * @brief Returns a typed pointer.
         */
        operator T*() noexcept;

        /**
         * @brief Returns a typed pointer.
         */
        operator const T*() const noexcept;

        /**
         * @brief Returns a typed pointer.
         */
        T* operator ->() noexcept;

        /**
         * @brief Returns a typed pointer.
         */
        const T* operator ->() const noexcept;

        /**
         * @brief Returns a typed reference.
         */
        T& operator *();

        /**
         * @brief Returns a typed reference.
         */
        const T& operator *() const;

    private:
        /**
         * @brief Returns a typed pointer.
         */
        T* p() const noexcept;

        /**
         * @brief Returns a typed reference.
         */
        T& deref() const;

    protected:
        page       _page;
        item_pos_t _byte_pos;
    };


    // --------------------------------------------------------------

} }
