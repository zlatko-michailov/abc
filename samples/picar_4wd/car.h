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


#include <cstdlib>
#include <cstdio>
#include <ratio>
#include <sstream>
#include <string>
#include <thread>

#include "../../src/root/ascii.h"
#include "../../src/net/endpoint.h"
#include "../../src/net/http.h"
#include "../../src/net/json.h"

#include "car.i.h"


inline picar_4wd_hat::picar_4wd_hat(abc::gpio::chip* chip, abc::smbus::address_t addr, abc::smbus::clock_frequency_t clock_frequency, bool requires_byte_swap)
    : base(addr, clock_frequency, requires_byte_swap)
    , _chip(chip) {

    reset();
}


inline void picar_4wd_hat::reset() {
    abc::gpio::output_line reset_line(_chip, 21);

    reset_line.put_level(abc::gpio::level::low,  std::chrono::milliseconds(1));
    reset_line.put_level(abc::gpio::level::high, std::chrono::milliseconds(3));
}

// --------------------------------------------------------------


static constexpr abc::smbus::clock_frequency_t     hat_clock_frequency         = 72 * std::mega::num;
static constexpr abc::smbus::address_t             hat_addr                    = 0x14;
static constexpr bool                              hat_requires_byte_swap      = true;
static constexpr abc::smbus::register_t            reg_pwm_base                = 0x20;
static constexpr abc::smbus::register_t            reg_autoreload_base         = 0x44;
static constexpr abc::smbus::register_t            reg_prescaler_base          = 0x40;

static constexpr abc::smbus::register_t            reg_wheel_front_left        = 0x0d;
static constexpr abc::smbus::register_t            reg_wheel_front_right       = 0x0c;
static constexpr abc::smbus::register_t            reg_wheel_rear_left         = 0x08;
static constexpr abc::smbus::register_t            reg_wheel_rear_right        = 0x09;
static constexpr abc::smbus::register_t            reg_timer_front_left        = reg_wheel_front_left / 4;
static constexpr abc::smbus::register_t            reg_timer_front_right       = reg_wheel_front_right / 4;
static constexpr abc::smbus::register_t            reg_timer_rear_left         = reg_wheel_rear_left / 4;
static constexpr abc::smbus::register_t            reg_timer_rear_right        = reg_wheel_rear_right / 4;

static constexpr abc::smbus::register_t            reg_grayscale_left          = 0x12;
static constexpr abc::smbus::register_t            reg_grayscale_center        = 0x11;
static constexpr abc::smbus::register_t            reg_grayscale_right         = 0x10;

static constexpr abc::smbus::pwm_pulse_frequency_t frequency                   = 50; // 50 Hz

static constexpr abc::gpio::line_pos_t             pos_line_dir_front_left     = 23;
static constexpr abc::gpio::line_pos_t             pos_line_dir_front_right    = 24;
static constexpr abc::gpio::line_pos_t             pos_line_dir_rear_left      = 13;
static constexpr abc::gpio::line_pos_t             pos_line_dir_rear_right     = 20;

static constexpr abc::gpio::line_pos_t             pos_line_ultrasonic_trigger = 5;
static constexpr abc::gpio::line_pos_t             pos_line_ultrasonic_echo    = 6;
static constexpr std::size_t                       ultrasonic_max_cm           = 100;

static constexpr abc::smbus::register_t            reg_servo                   = 0x00;
static constexpr abc::smbus::register_t            reg_timer_servo             = reg_servo / 4;

static constexpr std::chrono::microseconds         servo_pulse_width_min       = std::chrono::microseconds(500);
static constexpr std::chrono::microseconds         servo_pulse_width_max       = std::chrono::microseconds(2500);
static constexpr std::chrono::milliseconds         servo_duty_duration         = std::chrono::milliseconds(250);


// --------------------------------------------------------------


inline car_endpoint::car_endpoint(abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log)
    : base(std::move(config), log)

    , _chip(0, "picar_4wd", log)
    , _controller(1, log)
    , _hat(&_chip, hat_addr, hat_clock_frequency, hat_requires_byte_swap)

    , _motor_front_left(&_chip, pos_line_dir_front_left,
                        &_controller, _hat, frequency, reg_pwm_base + reg_wheel_front_left,
                        reg_autoreload_base + reg_timer_front_left, reg_prescaler_base + reg_timer_front_left, log)
    , _motor_front_right(&_chip, pos_line_dir_front_right,
                        &_controller, _hat, frequency, reg_pwm_base + reg_wheel_front_right,
                        reg_autoreload_base + reg_timer_front_right, reg_prescaler_base + reg_timer_front_right, log)
    , _motor_rear_left(&_chip, pos_line_dir_rear_left,
                        &_controller, _hat, frequency, reg_pwm_base + reg_wheel_rear_left,
                        reg_autoreload_base + reg_timer_rear_left, reg_prescaler_base + reg_timer_rear_left, log)
    , _motor_rear_right(&_chip, pos_line_dir_rear_right,
                        &_controller, _hat, frequency, reg_pwm_base + reg_wheel_rear_right,
                        reg_autoreload_base + reg_timer_rear_right, reg_prescaler_base + reg_timer_rear_right, log)

    , _ultrasonic(&_chip, pos_line_ultrasonic_trigger, pos_line_ultrasonic_echo, log)
    , _servo(&_controller, _hat, servo_pulse_width_min, servo_pulse_width_max,
            servo_duty_duration, frequency,
            reg_pwm_base + reg_servo, reg_autoreload_base + reg_timer_servo, reg_prescaler_base + reg_timer_servo, log)
    , _grayscale(&_controller, _hat, reg_grayscale_left, reg_grayscale_center, reg_grayscale_right, log)

    , _motion(&_controller, log)
    , _motion_tracker(&_motion, log)

    , _forward(true)
    , _power(0)
    , _turn(0)
    , _obstacle_cm(ultrasonic_max_cm)

    , _auto_thread(start_auto_loop, this) {

    _motion.calibrate(abc::smbus::motion_channel::all);
}


inline std::unique_ptr<abc::net::tcp_server_socket> car_endpoint::create_server_socket() {
    return std::unique_ptr<abc::net::tcp_server_socket>(new abc::net::tcp_server_socket(abc::net::socket::family::ipv4, base::log()));
}


inline void car_endpoint::process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_rest_request()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10678, "Begin:");

    if (abc::ascii::are_equal_i(request.resource.path.c_str(), "/power")) {
        process_power(http, request);
    }
    else if (abc::ascii::are_equal_i(request.resource.path.c_str(), "/turn")) {
        process_turn(http, request);
    }
    else if (abc::ascii::are_equal_i(request.resource.path.c_str(), "/autos")) {
        process_autos(http, request);
    }
    else if (abc::ascii::are_equal_i(request.resource.path.c_str(), "/servo")) {
        process_servo(http, request);
    }
    else if (abc::ascii::are_equal_i(request.resource.path.c_str(), "/shutdown")) {
        process_shutdown(http, request);
    }
    else {
        // 404
        throw_exception(suborigin, 0x10679,
            abc::net::http::status_code::Not_Found, abc::net::http::reason_phrase::Not_Found, abc::net::http::content_type::text, "The requested resource was not found.");
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1067a, "End:");
}


inline void car_endpoint::process_power(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_power()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    require_method_post(suborigin, __TAG__, request);
    require_content_type_json(suborigin, __TAG__, request);

    std::int32_t power;
    // Read power from JSON
    {
        std::streambuf* sb = static_cast<abc::net::http::request_reader&>(http).rdbuf();
        abc::net::json::reader json(sb, diag_base::log());

        abc::net::json::value val = json.get_value();
        require(suborigin, 0x1067b, val.type() == abc::net::json::value_type::object,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a JSON object.");

        abc::net::json::literal::object::const_iterator power_itr = val.object().find("power");
        require(suborigin, 0x1067d, power_itr != val.object().cend(),
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"power\" property.");
        require(suborigin, 0x1067e, power_itr->second.type() == abc::net::json::value_type::number,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"power\" number.");
        require(suborigin, 0x1067f, -100 <= power_itr->second.number() && power_itr->second.number() <= 100 && (int)power_itr->second.number() % 25 == 0,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected -100 <= power <= 100.");

        power = power_itr->second.number();
    }

    // If changing direction, stop the vehicle and reset the motion tracker.
    if (power * _power < 0) {
        std::int32_t turn = _turn;

        _power = 0;
        _turn = 0;
        drive_verified();

        _motion_tracker.stop();

        _turn = turn;
    }

    if (power != 0 && !_motion_tracker.is_running()) {
        _motion_tracker.start();
    }

    _forward = true;
    if (power == 0) {
        _turn = 0;
    }

    if (power < 0) {
        _forward = false;
        power = -power;
    }

    _power = power;
    drive_verified();

    // If vehicle stopped, reset the motion tracker.
    if (_power == 0) {
        _motion_tracker.stop();
    }

    // 200
    std::stringbuf sb;
    abc::net::json::writer json(&sb, diag_base::log());

    abc::net::json::literal::object obj = {
        { "forward", abc::net::json::literal::number((int)_forward) },
        { "power",   abc::net::json::literal::number((int)_power) },
        { "turn",    abc::net::json::literal::number((int)_turn) }
    };
    json.put_value(abc::net::json::value(std::move(obj)));

    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x10681);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline void car_endpoint::process_turn(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_turn()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    require_method_post(suborigin, __TAG__, request);
    require_content_type_json(suborigin, __TAG__, request);

    std::int32_t turn;
    // Read turn from JSON
    {
        std::streambuf* sb = static_cast<abc::net::http::request_reader&>(http).rdbuf();
        abc::net::json::reader json(sb, diag_base::log());

        abc::net::json::value val = json.get_value();
        require(suborigin, 0x10682, val.type() == abc::net::json::value_type::object,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a JSON object.");

        abc::net::json::literal::object::const_iterator turn_itr = val.object().find("turn");
        require(suborigin, 0x10683, turn_itr != val.object().cend(),
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"turn\" property.");
        require(suborigin, 0x10684, turn_itr->second.type() == abc::net::json::value_type::number,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"turn\" number.");
        require(suborigin, 0x10685, -90 <= turn_itr->second.number() && turn_itr->second.number() <= 90 && (int)turn_itr->second.number() % 30 == 0,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected -90 <= turn <= 90.");

        turn = turn_itr->second.number();
    }

    _turn = turn;
    drive_verified();

    // 200
    std::stringbuf sb;
    abc::net::json::writer json(&sb, diag_base::log());

    abc::net::json::literal::object obj = {
        { "forward", abc::net::json::literal::number((int)_forward) },
        { "power",   abc::net::json::literal::number((int)_power) },
        { "turn",    abc::net::json::literal::number((int)_turn) }
    };
    json.put_value(abc::net::json::value(std::move(obj)));

    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x10688);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline void car_endpoint::process_autos(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_autos()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    require_method_get(suborigin, __TAG__, request);

    // 200
    std::stringbuf sb;
    abc::net::json::writer json(&sb, diag_base::log());

    abc::net::json::literal::object obj = {
        { "obstacle", abc::net::json::literal::object {
            { "distance", abc::net::json::literal::number((int)_obstacle_cm.load()) },
            { "units",    abc::net::json::literal::string("cm") }
        } },
        { "grayscale", abc::net::json::literal::object {
            { "left",   abc::net::json::literal::number((int)_grayscale_left.load()) },
            { "center", abc::net::json::literal::number((int)_grayscale_center.load()) },
            { "right",  abc::net::json::literal::number((int)_grayscale_right.load()) }
        } },
        { "depth", abc::net::json::literal::object {
            { "distance", abc::net::json::literal::number((int)_motion_tracker.depth()) },
            { "units",    abc::net::json::literal::string("cm") }
        } },
        { "width", abc::net::json::literal::object {
            { "distance", abc::net::json::literal::number((int)_motion_tracker.width()) },
            { "units",    abc::net::json::literal::string("cm") }
        } },
    };
    json.put_value(abc::net::json::value(std::move(obj)));

    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x10689);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline void car_endpoint::process_servo(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_servo()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    require_method_post(suborigin, __TAG__, request);
    require_content_type_json(suborigin, __TAG__, request);

    std::int32_t angle;
    // Read turn from JSON
    {
        std::streambuf* sb = static_cast<abc::net::http::request_reader&>(http).rdbuf();
        abc::net::json::reader json(sb, diag_base::log());

        abc::net::json::value val = json.get_value();
        require(suborigin, 0x1068b, val.type() == abc::net::json::value_type::object,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a JSON object.");

        abc::net::json::literal::object::const_iterator angle_itr = val.object().find("angle");
        require(suborigin, 0x1068c, angle_itr != val.object().cend(),
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"angle\" property.");
        require(suborigin, 0x1068d, angle_itr->second.type() == abc::net::json::value_type::number,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected a \"angle\" number.");
        require(suborigin, 0x1068e, -90 <= angle_itr->second.number() && angle_itr->second.number() <= 90 && (int)angle_itr->second.number() % 30 == 0,
            abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Expected -90 <= angle <= 90.");

        angle = angle_itr->second.number();
    }

    abc::smbus::pwm_duty_cycle_t duty_cycle = 100 - static_cast<abc::smbus::pwm_duty_cycle_t>(100 * (angle + 92) / 184);
    _servo.set_duty_cycle(duty_cycle);

    // 200
    std::stringbuf sb;
    abc::net::json::writer json(&sb, diag_base::log());

    abc::net::json::literal::object obj = {
        { "angle", abc::net::json::literal::number((int)angle) },
    };
    json.put_value(abc::net::json::value(std::move(obj)));

    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::json, sb.str().c_str(), 0x10691);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline void car_endpoint::process_shutdown(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_shutdown()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    require_method_post(suborigin, __TAG__, request);

    base::set_shutdown_requested();
    _hat.reset();
    _auto_thread.join();

    // 200
    base::send_simple_response(http, abc::net::http::status_code::OK, abc::net::http::reason_phrase::OK, abc::net::http::content_type::text, "Server is shuting down...", 0x10692);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline void car_endpoint::drive_verified() {
    constexpr const char* suborigin = "drive_verified()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    side_powers powers = calculate_side_powers();

    abc::smbus::pwm_duty_cycle_t left_duty_cycle = static_cast<abc::smbus::pwm_duty_cycle_t>(powers.left);
    abc::smbus::pwm_duty_cycle_t right_duty_cycle = static_cast<abc::smbus::pwm_duty_cycle_t>(powers.right);

    _motor_front_left.set_forward(_forward);
    _motor_front_left.set_duty_cycle(left_duty_cycle);

    _motor_front_right.set_forward(_forward);
    _motor_front_right.set_duty_cycle(right_duty_cycle);

    _motor_rear_left.set_forward(_forward);
    _motor_rear_left.set_duty_cycle(left_duty_cycle);

    _motor_rear_right.set_forward(_forward);
    _motor_rear_right.set_duty_cycle(right_duty_cycle);

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline car_endpoint::side_powers car_endpoint::calculate_side_powers() noexcept {
    constexpr const char* suborigin = "calculate_side_powers()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    std::int32_t delta = calculate_delta_power();

    side_powers powers;
    powers.left  = _power + delta;
    powers.right = _power - delta;

    constexpr std::int32_t min_power = static_cast<std::int32_t>(abc::smbus::pwm_duty_cycle::min);
    constexpr std::int32_t max_power = static_cast<std::int32_t>(abc::smbus::pwm_duty_cycle::max);

    std::int32_t adjust = 0;
    if (powers.left > max_power) {
        adjust = max_power - powers.left;
    }
    else if (powers.right > max_power) {
        adjust = max_power - powers.right;
    }
    else if (powers.left < min_power) {
        adjust = min_power - powers.left;
    }
    else if (powers.right < min_power) {
        adjust = min_power - powers.right;
    }

    powers.left  += adjust;
    powers.right += adjust;

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10693, "End: powers.left=%3d, powers.right=%3d", (int)powers.left, (int)powers.right);

    return powers;
}


inline std::int32_t car_endpoint::calculate_delta_power() noexcept {
    constexpr const char* suborigin = "calculate_delta_power()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    std::int32_t delta = 0;

    switch (_turn) {
    case -30:
    case +30:
        delta = 11;
        break;

    case -45:
    case +45:
        delta = 18;
        break;

    case -60:
    case +60:
        delta = 25;
        break;

    case -90:
    case +90:
        delta = 50;
        break;
    }

    if (_turn < 0) {
        delta = -delta;
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x10694, "End: power=%3d, turn=%3d, delta=%3d", (int)_power, (int)_turn, (int)delta);

    return delta;
}


inline void car_endpoint::require_method_get(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request) {
    require(suborigin, tag, abc::ascii::are_equal_i(request.method.c_str(), abc::net::http::method::GET),
        abc::net::http::status_code::Method_Not_Allowed, abc::net::http::reason_phrase::Method_Not_Allowed, abc::net::http::content_type::text, "Method error: Expected 'GET'.");
}


inline void car_endpoint::require_method_post(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request) {
    require(suborigin, tag, abc::ascii::are_equal_i(request.method.c_str(), abc::net::http::method::POST),
        abc::net::http::status_code::Method_Not_Allowed, abc::net::http::reason_phrase::Method_Not_Allowed, abc::net::http::content_type::text, "Method error: Expected 'POST'.");
}


inline void car_endpoint::require_content_type_json(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request) {
    abc::net::http::headers::const_iterator content_type_itr = request.headers.find(abc::net::http::header::Content_Type);

    require(suborigin, tag, content_type_itr != request.headers.cend(),
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "The 'Content-Type' header was not supplied.");
    require(suborigin, tag, abc::ascii::are_equal_i(content_type_itr->second.c_str(), abc::net::http::content_type::json),
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "The value of header 'Content-Type' must be `application/json`.");
}


inline void car_endpoint::start_auto_loop(car_endpoint* this_ptr) {
    constexpr const char* suborigin = "start_auto_loop()";
    this_ptr->put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    this_ptr->auto_loop();

    this_ptr->put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


inline void car_endpoint::auto_loop() {
    constexpr const char* suborigin = "auto_loop()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x1069f, "Begin:");

    while (!base::is_shutdown_requested()) {
        // Refresh obstacle
        std::size_t distance_cm = _ultrasonic.measure_distance(ultrasonic_max_cm);
        _obstacle_cm.store(distance_cm);

        // Refresh grayscales
        abc::smbus::grayscale_values grayscales = _grayscale.get_values();
        _grayscale_left.store(grayscales.left);
        _grayscale_center.store(grayscales.center);
        _grayscale_right.store(grayscales.right);

        auto_limit_power();

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, 0x106a0, "End:");
}


inline void car_endpoint::auto_limit_power() {
    constexpr const char* suborigin = "auto_limit_power()";
    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

    std::size_t distance_cm = _obstacle_cm.load();
    std::int32_t power = _power;

    if (_forward) {
        if (_power > 75 && distance_cm < 30) {
            power = 75;
        }
        else if (_power > 50 && distance_cm < 20) {
            power = 50;
        }
        else if (_power > 25 && distance_cm < 10) {
            power = 25;
        }
        else if (_power > 0 && distance_cm < 5) {
            power = 0;
        }
    }

    if (power < _power) {
        diag_base::put_any(suborigin, abc::diag::severity::optional, 0x106a1, "old_power=%d, new_power=%d.", (int)_power, (int)power);

        _power = power;
        drive_verified();
    }

    diag_base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "End:");
}


