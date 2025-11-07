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


#include <fstream>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <ratio>
#include <thread>
#include <atomic>

#include "../../src/diag/diag_ready.h"
#include "../../src/net/endpoint.h"
#include "../../src/gpio/chip.h"
#include "../../src/gpio/ultrasonic.h"
#include "../../src/smbus/controller.h"
#include "../../src/smbus/motor.h"
#include "../../src/smbus/servo.h"
#include "../../src/smbus/grayscale.h"
#include "../../src/smbus/motion.h"
#include "../../src/smbus/motion_tracker.h"


class picar_4wd_hat
    : public abc::smbus::target {

    using base = abc::smbus::target;

public:
    picar_4wd_hat(abc::gpio::chip* chip, abc::smbus::address_t addr, abc::smbus::clock_frequency_t clock_frequency, bool requires_byte_swap);

public:
    void reset();

private:
    abc::gpio::chip* _chip;
};


// --------------------------------------------------------------


class car_endpoint
    : public abc::net::http::endpoint {

    using base = abc::net::http::endpoint;
    using diag_base = abc::diag::diag_ready<const char*>;

    struct side_powers {
        std::int32_t left;
        std::int32_t right;
    };

public:
    car_endpoint(abc::net::http::endpoint_config&& config, abc::diag::log_ostream* log);

protected:
    virtual std::unique_ptr<abc::net::tcp_server_socket> create_server_socket() override;
    virtual void process_rest_request(abc::net::http::server& http, const abc::net::http::request& request) override;

private:
    void process_power(abc::net::http::server& http, const abc::net::http::request& request);
    void process_turn(abc::net::http::server& http, const abc::net::http::request& request);
    void process_autos(abc::net::http::server& http, const abc::net::http::request& request);
    void process_servo(abc::net::http::server& http, const abc::net::http::request& request);
    void process_shutdown(abc::net::http::server& http, const abc::net::http::request& request);

    void drive_verified();
    side_powers calculate_side_powers() noexcept;
    std::int32_t calculate_delta_power() noexcept;

    void require_method_get(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request);
    void require_method_post(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request);
    void require_content_type_json(const char* suborigin, abc::diag::tag_t tag, const abc::net::http::request& request);

private:
    static void start_auto_loop(car_endpoint* this_ptr);
    void auto_loop();
    void auto_limit_power();

private:
    abc::gpio::chip                              _chip;
    abc::smbus::controller                       _controller;
    picar_4wd_hat                                _hat;

    abc::smbus::motor                            _motor_front_left;
    abc::smbus::motor                            _motor_front_right;
    abc::smbus::motor                            _motor_rear_left;
    abc::smbus::motor                            _motor_rear_right;

    abc::gpio::ultrasonic<std::centi>            _ultrasonic;
    abc::smbus::servo<std::chrono::milliseconds> _servo;
    abc::smbus::grayscale                        _grayscale;

    abc::smbus::motion                           _motion;
    abc::smbus::motion_tracker<std::centi>       _motion_tracker;

    bool                                         _forward;
    std::int32_t                                 _power;
    std::int32_t                                 _turn;
    std::atomic<std::size_t>                     _obstacle_cm;
    std::atomic<std::uint16_t>                   _grayscale_left;
    std::atomic<std::uint16_t>                   _grayscale_center;
    std::atomic<std::uint16_t>                   _grayscale_right;

    std::thread                                   _auto_thread;
};


// --------------------------------------------------------------
