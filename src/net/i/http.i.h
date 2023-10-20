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


namespace abc { namespace net { namespace http {

    
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
        constexpr item_t eof           = 8;
    }


    // --------------------------------------------------------------


    /**
     * @brief Complete http resource. 
     */
    struct resource {
        std::string                                  path;
        std::unordered_map<std::string, std::string> query;
        std::string                                  fragment;
    };


    /**
     * @brief Collection of http headers. 
     */
    using headers = std::unordered_map<std::string, std::string>;


    /**
     * @brief http status code. 
     */
    using status_code_t = std::uint16_t;


    /**
     * @brief Common request/response properties. 
     */
    struct common {
        std::string protocol { "HTTP/1.1" };
        headers     headers;
    };


    /**
     * @brief Request (minus the body). 
     */
    struct request
        : public common {

        std::string method;
        resource    resource;
    };


    /**
     * @brief Response (minus the body). 
     */
    struct response
        : public common {

        status_code_t status_code;
        std::string   reason_phrase;
    };


    // --------------------------------------------------------------


    /**
     * @brief         Internal. http semantic state. 
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
         * @param next   Next item.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        state(const char* origin, http::item_t next, const LogPtr& log = nullptr) noexcept;

        /**
         * @brief Move constructor.
         */
        state(state&& other) = default;

        /**
         * @brief Copy constructor.
         */
        state(const state& other) = default;

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
    class istream
        : public abc::istream
        , public state<LogPtr> {

        using base      = abc::istream;
        using state     = state<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param sb     `std::streambuf` to read from.
         * @param next   Next expected item.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        istream(const char* origin, std::streambuf* sb, http::item_t next, const LogPtr& log = nullptr);

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
         * @brief         Reads a chunk of a body from the http stream.
         * @details       The whole request/response must have been read before this method is called.
         * @param max_len Maximum length of the chunk.
         * @return        The chunk. Empty when there is no more to read.
         */
        std::string get_body(std::size_t max_len);

    public:
        /**
         * @brief   Reads headers from the http stream.
         * @details Consider using `request_istream::get_request()` / `response_istream::get_response()`.
         * @return  The headers.
         */
        headers get_headers();

        /**
         * @brief  Reads a header name from the http stream.
         * @details Consider using `request_istream::get_request()` / `response_istream::get_response()`.
         * @return The header name.
         */
        std::string get_header_name();

        /**
         * @brief  Reads a header value from the http stream.
         * @details Consider using `request_istream::get_request()` / `response_istream::get_response()`.
         * @return The header value.
         */
        std::string get_header_value();

    protected:
        /**
         * @brief  Reads the protocol.
         * @return The protocol.
         */
        std::string get_protocol();

        /**
         * @brief               Reads an http token.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @return              The token.
         */
        std::string get_token(std::size_t estimated_len);

        /**
         * @brief               Reads a sequence of printable chars.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @return              The sequence.
         */
        std::string get_prints(std::size_t estimated_len);

        /**
         * @brief               Reads a sequence of printable chars and spaces.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @return              The sequence.
         */
        std::string get_prints_and_spaces(std::size_t estimated_len);

        /**
         * @brief               Reads a sequence of letters.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @return              The sequence.
         */
        std::string get_alphas(std::size_t estimated_len);

        /**
         * @brief               Reads a sequence of digits.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @return              The sequence.
         */
        std::string get_digits(std::size_t estimated_len);

        /**
         * @brief               Reads a sequence of any chars through the end of the stream.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @param max_len       Maximum length of the chunk.
         * @return              The sequence.
         */
        std::string get_any_chars(std::size_t estimated_len, std::size_t max_len);

        /**
         * @brief               Reads a sequence of chars that match a predicate.
         * @param predicate     Predicate.
         * @param estimated_len Estimated capacity to reserve in the output string to prevent unnecessary reallocations.
         * @param max_len       Maximum length of the chunk.
         * @return              The sequence.
         */
        std::string get_chars(ascii::predicate_t&& predicate, std::size_t estimated_len, std::size_t max_len = size::strlen);

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
    class ostream
        : public abc::ostream
        , public state<LogPtr> {

        using base      = abc::ostream;
        using state     = state<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin Origin.
         * @param sb     `std::streambuf` to write to.
         * @param next   Next expected item.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        ostream(const char* origin, std::streambuf* sb, http::item_t next, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        ostream(ostream&& other);

        /**
         * @brief Deleted.
         */
        ostream(const ostream& other) = delete;

    public:
        /**
         * @brief          Writes a body to the http stream.
         * @details        The whole request/response must have been written before this method is called.
         * @param body     Body.
         * @param body_len Body length. Optional.
         */
        void put_body(const char* body, std::size_t body_len = size::strlen);

    public:
        /**
         * @brief         Writes headers and headers end to the http stream.
         * @details       Consider using `request_ostream::put_request()` / `response_ostream::put_response()`.
         * @note          If this is called on `request_ostream`, the caller is responsible for providing a `Host` header.
         * @param headers Headers.
         */
        void put_headers(const headers& headers);

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
         * @brief            Writes a sequence of chars that match a predicate to the http stream.
         * @param predicate  Predicate.
         * @param chars      Sequence of chars.
         * @param chars_len  Sequence length. Optional.
         * @return           The count of chars written. 
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
         * @brief      Sets the next expected item for the stream.
         * @param next The next expected item.
         */
        void set_pstate(http::item_t next);
    };


    // --------------------------------------------------------------


    /**
     * @brief         http request input stream. Used on the server side to read request streams sequentially.
     * @details       Using this class requires knowledge of the http protocol. Consider using `request_reader`.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class request_istream
        : public istream<LogPtr> {

        using base      = istream<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin-Origin.
         * @param sb     `std::streambuf` to read from.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        request_istream(const char* origin, std::streambuf* sb, const LogPtr& log = nullptr);

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        request_istream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        request_istream(request_istream&& other);

        /**
         * @brief Deleted.
         */
        request_istream(const request_istream& other) = delete;

    public:
        /**
         * @brief   Resets the read state.
         * @details Use only if you are certain you are the beginning of the stream.
         */
        void reset();

        /**
         * @brief   Reads an http method from the http stream.
         * @return  The method.
         */
        std::string get_method();

        /**
         * @brief   Reads an http resource from the http stream.
         * @return  The resource.
         */
        resource get_resource();

        /**
         * @brief   Reads an http protocol from the http stream.
         * @return  The protocol.
         */
        std::string get_protocol();

    protected:
        /**
         * @brief              Splits a raw http resource.
         * @param raw_resource Resource.
         */
        resource split_resource(const std::string& raw_resource);
    };


    /**
     * @brief         http request reader. Used on the server side to read whole requests.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class request_reader
        : protected request_istream<LogPtr> {

        using base      = request_istream<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    protected:
        /**
         * @brief        Constructor.
         * @param origin-Origin.
         * @param sb     `std::streambuf` to read from.
         * @param log    `LogPtr` pointer. May be `nullptr`.
         */
        request_reader(const char* origin, std::streambuf* sb, const LogPtr& log = nullptr);

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        request_reader(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        request_reader(request_reader&& other);

        /**
         * @brief Deleted.
         */
        request_reader(const request_reader& other) = delete;

    public:
        /**
         * @brief  Reads a whole http request from the http stream.
         * @return The request.
         */
        request get_request();

        /**
         * @brief         Reads a chunk of a body from the http stream.
         * @param max_len Maximum length of the chunk.
         * @return        The chunk. Empty when there is no more to read.
         */
        std::string get_body(std::size_t max_len);
    };


    // --------------------------------------------------------------


    /**
     * @brief  http request output stream. Used on the client side to write requests.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class request_ostream
        : public ostream<LogPtr> {

        using base      = ostream<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        request_ostream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        request_ostream(request_ostream&& other);

        /**
         * @brief Deleted.
         */
        request_ostream(const request_ostream& other) = delete;

    public:
        /**
         * @brief         Writes a whole http request to the http stream.
         * @param request Request.
         */
        void put_request(const request& request);

    public:
        /**
         * @brief   Resets the write state.
         * @details Use only if you are certain you are the beginning of the stream.
         */
        void reset();

        /**
         * @brief            Writes an http method to the http stream.
         * @details          Consider using `put_request()`.
         * @param method     Method.
         * @param method_len Method length. Optional.
         */
        void put_method(const char* method, std::size_t method_len = size::strlen);

        /**
         * @brief          Writes an http resource to the http stream.
         * @details        Consider using `put_request()`.
         * @param resource Resource.
         */
        void put_resource(const resource& resource);

        /**
         * @brief              Writes an http resource to the http stream.
         * @details            Consider using `put_request()`.
         * @param resource     Resource.
         * @param resource_len Resource length. Optional.
         */
        void put_resource(const char* resource, std::size_t resource_len = size::strlen);

        /**
         * @brief              Writes an http protocol to the http stream.
         * @details            Consider using `put_request()`.
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
    class response_istream
        : public istream<LogPtr> {

        using base      = istream<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        response_istream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        response_istream(response_istream&& other);

        /**
         * @brief Deleted.
         */
        response_istream(const response_istream& other) = delete;

    public:
        /**
         * @brief  Reads a whole http response from the http stream.
         * @return The response.
         */
        response get_response();

    public:
        /**
         * @brief   Resets the read state.
         * @details Use only if you are certain you are the beginning of the stream.
         */
        void reset();

        /**
         * @brief   Reads an http protocol from the http stream.
         * @details Consider using `get_response()`.
         */
        std::string get_protocol();

        /**
         * @brief   Reads an http status code from the http stream.
         * @details Consider using `get_response()`.
         */
        status_code_t get_status_code();

        /**
         * @brief   Reads an http reason phrase from the http stream.
         * @details Consider using `get_response()`.
         */
        std::string get_reason_phrase();
    };


    // --------------------------------------------------------------


    /**
     * @brief         http response output stream. Used on the server side to write responses.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class response_ostream
        : public ostream<LogPtr> {

        using base      = ostream<LogPtr>;
        using diag_base = diag::diag_ready<const char*, LogPtr>;

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        response_ostream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        response_ostream(response_ostream&& other);

        /**
         * @brief Deleted.
         */
        response_ostream(const response_ostream& other) = delete;

    public:
        /**
         * @brief          Writes a whole http response to the http stream.
         * @param response Response.
         */
        void put_response(const response& response);

    public:
        /**
         * @brief   Resets the write state.
         * @details Use only if you are certain you are the beginning of the stream.
         */
        void reset();

        /**
         * @brief              Writes an http protocol to the http stream.
         * @details            Consider using `put_response()`.
         * @param protocol     Protocol.
         * @param protocol_len Protocol length. Optional.
         */
        void put_protocol(const char* protocol, std::size_t protocol_len = size::strlen);

        /**
         * @brief Writes an http status code to the http stream.
         * @details            Consider using `put_response()`.
         * @param status_code  Status code.
         */
        void put_status_code(status_code_t status_code);

        /**
         * @brief                   Writes an http reason phrase to the http stream.
         * @details                 Consider using `put_response()`.
         * @param reason_phrase     Reason phrase.
         * @param reason_phrase_len Reason phrase length. Optional.
         */
        void put_reason_phrase(const char* reason_phrase, std::size_t reason_phrase_len = size::strlen);
    };


    // --------------------------------------------------------------


    /**
     * @brief         Combination of `request_ostream` and `response_istream`. Used on the client side.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class client_stream
        : public request_ostream<LogPtr>
        , public response_istream<LogPtr> {

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from and to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        client_stream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        client_stream(client_stream&& other);

        /**
         * @brief Deleted.
         */
        client_stream(const client_stream& other) = delete;
    };


    // --------------------------------------------------------------


    /**
     * @brief         Combination of `request_istream` and `response_ostream`. Used on the server side.
     * @tparam LogPtr Pointer type to `log_ostream`.
     */
    template <typename LogPtr = std::nullptr_t>
    class server_stream
        : public request_istream<LogPtr>
        , public response_ostream<LogPtr> {

    public:
        /**
         * @brief     Constructor.
         * @param sb  `std::streambuf` to read from and to write to.
         * @param log `LogPtr` pointer. May be `nullptr`.
         */
        server_stream(std::streambuf* sb, const LogPtr& log = nullptr);

        /**
         * @brief Move constructor.
         */
        server_stream(server_stream&& other);

        /**
         * @brief Deleted.
         */
        server_stream(const server_stream& other) = delete;
    };


    // --------------------------------------------------------------


    /**
     * @brief http utilities.
     */
    class util {
    public:
        /**
         * @brief URL-encodes the given character sequence.
         */
        static std::string url_encode(const char* chars, std::size_t chars_len = size::strlen);

        /**
         * @brief URL-decodes the given character sequence.
         */
        static std::string url_decode(const char* chars, std::size_t chars_len = size::strlen);
    };

} } }
