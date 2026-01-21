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


#include <iostream>
#include <string>
#include <future>

#include "../../src/root/ascii.h"

#include "car.h"


constexpr const char* origin = "sample_picar_4wd";


// --------------------------------------------------------------


extern void run_all();


int main(int argc, const char* argv[]) {
    constexpr const char* suborigin = "main()";
    std::srand(std::time(nullptr));

    if (argc >= 2 && abc::ascii::are_equal(argv[1], "hacks")) { 
        run_all();
        return 0;
    }

    // Create a log.
    abc::stream::table_ostream table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> line(&table);
    abc::diag::str_log_filter<const char *> filter("", abc::diag::severity::important);
    abc::diag::log_ostream log(&line, &filter);

    // Use the path to this program to build the path to the pool file.
    std::string process_dir = abc::parent_path(argv[0]);

    // Create an endpoint configuration.
    abc::net::http::endpoint_config config(
        "30305",             // port
        5,                   // listen_queue_size
        process_dir.c_str(), // root_dir (Note: No trailing slash!)
        "/resources/"        // files_prefix
    );

    // Create an endpoint.
    car_endpoint endpoint(std::move(config), &log);

    log.put_any(origin, suborigin, abc::diag::severity::warning, 0x105a9, "Open a browser and navigate to http://<host>:30305/resources/index.html.");
    log.put_blank_line(origin, abc::diag::severity::warning);

    // Let the endpoint listen in a separate thread.
    std::future<void> done = endpoint.start_async();
    done.wait();

    return 0;
}


