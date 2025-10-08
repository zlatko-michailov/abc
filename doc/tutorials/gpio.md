# How to Enable GUI and REST

Up to [Documentation](../README.md).

> A prerequisite to this tutorial is becoming familiar with these concepts:
>- [Diagnostics](../concepts/diagnostics.md)

## General Principles
Arguments passed as pointers are expected to be kept alive by the caller during the entire lifetime of the constructed instance.

Arguments passed in as references will be copied by the constructor.
The caller is not required to keep such arguments alive after the call to the given constructor. 

The following sample demonstrates how to use GPIO:
- [picar_4wd](../../samples/picar_4wd/car.h)

## Creating a `log_filter` and a `log_ostream`
It is strongly recommended to pass in a `log_ostream` to all `gpio_*` instances.
Visit the [How to Log Diagnostics](diagnostics.md) tutorial if needed.

## Access a Chip
You need to know your hardware - at minimum, you need to know how many GPIO chips your device has, and which one you want to access.
GPIO chips are accessible through `/dev/gpiochip*` starting with 0.

``` c++
// Variation 1:
abc::gpio::chip chip(0, "MyProgram", &log);

// Variation 2:
abc::gpio::chip chip("/dev/gpiochip0", "MyProgram", &log);
```

## Set/Read the Voltage Level on a Line
Before you can use a line, you need to look up the chip/device's documentation to find the logical number/position of the line as well as its physical pin where the peripheral should be connected.

Once the peripheral is connected to the correct pin, construct a `gpio_output_line`:
``` c++
abc::line_pos_t line_pos = 5;
abc::gpio_output_line line(&chip, line_pos, &log);
``` 

### Set the Voltage Level on `gpio_output_line`
When you set the voltage level on a line, you most likely intend to do that for a known duration:
``` c++
line.put_level(abc::gpio::level::high, std::chrono::microseconds(50));
```
The second argument is optional, i.e. you can set/clear the voltage level for an undetermined duration.

### Read the Voltage Level on `gpio_input_line`
If you simply want to probe the current level of a line, you can do:
``` c++
abc::gpio::level_t level = line.get_level();
```

If you want to wait until the level changes to an expected value, you can do:
``` c++
if (line.wait_for_level(abc::gpio::level::high, std::chrono::milliseconds(10)) != abc::gpio::level::invalid) {
    // ...
}
```

## Set the Duty Cycle on a Motor over SMBus
### Access an SMBus
Similar to GPIO chips, SMBus devices are also accessible as system devices - `/dev/i2c-*` - starting with 1:

``` c++
// Variation 1:
abc::gpio_smbus smbus(1, &log);

// Variation 2:
abc::gpio_smbus smbus("/dev/i2c-1", &log);
```

### Create an SMBus Target
``` c++
abc::gpio_smbus_address_t addr = 0x14;
abc::gpio_smbus_clock_frequency_t clock = 72 * std::mega::num; // 72 MHz
bool swap_bytes = true;

abc::gpio_smbus_target hat(addr, clock, swap_bytes, &log);
```

__Note__: A HAT may need to be sent a special signal to become usable.
Check your HAT's documentation.

### Create a PWM
``` c++
abc::gpio_pwm_pulse_frequency_t frequency = 50; // 50 Hz
abc::gpio_smbus_register_t reg_motor = 0x0d;
abc::gpio_smbus_register_t reg_timer = reg_motor / 4; // This HAT has 4 PWM channels per timer

abc::gpio_smbus_register_t reg_pwm_base = 0x20;
abc::gpio_smbus_register_t reg_pwm = reg_pwm_base + reg_motor;

abc::gpio_smbus_register_t reg_autoreload_base = 0x44;
abc::gpio_smbus_register_t reg_autoreload = reg_autoreload_base + reg_timer;

abc::gpio_smbus_register_t reg_prescaler_base = 0x40;
abc::gpio_smbus_register_t reg_prescaler = reg_prescaler_base + reg_timer;

abc::gpio_smbus_pwm pwm(&smbus, smbus_target, frequency, reg_pwm, reg_autoreload, reg_prescaler, &log);
```

### Set a Duty Cycle
``` c++
abc::gpio_pwm_duty_cycle_t duty_cycle = 75; // 75%
pwm.set_duty_cycle(duty_cycle);
```
