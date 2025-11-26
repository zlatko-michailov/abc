# GPIO

Up to [Documentation](../README.md).

GPIO (__General Purpose I/O__) is an interface between peripherals and a main board.
It appears as a set of pins each of which surfaces a logical line.

## Chip
There is a chip on the main board that defines the mapping of lines to pins as well as the meaning of each line.

## Line
Some lines are hardwired to provide constant voltage or ground.
The majority of lines are general purpose ones, and can be in one of two states:
- Voltage of a predefined level is present.
- No voltage is present.

That way, a line can carry logical 1's and 0's in one direction.
Based on its direction, a line is either _input_ or _output_.

A special dimension to the binary value could be the _duration_ of time a given level is sustained.
That is how the ultrasound sensor works to measure the distance to an obstacle - an output (trigger) line pulses a high voltage level for a brief duration, and an input (echo) line sets its voltage level to high until the pulse bounces back.
The distance can be calculated by multiplying the speed of sound by the time it took for the pulse to travel to the obstacle and back. 

## PWM
Many peripherals take or produce a whole _range_ of values, not just 1's and 0's.
The technique is to send a percentage of the range width.
This technique is called __Pulse Width Modulation__ (PWM).

PWM is best demonstrated with analog peripherals, like motors.
There is a short __Period__ of time during which the peripheral is inertial, i.e. if the voltage level drops down to 0 and then goes back up, the motor will continue spinning, eventually at a lower speed.

That could be taken further - to send a specific power level (or a specific percentage of the maximum power) to a motor using a line that can only have two discrete levels, we have to keep the voltage level at high for the desired percentage of the period and keep it at low for the rest of the period.

The percentage of the period during which the voltage level is high is called __Duty Cycle__.

The term __Frequency__ is used interchangeably with Period where the relationship between the two can be defined like this:
``` c++
period_time_units = time_units_per_second / frequency;
```

For instance, a frequency of 50Hz is equivalent to a period of 1,000ms / 50 = 20ms.

Emulating PWM over a regular (binary) GPIO line is fairly easy.
The code could be schematized like this:
``` c++
while (duty_cycle > 0) {
    line.set_level(high);
    sleep(period * duty_cycle / 100);
    line.set_level(low);
    sleep(period * (100 - duty_cycle) / 100);
}
```

The problem with software PWM emulation is that it consumes a lot of cycles on the main CPU.
And if you have multiple PWM peripherals, e.g. several motors, servos, etc. those consumed CPU cycles will be multiplied as many times.
As a result, the device's responsiveness may degrade, which may also include the precision of PWM emulation.
Furthermore, those CPU cycles would consume more energy (drain the battery faster) and would generate more heat that might require cooling (and additional energy).

That is why it is better to use a hardware module that implements PWM.
Such hardware modules are called HAT (__Hardware Attached on Top__).

HAT modules are quite primitive - they can't execute code like the one above, because they don't have CPUs.
What a HAT module has is a __Timer__ that operates at a module-specific __Frequency__.
It also has several __Registers__ where input parameters can be stored.

A HAT has no notion of "sleep" or "time units".
It operates entirely in terms of its own clock's ticks.
That is why knowing the HAT's clock frequency is critical.

The first thing is to calculate the duration of the Period in terms of clock ticks.
When we need to set its Duty Cycle, we need to calculate it in clock ticks as well.

__Example__: We want to set a Duty Cycle of 75% over a Frequency of 50Hz on a motor connected to a HAT whose clock frequency is 72MHz.

First, we calculate the motor Period in terms of HAT clock ticks.  
Since `Period = 1 sec / 50Hz`, then: `1 sec = 50 * Period`.  
Similarly for the clock: `1 sec = 72,000,000 ticks`.  
When we equate the right sides of the last two equations, we have: `50 * Period = 72,000,000 ticks`, which gives us: `Period = 1,440,000 ticks`.  
Lastly, `Duty Cycle = 75% * 1,444,000 ticks = 1,080,000 ticks`.

The register where the Period is stored is called __Auto Reload__ (ARR).  
The register where the Duty Cycle is stored is called __Capture Compare__ (CCR).

There is one last hurdle - we cannot send such big numbers down the SMBus.
The SMBus protocol provisions a transfer of only 8-bit and 16-bit values.
To overcome this limitation, HATs provision a __Prescaler__ register.

PWM channels use the Prescaler as follows:
>- `Period = Auto Reload * Prescaler`
>- `Duty Cycle = Capture Compare * Prescaler`

Choosing a good Prescaler value could be tricky.
If the Prescaler value is too small, there is a risk that the Period value may not fit in 16 bits.
If the Prescaler value is too big, the precision of the Duty Cycle would be too coarse.

One way to choose a Prescaler is to set Auto Reload to be a big number that fits in 16 bits, and to calculate the Prescaler based on that.
That would lead to a reasonably small Prescaler value, which would further lead to a good Duty Cycle precision.

Another way to choose a Prescaler value is to use the square root of the Period.
This method doesn't lead to the highest Duty Cycle precision, but it's simple to implement.

To summarize, using hardware PWM boils down to:
>1. Set the __Prescaler__ and __Auto Reload__ registers once, at initialization.
>2. Set the __Capture Compare__ register every time we want to change the power on the peripheral.

`abc` hides all this complexity behind a constructor and a `set_duty_cycle()` method.

## SMBus (I2C)
__SMBus__ is a subset of an older protocol - __I2C__.
The two terms are often used interchangeably.
While __I2C__ is the more widely used term, the protocol that peripherals support today is the SMBus subset.
That is why `abc` uses the term __SMBus__.

The main board has an SMBus __Controller__.
A HAT is an SMBus __Target__.
A Target is identified by an 8-bit __Address__.
A Target has a number of 8-bit __Registers__ associated with different peripherals.

An SMBus can send/receive a signal to/from a Register at an Address eventually along with an argument that could be an 8-bit, a 16-bit, or a block value.

__Note__: Check the order of bytes when 16-bit values are sent.
A swap may be needed.

`abc` exposes all the communication API on class `abc::smbus::controller`.
Class `abc::smbus::target` is only for identification and initialization purposes.

## Higher-Level Entities
For convenience, `abc` provides some classes that represent frequently used higher-level concepts:
- `abc::gpio::ultrasonic` - combines `abc::gpio::output_line` that sends an ultrasonic pulse and a `abc::gpio::input_line` that receives that pulse, and calculates the distance to the obstacle using the speed of sound.
- `abc::gpio::pwm_emulator` - implements a software PWM in case a HAT is not available.
- `abc::smbus::pwm` - communicates with the PWM on a HAT.
- `abc::smbus::adc` - implements an analog-to-digital converter (ADC).
- `abc::smbus::motor` - combines a `abc::smbus::pwm` that sends variable power to a motor connected to a HAT and a plain `abc::gpio::output_line` that controls the direction in which the motor spins.
- `abc::smbus::servo` - wraps around `abc::smbus::pwm`.
- `abc::smbus::grayscale` - communicates with a grayscale sensor connected through SMBus.
- `abc::smbus::motion` - communicates with a MPU-6000/6500 motion sensor (accelerometer).
- `abc::smbus::motion_tracker` - a continuous tracker (every 1 ms) of a `abc::smbus::motion` that calculates speed, distance, and turning.
