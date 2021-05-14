/*
MIT License

Copyright (c) 2018-2021 Zlatko Michailov 

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
#include <fstream>
#include <cstdint>
#include <cstring>
#include <algorithm>

#include "tictactoe.i.h"


int main(int argc, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::optional);
	abc::samples::log_ostream log(std::cout.rdbuf(), &filter);

	// Use the path to this program to build the path to the pool file.
	constexpr std::size_t max_path = abc::size::k1;
	char path[max_path];
	path[0] = '\0';

	constexpr const char kb_path[] = "kb.vmem";
	std::size_t kb_path_len = std::strlen(kb_path); 

	constexpr const char results_path[] = "results.csv";
	std::size_t results_path_len = std::strlen(results_path); 

	const char* prog_last_separator = std::strrchr(argv[0], '/');
	std::size_t prog_path_len = 0;

	if (prog_last_separator != nullptr) {
		prog_path_len = prog_last_separator - argv[0] + 1;
		std::size_t full_path_len = prog_path_len + std::max(kb_path_len, results_path_len);

		if (full_path_len >= max_path) {
			log.put_any(abc::category::abc::samples, abc::severity::critical, __TAG__,
				"This sample allows paths up to %zu chars. The path to this process is %zu chars. To continue, either move the current dir closer to the process, or increase the path limit in main.cpp.",
				max_path, full_path_len);

			return 1;
		}

		std::strncpy(path, argv[0], prog_path_len);
	}

	std::strcpy(path + prog_path_len, kb_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "kb_path='%s'", path);


	// Construct a pool instance.
	// If the file doesn't exist, the pool will be initialized.
	// If the fie exists, it should be a valid pool.
	abc::samples::vmem_pool pool(path, &log);


	std::strcpy(path + prog_path_len, results_path);
	log.put_any(abc::category::abc::samples, abc::severity::optional, __TAG__, "results_path='%s'", path);
	std::ofstream results_stream(path, std::ios::ate);

	abc::log_filter results_filter(abc::severity::optional);
	abc::samples::results_ostream results(results_stream.rdbuf(), &results_filter);

	return 0;
}


