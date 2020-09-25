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


#include <iostream>
#include <future>
#include <atomic>
#include <exception>

#include "equations.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
using limits = abc::samples::webserver_limits;


int main() {
	// Create a log.
	abc::log_filter filter(abc::severity::debug);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Create a webserver.
	abc::samples::webserver_config config(
		"30301",					// port
		5,							// listen_queue_size
		"out/samples/webserver",	// root_dir (Note: No trailing slash!)
		"/resources/"				// files_prefix
	);
	abc::samples::equations_webserver<limits, log_ostream> webserver(&config, &log);

	// Let the webserver listen in a separate thread.
	std::future done = webserver.start_async();
	done.wait();

	return 0;
}
