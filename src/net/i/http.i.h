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

#include <streambuf>
#include <istream>
#include <ostream>
#include <string>
#include <unordered_map>

#include "../../i/stream.i.h"
#include "../../diag/i/diag_ready.i.h"


namespace abc { namespace net {

    namespace http {
        using item_t = std::uint8_t;

        namespace item {
            constexpr item_t method        = 0;
            constexpr item_t resource      = 1;
            constexpr item_t protocol      = 2;
            constexpr item_t status_code   = 3;
            constexpr item_t reason_phrase = 4;
            constexpr item_t header_name   = 5;
            constexpr item_t header_value  = 6;
            constexpr item_t body          = 7;
        }
    }


    // --------------------------------------------------------------


    /**
     * @brief http resource parsed into path and parameters. 
     */
    struct http_resource {
        std::string                                  path;
        std::unordered_map<std::string, std::string> parameters;
    };


    /**
     * @brief Collection of http headers. 
     */
    using http_headers = std::unordered_map<std::string, std::string>;


    /**
     * @brief http status code. 
     */
    using http_status_code = std::uint16_t;


    // --------------------------------------------------------------


    /**
     * @brief         Internal. http semantic state. 
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_state
        : protected diag_ready<const char*, LogPtr> {

        using diag_base = diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param next   Next item.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        http_state(const char* origin, http::item_t next, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief Move constructor.
         */
        http_state(http_state&& other) = default;

        /**
         * @brief Copy constructor.
         */
        http_state(const http_state& other) = default;

    public:
        /**
         * @brief Returns the next expected item.
         */
        http::item_t next() const noexcept;

    protected:
        /**
         * @brief      Resets the next item.
         * @param next Next item.
         */
        void reset(http::item_t next);

        /**
         * @brief      Throws if `item` doesn't match `next()`.
         * @param item Item to verify.
         */
        void assert_next(http::item_t item);

    private:
        /**
         * @brief The next expected item.
         */
        http::item_t _next;
    };


    // --------------------------------------------------------------


    /**
     * @brief         Internal. Common http input stream. Used to read a request on the server or to read a response on the client.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_istream
        : public istream
        , public http_state<LogPtr> {

        using base      = istream;
        using state     = http_state<LogPtr>;
        using diag_base = diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param sb     `std::streambuf` to read from.
         * @param next   Next expected item.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        http_istream(const char* origin, std::streambuf* sb, http::item_t next, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_istream(http_istream&& other);

        /**
         * @brief Deleted.
         */
        http_istream(const http_istream& other) = delete;

    protected:
        /**
         * @brief  Reads the protocol.
         * @return The protocol.
         */
        std::string get_protocol();

        /**
         * @brief  Reads headers from the http stream.
         * @return The headers.
         */
        http_headers get_headers();

        /**
         * @brief  Reads a header name from the http stream.
         * @return The header name.
         */
        std::string get_header_name();

        /**
         * @brief  Reads a header value from the http stream.
         * @return The header value.
         */
        std::string get_header_value();

        /**
         * @brief         Reads a chunk of a body from the http stream.
         * @param max_len Maximum length of the chunk.
         * @return        The chunk. Empty when there is no more to read.
         */
        std::string get_body(std::size_t max_len);

        /**
         * @brief  Reads an http token.
         * @return The token.
         */
        std::string get_token();

        /**
         * @brief  Reads a sequence of printable chars.
         * @return The sequence.
         */
        std::string get_prints();

        /**
         * @brief  Reads a sequence of printable chars and spaces.
         * @return The sequence.
         */
        std::string get_prints_and_spaces();

        /**
         * @brief  Reads a sequence of letters.
         * @return The sequence.
         */
        std::string get_alphas();

        /**
         * @brief  Reads a sequence of digits.
         * @return The sequence.
         */
        std::string get_digits();

        /**
         * @brief         Reads a sequence of any chars through the end of the stream.
         * @param max_len Maximum length of the chunk.
         * @return        The sequence.
         */
        std::string get_any_chars(std::size_t max_len);

        /**
         * @brief           Reads a sequence of chars that match a predicate.
         * @param predicate Predicate.
         * @param max_len   Maximum length of the chunk.
         * @return          The sequence.
         */
        std::string get_chars(ascii::predicate_t&& predicate, std::size_t max_len = size::strlen);

        /**
         * @brief  Gets the next char from the stream and moves forward.
         * @return The next char.
         */
        char get_char();

        /**
         * @brief Returns the next char from the stream without moving forward.
         */
        char peek_char();

        /**
         * @brief  Skips a sequence of spaces.
         * @return The count of chars read.
         */
        std::size_t skip_spaces();

        /**
         * @brief  Skips a CR LF sequence.
         * @return The count of chars read.
         */
        std::size_t skip_crlf();

        /**
         * @brief           Skips a sequence of chars that match a predicate.
         * @param predicate Predicate.
         * @return          The count of chars read.
         */
        std::size_t skip_chars(ascii::predicate_t&& predicate);

        /**
         * @brief        Set the gcount and the next expected item for this input stream.
         * @param gcount gcount.
         * @param next   Next expected item.
         */
        void set_gstate(std::size_t gcount, http::item_t next);

    };


    // --------------------------------------------------------------


    /**
     * @brief         Internal. Common http output stream. Used to write a request on the client or to write a response on the server.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_ostream
        : public ostream
        , public http_state<LogPtr> {

        using base      = ostream;
        using state     = http_state<LogPtr>;
        using diag_base = diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param sb     `std::streambuf` to write to.
         * @param next   Next expected item.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        http_ostream(const char* origin, std::streambuf* sb, http::item_t next, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_ostream(http_ostream&& other);

        /**
         * @brief Deleted.
         */
        http_ostream(const http_ostream& other) = delete;

    public:
        /**
         * @brief         Writes headers and headers end to the http stream.
         * @param headers Headers.
         */
        void put_headers(const http_headers& headers);

        /**
         * @brief                 Writes a header name to the http stream.
         * @param header_name     Header name.
         * @param header_name_len Header name length. Optional.
         */
        void put_header_name(const char* header_name, std::size_t header_name_len = size::strlen);

        /**
         * @brief                  Writes a header value to the http stream.
         * @param header_value     Header value.
         * @param header_value_len Header value length. Optional.
         */
        void put_header_value(const char* header_value, std::size_t header_value_len = size::strlen);

        /**
         * @brief Writes the end of headers to the http stream.
         */
        void end_headers();

        /**
         * @brief          Writes a body to the http stream.
         * @param body     Body.
         * @param body_len Body length. Optional.
         */
        void put_body(const char* body, std::size_t body_len = size::strlen);

    protected:
        /**
         * @brief              Writes a protocol to the http stream.
         * @param protocol     Protocol.
         * @param protocol_len Protocol length. Optional.
         * @return             The count of chars written. 
         */
        std::size_t put_protocol(const char* protocol, std::size_t protocol_len = size::strlen);

        /**
         * @brief           Writes a token to the http stream.
         * @param token     Token.
         * @param token_len Token length. Optional.
         * @return          The count of chars written. 
         */
        std::size_t put_token(const char* token, std::size_t token_len = size::strlen);

        /**
         * @brief            Writes a sequence of printable chars to the http stream.
         * @param prints     Sequence of printable chars.
         * @param prints_len Sequence length. Optional.
         * @return           The count of chars written. 
         */
        std::size_t put_prints(const char* prints, std::size_t prints_len = size::strlen);

        /**
         * @brief                       Writes a sequence of printable chars and spaces to the http stream.
         * @param prints_and_spaces     Sequence of printable chars and spaces.
         * @param prints_and_spaces_len Sequence length. Optional.
         * @return                      The count of chars written. 
         */
        std::size_t put_prints_and_spaces(const char* prints_and_spaces, std::size_t prints_and_spaces_len = size::strlen);

        /**
         * @brief            Writes a sequence of letters to the http stream.
         * @param alphas     Sequence of letters.
         * @param alphas_len Sequence length. Optional.
         * @return           The count of chars written. 
         */
        std::size_t put_alphas(const char* alphas, std::size_t alphas_len = size::strlen);

        /**
         * @brief            Writes a sequence of digits to the http stream.
         * @param digits     Sequence of digits.
         * @param digits_len Sequence length. Optional.
         * @return           The count of chars written. 
         */
        std::size_t put_digits(const char* digits, std::size_t digits_len = size::strlen);

        /**
         * @brief  Writes a CR LF sequence to the http stream.
         * @return The count of chars written. 
         */
        std::size_t put_crlf();

        /**
         * @brief   Writes a space to the http stream.
         * @return The count of chars written. 
         */
        std::size_t put_space();

        /**
         * @brief               Writes a sequence of any chars to the http stream.
         * @param any_chars     Sequence of chars.
         * @param any_chars_len Sequence length.
         * @return              The count of chars written. 
         */
        std::size_t put_any_chars(const char* any_chars, std::size_t any_chars_len);

        /**
         * @brief           Writes a sequence of chars that match a predicate to the http stream.
         * @param predicate Predicate.
         * @param chars     Sequence of chars.
         * @param chars_len Sequence length. Optional.
         * @return          The count of chars written. 
         */
        std::size_t put_chars(ascii::predicate_t&& predicate, const char* chars, std::size_t chars_len = size::strlen);

        /**
         * @brief    Writes a char to the http stream.
         * @param ch Char to write.
         * @return   1 = success. 0 = error.
         */
        std::size_t put_char(char ch);

        /**
         * @brief                  Counts the leading spaces in a header value. Does not write to the http stream.
         * @param header_value     Header value.
         * @param header_value_len Header value length. Optional.
         * @return                 The count of chars. 
         */
        std::size_t count_leading_spaces_in_header_value(const char* header_value, std::size_t header_value_len = size::strlen);

        /**
         * @brief             Counts the leading spaces in a given content. Does not write to the http stream.
         * @param content     Content.
         * @param content_len Content length. Optional.
         * @return            The count of chars. 
         */
        //// TODO: std::size_t count_leading_spaces(const char* content, std::size_t content_len = size::strlen);

        /**
         * @brief      Sets the next expected item for the stream.
         * @param next The next expected item.
         */
        void set_pstate(http::item_t next);
    };


    // --------------------------------------------------------------


    /**
     * @brief         http request input stream. Used on the server side to read requests.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_request_istream
        : public http_istream<LogPtr> {

        using base      = http_istream<LogPtr>;
        using diag_base = diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        http_request_istream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_request_istream(http_request_istream&& other);

        /**
         * @brief Deleted.
         */
        http_request_istream(const http_request_istream& other) = delete;

        /**
         * @brief Resets the read state.
         */
        void reset();

    public:
        /**
         * @brief  Reads an http method from the http stream.
         * @return The method.
         */
        std::string get_method();

        /**
         * @brief  Reads an http resource from the http stream.
         * @return The resource.
         */
        http_resource get_resource();

        /**
         * @brief  Reads an http protocol from the http stream.
         * @return The protocol.
         */
        std::string get_protocol();

    protected:
        /**
         * @brief              Splits a raw http resource.
         * @param raw_resource Resource.
         */
        http_resource split_resource(const std::string& raw_resource);
    };


    // --------------------------------------------------------------


    /**
     * @brief  http request output stream. Used on the client side to write requests.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_request_ostream
        : public http_ostream<LogPtr> {

        using base      = http_ostream<LogPtr>;
        using diag_base = diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        http_request_ostream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_request_ostream(http_request_ostream&& other);

        /**
         * @brief Deleted.
         */
        http_request_ostream(const http_request_ostream& other) = delete;

        /**
         * @brief Resets the write state.
         */
        void reset();

    public:
        /**
         * @brief            Writes an http method to the http stream.
         * @param method     Method.
         * @param method_len Method length. Optional.
         */
        void put_method(const char* method, std::size_t method_len = size::strlen);

        /**
         * @brief              Writes an http resource to the http stream.
         * @param resource     Resource.
         * @param resource_len Resource length. Optional.
         */
        void put_resource(const char* resource, std::size_t resource_len = size::strlen);

        /**
         * @brief              Writes an http protocol to the http stream.
         * @param protocol     Protocol.
         * @param protocol_len Protocol length. Optional.
         */
        void put_protocol(const char* protocol, std::size_t protocol_len = size::strlen);
    };


    // --------------------------------------------------------------


    /**
     * @brief         http response input stream. Used on the client side to read responses.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_response_istream
        : public http_istream<LogPtr> {

        using base      = http_istream<Log>;
        using diag_base = diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        http_response_istream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_response_istream(http_response_istream&& other);

        /**
         * @brief Deleted.
         */
        http_response_istream(const http_response_istream& other) = delete;

        /**
         * @brief Resets the read state.
         */
        void reset();

    public:
        /**
         * @brief Reads an http protocol from the http stream.
         */
        std::string get_protocol();

        /**
         * @brief Reads an http status code from the http stream.
         */
        http_status_code get_status_code();

        /**
         * @brief Reads an http reason phrase from the http stream.
         */
        std::string get_reason_phrase();
    };


    // --------------------------------------------------------------


    /**
     * @brief         http response output stream. Used on the server side to write responses.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_response_ostream
        : public http_ostream<LogPtr> {

        using base      = http_ostream<LogPtr>;
        using diag_base = diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        http_response_ostream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_response_ostream(http_response_ostream&& other);

        /**
         * @brief Deleted.
         */
        http_response_ostream(const http_response_ostream& other) = delete;

        /**
         * @brief Resets the write state.
         */
        void reset();

    public:
        /**
         * @brief              Writes an http protocol to the http stream.
         * @param protocol     Protocol.
         * @param protocol_len Protocol length. Optional.
         */
        void put_protocol(const char* protocol, std::size_t protocol_len = size::strlen);

        /**
         * @brief Writes an http status code to the http stream.
         */
        void put_status_code(http_status_code status_code);

        /**
         * @brief                   Writes an http reason phrase to the http stream.
         * @param reason_phrase     Reason phrase.
         * @param reason_phrase_len Reason phrase length. Optional.
         */
        void put_reason_phrase(const char* reason_phrase, std::size_t reason_phrase_len = size::strlen);
    };


    // --------------------------------------------------------------


    /**
     * @brief         Combination of `http_request_ostream` and `http_response_istream`. Used on the client side.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_client_stream
        : public http_request_ostream<LogPtr>
        , public http_response_istream<LogPtr> {

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from and to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        http_client_stream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_client_stream(http_client_stream&& other);

        /**
         * @brief Deleted.
         */
        http_client_stream(const http_client_stream& other) = delete;
    };


    // --------------------------------------------------------------


    /**
     * @brief         Combination of `http_request_istream` and `http_response_ostream`. Used on the server side.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class http_server_stream
        : public http_request_istream<LogPtr>
        , public http_response_ostream<LogPtr> {

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from and to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        http_server_stream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        http_server_stream(http_server_stream&& other);

        /**
         * @brief Deleted.
         */
        http_server_stream(const http_server_stream& other) = delete;
    };

} }
