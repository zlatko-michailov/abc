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


#pragma once

#include <cstring>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "../../diag/i/diag_ready.i.h"
#include "i/controller.i.h"


namespace abc { namespace smbus {

    template <typename Log>
    inline gpio_smbus<Log>::gpio_smbus(int dev_i2c_pos, Log* log)
        : _fd(-1)
        , _functionality(0)
        , _addr(0)
        , _log(log) {
        char path[max_path];
        std::snprintf(path, max_path, "/dev/i2c-%d", dev_i2c_pos);

        init(path);
    }


    template <typename Log>
    inline gpio_smbus<Log>::gpio_smbus(const char* path, Log* log)
        : _fd(-1)
        , _functionality(0)
        , _addr(0)
        , _log(log) {
        init(path);
    }


    template <typename Log>
    inline gpio_smbus<Log>::~gpio_smbus() noexcept {
        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::optional, 0x106d9, "gpio_smbus::~gpio_smbus() Start.");
        }

        if (_fd >=0) {
            close(_fd);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::optional, 0x106da, "gpio_smbus::~gpio_smbus() Done.");
        }
    }


    template <typename Log>
    inline void gpio_smbus<Log>::init(const char* path) {
        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::optional, 0x106db, "gpio_smbus::gpio_smbus() Start.");
        }

        if (path == nullptr) {
            throw exception<std::logic_error, Log>("gpio_smbus::gpio_smbus() path == nullptr", 0x106dc);
        }

        if (std::strlen(path) >= max_path) {
            throw exception<std::logic_error, Log>("gpio_smbus::gpio_smbus() path >= max_path", 0x106dd);
        }

        _fd = open(path, O_RDWR);
        if (_fd < 0) {
            throw exception<std::logic_error, Log>("gpio_smbus::gpio_smbus() open() < 0", 0x106de);
        }

        if (safe_ioctl(I2C_FUNCS, &_functionality) < 0) {
            throw exception<std::logic_error, Log>("gpio_smbus::gpio_smbus() I2C_FUNCS failed", 0x106df);
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::optional, 0x106e0, "gpio_smbus::gpio_smbus() functionality = 0x%4.4lx %4.4lx", _functionality >> 16, _functionality & 0xffff);
        }

        std::strncpy(_path, path, max_path);

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::optional, 0x106e1, "gpio_smbus::gpio_smbus() Done. _fd = %d", _fd);
        }
    }


    template <typename Log>
    inline const char* gpio_smbus<Log>::path() const noexcept {
        return _path;
    }


    template <typename Log>
    inline gpio_smbus_functionality_t gpio_smbus<Log>::functionality() const noexcept {
        return _functionality;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::put_nodata(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106e2, "gpio_smbus::put_nodata() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_BYTE;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106e3, "gpio_smbus::put_nodata() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106e4, "gpio_smbus::put_nodata() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::put_byte(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint8_t byte) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106e5, "gpio_smbus::put_byte() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };
        data.byte = byte;

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_BYTE_DATA;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106e6, "gpio_smbus::put_byte() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106e7, "gpio_smbus::put_byte() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::put_word(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint16_t word) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106e8, "gpio_smbus::put_word() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };
        data.word = target.requires_byte_swap() ? swap_bytes(word) : word;

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_WORD_DATA;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106e9, "gpio_smbus::put_word() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106ea, "gpio_smbus::put_word() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::put_block(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, const void* block, std::size_t size) noexcept {
        if (size > I2C_SMBUS_BLOCK_MAX) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106eb, "gpio_smbus::put_block() size > I2C_SMBUS_BLOCK_MAX. errno = %d", errno);

                return false;
            }
        }

        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106ec, "gpio_smbus::put_block() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };
        data.block[0] = static_cast<std::uint8_t>(size);
        std::memmove(&data.block[1], block, size);

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_WRITE;
        msg.command = reg;
        msg.size = I2C_SMBUS_BLOCK_DATA;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106ed, "gpio_smbus::put_block() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106ee, "gpio_smbus::put_block() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::get_noreg(const gpio_smbus_target<Log>& target, std::uint8_t& byte) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106ef, "gpio_smbus::get_noreg() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.size = I2C_SMBUS_BYTE;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f0, "gpio_smbus::get_noreg() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        byte = data.byte;

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106f1, "gpio_smbus::get_noreg() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::get_noreg_2(const gpio_smbus_target<Log>& target, std::uint16_t& word) noexcept {
        std::uint8_t byte0;
        if (!get_noreg(target, byte0)) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f2, "gpio_smbus::get_noreg_2() byte0 failed. errno = %d", errno);

                return false;
            }
        }

        std::uint8_t byte1;
        if (!get_noreg(target, byte1)) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f3, "gpio_smbus::get_noreg_2() byte1 failed. errno = %d", errno);

                return false;
            }
        }

        word = target.requires_byte_swap() ? ((byte0 << 8) | byte1) : ((byte1 << 8) | byte0);

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106f4, "gpio_smbus::get_noreg_2() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::get_byte(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint8_t& byte) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f5, "gpio_smbus::get_byte() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.command = reg;
        msg.size = I2C_SMBUS_BYTE_DATA;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f6, "gpio_smbus::get_byte() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        byte = data.byte;

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106f7, "gpio_smbus::get_byte() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::get_word(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, std::uint16_t& word) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f8, "gpio_smbus::get_word() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.command = reg;
        msg.size = I2C_SMBUS_WORD_DATA;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106f9, "gpio_smbus::get_word() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        word = target.requires_byte_swap() ? swap_bytes(data.word) : data.word;

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106fa, "gpio_smbus::get_word() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::get_block(const gpio_smbus_target<Log>& target, gpio_smbus_register_t reg, void* block, std::size_t& size) noexcept {
        if (!ensure_address(target.address())) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106fb, "gpio_smbus::get_block() ensure_address() failed. errno = %d", errno);

                return false;
            }
        }

        i2c_smbus_data data{ };
        data.block[0] = static_cast<std::uint8_t>(size);

        i2c_smbus_ioctl_data msg{ };
        msg.read_write = I2C_SMBUS_READ;
        msg.command = reg;
        msg.size = I2C_SMBUS_BLOCK_DATA;
        msg.data = &data;

        if (safe_ioctl(I2C_SMBUS, &msg) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106fc, "gpio_smbus::get_block() I2C_SMBUS failed. errno = %d", errno);

                return false;
            }
        }

        if (data.block[0] > size) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x106fd, "gpio_smbus::get_block() block[0] = %d, size = %d", data.block[0], (int)size);

                return false;
            }
        }

        size = data.block[0];
        std::memmove(block, &data.block[1], data.block[0]);

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x106fe, "gpio_smbus::get_block() Done.");
        }

        return true;
    }


    template <typename Log>
    inline bool gpio_smbus<Log>::ensure_address(gpio_smbus_address_t addr) noexcept {
        if (_addr == addr) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::debug, 0x106ff, "gpio_smbus::ensure_address() Skip.");

                return true;
            }
        }

        long laddr = addr;
        if (safe_ioctl(I2C_SLAVE_FORCE, laddr) < 0) {
            if (_log != nullptr) {
                _log->put_any(category::abc::gpio, severity::abc::important, 0x10700, "gpio_smbus::ensure_address() I2C_SLAVE failed. errno = %d", errno);

                return false;
            }
        }

        _addr = addr;

        if (_log != nullptr) {
            _log->put_any(category::abc::gpio, severity::abc::debug, 0x10701, "gpio_smbus::ensure_address() Done. _addr = 0x%2.2x", _addr);
        }

        return true;
    }


    template <typename Log>
    template <typename Arg>
    int gpio_smbus<Log>::safe_ioctl(int command, Arg arg) noexcept {
        try {
            std::lock_guard<std::mutex> lock(_ioctl_mutex);
            return ioctl(_fd, command, arg);
        }
        catch (...) {
            return ENOLCK;
        }
    }


    template <typename Log>
    inline std::uint16_t gpio_smbus<Log>::swap_bytes(std::uint16_t word) noexcept {
        std::uint16_t lo = word & 0x00ff;
        std::uint16_t hi = (word >> 8) & 0x00ff;

        return (lo << 8) | hi;
    }


    // --------------------------------------------------------------


    template <typename Log>
    inline gpio_smbus_target<Log>::gpio_smbus_target(gpio_smbus_address_t addr, gpio_smbus_clock_frequency_t clock_frequency, bool requires_byte_swap, Log* log)
        : _addr(addr)
        , _clock_frequency(clock_frequency)
        , _requires_byte_swap(requires_byte_swap)
        , _log(log) {
        if (log != nullptr) {
            log->put_any(category::abc::gpio, severity::abc::optional, 0x10702, "gpio_smbus_target::gpio_smbus_target() Done.");
        }
    }


    template <typename Log>
    inline gpio_smbus_address_t gpio_smbus_target<Log>::address() const noexcept {
        return _addr;
    }


    template <typename Log>
    inline gpio_smbus_clock_frequency_t gpio_smbus_target<Log>::clock_frequency() const noexcept {
        return _clock_frequency;
    }


    template <typename Log>
    inline bool gpio_smbus_target<Log>::requires_byte_swap() const noexcept {
        return _requires_byte_swap;
    }

    // --------------------------------------------------------------

} }
