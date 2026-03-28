/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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

#include <iostream>
#include <fstream>
#include <system_error>
#include <future>
#include <thread>
#include <atomic>
#include <exception>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../root/ascii.h"
#include "../diag/diag_ready.h"
#include "daemon.h"
#include "socket.h"
#include "http.h"
#include "i/endpoint.i.h"


namespace abc { namespace net { namespace http {

    inline endpoint_error::endpoint_error(diag::tag_t tag, status_code_t status_code, const char* reason_phrase, const char* content_type, const char* body)
        : base(body)
        , tag(tag)
        , status_code(status_code)
        , reason_phrase(reason_phrase)
        , content_type(content_type)
        , body(body) {
    }


    // --------------------------------------------------------------


    inline endpoint::endpoint(endpoint_config&& config, diag::log_ostream* log)
        : endpoint("abc::net::http::endpoint", std::move(config), log) {
    }


    inline endpoint::endpoint(const char* origin, endpoint_config&& config, diag::log_ostream* log)
        : daemon(origin, log)
        , _config(std::move(config))
        , _requests_in_progress(0) {

        constexpr const char* suborigin = "endpoint()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x108b7, "Begin: port='%s', queue_size=%d, rood_dir='%s', files_prefix='%s'",
                            _config.port.c_str(), _config.listen_queue_size, _config.root_dir.c_str(), _config.files_prefix.c_str());

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108b8, "End:");
    }


    inline void endpoint::request_stop() {
        constexpr const char* suborigin = "request_stop()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // Request a daemon stop.
        base::request_stop();

        // Interrupt an eventual blocking `accept()`.
        _listener->interrupt_accept();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline bool endpoint::can_stop() const {
        diag_base::put_any("can_stop()", diag::severity::optional, __TAG__, "requests_in_progress=%d", _requests_in_progress.load());
        return _requests_in_progress.load() == 0;
    }


    inline void endpoint::on_started() {
        constexpr const char* suborigin = "on_started()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x102f1, "Begin:");

        // Create a listener, bind to a port, and start listening.
        _listener = create_server_socket();
        _listener->bind(_config.port.c_str());
        _listener->listen(_config.listen_queue_size);

        diag_base::put_any(suborigin, diag::severity::important, 0x102f2, "Listening (port='%s')", _config.port.c_str());
        diag_base::put_blank_line(diag::severity::important);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108bb, "End:");
    }


    inline void endpoint::on_idle() {
        constexpr const char* suborigin = "on_idle()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        // Accept the next request and process it asynchronously.
        std::unique_ptr<net::tcp_client_socket> connection = _listener->accept();
        std::thread(process_request_thread_func, this, std::move(connection)).detach();

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void endpoint::on_stopped() {
        constexpr const char* suborigin = "on_stopped()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        diag_base::put_blank_line(diag::severity::important);
        diag_base::put_any(suborigin, diag::severity::important, 0x102f3, "Stopped (port='%s')", _config.port.c_str());

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline void endpoint::process_request_thread_func(endpoint* this_ptr, std::unique_ptr<net::tcp_client_socket>&& connection) {
        this_ptr->process_request(std::move(connection));
    }


    inline void endpoint::process_request(std::unique_ptr<net::tcp_client_socket>&& connection) {
        constexpr const char* suborigin = "process_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x102de, "Begin:");

        // If stop has been requested, bail out without any processing.
        if (is_stop_requested()) {
            diag_base::put_any(suborigin, diag::severity::callstack, 0x108bc, "Return: Shutting down.");
            return;
        }

        // Create a tcp_client_socket_streambuf over the connection.
        tcp_client_socket_streambuf sb(connection.get(), diag_base::log());

        // Create an http::server, which combines http::request_reader and http::response_writer.
        http::server http(&sb, diag_base::log());

        // Read the request.
        http::request request = http.get_request();
        diag_base::put_any(suborigin, diag::severity::optional, 0x102e1, "Request received: protocol='%s', method='%s', path='%s'", request.protocol.c_str(), request.method.c_str(), request.resource.path.c_str());

        bool ok = true;
        increment_requests_in_progress();

        try {
            // This endpoint supports two kinds of requests:
            //    a) requests for static files
            //    b) REST requests
            if (is_file_request(request)) {
                process_file_request(http, request);
            }
            else {
                process_rest_request(http, request);
            }
        }
        catch (const abc::net::http::endpoint_error& err) {
            ok = false;
            send_simple_response(http, err.status_code, err.reason_phrase.c_str(), err.content_type.c_str(), err.body.c_str(), err.tag);
        }
        catch (const std::runtime_error& err) {
            ok = false;
            send_simple_response(http, abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, err.what(), __TAG__);
        }
        catch (const std::exception& ex) {
            ok = false;
            send_simple_response(http, abc::net::http::status_code::Internal_Server_Error, abc::net::http::reason_phrase::Internal_Server_Error, abc::net::http::content_type::text, ex.what(), __TAG__);
        }

        decrement_requests_in_progress();

        diag_base::put_any(suborigin, diag::severity::optional, 0x102e1, "Done processing request: protocol='%s', method='%s', path='%s', ok=%d", request.protocol.c_str(), request.method.c_str(), request.resource.path.c_str(), (int)ok);
        diag_base::put_blank_line(diag::severity::optional);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108bd, "End:");
    }


    inline void endpoint::process_file_request(server& http, const request& request) {
        constexpr const char* suborigin = "process_file_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x102e4, "Begin: method='%s', path='%s'", request.method.c_str(), request.resource.path.c_str());

        // If the method is not GET, return 405.
        if (!ascii::are_equal_i(request.method.c_str(), method::GET)) {
            send_simple_response(http, status_code::Method_Not_Allowed, reason_phrase::Method_Not_Allowed, content_type::text, "GET is the only supported method for static files.", 0x102e5);
            diag_base::put_any(suborigin, diag::severity::callstack, 0x108be, "Return: 405");
            return;
        }

        std::string filepath = make_root_dir_path(request);
        diag_base::put_any(suborigin, diag::severity::optional, 0x102e6, "filepath='%s'", filepath.c_str());

        struct stat st;
        int err = ::stat(filepath.c_str(), &st);

        // If the file was not found, return 404.
        if (err != 0) {
            send_simple_response(http, status_code::Not_Found, reason_phrase::Not_Found, content_type::text, "Error: The requested resource was not found.", 0x102e7);
            diag_base::put_any(suborigin, diag::severity::callstack, 0x108bf, "Return: 404");
            return;
        }

        // The file was found, return 200.
        std::size_t fsize = static_cast<std::size_t>(st.st_size);
        std::string content_length = std::to_string(fsize);

        diag_base::put_any(suborigin, diag::severity::optional, 0x102e9, "Status Code    = 200");
        diag_base::put_any(suborigin, diag::severity::optional, 0x102e8, "Content-Length = %s", content_length.c_str());

        response response;
        response.protocol = protocol::HTTP_11;
        response.status_code = status_code::OK;
        response.reason_phrase = reason_phrase::OK;
        response.headers = {
            { header::Content_Length, std::move(content_length) },
        }; 

        const char* content_type = get_content_type_from_path(filepath.c_str());
        if (content_type != nullptr) {
            response.headers[header::Content_Type] = std::string(content_type);
        }

        http.put_response(response);

        std::ifstream file(filepath);
        char file_chunk[size::k4];
        for (std::size_t sent_size = 0; sent_size < fsize; sent_size += sizeof(file_chunk)) {
            file.read(file_chunk, sizeof(file_chunk));
            http.put_body(file_chunk, file.gcount());
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108c0, "End:");
    }


    inline void endpoint::process_rest_request(server& http, const request& request) {
        constexpr const char* suborigin = "process_rest_request()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x102ea, "Begin: method='%s', path='%s'", request.method.c_str(), request.resource.path.c_str());

        if (ascii::are_equal_i(request.method.c_str(), method::POST) && ascii::are_equal_i(request.resource.path.c_str(), "/shutdown")) {
            request_stop();
        }

        send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Consider overriding process_rest_request().", 0x102eb);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108c1, "End:");
    }


    inline void endpoint::send_simple_response(server& http, status_code_t status_code, const char* reason_phrase, const char* content_type, const char* body, diag::tag_t tag) {
        constexpr const char* suborigin = "send_simple_response()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x102ec, "Begin:");

        std::string content_length = std::to_string(std::strlen(body));

        diag_base::put_any(suborigin, diag::severity::callstack, tag, "Status Code    = %u", (unsigned)status_code);
        diag_base::put_any(suborigin, diag::severity::callstack, tag, "Content-Type   = %s", content_type);
        diag_base::put_any(suborigin, diag::severity::callstack, tag, "Content-Length = %s", content_length.c_str());
        diag_base::put_any(suborigin, diag::severity::callstack, tag, "Body           = %s", body);

        response response;
        response.protocol = protocol::HTTP_11;
        response.status_code = status_code;
        response.reason_phrase = reason_phrase;
        response.headers = {
            { header::Content_Type,   content_type },
            { header::Content_Length, std::move(content_length) },
        }; 

        http.put_response(response);

        http.put_body(body);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108c2, "End:");
    }


    inline const char* endpoint::get_content_type_from_path(const char* path) {
        const char* ext = std::strrchr(path, '.');
        if (ext == nullptr) {
            return nullptr;
        }

        if (ascii::are_equal_i(ext, ".html")) {
            return content_type::html;
        }
        else if (ascii::are_equal_i(ext, ".css")) {
            return content_type::css;
        }
        else if (ascii::are_equal_i(ext, ".js")) {
            return content_type::javascript;
        }
        else if (ascii::are_equal_i(ext, ".txt")) {
            return content_type::text;
        }
        else if (ascii::are_equal_i(ext, ".xml")) {
            return content_type::xml;
        }
        else if (ascii::are_equal_i(ext, ".png")) {
            return content_type::png;
        }
        else if (ascii::are_equal_i(ext, ".jpeg")) {
            return content_type::jpeg;
        }
        else if (ascii::are_equal_i(ext, ".jpg")) {
            return content_type::jpeg;
        }
        else if (ascii::are_equal_i(ext, ".gif")) {
            return content_type::gif;
        }
        else if (ascii::are_equal_i(ext, ".bmp")) {
            return content_type::bmp;
        }
        else if (ascii::are_equal_i(ext, ".svg")) {
            return content_type::svg;
        }
        
        return nullptr;
    }


    inline bool endpoint::is_file_request(const request& request) {
        return (!_config.files_prefix.empty() && ascii::are_equal_i_n(request.resource.path.c_str(), _config.files_prefix.c_str(), _config.files_prefix.size()))
            || (ascii::are_equal_i(request.method.c_str(), method::GET) && ascii::are_equal_i(request.resource.path.c_str(), "/favicon.ico"));
    }


    inline std::string endpoint::make_root_dir_path(const request& request) const {
        constexpr const char* suborigin = "make_root_dir_path()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x108c5, "Begin: root_dir='%s', path='%s'", _config.root_dir.c_str(), request.resource.path.c_str());

        std::string filepath(_config.root_dir);
        if (filepath.back() != '/') {
            filepath.append("/");
        }
        filepath.append(request.resource.path);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x108c6, "End: path='%s'", filepath.c_str());

        return filepath;
    }


    inline void endpoint::require(const char* suborigin, diag::tag_t tag, bool condition, status_code_t status_code, const char* reason_phrase, const char* content_type, const char* body) const {
        if (!condition) {
            throw_exception(suborigin, tag, status_code, reason_phrase, content_type, body);
        }
    }


    inline void endpoint::throw_exception(const char* suborigin, diag::tag_t tag, status_code_t status_code, const char* reason_phrase, const char* content_type, const char* body) const {
        diag_base::throw_exception(suborigin, tag, abc::net::http::endpoint_error(tag, status_code, reason_phrase, content_type, body));
    }


    inline const endpoint_config& endpoint::config() const {
        return _config;
    }


    inline void endpoint::increment_requests_in_progress() noexcept {
        ++_requests_in_progress;
    }


    inline void endpoint::decrement_requests_in_progress() noexcept {
        --_requests_in_progress;
    }


    // --------------------------------------------------------------


    inline endpoint_config::endpoint_config(const char* port, std::size_t listen_queue_size, const char* root_dir, const char* files_prefix,
                                            const char* cert_file_path, const char* pkey_file_path, const char* pkey_file_password)
        : port(port)

        , listen_queue_size(listen_queue_size)

        , root_dir(root_dir)
        , files_prefix(files_prefix)
        
        , cert_file_path(cert_file_path)
        , pkey_file_path(pkey_file_path)
        , pkey_file_password(pkey_file_password) {
    }


    // --------------------------------------------------------------

} } }
