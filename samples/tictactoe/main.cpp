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


#include <iostream>
#include <string>
#include <future>

#include "game.h"


constexpr const char* origin = "sample_tictactoe";

vmem_bundle* player_agent::_vmem = nullptr;


int main(int /*argc*/, const char* argv[]) {
    constexpr const char* suborigin = "main()";
    std::srand(std::time(nullptr));

    // Create a log.
    abc::table_ostream table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> line(&table);
    abc::diag::str_log_filter<const char *> filter("", abc::diag::severity::important);
    abc::diag::log_ostream log(&line, &filter);

    // Use the path to this program to build the path to the pool file.
    std::string process_dir = abc::parent_path(argv[0]);
    std::string vmem_path = process_dir.append("/tictactoe.vmem");
    std::string results_path = process_dir.append("/results.csv");

    // Construct a pool and a map on it.
    // If the file doesn't exist, the pool will be initialized.
    // If the fie exists, it should be a valid pool.
    vmem_bundle vmem(vmem_path.c_str(), &log);
    player_agent::_vmem = &vmem;

    log.put_any(origin, suborigin, abc::diag::severity::debug, 0x105a5, "KB >>>");
    for (state_scores_map::const_iterator itr = vmem.state_scores_map.cbegin(); itr != vmem.state_scores_map.cend(); itr++) {
        log.put_any(origin, suborigin, abc::diag::severity::debug, 0x105a6, "%8.8x: %d %d %d %d %d %d %d %d %d",
            itr->key,
            itr->value[0][0], itr->value[0][1], itr->value[0][2],
            itr->value[1][0], itr->value[1][1], itr->value[1][2],
            itr->value[2][0], itr->value[2][1], itr->value[2][2]);
    }
    log.put_any(origin, suborigin, abc::diag::severity::debug, 0x105a7, "<<< KB");

    log.put_any(origin, suborigin, abc::diag::severity::optional, 0x105a8, "results_path='%s'", results_path.c_str());
    std::ofstream results_stream(results_path, std::ios::ate);

    abc::table_ostream results_table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> results_line(&table);
    abc::diag::str_log_filter<const char *> results_filter("", abc::diag::severity::optional);
    abc::diag::log_ostream results(&results_line, &results_filter);

    // Create an endpoint configuration.
    abc::net::http::endpoint_config config(
        "30303",             // port
        5,                   // listen_queue_size
        process_dir.c_str(), // root_dir (Note: No trailing slash!)
        "/resources/"        // files_prefix
    );

    // Create an endpoint.
    game_endpoint endpoint(std::move(config), &log);

    log.put_any(origin, suborigin, abc::diag::severity::warning, 0x105a9, "Open a browser and navigate to http://<host>:30303/resources/index.html.");
    log.put_blank_line(origin, abc::diag::severity::warning);

    // Let the endpoint listen in a separate thread.
    std::future<void> done = endpoint.start_async();
    done.wait();

    return 0;
}
