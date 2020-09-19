/*
MIT License

Copyright (c) 2018-2020 Zlatko Michailov 

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


#include "webserver.h"


namespace abc { namespace samples {

	template <typename Log>
	class equations_webserver : public webserver<Log> {
		using base = webserver<Log>;

	public:
		equations_webserver(webserver_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;
	};


	// --------------------------------------------------------------


	template <typename Log>
	inline equations_webserver<Log>::equations_webserver(webserver_config* config, Log* log)
		: base(config, log) {
	}


	template <typename Log>
	inline void equations_webserver<Log>::process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) {
		base::_log->put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "Received REST");

		if (std::strcmp(method, "POST") == 0 && std::strcmp(resource, "/shutdown") == 0) {
			base::set_shutdown_requested();
		}

		base::_log->put_any(abc::category::abc::samples, abc::severity::debug, __TAG__, "Sending response 200");
		http.put_protocol("HTTP/1.1");
		http.put_status_code("200");
		http.put_reason_phrase("OK");
		http.put_header_name("Content-Length");
		http.put_header_value("24");
		http.end_headers();
		http.put_body("Yae: I have an override!");
	}


	// --------------------------------------------------------------

}}


