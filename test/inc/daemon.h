/*
MIT License

Copyright (c) 2018-2026 Zlatko Michailov 

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


#pragma once

#include "../../src/root/size.h"
#include "../../src/net/daemon.h"

#include "test.h"


class test_daemon
    : public abc::net::daemon {

    using base = abc::net::daemon;

public:
    test_daemon(test_context& context)
        : base(context.log())
        , _context(context)
        , _count(10)
        , _passed(true) {
    }

public:
    bool passed() const noexcept {
        return _passed;
    }

protected:
    virtual bool can_stop() const override {
        return _count == 0;
    }

    virtual void on_started() override {
        _passed = _context.are_equal(_count, 10, __TAG__, "%d") && _passed;
    }

    virtual void on_idle() override {
        _passed = _context.are_equal(_count > 5, true, __TAG__, "%d") && _passed;

        if (--_count == 5) {
            request_stop();
        }
    }

    virtual void on_stopping() override {
        _passed = _context.are_equal(_count <= 5 && _count > 0, true, __TAG__, "%d") && _passed;

        --_count;
    }

    virtual void on_stopped() override {
        _passed = _context.are_equal(_count, 0, __TAG__, "%d") && _passed;
    }

private:
    test_context& _context;
    int _count;
    bool _passed;
};


bool test_daemon_events(test_context& context);
