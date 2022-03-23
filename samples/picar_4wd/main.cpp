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
#include <ratio>
#include <vector>

#include "../../src/gpio.h"


using log_ostream = abc::log_ostream<abc::debug_line_ostream<>, abc::log_filter>;

constexpr abc::gpio_smbus_clock_frequency_t	smbus_hat_clock_frequency		= 72 * std::mega::num;
constexpr abc::gpio_smbus_address_t			smbus_hat_addr					= 0x14;
constexpr bool								smbus_hat_requires_byte_swap	= true;
constexpr abc::gpio_smbus_register_t		smbus_hat_reg_base_pwm			= 0x20;
constexpr abc::gpio_smbus_register_t		reg_base_autoreload				= 0x44;
constexpr abc::gpio_smbus_register_t		reg_base_prescaler				= 0x40;



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


void measure_obstacle(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	using clock = std::chrono::steady_clock;
	using microseconds = std::chrono::microseconds;

	abc::gpio_output_line<log_ostream> trigger_line(&chip, 5, &log);
	abc::gpio_input_line<log_ostream> echo_line(&chip, 6, &log);

	for (int i = 0; i < 10; i++) {
		// Clear and send a pulse.
		trigger_line.put_level(abc::gpio_level::low, microseconds(10));
		trigger_line.put_level(abc::gpio_level::high, microseconds(10));
		trigger_line.put_level(abc::gpio_level::low);

		clock::time_point echo_not_ready_tp = clock::now();
		microseconds timeout((2 * std::mega::num) / 340); // ~6,000 us

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


void turn_servo(log_ostream& log) {
	using clock = std::chrono::steady_clock;
	using microseconds = std::chrono::microseconds;
	using milliseconds = std::chrono::milliseconds;

	const microseconds min_pulse_width = microseconds(500);
	const microseconds max_pulse_width = microseconds(2500);
	const abc::gpio_pwm_pulse_frequency_t frequency = 50; // 50 Hz

	const abc::gpio_smbus_register_t reg_servo = 0x00;
	const abc::gpio_smbus_register_t reg_timer = reg_servo / 4;

	abc::gpio_smbus<log_ostream> smbus(1, &log);
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_servo(&smbus, hat, min_pulse_width, max_pulse_width, frequency, smbus_hat_reg_base_pwm + reg_servo, reg_base_autoreload + reg_timer, reg_base_prescaler + reg_timer, &log);

	const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

	for (std::uint32_t duty_cycle : duty_cycles) {
		const milliseconds duty_duration = milliseconds(250);
		const milliseconds sleep_duration = milliseconds(500);

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "duty_cycle = %u", duty_cycle);

		pwm_servo.set_duty_cycle(duty_cycle, duty_duration);

		std::this_thread::sleep_for(sleep_duration);
	}
}


void reset_hat(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	using milliseconds = std::chrono::milliseconds;

	abc::gpio_output_line<log_ostream> reset_line(&chip, 21, &log);

	reset_line.put_level(abc::gpio_level::low, milliseconds(1));
	reset_line.put_level(abc::gpio_level::high, milliseconds(3));
}


void turn_wheels(log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus(1, &log);
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

	const abc::gpio_smbus_register_t reg_front_left		= 0x0d;
	const abc::gpio_smbus_register_t reg_front_right	= 0x0c;
	const abc::gpio_smbus_register_t reg_rear_left		= 0x08;
	const abc::gpio_smbus_register_t reg_rear_right		= 0x09;

	const abc::gpio_smbus_register_t reg_wheels[] = { reg_front_left, reg_front_right, reg_rear_left, reg_rear_right };

	for (const abc::gpio_smbus_register_t reg_wheel : reg_wheels) {
		const abc::gpio_smbus_register_t reg_timer		= reg_wheel / 4;
		const abc::gpio_pwm_pulse_frequency_t frequency	= 50; // 50 Hz
		const std::chrono::milliseconds duty_duration(500);

		abc::gpio_smbus_pwm<log_ostream> pwm_wheel(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel, reg_base_autoreload + reg_timer, reg_base_prescaler + reg_timer, &log);

		const abc::gpio_pwm_duty_cycle_t duty_cycles[] = { 25, 50, 75, 100, 75, 50, 25 };
		for (const abc::gpio_pwm_duty_cycle_t duty_cycle : duty_cycles) {
			pwm_wheel.set_duty_cycle(duty_cycle, duty_duration);
		}
	}
}


void measure_speed(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus(1, &log);
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

	const abc::gpio_smbus_register_t reg_wheel_front_left	= 0x0d;
	const abc::gpio_smbus_register_t reg_wheel_front_right	= 0x0c;
	const abc::gpio_smbus_register_t reg_wheel_rear_left	= 0x08;
	const abc::gpio_smbus_register_t reg_wheel_rear_right	= 0x09;
	const abc::gpio_smbus_register_t reg_timer_front_left	= reg_wheel_front_left / 4;
	const abc::gpio_smbus_register_t reg_timer_front_right	= reg_wheel_front_right / 4;
	const abc::gpio_smbus_register_t reg_timer_rear_left	= reg_wheel_rear_left / 4;
	const abc::gpio_smbus_register_t reg_timer_rear_right	= reg_wheel_rear_right / 4;
	const abc::gpio_pwm_pulse_frequency_t frequency			= 50; // 50 Hz
	const std::chrono::milliseconds duty_duration(500);

	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_left,
														reg_base_autoreload + reg_timer_front_left, reg_base_prescaler + reg_timer_front_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_right,
														reg_base_autoreload + reg_timer_front_right, reg_base_prescaler + reg_timer_front_right, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_left,
														reg_base_autoreload + reg_timer_rear_left, reg_base_prescaler + reg_timer_rear_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_right,
														reg_base_autoreload + reg_timer_rear_right, reg_base_prescaler + reg_timer_rear_right, &log);

	abc::gpio_input_line<log_ostream> speed_rear_left(&chip, 25, &log);
	abc::gpio_input_line<log_ostream> speed_rear_right(&chip, 4, &log);

	const abc::gpio_pwm_duty_cycle_t duty_cycles[] = { 25, 50, 75, 100, 75, 50, 25 };
	for (const abc::gpio_pwm_duty_cycle_t duty_cycle : duty_cycles) {
		const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_left	= duty_cycle;
		const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_right	= duty_cycle;

		pwm_wheel_front_left.set_duty_cycle(duty_cycle_rear_left);
		pwm_wheel_front_right.set_duty_cycle(duty_cycle_rear_right);
		pwm_wheel_rear_left.set_duty_cycle(duty_cycle_rear_left);
		pwm_wheel_rear_right.set_duty_cycle(duty_cycle_rear_right);

		std::size_t total_count_rear_left  = 0;
		std::size_t total_count_rear_right = 0;

		for (int b = 0; b < 5; b++) {
			std::size_t count_rear_left  = 0;
			std::size_t count_rear_right = 0;

			abc::gpio_level_t level_prev_rear_left	= abc::gpio_level::invalid;
			abc::gpio_level_t level_prev_rear_right	= abc::gpio_level::invalid;

			for (int c = 0; c < 200; c++) {
				abc::gpio_level_t level_curr_rear_left	= speed_rear_left.get_level();
				abc::gpio_level_t level_curr_rear_right	= speed_rear_right.get_level();

				const std::size_t change_rear_left  = level_prev_rear_left  != level_curr_rear_left  ? 1 : 0;
				const std::size_t change_rear_right = level_prev_rear_right != level_curr_rear_right ? 1 : 0;

				count_rear_left  += change_rear_left;
				count_rear_right += change_rear_right;

				level_prev_rear_left  = level_curr_rear_left;
				level_prev_rear_right = level_curr_rear_right;

				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			count_rear_left  /= 2;
			count_rear_right /= 2;

			log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "duty_left = %3u, count_left = %3u, duty_right = %3u, count_right = %3u",
											(unsigned)duty_cycle_rear_left, (unsigned)count_rear_left, (unsigned)duty_cycle_rear_right, (unsigned)count_rear_right);

			total_count_rear_left  += count_rear_left;
			total_count_rear_right += count_rear_right;
		}

		total_count_rear_left  /= 5;
		total_count_rear_right /= 5;

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "----------------------------------------------------------------------");
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "duty_left = %3u, count_left = %3u, duty_right = %3u, count_right = %3u",
										(unsigned)duty_cycle_rear_left, (unsigned)total_count_rear_left, (unsigned)duty_cycle_rear_right, (unsigned)total_count_rear_right);
		log.put_blank_line();

	}

	pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
}


void make_turns(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus(1, &log);
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

	const abc::gpio_smbus_register_t reg_wheel_front_left	= 0x0d;
	const abc::gpio_smbus_register_t reg_wheel_front_right	= 0x0c;
	const abc::gpio_smbus_register_t reg_wheel_rear_left	= 0x08;
	const abc::gpio_smbus_register_t reg_wheel_rear_right	= 0x09;
	const abc::gpio_smbus_register_t reg_timer_front_left	= reg_wheel_front_left / 4;
	const abc::gpio_smbus_register_t reg_timer_front_right	= reg_wheel_front_right / 4;
	const abc::gpio_smbus_register_t reg_timer_rear_left	= reg_wheel_rear_left / 4;
	const abc::gpio_smbus_register_t reg_timer_rear_right	= reg_wheel_rear_right / 4;
	const abc::gpio_pwm_pulse_frequency_t frequency			= 50; // 50 Hz
	const std::chrono::milliseconds duty_duration(500);

	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_left,
														reg_base_autoreload + reg_timer_front_left, reg_base_prescaler + reg_timer_front_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_right,
														reg_base_autoreload + reg_timer_front_right, reg_base_prescaler + reg_timer_front_right, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_left,
														reg_base_autoreload + reg_timer_rear_left, reg_base_prescaler + reg_timer_rear_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_right,
														reg_base_autoreload + reg_timer_rear_right, reg_base_prescaler + reg_timer_rear_right, &log);

	const int degs[] = { +30, -30 };
	for (const int deg : degs) {
		abc::gpio_pwm_duty_cycle_t duty_cycle_left	= 50;
		abc::gpio_pwm_duty_cycle_t duty_cycle_right	= 50;

		pwm_wheel_front_left.set_duty_cycle(duty_cycle_left);
		pwm_wheel_front_right.set_duty_cycle(duty_cycle_right);
		pwm_wheel_rear_left.set_duty_cycle(duty_cycle_left);
		pwm_wheel_rear_right.set_duty_cycle(duty_cycle_right);

		std::this_thread::sleep_for(std::chrono::milliseconds(500));

		abc::gpio_pwm_duty_cycle_t delta_duty_cycle = 0;
		switch (deg) {
		case -30:
		case +30:
			delta_duty_cycle = 11;
			break;

		case -45:
		case +45:
			delta_duty_cycle = 18;
			break;

		case -60:
		case +60:
			delta_duty_cycle = 25;
			break;
		}

		if (deg < 0) {
			duty_cycle_left  += delta_duty_cycle;
			duty_cycle_right -= delta_duty_cycle;
		}
		else {
			duty_cycle_left  -= delta_duty_cycle;
			duty_cycle_right += delta_duty_cycle;
		}

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "deg = %3d, delta = %3d", deg, delta_duty_cycle);

		pwm_wheel_front_left.set_duty_cycle(duty_cycle_left);
		pwm_wheel_front_right.set_duty_cycle(duty_cycle_right);
		pwm_wheel_rear_left.set_duty_cycle(duty_cycle_left);
		pwm_wheel_rear_right.set_duty_cycle(duty_cycle_right);

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
}


void measure_grayscale(log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus(1, &log);
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, &log);

	const abc::gpio_smbus_register_t reg_left = 0x12;
	const abc::gpio_smbus_register_t reg_center = 0x11;
	const abc::gpio_smbus_register_t reg_right = 0x10;
	const std::uint16_t zero = 0x0000;

	for (int i = 0; i < 10; i++) {
		std::uint16_t left;
		smbus.put_word(hat, reg_left, zero);
		smbus.get_noreg_2(hat, left);

		std::uint16_t center;
		smbus.put_word(hat, reg_center, zero);
		smbus.get_noreg_2(hat, center);

		std::uint16_t right;
		smbus.put_word(hat, reg_right, zero);
		smbus.get_noreg_2(hat, right);

		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "left = %4.4x, center = %4.4x, right = %4.4x", left, center, right);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


int main(int argc, const char* argv[]) {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::important);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Create a chip.
	abc::gpio_chip<log_ostream> chip(0, "picar_4wd", &log);

	// Init hat
	reset_hat(chip, log);

#ifdef TEMP
	// Info
	log_chip_info(chip, log);
	log_all_line_info(chip, log);

	// Ultrasonic - binary input
	measure_obstacle(chip, log);

	// Servo - pwm output
	turn_servo(log);

	// Wheels - pwm output
	turn_wheels(log);

	// Speed - binary input
	measure_speed(chip, log);
#endif

	// Wheels - pwm output
	make_turns(chip, log);

#ifdef TEMP
	// Grayscale - pwm input
	measure_grayscale(log);
#endif

	return 0;
}
