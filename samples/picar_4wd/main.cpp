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


#include "../../src/root/ascii.h"
#if 0
#include "car.h"
#endif


// --------------------------------------------------------------

extern void run_all();


int main(int argc, const char* argv[]) {
	if (argc >= 2 && abc::ascii::are_equal(argv[1], "hacks")) { 
		run_all();
		return 0;
	}

#if 0
	// Create a log.
	abc::log_filter filter(abc::severity::abc::important);
	abc::samples::log_ostream log(std::cout.rdbuf(), &filter);

	// Use the path to this program to build the path to the pool file.
	constexpr std::size_t max_path = abc::size::k1;
	char path[max_path];
	path[0] = '\0';

	const char* prog_last_separator = std::strrchr(argv[0], '/');
	std::size_t prog_path_len = 0;

	if (prog_last_separator != nullptr) {
		prog_path_len = prog_last_separator - argv[0];

		if (prog_path_len >= max_path) {
			log.put_any(abc::category::abc::samples, abc::severity::critical, 0x106b7,
				"This sample allows paths up to %zu chars. The path to this process is %zu chars. To continue, either move the current dir closer to the process, or increase the path limit in main.cpp.",
				max_path, prog_path_len);

			return 1;
		}

		std::strncpy(path, argv[0], prog_path_len);
	}

	path[prog_path_len] = '\0';


	// Create a endpoint.
	abc::endpoint_config config(
		"30305",			// port
		5,					// listen_queue_size
		path,				// root_dir (Note: No trailing slash!)
		"/resources/"		// files_prefix
	);
	abc::samples::car_endpoint<abc::samples::limits, abc::samples::log_ostream> endpoint(&config, &log);

	log.put_any(abc::category::abc::samples, abc::severity::warning, 0x106b8, "Open a browser and navigate to http://<host>:30305/resources/index.html.");
	log.put_blank_line(abc::category::abc::samples, abc::severity::warning);

	// Let the endpoint listen in a separate thread.
	std::future<void> done = endpoint.start_async();
	done.wait();
#endif

	return 0;
}


