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


#include <chrono>
#include <ratio>
#include <vector>
#include <cmath>
#include <iostream>

#include "../../src/diag/log.h"
#include "../../src/gpio/chip.h"
#include "../../src/gpio/line.h"
#include "../../src/gpio/ultrasonic.h"
#include "../../src/gpio/pwm_emulator.h"
#include "../../src/smbus/pwm.h"
#include "../../src/smbus/servo.h"
#include "../../src/smbus/motor.h"


constexpr abc::smbus::clock_frequency_t smbus_hat_clock_frequency    = 72 * std::mega::num;
constexpr abc::smbus::address_t         smbus_hat_addr               = 0x14;
constexpr bool                          smbus_hat_requires_byte_swap = true;
constexpr abc::smbus::register_t        smbus_hat_reg_base_pwm       = 0x20;
constexpr abc::smbus::register_t        reg_base_autoreload          = 0x44;
constexpr abc::smbus::register_t        reg_base_prescaler           = 0x40;


constexpr const char* origin = "sample_picar_4wd_hacks";


void log_chip_info(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "log_chip_info()";

    abc::gpio::chip_info chip_info = chip.chip_info();

    log.put_blank_line(origin, abc::diag::severity::important);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x106a2, "chip info:");
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x106a4, "  name  = %s", chip_info.name);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x106a5, "  label = %s", chip_info.label);
    log.put_any(origin, suborigin, abc::diag::severity::important, 0x106a6, "  lines = %u", chip_info.lines);
    log.put_blank_line(origin, abc::diag::severity::important);
}


void log_all_line_info(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "log_all_line_info()";

    abc::gpio::chip_info chip_info = chip.chip_info();

    log.put_blank_line(origin, abc::diag::severity::important);
    for (abc::gpio::line_pos_t pos = 0; pos < chip_info.lines; pos++) {
        abc::gpio::line_info line_info = chip.line_info(pos);

        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106a7, "line %2u info:", (unsigned)pos);
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106a9, "  name     = %s", line_info.name);
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106aa, "  consumer = %s", line_info.consumer);
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106ab, "  flags    = %llx", (long long)line_info.flags);
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106ac, "  in/out   = %s", (line_info.flags & abc::gpio::line_flags::output) != 0 ? "OUTPUT" : "INPUT");
    log.put_blank_line(origin, abc::diag::severity::important);
    }
}


void reset_hat(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    using milliseconds = std::chrono::milliseconds;

    abc::gpio::output_line reset_line(&chip, 21, &log);

    reset_line.put_level(abc::gpio::level::low, milliseconds(1));
    reset_line.put_level(abc::gpio::level::high, milliseconds(3));
}


void measure_obstacle_raw(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "measure_obstacle_raw()";

    using clock = std::chrono::steady_clock;
    using microseconds = std::chrono::microseconds;

    abc::gpio::output_line trigger_line(&chip, 5, &log);
    abc::gpio::input_line echo_line(&chip, 6, &log);

    for (int i = 0; i < 10; i++) {
        // Clear and send a pulse.
        trigger_line.put_level(abc::gpio::level::low, microseconds(10));
        trigger_line.put_level(abc::gpio::level::high, microseconds(10));
        trigger_line.put_level(abc::gpio::level::low);

        clock::time_point echo_not_ready_tp = clock::now();
        microseconds timeout((2 * std::mega::num) / 340); // ~6,000 us

        clock::time_point echo_start_tp;
        clock::time_point echo_end_tp;

        // Make sure there is no echo in progress.
        abc::gpio::level_t level = echo_line.wait_for_level(abc::gpio::level::low, timeout);
        clock::time_point echo_ready_tp = clock::now();

        // Wait until the echo starts.
        if (level != abc::gpio::level::invalid) {
            timeout -= std::chrono::duration_cast<microseconds>(echo_ready_tp - echo_not_ready_tp);
            level = echo_line.wait_for_level(abc::gpio::level::high, timeout);
        }
        echo_start_tp = clock::now();

        // Wait until the echo ends.
        if (level != abc::gpio::level::invalid) {
            timeout -= std::chrono::duration_cast<microseconds>(echo_start_tp - echo_ready_tp);
            level = echo_line.wait_for_level(abc::gpio::level::low, timeout);
        }
        echo_end_tp = clock::now();

        if (level == abc::gpio::level::invalid) {
            log.put_any(origin, suborigin, abc::diag::severity::important, 0x106ad, "TIMEOUT us = %llu", (long long)timeout.count());
            continue;
        }

        microseconds echo_us = std::chrono::duration_cast<microseconds>(echo_end_tp - echo_start_tp);
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106ae, "us = %llu", (long long)echo_us.count());

        // Calculate roundtrip at the speed of sound - 340 m/s.
        double echo_s = echo_us.count() / (1000.0 * 1000.0);
        double roundtrip_m = 340.0 * echo_s;
        double roundtrip_cm = roundtrip_m * 100.0;
        double distance_cm = roundtrip_cm / 2.0;
        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106af, "cm = %.2f", distance_cm);
        log.put_blank_line(origin, abc::diag::severity::important);

        // Sleep for 1 sec between iterations.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void measure_obstacle_ultrasonic(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "measure_obstacle_ultrasonic()";

    const abc::gpio::line_pos_t trigger_line_pos = 5;
    const abc::gpio::line_pos_t echo_line_pos = 6;
    abc::gpio::ultrasonic<std::milli> ultrasonic_mm(&chip, trigger_line_pos, echo_line_pos, &log);

    for (int i = 0; i < 10; i++) {
        const std::size_t max_distance_mm = 500;
        std::size_t distance_mm = ultrasonic_mm.measure_distance(max_distance_mm);

        log.put_any(origin, suborigin, abc::diag::severity::important, __TAG__, "mm = %zu", distance_mm);

        // Sleep for 1 sec between iterations.
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


void turn_servo_emulator(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "turn_servo_emulator()";

    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;

    const microseconds min_pulse_width = microseconds(500);
    const microseconds max_pulse_width = microseconds(2500);
    const abc::gpio::pwm_pulse_frequency_t frequency = 50; // 50 Hz

    abc::gpio::pwm_emulator pwm_servo(&chip, 5, min_pulse_width, max_pulse_width, frequency, &log);

    const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

    for (std::uint32_t duty_cycle : duty_cycles) {
        const milliseconds duty_duration = milliseconds(250);
        const milliseconds sleep_duration = milliseconds(500);

        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106b1, "duty_cycle = %u", duty_cycle);

        pwm_servo.set_duty_cycle(duty_cycle, duty_duration);

        std::this_thread::sleep_for(sleep_duration);
    }
}


void turn_servo_pwm(abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "turn_servo_pwm()";

    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;

    const microseconds min_pulse_width = microseconds(500);
    const microseconds max_pulse_width = microseconds(2500);
    const abc::smbus::pwm_pulse_frequency_t frequency = 50; // 50 Hz

    const abc::smbus::register_t reg_servo = 0x00;
    const abc::smbus::register_t reg_timer = reg_servo / 4;

    abc::smbus::controller controller(1, &log);
    abc::smbus::target hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap);
    abc::smbus::pwm pwm_servo(&controller, hat, min_pulse_width, max_pulse_width, frequency, smbus_hat_reg_base_pwm + reg_servo, reg_base_autoreload + reg_timer, reg_base_prescaler + reg_timer, &log);

    const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

    for (std::uint32_t duty_cycle : duty_cycles) {
        const milliseconds duty_duration = milliseconds(250);
        const milliseconds sleep_duration = milliseconds(500);

        log.put_any(origin, suborigin, abc::diag::severity::important, 0x106b0, "duty_cycle = %u", duty_cycle);

        pwm_servo.set_duty_cycle(duty_cycle, duty_duration);

        std::this_thread::sleep_for(sleep_duration);
    }
}


void turn_servo(abc::diag::log_ostream& log) {
    constexpr const char* suborigin = "turn_servo()";

    using microseconds = std::chrono::microseconds;
    using milliseconds = std::chrono::milliseconds;

    const microseconds min_pulse_width = microseconds(500);
    const microseconds max_pulse_width = microseconds(2500);
    const milliseconds duty_duration = milliseconds(250);
    const abc::smbus::pwm_pulse_frequency_t frequency = 50; // 50 Hz

    const abc::smbus::register_t reg_servo = 0x00;
    const abc::smbus::register_t reg_timer = reg_servo / 4;

    abc::smbus::controller controller(1, &log);
    abc::smbus::target hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap);
    abc::smbus::servo<microseconds> servo(&controller, hat, min_pulse_width, max_pulse_width, duty_duration, frequency, smbus_hat_reg_base_pwm + reg_servo, reg_base_autoreload + reg_timer, reg_base_prescaler + reg_timer, &log);

    const std::vector<std::uint32_t> duty_cycles{ 25, 75, 50 };

    for (std::uint32_t duty_cycle : duty_cycles) {
        const milliseconds sleep_duration = milliseconds(500);

        log.put_any(origin, suborigin, abc::diag::severity::important, __TAG__, "duty_cycle = %u", duty_cycle);

        servo.set_duty_cycle(duty_cycle);

        std::this_thread::sleep_for(sleep_duration);
    }
}


void turn_wheels_pwm(abc::diag::log_ostream& log) {
    abc::smbus::controller controller(1, &log);
    abc::smbus::target hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap);
    const abc::smbus::register_t reg_front_left  = 0x0d;
    const abc::smbus::register_t reg_front_right = 0x0c;
    const abc::smbus::register_t reg_rear_left   = 0x08;
    const abc::smbus::register_t reg_rear_right  = 0x09;

    const abc::smbus::register_t reg_wheels[] = { reg_front_left, reg_front_right, reg_rear_left, reg_rear_right };

    for (const abc::smbus::register_t reg_wheel : reg_wheels) {
        const abc::smbus::register_t reg_timer = reg_wheel / 4;
        const abc::smbus::pwm_pulse_frequency_t frequency = 50; // 50 Hz
        const std::chrono::milliseconds duty_duration(500);

        abc::smbus::pwm pwm_wheel(&controller, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel, reg_base_autoreload + reg_timer, reg_base_prescaler + reg_timer, &log);

        const abc::smbus::pwm_duty_cycle_t duty_cycles[] = { 25, 50, 75, 100, 75, 50, 25 };
        for (const abc::smbus::pwm_duty_cycle_t duty_cycle : duty_cycles) {
            pwm_wheel.set_duty_cycle(duty_cycle, duty_duration);
        }
    }
}


void turn_wheels_motor(const abc::gpio::chip& chip, abc::diag::log_ostream& log) {
    abc::smbus::controller controller(1, &log);
    abc::smbus::target hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap);
    const abc::smbus::register_t reg_front_left  = 0x0d;
    const abc::smbus::register_t reg_front_right = 0x0c;
    const abc::smbus::register_t reg_rear_left   = 0x08;
    const abc::smbus::register_t reg_rear_right  = 0x09;

    const abc::gpio::line_pos_t direction_line_wheels[] = {             23,              24,            13,             20 };
    const abc::smbus::register_t reg_wheels[]           = { reg_front_left, reg_front_right, reg_rear_left, reg_rear_right };

    for (std::size_t i = 0; i < sizeof(reg_wheels) / sizeof(reg_wheels[0]); i++) {
        const abc::gpio::line_pos_t direction_line_wheel = direction_line_wheels[i];
        const abc::smbus::register_t reg_wheel = reg_wheels[i];
        const abc::smbus::register_t reg_timer = reg_wheel / 4;
        const abc::smbus::pwm_pulse_frequency_t frequency = 50; // 50 Hz

        abc::smbus::motor motor(&chip, direction_line_wheel, &controller, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel, reg_base_autoreload + reg_timer, reg_base_prescaler + reg_timer, &log);

        const abc::smbus::pwm_duty_cycle_t duty_cycles[] = { 25, 50, 75, 100, 75, 50, 25 };
        for (const abc::smbus::pwm_duty_cycle_t duty_cycle : duty_cycles) {
            motor.set_duty_cycle(duty_cycle);
        }
    }
}


#if 0
void measure_speed(const abc::chip<log_ostream>& chip, log_ostream& log) {
    abc::gpio_smbus<log_ostream> smbus(1, &log);
    abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

    const abc::gpio_smbus_register_t reg_wheel_front_left    = 0x0d;
    const abc::gpio_smbus_register_t reg_wheel_front_right    = 0x0c;
    const abc::gpio_smbus_register_t reg_wheel_rear_left    = 0x08;
    const abc::gpio_smbus_register_t reg_wheel_rear_right    = 0x09;
    const abc::gpio_smbus_register_t reg_timer_front_left    = reg_wheel_front_left / 4;
    const abc::gpio_smbus_register_t reg_timer_front_right    = reg_wheel_front_right / 4;
    const abc::gpio_smbus_register_t reg_timer_rear_left    = reg_wheel_rear_left / 4;
    const abc::gpio_smbus_register_t reg_timer_rear_right    = reg_wheel_rear_right / 4;
    const abc::gpio_pwm_pulse_frequency_t frequency            = 50; // 50 Hz

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
        const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_left    = duty_cycle;
        const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_right    = duty_cycle;

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

            abc::level_t level_prev_rear_left    = abc::level::invalid;
            abc::level_t level_prev_rear_right    = abc::level::invalid;

            char bit_rear_left[101] = { };
            char bit_rear_right[101] = { };

            for (std::size_t c = 0; c < 100; c++) {
                abc::level_t level_curr_rear_left    = speed_rear_left.get_level();
                abc::level_t level_curr_rear_right    = speed_rear_right.get_level();

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

            log.put_any(abc::category::abc::samples, abc::severity::important, 0x10743, "left =%s", bit_rear_left);
            log.put_any(abc::category::abc::samples, abc::severity::important, 0x10744, "right=%s", bit_rear_right);

            log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b2, "duty_left = %3u, count_left = %3u, duty_right = %3u, count_right = %3u",
                                            (unsigned)duty_cycle_rear_left, (unsigned)count_rear_left, (unsigned)duty_cycle_rear_right, (unsigned)count_rear_right);

            total_count_rear_left  += count_rear_left;
            total_count_rear_right += count_rear_right;
        }

        grand_total_count_rear_left  += total_count_rear_left;
        grand_total_count_rear_right += total_count_rear_right;

        total_count_rear_left  = (total_count_rear_left + reps / 2) / reps;
        total_count_rear_right = (total_count_rear_right + reps / 2) / reps;

        ////log.put_any(abc::category::abc::samples, abc::severity::important, 0x10745, "----------------------------------------------------------------------");
        ////std::this_thread::sleep_for(std::chrono::seconds(1));

        log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b3, "----------------------------------------------------------------------");
        log.put_any(abc::category::abc::samples, abc::severity::important, 0x106b4, "duty_left = %3u, count_left = %3u, duty_right = %3u, count_right = %3u",
                                        (unsigned)duty_cycle_rear_left, (unsigned)total_count_rear_left, (unsigned)duty_cycle_rear_right, (unsigned)total_count_rear_right);
        log.put_blank_line(abc::category::abc::samples, abc::severity::important);
    }

    std::size_t dist_left  = (21 * grand_total_count_rear_left)  / 10;
    std::size_t dist_right = (21 * grand_total_count_rear_right) / 10;

    log.put_any(abc::category::abc::samples, abc::severity::important, 0x10746, "count_left = %3u, dist_left = %3u, count_right = %3u, dist_right = %3u",
                                    (unsigned)grand_total_count_rear_left, (unsigned)dist_left, (unsigned)grand_total_count_rear_right, (unsigned)dist_right);
    log.put_blank_line(abc::category::abc::samples, abc::severity::important);

    pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
    pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
    pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
    pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
}


inline int mod(int i, int n) {
    return (i + n) % n;
}

void measure_accel_and_spin(log_ostream& log) {
    abc::gpio_smbus<log_ostream> smbus(1, &log);

    abc::gpio_smbus_target<log_ostream> hat(smbus_hat_addr, smbus_hat_clock_frequency, smbus_hat_requires_byte_swap, &log);

    const abc::gpio_smbus_register_t reg_wheel_front_left    = 0x0d;
    const abc::gpio_smbus_register_t reg_wheel_front_right    = 0x0c;
    const abc::gpio_smbus_register_t reg_wheel_rear_left    = 0x08;
    const abc::gpio_smbus_register_t reg_wheel_rear_right    = 0x09;
    const abc::gpio_smbus_register_t reg_timer_front_left    = reg_wheel_front_left / 4;
    const abc::gpio_smbus_register_t reg_timer_front_right    = reg_wheel_front_right / 4;
    const abc::gpio_smbus_register_t reg_timer_rear_left    = reg_wheel_rear_left / 4;
    const abc::gpio_smbus_register_t reg_timer_rear_right    = reg_wheel_rear_right / 4;
    const abc::gpio_pwm_pulse_frequency_t frequency            = 50; // 50 Hz

    abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_left,
                                                        reg_base_autoreload + reg_timer_front_left, reg_base_prescaler + reg_timer_front_left, &log);
    abc::gpio_smbus_pwm<log_ostream> pwm_wheel_front_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_front_right,
                                                        reg_base_autoreload + reg_timer_front_right, reg_base_prescaler + reg_timer_front_right, &log);
    abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_left(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_left,
                                                        reg_base_autoreload + reg_timer_rear_left, reg_base_prescaler + reg_timer_rear_left, &log);
    abc::gpio_smbus_pwm<log_ostream> pwm_wheel_rear_right(&smbus, hat, frequency, smbus_hat_reg_base_pwm + reg_wheel_rear_right,
                                                        reg_base_autoreload + reg_timer_rear_right, reg_base_prescaler + reg_timer_rear_right, &log);

    abc::gpio_smbus_motion<log_ostream> motion(&smbus, &log);
    abc::gpio_smbus_motion_tracker<std::centi, log_ostream> motion_tracker(&motion, &log);

    motion.calibrate(abc::gpio_smbus_motion_channel::all);

    constexpr abc::gpio_pwm_duty_cycle_t duty_cycle = 50;
    const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_left    = duty_cycle;
    const abc::gpio_pwm_duty_cycle_t duty_cycle_rear_right    = duty_cycle;

    const std::chrono::system_clock::duration dur_drive = std::chrono::milliseconds(1 * 1000);
    const std::chrono::system_clock::duration dur_inertia = std::chrono::milliseconds(200);

    std::chrono::system_clock::time_point tp_begin_drive = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point tp_begin_iteration = tp_begin_drive;

    // Start tracking.
    motion_tracker.start();

    // Start driving.
    bool is_driving = true;
    pwm_wheel_front_left.set_duty_cycle(duty_cycle_rear_left);
    pwm_wheel_front_right.set_duty_cycle(duty_cycle_rear_right);
    pwm_wheel_rear_left.set_duty_cycle(duty_cycle_rear_left);
    pwm_wheel_rear_right.set_duty_cycle(duty_cycle_rear_right);
    log.put_blank_line(abc::category::abc::samples, abc::severity::important);

    while (std::chrono::duration_cast<std::chrono::milliseconds>(tp_begin_iteration - tp_begin_drive) < dur_drive + dur_inertia) {
        if (is_driving && std::chrono::duration_cast<std::chrono::milliseconds>(tp_begin_iteration - tp_begin_drive) >= dur_drive) {
            // Stop driving.
            is_driving = false;
            pwm_wheel_front_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
            pwm_wheel_front_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
            pwm_wheel_rear_left.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
            pwm_wheel_rear_right.set_duty_cycle(abc::gpio_pwm_duty_cycle::min);
            log.put_blank_line(abc::category::abc::samples, abc::severity::important);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        tp_begin_iteration = std::chrono::system_clock::now();

        log.put_any(abc::category::abc::samples, abc::severity::important, 0x10747, "depth = %8.3f | width = %8.3f | direction = %8.3f | speed = %8.3f", 
            motion_tracker.depth(), motion_tracker.width(), motion_tracker.direction(), motion_tracker.speed());
    }

    // Stop tracking.
    motion_tracker.set_speed(0);
    motion_tracker.stop();

    log.put_blank_line(abc::category::abc::samples, abc::severity::important);
    log.put_any(abc::category::abc::samples, abc::severity::important, 0x10748, "depth = %8.3f | width = %8.3f | direction = %8.3f | speed = %8.3f", 
        motion_tracker.depth(), motion_tracker.width(), motion_tracker.direction(), motion_tracker.speed());
}


void make_turns(log_ostream& log) {
    const abc::gpio_smbus_register_t reg_wheel_front_left    = 0x0d;
    const abc::gpio_smbus_register_t reg_wheel_front_right    = 0x0c;
    const abc::gpio_smbus_register_t reg_wheel_rear_left    = 0x08;
    const abc::gpio_smbus_register_t reg_wheel_rear_right    = 0x09;
    const abc::gpio_smbus_register_t reg_timer_front_left    = reg_wheel_front_left / 4;
    const abc::gpio_smbus_register_t reg_timer_front_right    = reg_wheel_front_right / 4;
    const abc::gpio_smbus_register_t reg_timer_rear_left    = reg_wheel_rear_left / 4;
    const abc::gpio_smbus_register_t reg_timer_rear_right    = reg_wheel_rear_right / 4;
    const abc::gpio_pwm_pulse_frequency_t frequency            = 50; // 50 Hz

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
        abc::gpio_pwm_duty_cycle_t duty_cycle_left    = 50;
        abc::gpio_pwm_duty_cycle_t duty_cycle_right    = 50;

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
#endif


void run_all() {
    // Create a log.
    abc::stream::table_ostream table(std::cout.rdbuf());
    abc::diag::debug_line_ostream<> line(&table);
    abc::diag::str_log_filter<const char*> filter("", abc::diag::severity::important);
    abc::diag::log_ostream log(&line, &filter);

    // Create a chip.
    abc::gpio::chip chip(0, "picar_4wd", &log);

#if 0
    // Info
    log_chip_info(chip, log);
    log_all_line_info(chip, log);
#endif

    // Init hat
    reset_hat(chip, log);

#if 0
    // Ultrasonic - binary input
    measure_obstacle_raw(chip, log);
    measure_obstacle_ultrasonic(chip, log);
#endif

#if 0
    // Servo - pwm output
    turn_servo_emulator(chip, log);
    turn_servo_pwm(log);
    turn_servo(log);
#endif

    // Wheels - pwm output
    turn_wheels_pwm(log);
    turn_wheels_motor(chip, log);

#if 0
    // Speed - photo interrupter
    measure_speed(chip, log);
#endif

#if 0
    // Speed - accelerometer
    measure_accel_and_spin(log);

    // Wheels - pwm output
    make_turns(log);

    // Grayscale - pwm input
    measure_grayscale(log);
#endif
}
