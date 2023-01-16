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
#include <cmath>

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
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a2, "chip info:");
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a3, "  is_valid = %d", chip_info.is_valid);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a4, "  name     = %s", chip_info.name);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a5, "  label    = %s", chip_info.label);
	log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a6, "  lines    = %u", chip_info.lines);
	log.put_blank_line();
}


void log_all_line_info(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	abc::gpio_chip_info chip_info = chip.chip_info();

	for (abc::gpio_line_pos_t pos = 0; pos < chip_info.lines; pos++) {
		abc::gpio_line_info line_info = chip.line_info(pos);

		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a7, "line %2u info:", (unsigned)pos);
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a8, "  is_valid = %d", line_info.is_valid);
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106a9, "  name     = %s", line_info.name);
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106aa, "  consumer = %s", line_info.consumer);
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106ab, "  flags    = %llx", (long long)line_info.flags);
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106ac, "  in/out   = %s", (line_info.flags & abc::gpio_line_flag::output) != 0 ? "OUTPUT" : "INPUT");
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
			log.put_any(abc::category::abc::samples, abc::severity::important, 0x106ad, "TIMEOUT us = %llu", (long long)timeout.count());
			continue;
		}

		microseconds echo_us = std::chrono::duration_cast<microseconds>(echo_end_tp - echo_start_tp);
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106ae, "us = %llu", (long long)echo_us.count());

		// Calculate roundtip at the speed of sound - 340 m/s.
		double echo_s = echo_us.count() / (1000.0 * 1000.0);
		double roundtrip_m = 340.0 * echo_s;
		double roundtrip_cm = roundtrip_m * 100.0;
		double distance_cm = roundtrip_cm / 2.0;
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106af, "cm = %.2f", distance_cm);
		log.put_blank_line();

		// Sleep for 1 sec between iterations.
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


void turn_servo(log_ostream& log) {
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

		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b0, "duty_cycle = %u", duty_cycle);

		pwm_servo.set_duty_cycle(duty_cycle, duty_duration);

		std::this_thread::sleep_for(sleep_duration);
	}
}


void turn_servo_emulator(const abc::gpio_chip<log_ostream>& chip, log_ostream& log) {
	using microseconds = std::chrono::microseconds;
	using milliseconds = std::chrono::milliseconds;

	const microseconds min_pulse_width = microseconds(500);
	const microseconds max_pulse_width = microseconds(2500);
	const abc::gpio_pwm_pulse_frequency_t frequency = 50; // 50 Hz

	abc::gpio_pwm_emulator<log_ostream> pwm_servo(&chip, 5, min_pulse_width, max_pulse_width, frequency, &log);

	const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

	for (std::uint32_t duty_cycle : duty_cycles) {
		const milliseconds duty_duration = milliseconds(250);
		const milliseconds sleep_duration = milliseconds(500);

		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b1, "duty_cycle = %u", duty_cycle);

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

	std::size_t grand_total_count_rear_left  = 0;
	std::size_t grand_total_count_rear_right = 0;

	const abc::gpio_pwm_duty_cycle_t duty_cycles[] = { 25, 50, 75, 100 }; ////{ 25, 50, 75, 100, 75, 50, 25 };
	for (const abc::gpio_pwm_duty_cycle_t duty_cycle : duty_cycles) {
		const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_left	= duty_cycle;
		const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_right	= duty_cycle;

		pwm_wheel_front_left.set_duty_cycle(duty_cycle_rear_left);
		pwm_wheel_front_right.set_duty_cycle(duty_cycle_rear_right);
		pwm_wheel_rear_left.set_duty_cycle(duty_cycle_rear_left);
		pwm_wheel_rear_right.set_duty_cycle(duty_cycle_rear_right);

		std::size_t total_count_rear_left  = 0;
		std::size_t total_count_rear_right = 0;

		constexpr std::size_t reps = 4;

		for (std::size_t b = 0; b < reps; b++) {
			std::size_t count_rear_left  = 0;
			std::size_t count_rear_right = 0;

			abc::gpio_level_t level_prev_rear_left	= abc::gpio_level::invalid;
			abc::gpio_level_t level_prev_rear_right	= abc::gpio_level::invalid;

			char bit_rear_left[101] = { };
			char bit_rear_right[101] = { };

			for (std::size_t c = 0; c < 100; c++) {
				abc::gpio_level_t level_curr_rear_left	= speed_rear_left.get_level();
				abc::gpio_level_t level_curr_rear_right	= speed_rear_right.get_level();

				if (c < 100) {
					bit_rear_left[c] = '0' + level_curr_rear_left;
					bit_rear_right[c] = '0' + level_curr_rear_right;
				}

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

			log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "left =%s", bit_rear_left);
			log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "right=%s", bit_rear_right);

			log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b2, "duty_left = %3u, count_left = %3u, duty_right = %3u, count_right = %3u",
											(unsigned)duty_cycle_rear_left, (unsigned)count_rear_left, (unsigned)duty_cycle_rear_right, (unsigned)count_rear_right);

			total_count_rear_left  += count_rear_left;
			total_count_rear_right += count_rear_right;
		}

		grand_total_count_rear_left  += total_count_rear_left;
		grand_total_count_rear_right += total_count_rear_right;

		total_count_rear_left  = (total_count_rear_left + reps / 2) / reps;
		total_count_rear_right = (total_count_rear_right + reps / 2) / reps;

		////log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "----------------------------------------------------------------------");
		////std::this_thread::sleep_for(std::chrono::seconds(1));

		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b3, "----------------------------------------------------------------------");
		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b4, "duty_left = %3u, count_left = %3u, duty_right = %3u, count_right = %3u",
										(unsigned)duty_cycle_rear_left, (unsigned)total_count_rear_left, (unsigned)duty_cycle_rear_right, (unsigned)total_count_rear_right);
		log.put_blank_line();
	}

	std::size_t dist_left  = (21 * grand_total_count_rear_left)  / 10;
	std::size_t dist_right = (21 * grand_total_count_rear_right) / 10;

	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "count_left = %3u, dist_left = %3u, count_right = %3u, dist_right = %3u",
									(unsigned)grand_total_count_rear_left, (unsigned)dist_left, (unsigned)grand_total_count_rear_right, (unsigned)dist_right);
	log.put_blank_line();

	pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
	pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
}


inline int mod(int i, int n) {
	return (i + n) % n;
}

void measure_accel_and_spin_original(log_ostream& log) {
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

	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_left,
														reg_base_autoreload + reg_timer_front_left, reg_base_prescaler + reg_timer_front_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_right,
														reg_base_autoreload + reg_timer_front_right, reg_base_prescaler + reg_timer_front_right, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_left,
														reg_base_autoreload + reg_timer_rear_left, reg_base_prescaler + reg_timer_rear_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_right,
														reg_base_autoreload + reg_timer_rear_right, reg_base_prescaler + reg_timer_rear_right, &log);

	constexpr abc::gpio_smbus_address_t			smbus_accel_addr				= 0x68;
	constexpr abc::gpio_smbus_clock_frequency_t	smbus_accel_clock_frequency		= 1 * std::kilo::num; // Not true, but doesn't matter.
	constexpr bool								smbus_accel_requires_byte_swap	= true;

	abc::gpio_smbus_target<log_ostream> accel(smbus_accel_addr, smbus_accel_clock_frequency, smbus_accel_requires_byte_swap, &log);

	const abc::gpio_smbus_register_t reg_pwr_mgmt_1		= 0x6b;
	const abc::gpio_smbus_register_t reg_gyro_config	= 0x1b;
	const abc::gpio_smbus_register_t reg_accel_config	= 0x1c;
	////const abc::gpio_smbus_register_t reg_int			= 0x3a;
	const abc::gpio_smbus_register_t reg_accel_xout_h	= 0x3b;
	////const abc::gpio_smbus_register_t reg_accel_xout_l	= 0x3c;
	const abc::gpio_smbus_register_t reg_accel_yout_h	= 0x3d;
	const abc::gpio_smbus_register_t reg_accel_zout_h	= 0x3f;
	const abc::gpio_smbus_register_t reg_gyro_zout_h	= 0x47;

	smbus.put_byte(accel, reg_pwr_mgmt_1, 0x03);		// Gyro Z clock
	smbus.put_byte(accel, reg_accel_config, 0x02 << 3);	// +/-8 g
	smbus.put_byte(accel, reg_gyro_config, 0x02 << 3);	// +/-1000 dps

	constexpr double g = 100.0 * 9.8;

	constexpr std::int32_t reps_base = 100;

	std::int32_t base_accel_x = 0x0214; //0.130 * (double)0x8000U / 8.0;
	std::int32_t base_accel_y = 0x0038; //0.014 * (double)0x8000U / 8.0;
	std::int32_t base_accel_z = 0x0d34; //0.825 * (double)0x8000U / 8.0;
	std::int32_t base_gyro_z  = 0x0037; //1.678 * (double)0x8000U / 1000.0;

#if 0
	for (int c = 0; c < reps_base; c++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::uint16_t w;

		smbus.get_word(accel, reg_accel_xout_h, w);
		std::int16_t accel_x_temp = static_cast<std::int16_t>(w);
		base_accel_x += accel_x_temp;

		smbus.get_word(accel, reg_accel_yout_h, w);
		std::int16_t accel_y_temp = static_cast<std::int16_t>(w);
		base_accel_y += accel_y_temp;

		smbus.get_word(accel, reg_accel_zout_h, w);
		std::int16_t accel_z_temp = static_cast<std::int16_t>(w);
		base_accel_z += accel_z_temp;

		smbus.get_word(accel, reg_gyro_zout_h, w);
		std::int16_t gyro_z_temp = static_cast<std::int16_t>(w);
		base_gyro_z += gyro_z_temp;

		//// log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "accel_x_raw = %d | accel_x_g = %f | gyro_z_raw = %d | gyro_z_dps = %f", 
		////	(int)accel_x, ((double)accel_x * 8.0) / (double)0x8000U, (int)gyro_z, (double)gyro_z / 1000.0);
	}

	base_accel_x /= reps_base;
	base_accel_y /= reps_base;
	base_accel_z /= reps_base;
	base_gyro_z /= reps_base;
#endif

	log.put_blank_line();
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "BASE accel_x = %8x | BASE accel_y = %8x | BASE accel_z = %8x | BASE gyro_z = %8x", 
		base_accel_x, base_accel_y, base_accel_z, base_gyro_z);
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "BASE accel_x = %8.3f | BASE accel_y = %8.3f | BASE accel_z = %8.3f | BASE gyro_z = %8.3f", 
		((double)base_accel_x * 8.0) / (double)0x8000U, ((double)base_accel_y * 8.0) / (double)0x8000U, ((double)base_accel_z * 8.0) / (double)0x8000U, ((double)base_gyro_z * 1000.0) / (double)0x8000U);
	log.put_blank_line();

	return; ////

	constexpr abc::gpio_pwm_duty_cycle_t duty_cycle = 50;
	const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_left	= duty_cycle;
	const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_right	= duty_cycle;

	double curr_speed = 0;
	double total_distance = 0;

	std::chrono::system_clock::time_point tp_begin = std::chrono::system_clock::now();
	
	constexpr std::int16_t reps = 300;
	for (int rep = 0; rep < reps; rep++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		if (rep == 0) {
			pwm_wheel_front_left.set_duty_cycle(duty_cycle_rear_left);
			pwm_wheel_front_right.set_duty_cycle(duty_cycle_rear_right);
			pwm_wheel_rear_left.set_duty_cycle(duty_cycle_rear_left);
			pwm_wheel_rear_right.set_duty_cycle(duty_cycle_rear_right);
			log.put_blank_line();
		}

		if (rep == reps - 25 - 1) {
			pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			log.put_blank_line();
		}

		std::uint16_t w;

		smbus.get_word(accel, reg_accel_xout_h, w);
		std::int32_t accel_x = static_cast<std::int16_t>(w) - base_accel_x;

		////smbus.get_word(accel, reg_accel_yout_h, w);
		////std::int32_t accel_y = static_cast<std::int16_t>(w) - base_accel_y;

		////smbus.get_word(accel, reg_accel_zout_h, w);
		////std::int32_t accel_z = static_cast<std::int16_t>(w) - base_accel_z;

		smbus.get_word(accel, reg_gyro_zout_h, w);
		std::int32_t gyro_z = static_cast<std::int16_t>(w) - base_gyro_z;

		std::chrono::system_clock::time_point tp_end = std::chrono::system_clock::now();
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_begin);
		tp_begin = std::chrono::system_clock::now();

		double sec = (double)ms.count() / 1000.0;
		double curr_accel_x = ((double)accel_x * 8.0) / (double)0x8000U;
		double new_speed = curr_speed + (curr_accel_x * g * sec);
		total_distance += (curr_speed + new_speed) * sec / 2.0;
		curr_speed = new_speed;

		////log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "accel_x_raw = %d | accel_x_g = %f | gyro_z_raw = %d | gyro_z_dps = %f", 
		////	(int)accel_x, ((double)accel_x * 8.0) / (double)0x8000U, (int)gyro_z, (double)gyro_z / 1000.0);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "curr_accel_x = %8.3f | curr_speed = %8.3f | total_dist = %8.3f | ms = %zu", 
			curr_accel_x, curr_speed, total_distance, (size_t)ms.count());
	}

	log.put_blank_line();
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "curr_speed = %f | total_dist = %f", 
		curr_speed, total_distance);
}


void measure_accel_and_spin(log_ostream& log) {
	abc::gpio_smbus<log_ostream> smbus(1, &log);

	abc::gpio_smbus_motion<log_ostream> motion(&smbus, &log);

	motion.calibrate(abc::gpio_smbus_motion_channel::all);
	const abc::gpio_smbus_motion_measurements& calibration = motion.calibration();

	const abc::gpio_smbus_motion_measurements zeros{ };
	abc::gpio_smbus_motion_values values;
	motion.get_values_from_measurements(abc::gpio_smbus_motion_channel::all, calibration, zeros, values);

	log.put_blank_line();
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "BASE accel_x = %8x | BASE accel_y = %8x | BASE accel_z = %8x | BASE gyro_x = %8x | BASE gyro_y = %8x | BASE gyro_z = %8x",
		calibration.accel_x, calibration.accel_y, calibration.accel_z, calibration.gyro_x, calibration.gyro_y, calibration.gyro_z);

	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "BASE accel_x = %8.3f | BASE accel_y = %8.3f | BASE accel_z = %8.3f | BASE gyro_x = %8.3f | BASE gyro_y = %8.3f | BASE gyro_z = %8.3f",
		values.accel_x, values.accel_y, values.accel_z, values.gyro_x, values.gyro_y, values.gyro_z);
	log.put_blank_line();

	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	motion.get_values(abc::gpio_smbus_motion_channel::all, values);

	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "(01) accel_x = %8.3f | (01) accel_y = %8.3f | (01) accel_z = %8.3f | (01) gyro_x = %8.3f | (01) gyro_y = %8.3f | (01) gyro_z = %8.3f | (01) temp = %5.2f",
		values.accel_x, values.accel_y, values.accel_z, values.gyro_x, values.gyro_y, values.gyro_z, values.temperature);
	log.put_blank_line();

	return;


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

	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_left,
														reg_base_autoreload + reg_timer_front_left, reg_base_prescaler + reg_timer_front_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_right,
														reg_base_autoreload + reg_timer_front_right, reg_base_prescaler + reg_timer_front_right, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_left,
														reg_base_autoreload + reg_timer_rear_left, reg_base_prescaler + reg_timer_rear_left, &log);
	abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_right,
														reg_base_autoreload + reg_timer_rear_right, reg_base_prescaler + reg_timer_rear_right, &log);

	constexpr abc::gpio_smbus_address_t			smbus_accel_addr				= 0x68;
	constexpr abc::gpio_smbus_clock_frequency_t	smbus_accel_clock_frequency		= 1 * std::kilo::num; // Not true, but doesn't matter.
	constexpr bool								smbus_accel_requires_byte_swap	= true;

	abc::gpio_smbus_target<log_ostream> accel(smbus_accel_addr, smbus_accel_clock_frequency, smbus_accel_requires_byte_swap, &log);

	const abc::gpio_smbus_register_t reg_pwr_mgmt_1		= 0x6b;
	const abc::gpio_smbus_register_t reg_gyro_config	= 0x1b;
	const abc::gpio_smbus_register_t reg_accel_config	= 0x1c;
	////const abc::gpio_smbus_register_t reg_int			= 0x3a;
	const abc::gpio_smbus_register_t reg_accel_xout_h	= 0x3b;
	////const abc::gpio_smbus_register_t reg_accel_xout_l	= 0x3c;
	const abc::gpio_smbus_register_t reg_accel_yout_h	= 0x3d;
	const abc::gpio_smbus_register_t reg_accel_zout_h	= 0x3f;
	const abc::gpio_smbus_register_t reg_gyro_zout_h	= 0x47;

	smbus.put_byte(accel, reg_pwr_mgmt_1, 0x03);		// Gyro Z clock
	smbus.put_byte(accel, reg_accel_config, 0x02 << 3);	// +/-8 g
	smbus.put_byte(accel, reg_gyro_config, 0x02 << 3);	// +/-1000 dps

	constexpr double g = 100.0 * 9.8;

	constexpr std::int32_t reps_base = 100;

	std::int32_t base_accel_x = 0x0214; //0.130 * (double)0x8000U / 8.0;
	std::int32_t base_accel_y = 0x0038; //0.014 * (double)0x8000U / 8.0;
	std::int32_t base_accel_z = 0x0d34; //0.825 * (double)0x8000U / 8.0;
	std::int32_t base_gyro_z  = 0x0037; //1.678 * (double)0x8000U / 1000.0;

#if 0
	for (int c = 0; c < reps_base; c++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		std::uint16_t w;

		smbus.get_word(accel, reg_accel_xout_h, w);
		std::int16_t accel_x_temp = static_cast<std::int16_t>(w);
		base_accel_x += accel_x_temp;

		smbus.get_word(accel, reg_accel_yout_h, w);
		std::int16_t accel_y_temp = static_cast<std::int16_t>(w);
		base_accel_y += accel_y_temp;

		smbus.get_word(accel, reg_accel_zout_h, w);
		std::int16_t accel_z_temp = static_cast<std::int16_t>(w);
		base_accel_z += accel_z_temp;

		smbus.get_word(accel, reg_gyro_zout_h, w);
		std::int16_t gyro_z_temp = static_cast<std::int16_t>(w);
		base_gyro_z += gyro_z_temp;

		//// log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "accel_x_raw = %d | accel_x_g = %f | gyro_z_raw = %d | gyro_z_dps = %f", 
		////	(int)accel_x, ((double)accel_x * 8.0) / (double)0x8000U, (int)gyro_z, (double)gyro_z / 1000.0);
	}

	base_accel_x /= reps_base;
	base_accel_y /= reps_base;
	base_accel_z /= reps_base;
	base_gyro_z /= reps_base;
#endif

	log.put_blank_line();
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "BASE accel_x = %8x | BASE accel_y = %8x | BASE accel_z = %8x | BASE gyro_z = %8x", 
		base_accel_x, base_accel_y, base_accel_z, base_gyro_z);
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "BASE accel_x = %8.3f | BASE accel_y = %8.3f | BASE accel_z = %8.3f | BASE gyro_z = %8.3f", 
		((double)base_accel_x * 8.0) / (double)0x8000U, ((double)base_accel_y * 8.0) / (double)0x8000U, ((double)base_accel_z * 8.0) / (double)0x8000U, ((double)base_gyro_z * 1000.0) / (double)0x8000U);
	log.put_blank_line();

	return; ////

	constexpr abc::gpio_pwm_duty_cycle_t duty_cycle = 50;
	const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_left	= duty_cycle;
	const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_right	= duty_cycle;

	double curr_speed = 0;
	double total_distance = 0;

	std::chrono::system_clock::time_point tp_begin = std::chrono::system_clock::now();
	
	constexpr std::int16_t reps = 300;
	for (int rep = 0; rep < reps; rep++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(2));

		if (rep == 0) {
			pwm_wheel_front_left.set_duty_cycle(duty_cycle_rear_left);
			pwm_wheel_front_right.set_duty_cycle(duty_cycle_rear_right);
			pwm_wheel_rear_left.set_duty_cycle(duty_cycle_rear_left);
			pwm_wheel_rear_right.set_duty_cycle(duty_cycle_rear_right);
			log.put_blank_line();
		}

		if (rep == reps - 25 - 1) {
			pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
			log.put_blank_line();
		}

		std::uint16_t w;

		smbus.get_word(accel, reg_accel_xout_h, w);
		std::int32_t accel_x = static_cast<std::int16_t>(w) - base_accel_x;

		////smbus.get_word(accel, reg_accel_yout_h, w);
		////std::int32_t accel_y = static_cast<std::int16_t>(w) - base_accel_y;

		////smbus.get_word(accel, reg_accel_zout_h, w);
		////std::int32_t accel_z = static_cast<std::int16_t>(w) - base_accel_z;

		smbus.get_word(accel, reg_gyro_zout_h, w);
		std::int32_t gyro_z = static_cast<std::int16_t>(w) - base_gyro_z;

		std::chrono::system_clock::time_point tp_end = std::chrono::system_clock::now();
		std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(tp_end - tp_begin);
		tp_begin = std::chrono::system_clock::now();

		double sec = (double)ms.count() / 1000.0;
		double curr_accel_x = ((double)accel_x * 8.0) / (double)0x8000U;
		double new_speed = curr_speed + (curr_accel_x * g * sec);
		total_distance += (curr_speed + new_speed) * sec / 2.0;
		curr_speed = new_speed;

		////log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "accel_x_raw = %d | accel_x_g = %f | gyro_z_raw = %d | gyro_z_dps = %f", 
		////	(int)accel_x, ((double)accel_x * 8.0) / (double)0x8000U, (int)gyro_z, (double)gyro_z / 1000.0);
		log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "curr_accel_x = %8.3f | curr_speed = %8.3f | total_dist = %8.3f | ms = %zu", 
			curr_accel_x, curr_speed, total_distance, (size_t)ms.count());
	}

	log.put_blank_line();
	log.put_any(abc::category::abc::samples, abc::severity::important, __TAG__, "curr_speed = %f | total_dist = %f", 
		curr_speed, total_distance);
}


void make_turns(log_ostream& log) {
	const abc::gpio_smbus_register_t reg_wheel_front_left	= 0x0d;
	const abc::gpio_smbus_register_t reg_wheel_front_right	= 0x0c;
	const abc::gpio_smbus_register_t reg_wheel_rear_left	= 0x08;
	const abc::gpio_smbus_register_t reg_wheel_rear_right	= 0x09;
	const abc::gpio_smbus_register_t reg_timer_front_left	= reg_wheel_front_left / 4;
	const abc::gpio_smbus_register_t reg_timer_front_right	= reg_wheel_front_right / 4;
	const abc::gpio_smbus_register_t reg_timer_rear_left	= reg_wheel_rear_left / 4;
	const abc::gpio_smbus_register_t reg_timer_rear_right	= reg_wheel_rear_right / 4;
	const abc::gpio_pwm_pulse_frequency_t frequency			= 50; // 50 Hz

	abc::gpio_smbus<log_ostream> smbus(1, &log);
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

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

		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b5, "deg = %3d, delta = %3d", deg, delta_duty_cycle);

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
	abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

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

		log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b6, "left = %4.4x, center = %4.4x, right = %4.4x", left, center, right);

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}


void run_all() {
	// Create a log.
	abc::log_filter filter(abc::severity::abc::important);
	log_ostream log(std::cout.rdbuf(), &filter);

	// Create a chip.
	abc::gpio_chip<log_ostream> chip(0, "picar_4wd", &log);

	// Init hat
	reset_hat(chip, log);

#if 0
	// Info
	log_chip_info(chip, log);
	log_all_line_info(chip, log);

	// Ultrasonic - binary input
	measure_obstacle(chip, log);

	// Servo - pwm output
	turn_servo(log);
	turn_servo_emulator(chip, log);

	// Wheels - pwm output
	turn_wheels(log);

	// Speed - photo interrupter
	measure_speed(chip, log);
#endif

	// Speed - accelerometer
	measure_accel_and_spin(log);

#if 0
	// Wheels - pwm output
	make_turns(log);

	// Grayscale - pwm input
	measure_grayscale(log);
#endif
}
