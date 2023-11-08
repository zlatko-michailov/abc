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
     * @brief JSON value.
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
        none         =  0,
        
        null         =  2,
        boolean      =  3,
        number       =  4,
        string       =  5,
        begin_array  =  8,
        end_array    =  9,
        begin_object = 10,
        end_object   = 11,
        property     = 12,
    };


    /**
     * @brief JSON stream token.
     */
    struct token {
        token_type type = token_type::none;

        ~token() noexcept;
        
        union {
            literal::boolean boolean;
            literal::number  number;
            literal::string  string;
            literal::string  property;
        };
    };



#if 0 //// TODO:
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


    using level_t = bool;

    namespace level {
        constexpr level_t array  = false;
        constexpr level_t object = true;
    }


    // --------------------------------------------------------------


#if 0 //// TODO:
    /**
     * @brief                Internal. State keeper.
     * @tparam MaxLevels    Maximum levels of nesting. Needed to define the stack.
     * @tparam Log            Logging facility.
     */
    template <std::size_t MaxLevels, typename Log>
    class json_state {
    protected:
        /**
         * @brief            Constructor.
         * @param log        Pointer to a `Log` instance. May be `nullptr`.
         */
        json_state(Log* log);

        /**
         * @brief            Move constructor.
         */
        json_state(json_state&& other) = default;

        /**
         * @brief            Copy constructor.
         */
        json_state(const json_state& other) = default;

    public:
        /**
         * @brief            Returns the current number of nesting levels.
         */
        std::size_t levels() const noexcept;

        /**
         * @brief            Returns the current nesting level - object or array.
         */
        json::level_t top_level() const noexcept;

    protected:
        /**
         * @brief            Resets the state.
         */
        void reset() noexcept;

        /**
         * @brief            Returns whether a property name is expected.
         */
        bool expect_property() const noexcept;

        /**
         * @brief            Sets whether a property name is expected.
         */
        void set_expect_property(bool expect) noexcept;

        /**
         * @brief            Pushes a nesting level into the stack.
         * @param level        Level - object or array.
         * @return            true = success. false = error. 
         */
        bool push_level(json::level_t level) noexcept;

        /**
         * @brief            Checks whether the nesting level at the top matches the given one, and pops it from the stack.
         * @param level        Level - object or array.
         * @return            true = success. false = error.
         */
        bool pop_level(json::level_t level) noexcept;

        /**
         * @brief            Returns the Log pointer.
         */
        Log* log() const noexcept;

    private:
        /**
         * @brief            Flag whether a property name is expected.
         */
        bool _expect_property;

        /**
         * @brief            Current top position on the stack.
         */
        std::size_t _level_top;

        /**
         * @brief            Nesting level stack.
         */
        std::bitset<MaxLevels> _level_stack;

        /**
         * @brief            The log passed in to the constructor.
         */
        Log* _log;
    };


    // --------------------------------------------------------------


    /**
     * @brief                JSON input stream.
     * @tparam MaxLevels    Maximum nesting levels - object/array.
     * @tparam Log            Logging facility.
     */
    template <std::size_t MaxLevels = size::_64, typename Log>
    class json_istream : public istream, public json_state<MaxLevels, Log> {
        using base  = istream;
        using state = json_state<MaxLevels, Log>;

    public:
        /**
         * @brief            Constructor.
         * @param sb        `std::streambuf` to read from.
         * @param log        Pointer to a `Log` instance. May be `nullptr`.
         */
        json_istream(std::streambuf* sb, Log* log = nullptr);

        /**
         * @brief            Move constructor.
         */
        json_istream(json_istream&& other);

        /**
         * @brief            Deleted.
         */
        json_istream(const json_istream& other) = delete;

    public:
        /**
         * @brief            Reads a JSON token from the stream, and copies it into the given buffer.
         * @param buffer    Buffer to copy to.
         * @param size        Buffer size.
         */
        void get_token(json::token_t* buffer, std::size_t size);

        /**
         * @brief            Skips a JSON value from the stream.
         * @return            The type of value skipped.
         */
        json::item_t skip_value();

    protected:
        /**
         * @brief            Gets or skips a JSON token from the stream.
         * @param buffer    If `nullptr`, the next token will be skipped. Otherwise, the next token will be copied to  the buffer.
         * @param size        Buffer size.
         * @return            The type of token.
         */
        json::item_t get_or_skip_token(json::token_t* buffer, std::size_t size);

        /**
         * @brief            Gets or skips a JSON number from the stream.
         * @param buffer    If `nullptr`, the next number will be skipped. Otherwise, the next number will be copied to the buffer.
         */
        void get_or_skip_number(double* buffer);

        /**
         * @brief            Gets or skips a JSON string from the stream.
         * @param buffer    If `nullptr`, the next string will be skipped. Otherwise, the next string will be copied to the buffer.
         * @param size        Buffer size.
         * @return            The length of the string.
         */
        std::size_t get_or_skip_string(char* buffer, std::size_t size);

    protected:
        /**
         * @brief            Reads the specified literal (any sequence of chars) from the stream.
         * @details            If the input does not match exactly, the "bad" flag on the stream is set.
         * @param literal    Sequence of chars.
         */
        void get_literal(const char* literal);

        /**
         * @brief            Reads an escape char. Note: For `\u????` escape sequences, only `\u00??` are supported.
         * @return            The char.
         */
        char get_escaped_char();

        /**
         * @brief            Gets or skips the inner part of a string.
         * @param buffer    If `nullptr` the content is skipped. Otherwise, the content is copied to the buffer.
         * @param size        Buffer size.
         * @return            The length of the content read. 
         */
        std::size_t get_or_skip_string_content(char* buffer, std::size_t size);

        /**
         * @brief            Read a hexadecimal number from the stream and copies it to the buffer.
         * @param buffer    Buffer to copy to.
         * @param size        Buffer size.
         * @return            Number of chars read.
         */
        std::size_t get_hex(char* buffer, std::size_t size);

        /**
         * @brief            Read a decimal number from the stream and copies it to the buffer.
         * @param buffer    Buffer to copy to.
         * @param size        Buffer size.
         * @return            Number of chars read.
         */
        std::size_t get_digits(char* buffer, std::size_t size);

        /**
         * @brief            Skips a sequence of spaces from the stream.
         * @return            Number of chars skipped.
         */
        std::size_t skip_spaces();

        /**
         * @brief            Reads a sequence of chars that match a predicate and copies it to the buffer.
         * @param predicate    Predicate.
         * @param buffer    Buffer
         * @param size        Buffer size.
         * @return            Number of chars read.
         */
        std::size_t get_chars(ascii::predicate_t&& predicate, char* buffer, std::size_t size);

        /**
         * @brief            Skips a sequence of chars that match a predicate.
         * @param predicate    Predicate.
         * @return            Number of chars read.
         */
        std::size_t skip_chars(ascii::predicate_t&& predicate);

        /**
         * @brief            Reads a char from the stream.
         * @return            The char read.
         */
        char get_char();

        /**
         * @brief            Returns the next char from the stream without advancing.
         */
        char peek_char();
    };


    // --------------------------------------------------------------


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

