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

	for (int i = 0; i < 10; i++) {
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

	const microseconds min_pulse_width = microseconds(500);
	const microseconds max_pulse_width = microseconds(2500);
	const abc::gpio_pwm_frequency_t frequency = 50; // 50 Hz

	const microseconds duty_duration = microseconds(250 * 1000);

	const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

	abc::gpio_output_line<log_ostream> line(chip, 4, &log);
	abc::gpio_pwm_emulator<microseconds, log_ostream> pwm(std::move(line), min_pulse_width, max_pulse_width, frequency, &log);

	// Make sure it wakes up after a const level.
	pwm.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	std::this_thread::sleep_for(std::chrono::seconds(1));

	for (std::uint32_t duty_cycle : duty_cycles) {
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "duty_cycle = %u", duty_cycle);

		pwm.set_duty_cycle(duty_cycle, duty_duration);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


void reset_hat(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	using milliseconds = std::chrono::milliseconds;

	abc::gpio_output_line<log_ostream> reset_line(chip, 21, &log);

	reset_line.put_level(abc::gpio_level::low, milliseconds(1));
	reset_line.put_level(abc::gpio_level::high, milliseconds(3));
}


void turn_motors(log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus("/dev/i2c-1", &log);

	const abc::gpio_smbus_address_t addr_hat = 0x14;
	const abc::gpio_smbus_register_t reg_front_left = 0x2d;
	const abc::gpio_smbus_register_t reg_front_right = 0x2c;
	const abc::gpio_smbus_register_t reg_rear_left = 0x28;
	const abc::gpio_smbus_register_t reg_rear_right = 0x29;

	smbus.put_word(addr_hat, reg_front_left, 0x0010);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	smbus.put_word(addr_hat, reg_front_left, 0x0000);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	smbus.put_word(addr_hat, reg_front_right, 0x0010);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	smbus.put_word(addr_hat, reg_front_right, 0x0000);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	smbus.put_word(addr_hat, reg_rear_left, 0x0010);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	smbus.put_word(addr_hat, reg_rear_left, 0x0000);

	std::this_thread::sleep_for(std::chrono::seconds(1));

	smbus.put_word(addr_hat, reg_rear_right, 0x0010);
		std::this_thread::sleep_for(std::chrono::seconds(1));
	smbus.put_word(addr_hat, reg_rear_right, 0x0000);
}


void measure_grayscale(log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus("/dev/i2c-1", &log);

	const abc::gpio_smbus_address_t addr_hat = 0x14;
	const abc::gpio_smbus_register_t reg_left = 0x12;
	const abc::gpio_smbus_register_t reg_center = 0x11;
	const abc::gpio_smbus_register_t reg_right = 0x10;
	const std::uint16_t zero = 0x0000;

	for (int i = 0; i < 10; i++) {
		std::uint8_t high_byte;
		std::uint8_t low_byte;

		std::uint16_t left;
		smbus.put_word(addr_hat, reg_left, zero);
		smbus.get_noreg(addr_hat, high_byte);
		smbus.get_noreg(addr_hat, low_byte);
		left = (high_byte << 8) | low_byte;

		std::uint16_t center;
		smbus.put_word(addr_hat, reg_center, zero);
		smbus.get_noreg(addr_hat, high_byte);
		smbus.get_noreg(addr_hat, low_byte);
		center = (high_byte << 8) | low_byte;

		std::uint16_t right;
		smbus.put_word(addr_hat, reg_right, zero);
		smbus.get_noreg(addr_hat, high_byte);
		smbus.get_noreg(addr_hat, low_byte);
		right = (high_byte << 8) | low_byte;

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "left = %4.4x, center = %4.4x, right = %4.4x", left, center, right);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


int main(int argc, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::important);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Create a chip.
	abc::gpio_chip<log_ostream> chip("/dev/gpiochip0", "picar_4wd", &log);

	// Init hat
	reset_hat(chip, log);

	// Info
	log_chip_info(chip, log);
	log_all_line_info(chip, log);

	// Ultrasonic - binary signal
	measure_distance(chip, log);

	// Servo - pwm
	move_servo(chip, log);

#ifdef TEMP ////
	// Motors - smbus
	turn_motors(log);
#endif

	measure_grayscale(log);

	return 0;
}
