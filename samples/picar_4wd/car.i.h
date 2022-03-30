/*
MIT License

Copyright (c) 2018-2022 Zlatko Michailov 

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


#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ratio>
#include <thread>

#include "../../src/log.h"
#include "../../src/endpoint.h"
#include "../../src/gpio.h"


namespace abc { namespace samples {

	using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
	using limits = abc::endpoint_limits;


	// --------------------------------------------------------------


	template <typename Limits, typename Log>
	class car_endpoint : public endpoint<Limits, Log> {
		using base = endpoint<Limits, Log>;

	public:
		car_endpoint(endpoint_config* config, Log* log);

	protected:
		virtual void	process_rest_request(abc::http_server_stream<Log>& http, const char* method, const char* resource) override;

	private:
		void			process_shutdown(abc::http_server_stream<Log>& http, const char* method);

		void			set_power(abc::http_server_stream<Log>& http, const char* method, std::int32_t power);
		void			set_turn(abc::http_server_stream<Log>& http, const char* method, std::int32_t turn);

		bool			verify_method_get(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_method_post(abc::http_server_stream<Log>& http, const char* method);
		bool			verify_header_json(abc::http_server_stream<Log>& http);
		template <typename T>
		bool			verify_range(abc::http_server_stream<Log>& http, T value, T lo_bound, T hi_bound, T step);

		void			drive(abc::http_server_stream<Log>& http, std::int32_t power, std::int32_t turn);

	private:
		std::int32_t	_power;
		std::int32_t	_turn;
	};


	// --------------------------------------------------------------

}}

