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


#include <iostream>
#include <future>
#include <atomic>
#include <exception>
#include <cstring>

#include "equations.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;
using limits = abc::endpoint_limits;


int main(int /*argc*/, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::important);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Use the path to this process to determine the root_dir.
	constexpr std::size_t max_path = abc::size::k1;
	char path[max_path];
	path[0] = '\0';

	const char* last_separator = std::strrchr(argv[0], '/');

	if (last_separator != nullptr) {
		std::size_t path_len = last_separator - argv[0];
		if (path_len >= max_path) {
			log.put_any(abc::category::abc::samples, abc::severity::critical, 0x102f4,
				"This sample allows paths up to %zu chars. The path to this process is %zu chars. To continue, either move the current dir closer to the process, or increase the path limit in main.cpp.",
				max_path, path_len);

			return 1;
		}

		std::strncpy(path, argv[0], path_len);
		path[path_len] = '\0';
	}

	// Create a endpoint.
	abc::endpoint_config config(
		"30301",			// port
		5,					// listen_queue_size
		path,				// root_dir (Note: No trailing slash!)
		"/resources/"		// files_prefix
	);
	abc::samples::equations_endpoint<limits, log_ostream> endpoint(&config, &log);

	log.put_any(abc::category::abc::samples, abc::severity::warning, 0x102f5, "Open a browser and navigate to http://<host>:30301/resources/index.html.");
	log.put_blank_line();

	// Let the endpoint listen in a separate thread.
	std::future<void> done = endpoint.start_async();
	done.wait();

	return 0;
}
