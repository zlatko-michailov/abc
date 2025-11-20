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


#pragma once

#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../diag/diag_ready.h"
#include "../concurrent/mutex.h"
#include "i/controller.i.h"


namespace abc { namespace smbus {

    inline controller::controller(int dev_i2c_pos, diag::log_ostream* log)
        : diag_base("abc::smbus::controller", log)
        , _fd(-1)
        , _functionality(0)
        , _addr(0)
        , _mutex(log) {

        char path[max_path];
        std::snprintf(path, max_path, "/dev/i2c-%d", dev_i2c_pos);

        init(path);
    }


    inline controller::controller(const char* path, diag::log_ostream* log)
        : diag_base("abc::smbus::controller", log)
        , _fd(-1)
        , _functionality(0)
        , _addr(0)
        , _mutex(log) {

        init(path);
    }


    inline controller::controller(controller&& other) noexcept
        : diag_base("abc::smbus::controller", other.log())
        , _fd(other._fd)
        , _functionality(other._functionality)
        , _addr(other._addr)
        , _mutex(other.log()) {

        std::strncpy(_path, other._path, max_path);

        other._fd = -1;
        other._addr = 0;
    }


    inline controller::~controller() noexcept {
        constexpr const char* suborigin = "~controller()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106d9, "Begin:");

        if (_fd >=0) {
            ::close(_fd);
        }

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106da, "End:");
    }


    inline void controller::init(const char* path) {
        constexpr const char* suborigin = "init()";
        diag_base::put_any(suborigin, diag::severity::callstack, 0x106db, "Begin:");

        diag_base::expect(suborigin, path != nullptr, 0x106dc, "path != nullptr");
        diag_base::expect(suborigin, std::strlen(path) < max_path, 0x106dd, "std::strlen(path) < max_path");

        std::strncpy(_path, path, max_path);

        _fd = open(path, O_RDWR);
        diag_base::expect(suborigin, _fd >= 0, 0x106de, "_fd >= 0, errno = %d", errno);

        ensure_ioctl(I2C_FUNCS, &_functionality, 0x106df);
        diag_base::put_any(suborigin, diag::severity::optional, 0x106e0, "functionality=0x%4.4lx %4.4lx", _functionality >> 16, _functionality & 0xffff);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106e1, "End: _fd=%d", _fd);
    }


    inline const char* controller::path() const noexcept {
        return _path;
    }


    inline functionality_t controller::functionality() const noexcept {
        return _functionality;
    }


    inline concurrent::mutex& controller::mutex() noexcept {
        return _mutex;
    }


    inline void controller::put_nodata(const target& target, register_t reg) {
        constexpr const char* suborigin = "put_nodata()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), __TAG__);

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_BYTE;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106e3);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106e4, "End:");
    }


    inline void controller::put_byte(const target& target, register_t reg, std::uint8_t byte) {
        constexpr const char* suborigin = "put_byte()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), __TAG__);

        i2c_smbus_data data{ };
        data.byte = byte;

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_BYTE_DATA;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106e6);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106e7, "End:");
    }


    inline void controller::put_word(const target& target, register_t reg, std::uint16_t word) {
        constexpr const char* suborigin = "put_word()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), __TAG__);

        i2c_smbus_data data{ };
        data.word = target.requires_byte_swap() ? swap_bytes(word) : word;

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_WORD_DATA;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106e9);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106ea, "End:");
    }


    inline void controller::put_block(const target& target, register_t reg, const void* block, std::size_t size) {
        constexpr const char* suborigin = "put_block()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, size <= I2C_SMBUS_BLOCK_MAX, 0x106eb, "size (%zu) <= I2C_SMBUS_BLOCK_MAX", size);
        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), __TAG__);

        i2c_smbus_data data{ };
        data.block[0] = static_cast<std::uint8_t>(size);
        std::memmove(&data.block[1], block, size);

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_BLOCK_DATA;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106ed);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106ee, "End:");
    }


    inline std::uint8_t controller::get_noreg_byte(const target& target) {
        constexpr const char* suborigin = "get_noreg_byte()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x", target.address());

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), 0x106ef);

        i2c_smbus_data data{ };

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.size = I2C_SMBUS_BYTE;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106f0);
        std::uint8_t byte = data.byte;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106f1, "End: byte=0x%2.2x", byte);

        return byte;
    }


    inline std::uint16_t controller::get_noreg_word(const target& target) {
        constexpr const char* suborigin = "get_noreg_word()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x", target.address());

        std::uint16_t byte0 = get_noreg_byte(target);
        std::uint16_t byte1 = get_noreg_byte(target);

        std::uint16_t word = target.requires_byte_swap() ? ((byte0 << 8) | byte1) : ((byte1 << 8) | byte0);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106f4, "End: word=0x%4.4x", word);

        return word;
    }


    inline std::uint8_t controller::get_byte(const target& target, register_t reg) {
        constexpr const char* suborigin = "get_byte()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), 0x106f5);

        i2c_smbus_data data{ };

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.command = reg;
        msg.size = I2C_SMBUS_BYTE_DATA;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106f6);
        std::uint8_t byte = data.byte;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106f7, "End: byte=0x%2.2x", byte);

        return byte;
    }


    inline std::uint16_t controller::get_word(const target& target, register_t reg) {
        constexpr const char* suborigin = "get_word()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), 0x106f8);

        i2c_smbus_data data{ };

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.command = reg;
        msg.size = I2C_SMBUS_WORD_DATA;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106f9);
        std::uint16_t word = target.requires_byte_swap() ? swap_bytes(data.word) : data.word;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106fa, "End: word=0x%4.4x", word);

        return word;
    }


    inline std::size_t controller::get_block(const target& target, register_t reg, void* block, std::size_t size) {
        constexpr const char* suborigin = "get_block()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: target_addr=0x%2.2x, reg=0x%2.2x", target.address(), reg);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), __TAG__, "_mutex");

        ensure_address(target.address(), 0x106fb);

        i2c_smbus_data data{ };
        data.block[0] = static_cast<std::uint8_t>(size);

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.command = reg;
        msg.size = I2C_SMBUS_BLOCK_DATA;
        msg.data = &data;

        ensure_ioctl(I2C_SMBUS, &msg, 0x106fc);
        diag_base::expect(suborigin, data.block[0] <= size, 0x106fd, "data.block[0] <= size");

        std::size_t ret_size = data.block[0];
        std::memmove(block, &data.block[1], data.block[0]);

        diag_base::put_any(suborigin, diag::severity::callstack, 0x106fe, "End: size=%zu", ret_size);

        return ret_size;
    }


    inline void controller::ensure_address(address_t addr, diag::tag_t tag) {
        constexpr const char* suborigin = "ensure_address()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin: addr=0x%2.2x", addr);

        diag_base::expect(suborigin, static_cast<bool>(_mutex), tag, "_mutex");

        if (_addr == addr) {
            diag_base::put_any(suborigin, diag::severity::callstack, 0x106ff, "End: (Skip)");

            return;
        }

        long laddr = addr;
        ensure_ioctl(I2C_SLAVE_FORCE, laddr, tag);

        _addr = addr;

        diag_base::put_any(suborigin, diag::severity::callstack, 0x10701, "End: addr=0x%2.2x", _addr);
    }


    template <typename Arg>
    inline void controller::ensure_ioctl(int command, Arg arg, diag::tag_t tag) {
        constexpr const char* suborigin = "ensure_ioctl()";
        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "Begin:");

        int ret = ioctl(_fd, command, arg);
        diag_base::expect(suborigin, ret >= 0, tag, "ret (%d) >= 0, errno = %d", ret, errno);

        diag_base::put_any(suborigin, diag::severity::callstack, __TAG__, "End:");
    }


    inline std::uint16_t controller::swap_bytes(std::uint16_t word) noexcept {
        std::uint16_t lo = word & 0x00ff;
        std::uint16_t hi = (word >> 8) & 0x00ff;

        return (lo << 8) | hi;
    }


    // --------------------------------------------------------------


    inline target::target(address_t addr, clock_frequency_t clock_frequency, bool requires_byte_swap)
        : _addr(addr)
        , _clock_frequency(clock_frequency)
        , _requires_byte_swap(requires_byte_swap) {
    }


    inline address_t target::address() const noexcept {
        return _addr;
    }


    inline clock_frequency_t target::clock_frequency() const noexcept {
        return _clock_frequency;
    }


    inline bool target::requires_byte_swap() const noexcept {
        return _requires_byte_swap;
    }


    // --------------------------------------------------------------

} }
