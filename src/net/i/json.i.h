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

#include <cstddef>
#include <streambuf>
#include <istream>
#include <ostream>
#include <bitset>
#include <string>
#include <deque>
#include <map>
#include <stack>

#include "../../i/stream.i.h"
#include "../../diag/i/diag_ready.i.h"


namespace abc { namespace net { namespace json {

    /**
     * @brief Enumeration of JSON value types.
     */
    enum class value_type : std::uint8_t {
        empty        =  0,
        null         =  1,
        boolean      =  2,
        number       =  3,
        string       =  4,
        array        =  5,
        object       =  6,
    };


    /**
     * @brief Literal/primitive value types.
     */
    namespace literal {
        using null    = std::nullptr_t;
        using boolean = bool;
        using number  = double;
        using string  = std::string;

        template <typename Value>
        using _array  = std::deque<Value>;

        template <typename Value>
        using _object = std::map<string, Value>;
    }


    /**
     * @brief         JSON value.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class value
        : diag::diag_ready<const char*, LogPtr> {

        using diag_base = diag::diag_ready<const char*, LogPtr>;

        const char* _origin = "abc::net::json::value";

    public:
        /**
         * @brief     Constructor - empty value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(const LogPtr& log = nullptr) noexcept;

        /**
         * @brief     Constructor - null value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(literal::null, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief     Constructor - boolean value.
         * @param b   Boolean value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(literal::boolean b, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief     Constructor - number value.
         * @param n   Number value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(literal::number n, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief     Constructor - string value.
         * @param str String value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(const char* str, const LogPtr& log = nullptr);

        /**
         * @brief     Constructor - string value.
         * @param str String value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(literal::string&& str, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief     Constructor - array value.
         * @param arr Array value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(literal::_array<value<LogPtr>>&& arr, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief     Constructor - object value.
         * @param obj Object value.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        value(literal::_object<value<LogPtr>>&& obj, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief Copy constructor.
         */
        value(const value& other);

        /**
         * @brief Move constructor.
         */
        value(value&& other) noexcept;

        /**
         * @brief Destructor.
         */
        ~value() noexcept;

    public:
        /**
         * @brief Clears.
         */
        void clear() noexcept;

        /**
         * @brief Copy assignment.
         */
        value& operator = (const value& other);

        /**
         * @brief Move assignment.
         */
        value& operator = (value&& other) noexcept;

    public:
        /**
         * @brief Returns the type of the JSON value.
         */
        value_type type() const noexcept;

        /**
         * @brief   Returns the boolean value.
         * @details Throws if the type is not boolean.
         */
        literal::boolean boolean() const;

        /**
         * @brief   Returns the number value.
         * @details Throws if the type is not number.
         */
        literal::number number() const;

        /**
         * @brief   Returns the string value.
         * @details Throws if the type is not string.
         */
        const literal::string& string() const;

        /**
         * @brief   Returns a mutable reference to the string value.
         * @details Throws if the type is not string.
         */
        literal::string& string();

        /**
         * @brief   Returns the array value.
         * @details Throws if the type is not array.
         */
        const literal::_array<value<LogPtr>>& array() const;

        /**
         * @brief   Returns a mutable reference to the array value.
         * @details Throws if the type is not array.
         */
        literal::_array<value<LogPtr>>& array();

        /**
         * @brief   Returns the object value.
         * @details Throws if the type is not object.
         */
        const literal::_object<value<LogPtr>>& object() const;

        /**
         * @brief   Returns a mutable reference to the object value.
         * @details Throws if the type is not object.
         */
        literal::_object<value<LogPtr>>& object();

    public:
        /**
         * @brief Returns `true` iff the two values are equal.
         */
        bool operator == (const value& other) const noexcept;

        /**
         * @brief Returns `true` iff the two values are not equal.
         */
        bool operator != (const value& other) const noexcept;

    private:
        /**
         * @brief Copy constructions.
         */
        void copy_from(const value& other);

        /**
         * @brief Move construction.
         */
        void move_from(value&& other) noexcept;

    private:
        /**
         * @brief The type of the JSON value.
         */
        value_type _type = value_type::empty;

        union {
            /**
             * @brief Potential boolean value.
             */
            literal::boolean _boolean;

            /**
             * @brief Potential number value.
             */
            literal::number _number;

            /**
             * @brief Potential string value.
             */
            literal::string _string;

            /**
             * @brief Potential array value.
             */
            literal::_array<value> _array;

            /**
             * @brief Potential object value.
             */
            literal::_object<value> _object;
        };
    };


    namespace literal {
        template <typename LogPtr = std::nullptr_t>
        using array  = _array<value<LogPtr>>;

        template <typename LogPtr = std::nullptr_t>
        using object = _object<value<LogPtr>>;
    }


    /**
     * @brief Enumeration of JSON stream token types.
     */
    enum class token_type : std::uint8_t {
        empty        =  0,
        null         =  1,
        boolean      =  2,
        number       =  3,
        string       =  4,
        begin_array  = 11,
        end_array    = 12,
        begin_object = 13,
        end_object   = 14,
        property     = 15,
    };


    /**
     * @brief JSON stream token.
     */
    struct token {
        token_type type = token_type::empty;

        literal::boolean boolean;
        literal::number  number;
        literal::string  string;
    };



#if 0 //// TODO: REMOVE
    using item_t = std::uint16_t;

    namespace item {
        constexpr item_t none            = 0;

        constexpr item_t null            = 0x0001;
        constexpr item_t boolean        = 0x0002;
        constexpr item_t number            = 0x0004;
        constexpr item_t string            = 0x0008;
        constexpr item_t begin_array    = 0x0010;
        constexpr item_t end_array        = 0x0020;
        constexpr item_t begin_object    = 0x0040;
        constexpr item_t end_object        = 0x0080;
        constexpr item_t property        = 0x0100;
    }


    /**
     * @brief            JSON value union.
     */
    union value_t {
        bool    boolean;
        double    number;
        char    string[1];
        char    property[1];
    };
    
    /**
     * @brief            JSON token.
     */
    struct token_t {
        item_t    item;
        value_t    value;
    };
#endif


    enum class nest_type : std::uint8_t {
        none   = 0,
        array  = 1,
        object = 2,
    };


    // --------------------------------------------------------------


    /**
     * @brief         Internal. Keeps stream state.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class state
        : protected diag::diag_ready<const char*, LogPtr> {

        using diag_base = diag::diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        state(const char* origin, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        state(state&& other) = default;

        /**
         * @brief Copy constructor.
         */
        state(const state& other) = default;

    protected:
        /**
         * @brief Returns a reference to the nest stack - arrays and objects.
         */
        const std::stack<nest_type>& nest_stack() const noexcept;

    protected:
        /**
         * @brief Resets the state.
         */
        void reset() noexcept;

        /**
         * @brief Returns whether a property name is expected.
         */
        bool expect_property() const noexcept;

        /**
         * @brief Sets whether a property name is expected.
         */
        void set_expect_property(bool expect);

        /**
         * @brief      Pushes a nest into the stack.
         * @param type Nest type - array or object.
         */
        void nest(nest_type type);

        /**
         * @brief      Checks whether the nest type at the top of the stack matches the given one, and pops it from the stack.
         * @param type Expected nest type - array or object.
         */
        void unnest(nest_type type);

    private:
        /**
         * @brief Flag whether a property name is expected.
         */
        bool _expect_property;

        /**
         * @brief Nest stack - arrays and objects.
         */
        std::stack<nest_type> _nest_stack;
    };


    // --------------------------------------------------------------


    /**
     * @brief         JSON input stream.
     * @details       Reads a JSON payload token by token. To deserialize a `json::value`, see `json::reader`.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class istream
        : public abc::istream
        , public state<LogPtr> {

        using base       = abc::istream;
        using state_base = state<LogPtr>;
        using diag_base  = diag::diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        istream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        istream(istream&& other);

        /**
         * @brief Deleted.
         */
        istream(const istream& other) = delete;

    public:
        /**
         * @brief Reads a JSON token from the stream.
         */
        token get_token();

        /**
         * @brief Skips a JSON value from the stream.
         */
        void skip_value();

    protected:
        /**
         * @brief Reads a number from the stream.
         */
        literal::number get_number();

        /**
         * @brief Reads a quoted string from the stream.
         */
        literal::string get_string();

        /**
         * @brief Reads a literal from the stream.
         */
        literal::string get_literal();

        /**
         * @brief Reads an escaped char.
         * @note  For `\u????` escape sequences, only `\u00??` are supported.
         */
        char get_escaped_char();

        /**
         * @brief Reads the inner part of a string from the stream.
         */
        literal::string get_string_content();

        /**
         * @brief Read a hexadecimal number from the stream.
         */
        literal::string get_hex();

        /**
         * @brief Read a decimal number from the stream.
         */
        literal::string get_digits();

        /**
         * @brief  Skips a sequence of spaces from the stream.
         * @return Number of chars skipped.
         */
        std::size_t skip_spaces();

        /**
         * @brief           Reads a sequence of chars from the stream that match a predicate.
         * @param predicate Predicate.
         */
        literal::string get_chars(ascii::predicate_t&& predicate);

        /**
         * @brief           Skips a sequence of chars from the stream that match a predicate.
         * @param predicate Predicate.
         * @return          Number of chars skipped.
         */
        std::size_t skip_chars(ascii::predicate_t&& predicate);

        /**
         * @brief Reads a char from the stream.
         */
        char get_char();

        /**
         * @brief Returns the next char from the stream without advancing.
         */
        char peek_char();
    };


    // --------------------------------------------------------------


#if 0 //// TODO:
    /**
     * @brief                JSON output stream.
     * @tparam MaxLevels    Maximum nesting levels - object/array.
     * @tparam Log            Logging facility.
     */
    template <std::size_t MaxLevels = size::_64, typename Log>
    class json_ostream : public ostream, public json_state<MaxLevels, Log> {
        using base  = ostream;
        using state = json_state<MaxLevels, Log>;

    public:
        /**
         * @brief            Constructor.
         * @param sb        `std::streambuf` to read from.
         * @param log        Pointer to a `Log` instance. May be `nullptr`.
         */
        json_ostream(std::streambuf* sb, Log* log = nullptr);

        /**
         * @brief            Move constructor.
         */
        json_ostream(json_ostream&& other);

        /**
         * @brief            Deleted.
         */
        json_ostream(const json_ostream& other) = delete;

    public:
        /**
         * @brief            Writes a JSON token to the stream from the buffer.
         * @param buffer    Buffer.
         * @param size        Content size.
         */
        void put_token(const json::token_t* buffer, std::size_t size = size::strlen);

        /**
         * @brief            Writes a space to the stream.
         */
        void put_space();

        /**
         * @brief            Writes a tab to the stream.
         */
        void put_tab();

        /**
         * @brief            Writes a CR to the stream.
         */
        void put_cr();

        /**
         * @brief            Writes a LF to the stream.
         */
        void put_lf();

        /**
         * @brief            Writes `null` to the stream.
         */
        void put_null();

        /**
         * @brief            Writes a boolean value to the stream.
         * @param value        The value to write.
         */
        void put_boolean(bool value);

        /**
         * @brief            Writes a number value to the stream.
         * @param value        The value to write.
         */
        void put_number(double value);

        /**
         * @brief            Writes a string value to the stream.
         * @param buffer    The string to write.
         * @param size        String length.
         */
        void put_string(const char* buffer, std::size_t size = size::strlen);

        /**
         * @brief            Writes a property name to the stream.
         * @param buffer    The property name to write.
         * @param size        Property name length.
         */
        void put_property(const char* buffer, std::size_t size = size::strlen);

        /**
         * @brief            Writes `[` to the stream.
         */
        void put_begin_array();

        /**
         * @brief            Writes `]` to the stream.
         */
        void put_end_array();

        /**
         * @brief            Writes `{` to the stream.
         */
        void put_begin_object();

        /**
         * @brief            Writes `}` to the stream.
         */
        void put_end_object();

    public:
        /**
         * @brief            Writes a sequence of chars to the stream.
         * @param buffer    The sequence to write.
         * @param size        Sequence length.
         */
        std::size_t put_chars(const char* buffer, std::size_t size);

        /**
         * @brief            Writes a char to the stream.
         * @param ch        Char to write.
         * @return            1 = success. 0 = error.
         */
        std::size_t put_char(char ch);

    private:
        /**
         * @brief            State flag whether comma `,` should be written before the next value. true = skip. false = write.
         */
        bool _skip_comma;
    };
#endif

} } }

