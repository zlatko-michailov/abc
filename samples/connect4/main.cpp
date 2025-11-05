/*
MIT License

Copyright (c) 2018-2025 Zlatko Michailov 

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

#include "game.h"


constexpr const char* origin = "sample_connect4";

vmem_bundle* player_agent::_vmem = nullptr;


int main(int /*argc*/, const char* argv[]) {
    constexpr const char* suborigin = "main()";
    std::srand(std::time(nullptr));

    // Create a log.
    abc::stream::table_ostream table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> line(&table);
    abc::diag::str_log_filter<const char *> filter("", abc::diag::severity::important);
    abc::diag::log_ostream log(&line, &filter);

    // Use the path to this program to build the path to the pool file.
    std::string process_dir = abc::parent_path(argv[0]);
    std::string vmem_path = process_dir + "/connect4.vmem";
    std::string results_path = process_dir + "/results.csv";

    // Construct a pool and a map on it.
    // If the file doesn't exist, the pool will be initialized.
    // If the fie exists, it should be a valid pool.
    vmem_bundle vmem(vmem_path.c_str(), &log);
    player_agent::_vmem = &vmem;

    log.put_any(origin, suborigin, abc::diag::severity::debug, 0x10673, "KB >>>");
    for (state_scores_map::const_iterator itr = vmem.state_scores_map.cbegin(); itr != vmem.state_scores_map.cend(); itr++) {
        log.put_any(origin, suborigin, abc::diag::severity::debug, 0x10674, "%16.16llx: %d %d %d %d %d %d %d",
            (unsigned long long)itr->key,
            itr->value[0], itr->value[1], itr->value[2], itr->value[3], itr->value[4], itr->value[5], itr->value[6]);
    }
    log.put_any(origin, suborigin, abc::diag::severity::debug, 0x10675, "<<< KB");

    log.put_any(origin, suborigin, abc::diag::severity::optional, 0x10676, "results_path='%s'", results_path.c_str());
    std::ofstream results_stream(results_path, std::ios::ate);

    abc::stream::table_ostream results_table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> results_line(&table);
    abc::diag::str_log_filter<const char *> results_filter("", abc::diag::severity::optional);
    abc::diag::log_ostream results(&results_line, &results_filter);

    // Create an endpoint configuration.
    abc::net::http::endpoint_config config(
        "30304",             // port
        5,                   // listen_queue_size
        process_dir.c_str(), // root_dir (Note: No trailing slash!)
        "/resources/"        // files_prefix
    );

    // Create an endpoint.
    game_endpoint endpoint(std::move(config), &log);

    log.put_any(origin, suborigin, abc::diag::severity::warning, 0x10677, "Open a browser and navigate to http://<host>:30304/resources/index.html.");
    log.put_blank_line(origin, abc::diag::severity::warning);

    // Let the endpoint listen in a separate thread.
    std::future<void> done = endpoint.start_async();
    done.wait();

    return 0;
}
