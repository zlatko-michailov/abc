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
#include <unistd.h>
#include <time.h>

#include "../../src/gpio.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;


void log_chip_info(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	abc::gpio_chip_info chip_info = chip.chip_info();

	log.put_blank_line();
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "chip info:");
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  is_valid = %d", chip_info.is_valid);
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  name     = %s", chip_info.name);
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  label    = %s", chip_info.label);
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  lines    = %u", chip_info.lines);
	log.put_blank_line();
}


void log_all_line_info(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	abc::gpio_chip_info chip_info = chip.chip_info();

	for (abc::gpio_line_pos_t pos = 0; pos < chip_info.lines; pos++) {
		abc::gpio_line_info line_info = chip.line_info(pos);

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "line %2u info:", (unsigned)pos);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  is_valid = %d", line_info.is_valid);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  name     = %s", line_info.name);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  consumer = %s", line_info.consumer);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  flags    = %llx", (long long)line_info.flags);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "  in/out   = %s", (line_info.flags & abc::gpio_line_flag::output) != 0 ? "OUTPUT" : "INPUT");
		log.put_blank_line();
	}
}


void measure_distance(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	abc::gpio_output_line<log_ostream> trigger_line(chip, 5, &log);
	abc::gpio_input_line<log_ostream> echo_line(chip, 6, &log);

	for (int i = 0; i < 20; i++) {
		// Clear and send a pulse.
		trigger_line.put_value(abc::gpio_bit_value::low);
		usleep(10);
		trigger_line.put_value(abc::gpio_bit_value::high);
		usleep(10);
		trigger_line.put_value(abc::gpio_bit_value::low);

		timespec base_tp;
		timespec pulse_start_tp;
		timespec pulse_end_tp;

		clock_gettime(CLOCK_REALTIME, &base_tp);
		
		abc::gpio_bit_value_t value;
		value = echo_line.get_value();
		while (value == abc::gpio_bit_value::low) {
			clock_gettime(CLOCK_REALTIME, &pulse_start_tp);
			value = echo_line.get_value();
		}

		// Measure the duration of the high echo.
		while (value == abc::gpio_bit_value::high) {
			clock_gettime(CLOCK_REALTIME, &pulse_end_tp);
			value = echo_line.get_value();
		}

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "base= %llu : %lu", (long long)base_tp.tv_sec, base_tp.tv_nsec);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "start=%llu : %lu (%lu)", (long long)pulse_start_tp.tv_sec, pulse_start_tp.tv_nsec, pulse_start_tp.tv_nsec - base_tp.tv_nsec);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "end=  %llu : %lu (%lu)", (long long)pulse_end_tp.tv_sec, pulse_end_tp.tv_nsec, pulse_end_tp.tv_nsec - pulse_start_tp.tv_nsec);

		double duration = (pulse_end_tp.tv_sec - pulse_start_tp.tv_sec) + (pulse_end_tp.tv_nsec - pulse_start_tp.tv_nsec) / 1000000000.0;
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "duration=%.6f", duration);

		// Calculate length in cm based on the speed of sound - 340 m/s.
		double cm = 340.0 * 100.0 * duration / 2.0;
		log.put_blank_line();
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "cm=%.2f", cm);
		log.put_blank_line();

		// Sleep for 1 sec between iterations.
		usleep(1 * 1000 * 1000);
	}
}


int main(int argc, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::important);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Create a chip.
	abc::gpio_chip<log_ostream> chip("/dev/gpiochip0", "picar_4wd", &log);

	// Info
	log_chip_info(chip, log);
	log_all_line_info(chip, log);

	measure_distance(chip, log);

	return 0;
}
