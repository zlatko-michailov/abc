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


#include <iostream>
#include <string>
#include <future>

#include "equations.h"


constexpr const char* origin = "basic_sample";


int main(int /*argc*/, const char* argv[]) {
    constexpr const char* suborigin = "main()";

    // Create a log.
    abc::table_ostream table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> line(&table);
    abc::diag::str_log_filter<const char *> filter("", abc::diag::severity::important);
    abc::diag::log_ostream log(&line, &filter);

    // Create an endpoint configuration.
    std::string process_dir = abc::parent_path(argv[0]);
    abc::net::http::endpoint_config config(
        "30301",              // port
        5,                    // listen_queue_size
        process_dir.c_str(), // root_dir (Note: No trailing slash!)
        "/resources/"         // files_prefix
    );

    // Create an endpoint.
    equations_endpoint endpoint(std::move(config), &log);

    log.put_any(origin, suborigin, abc::diag::severity::warning, 0x102f5, "Open a browser and navigate to http://<host>:30301/resources/index.html.");
    log.put_blank_line(origin, abc::diag::severity::warning);

    // Let the endpoint listen in a separate thread.
    std::future<void> done = endpoint.start_async();
    done.wait();

    return 0;
}
