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
#include <chrono>
#include <vector>

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
	using clock = std::chrono::steady_clock;
	using microseconds = std::chrono::microseconds;

	abc::gpio_output_line<log_ostream> trigger_line(chip, 5, &log);
	abc::gpio_input_line<log_ostream> echo_line(chip, 6, &log);

	for (int i = 0; i < 20; i++) {
		// Clear and send a pulse.
		trigger_line.put_level(abc::gpio_level::low, microseconds(10));
		trigger_line.put_level(abc::gpio_level::high, microseconds(10));
		trigger_line.put_level(abc::gpio_level::low);

		clock::time_point echo_not_ready_tp = clock::now();
		microseconds timeout((2 * 1000 * 1000) / 340); // ~6,000 us

		// Make sure there is no echo in progress.
		abc::gpio_level_t level = echo_line.expect_level(abc::gpio_level::low, timeout);
		clock::time_point echo_ready_tp = clock::now();

		// Wait until the echo starts.
		if (level != abc::gpio_level::invalid) {
			timeout -= std::chrono::duration_cast<microseconds>(echo_ready_tp - echo_not_ready_tp);
			level = echo_line.expect_level(abc::gpio_level::high, timeout);
		}
		clock::time_point echo_start_tp = clock::now();

		// Wait until the echo ends.
		if (level != abc::gpio_level::invalid) {
			timeout -= std::chrono::duration_cast<microseconds>(echo_start_tp - echo_ready_tp);
			level = echo_line.expect_level(abc::gpio_level::low, timeout);
		}
		clock::time_point echo_end_tp = clock::now();

		if (level == abc::gpio_level::invalid) {
			log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "TIMEOUT us = %llu", (long long)timeout.count());
			continue;
		}

		microseconds echo_us = std::chrono::duration_cast<microseconds>(echo_end_tp - echo_start_tp);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "us = %llu", (long long)echo_us.count());

		// Calculate roundtip at the speed of sound - 340 m/s.
		double echo_s = echo_us.count() / (1000.0 * 1000.0);
		double roundtrip_m = 340.0 * echo_s;
		double roundtrip_cm = roundtrip_m * 100.0;
		double distance_cm = roundtrip_cm / 2.0;
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "cm = %.2f", distance_cm);
		log.put_blank_line();

		// Sleep for 1 sec between iterations.
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


void move_servo(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	using clock = std::chrono::steady_clock;
	using microseconds = std::chrono::microseconds;

	const std::uint32_t min_pulse_width_us = 500;
	const std::uint32_t max_pulse_width_us = 2500;
	const std::uint32_t frequency = 50; // 50 Hz
	const std::uint32_t period_us = 1000 * 1000 / frequency;

	const std::uint32_t duration_us = 250 * 1000; // 250 ms
	const std::uint32_t period_count = duration_us / period_us;

	const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

	abc::gpio_output_line<log_ostream> line(chip, 4, &log);

	for (std::uint32_t duty_cycle : duty_cycles) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "duty_cycle = %u", duty_cycle);

		const std::uint32_t high_duration_us = min_pulse_width_us + duty_cycle * (max_pulse_width_us - min_pulse_width_us) / 100;
		const std::uint32_t low_duration_us = period_us - high_duration_us;

		line.put_level(abc::gpio_level::low, microseconds(10));
		for (std::uint32_t p = 0; p < period_count; p++) {
			line.put_level(abc::gpio_level::high, microseconds(high_duration_us));
			line.put_level(abc::gpio_level::low, microseconds(low_duration_us));
		}
		line.put_level(abc::gpio_level::low, microseconds(10));

		std::this_thread::sleep_for(std::chrono::seconds(1));
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

	// Ultrasonic - binary signal
	measure_distance(chip, log);

	// Servo - pwm
	move_servo(chip, log);

	return 0;
}
