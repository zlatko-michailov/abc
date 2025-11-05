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
    base::put_any(suborigin, abc::diag::severity::callstack, 0x10678, "Begin:");

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

    base::put_any(suborigin, abc::diag::severity::callstack, 0x1067a, "End:");
}


inline void car_endpoint::process_power(abc::net::http::server& http, const abc::net::http::request& request) {
    constexpr const char* suborigin = "process_power()";
    base::put_any(suborigin, abc::diag::severity::callstack, __TAG__, "Begin:");

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

}


template <typename Limits, typename Log>
inline void car_endpoint::process_turn(abc::http_server_stream<Log>& http, const char* method) {
    if (!verify_method_post(http, method)) {
        return;
    }

    if (!verify_header_json(http)) {
        return;
    }

    std::int32_t turn;
    // Read turn from JSON
    {
        std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
        abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
        char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
        abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
        constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"turn\": 50}.";

        json.get_token(token, sizeof(buffer));
        if (token->item != abc::json::item::begin_object) {
            // Not {.
            if (base::_log != nullptr) {
                base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x10682, "Content error: Expected '{'.");
            }

            // 400
            base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10683);
            return;
        }

        json.get_token(token, sizeof(buffer));
        if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "turn")) {
            // Not "turn".
            if (base::_log != nullptr) {
                base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x10684, "Content error: Expected \"turn\".");
            }

            // 400
            base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10685);
            return;
        }

        json.get_token(token, sizeof(buffer));
        if (token->item != abc::json::item::number || !(-90 <= token->value.number && token->value.number <= 90)) {
            // Not a valid turn.
            if (base::_log != nullptr) {
                base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x10686, "Content error: Expected -90 <= number <= 90.");
            }

            // 400
            base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10687);
            return;
        }

        turn = token->value.number;
    }

    if (!verify_range(http, turn, -90, 90, 30)) {
        return;
    }

    _turn = turn;
    drive_verified();

    // 200
    char body[abc::size::_256 + 1];
    std::snprintf(body, sizeof(body), "turn: power=%d, turn=%d", (int)_power, (int)_turn);
    base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, 0x10688);
}


template <typename Limits, typename Log>
inline void car_endpoint::process_autos(abc::http_server_stream<Log>& http, const char* method) {
    if (!verify_method_get(http, method)) {
        return;
    }

    // Write the JSON to a char buffer, so we can calculate the Content-Length before we start sending the body.
    char body[abc::size::_512 + 1];
    abc::buffer_streambuf sb(nullptr, 0, 0, body, 0, sizeof(body));
    abc::json_ostream<abc::size::_16, Log> json(&sb, base::_log);
    json.put_begin_object();
        json.put_property("obstacle");
        json.put_begin_object();
            json.put_property("distance");
            json.put_number(_obstacle_cm.load());
            json.put_property("units");
            json.put_string("cm");
        json.put_end_object();
        json.put_property("grayscale");
        json.put_begin_object();
            json.put_property("left");
            json.put_number(_grayscale_left.load());
            json.put_property("center");
            json.put_number(_grayscale_center.load());
            json.put_property("right");
            json.put_number(_grayscale_right.load());
        json.put_end_object();
        json.put_property("depth");
        json.put_begin_object();
            json.put_property("distance");
            json.put_number(_motion_tracker.depth());
            json.put_property("units");
            json.put_string("cm");
        json.put_end_object();
        json.put_property("width");
        json.put_begin_object();
            json.put_property("distance");
            json.put_number(_motion_tracker.width());
            json.put_property("units");
            json.put_string("cm");
        json.put_end_object();
    json.put_end_object();
    json.put_char('\0');
    json.flush();

    char content_length[abc::size::_32 + 1];
    std::snprintf(content_length, sizeof(content_length), "%zu", std::strlen(body));

    // Send the http response
    if (base::_log != nullptr) {
        base::_log->put_any(abc::category::abc::samples, abc::severity::debug, 0x10689, "Sending response 200");
    }

    http.put_protocol(protocol::HTTP_11);
    http.put_status_code(status_code::OK);
    http.put_reason_phrase(reason_phrase::OK);

    http.put_header_name(header::Connection);
    http.put_header_value(connection::close);
    http.put_header_name(header::Content_Type);
    http.put_header_value(content_type::json);
    http.put_header_name(header::Content_Length);
    http.put_header_value(content_length);
    http.end_headers();

    http.put_body(body);

    if (base::_log != nullptr) {
        base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1068a, "car::process_autos: Done.");
    }
}


template <typename Limits, typename Log>
inline void car_endpoint::process_servo(abc::http_server_stream<Log>& http, const char* method) {
    if (!verify_method_post(http, method)) {
        return;
    }

    if (!verify_header_json(http)) {
        return;
    }

    std::int32_t angle;
    // Read turn from JSON
    {
        std::streambuf* sb = static_cast<abc::http_request_istream<Log>&>(http).rdbuf();
        abc::json_istream<abc::size::_64, Log> json(sb, base::_log);
        char buffer[sizeof(abc::json::token_t) + abc::size::k1 + 1];
        abc::json::token_t* token = reinterpret_cast<abc::json::token_t*>(buffer);
        constexpr const char* invalid_json = "An invalid JSON payload was supplied. Must be: {\"servo\": 50}.";

        json.get_token(token, sizeof(buffer));
        if (token->item != abc::json::item::begin_object) {
            // Not {.
            if (base::_log != nullptr) {
                base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1068b, "Content error: Expected '{'.");
            }

            // 400
            base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x1068c);
            return;
        }

        json.get_token(token, sizeof(buffer));
        if (token->item != abc::json::item::property || !ascii::are_equal(token->value.property, "angle")) {
            // Not "servo".
            if (base::_log != nullptr) {
                base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1068d, "Content error: Expected \"angle\".");
            }

            // 400
            base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x1068e);
            return;
        }

        json.get_token(token, sizeof(buffer));
        if (token->item != abc::json::item::number || !(-90 <= token->value.number && token->value.number <= 90)) {
            // Not a valid angle.
            if (base::_log != nullptr) {
                base::_log->put_any(abc::category::abc::samples, abc::severity::important, 0x1068f, "Content error: Expected -90 <= number <= 90.");
            }

            // 400
            base::send_simple_response(http, status_code::Bad_Request, reason_phrase::Bad_Request, content_type::text, invalid_json, 0x10690);
            return;
        }

        angle = token->value.number;
    }

    if (!verify_range(http, angle, -90, 90, 30)) {
        return;
    }

    abc::pwm_duty_cycle_t duty_cycle = 100 - static_cast<abc::pwm_duty_cycle_t>(100 * (angle + 92) / 184);
    _servo.set_duty_cycle(duty_cycle);

    // 200
    char body[abc::size::_256 + 1];
    std::snprintf(body, sizeof(body), "servo: angle=%d", (int)angle);
    base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, body, 0x10691);
}


template <typename Limits, typename Log>
inline void car_endpoint::process_shutdown(abc::http_server_stream<Log>& http, const char* method) {
    if (!verify_method_post(http, method)) {
        return;
    }

    base::set_shutdown_requested();
    _hat.reset();
    _auto_thread.join();

    // 200
    base::send_simple_response(http, status_code::OK, reason_phrase::OK, content_type::text, "Server is shuting down...", 0x10692);
}


template <typename Limits, typename Log>
inline void car_endpoint::drive_verified() noexcept {
    std::int32_t left_power;
    std::int32_t right_power;
    get_side_powers(left_power, right_power);

    pwm_duty_cycle_t left_duty_cycle = static_cast<pwm_duty_cycle_t>(left_power);
    pwm_duty_cycle_t right_duty_cycle = static_cast<pwm_duty_cycle_t>(right_power);

    _motor_front_left.set_forward(_forward);
    _motor_front_left.set_duty_cycle(left_duty_cycle);

    _motor_front_right.set_forward(_forward);
    _motor_front_right.set_duty_cycle(right_duty_cycle);

    _motor_rear_left.set_forward(_forward);
    _motor_rear_left.set_duty_cycle(left_duty_cycle);

    _motor_rear_right.set_forward(_forward);
    _motor_rear_right.set_duty_cycle(right_duty_cycle);
}


template <typename Limits, typename Log>
inline void car_endpoint::get_side_powers(std::int32_t& left_power, std::int32_t& right_power) noexcept {
    std::int32_t delta = get_delta_power();

    left_power  = _power + delta;
    right_power = _power - delta;

    constexpr std::int32_t min_power = static_cast<std::int32_t>(abc::pwm_duty_cycle::min);
    constexpr std::int32_t max_power = static_cast<std::int32_t>(abc::pwm_duty_cycle::max);

    std::int32_t adjust = 0;
    if (left_power > max_power) {
        adjust = max_power - left_power;
    }
    else if (right_power > max_power) {
        adjust = max_power - right_power;
    }
    else if (left_power < min_power) {
        adjust = min_power - left_power;
    }
    else if (right_power < min_power) {
        adjust = min_power - right_power;
    }

    left_power  += adjust;
    right_power += adjust;

    if (base::_log != nullptr) {
        base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10693, "left_power = %3d, right_power = %3d", (int)left_power, (int)right_power);
    }
}


template <typename Limits, typename Log>
inline std::int32_t car_endpoint::get_delta_power() noexcept {
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

    if (base::_log != nullptr) {
        base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x10694, "power = %3d, turn = %3d, delta = %3d", (int)_power, (int)_turn, (int)delta);
    }

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


template <typename T>
inline void car_endpoint::require_range(const char* suborigin, abc::diag::tag_t tag, T value, T lo_bound, T hi_bound, T step) {
    require(suborigin, tag, lo_bound <= value && value <= hi_bound && value % step == 0,
        abc::net::http::status_code::Bad_Request, abc::net::http::reason_phrase::Bad_Request, abc::net::http::content_type::text, "Content error: Bad value.");
}


template <typename Limits, typename Log>
inline void car_endpoint::start_auto_loop(car_endpoint* this_ptr) noexcept {
    this_ptr->auto_loop();
}


template <typename Limits, typename Log>
inline void car_endpoint::auto_loop() noexcept {
    if (base::_log != nullptr) {
        base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x1069f, "car_endpoint::auto_loop: Start.");
    }

    while (!base::is_shutdown_requested()) {
        // Refresh obstacle
        std::size_t distance_cm = _ultrasonic.measure_distance(ultrasonic_max_cm);
        _obstacle_cm.store(distance_cm);

        // Refresh grayscales
        std::uint16_t grayscale_left = 0;
        std::uint16_t grayscale_center = 0;
        std::uint16_t grayscale_right = 0;
        _grayscale.get_values(grayscale_left, grayscale_center, grayscale_right);
        _grayscale_left.store(grayscale_left);
        _grayscale_center.store(grayscale_center);
        _grayscale_right.store(grayscale_right);

        auto_limit_power();

        std::this_thread::sleep_for(std::chrono::milliseconds(250));
    }

    if (base::_log != nullptr) {
        base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x106a0, "car_endpoint::auto_loop: Done.");
    }
}


template <typename Limits, typename Log>
inline void car_endpoint::auto_limit_power() noexcept {
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
        if (base::_log != nullptr) {
            base::_log->put_any(abc::category::abc::samples, abc::severity::optional, 0x106a1, "car_endpoint::auto_limit: old_power=%d, new_power=%d.", (int)_power, (int)power);
        }

        _power = power;
        drive_verified();
    }
}


