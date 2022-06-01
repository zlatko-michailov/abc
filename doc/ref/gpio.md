# gpio

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [gpio.h](../../src/gpio.h)
Interface        | [gpio.i.h](../../src/i/gpio.i.h)
Tests / Examples | [samples/picar_4wd/main.cpp](../../samples/picar_4wd/main.cpp)

`gpio_chip` provides information about the line functions.
It is also needed to access a particular line on it.
The `gpio_chip` instance must be kept alive for as long as any line that was constructed with it is alive.

`gpio_line` is a generic representation of a GPIO line.
It is recommended to use one of the specializations:
- `gpio_input_line`
- `gpio_output_line`

`gpio_pwm_emulator` is a software implementation of PWM over a regular output line.
It consumes cycles on the main CPU. 
That's why it should only be used when hardware PWM is not physically available.

`gpio_smbus` is used to send and receive values (typically non-binary) to peripherals/targets - sensors, motors, servos, etc. using the SMBus protocol.
The `gpio_smbus` instance must be kept alive for as long as any target that was constructed with it is alive.

`gpio_smbus_target` is a struct that identifies a peripheral/target connected to an SMBus.

`gpio_smbus_pwm` sends or receives non-binary values using hardware PWM.
This is the recommended way to use PWM because it doesn't use any cycles on the main CPU.

`gpio_smbus_motor` is a higher-level entity that drives a motor using hardware PWM.

`gpio_smbus_servo` is a higher-level entity that turns a servo using hardware PWM.

`gpio_smbus_ultrasonic` is a higher-level entity that measures distance to an obstacle by sending an ultrasound pulse using `gpio_line`s.

`gpio_smbus_grayscale` is a higher-level entity that reads a 3-way grayscale channel using hardware PWM.
