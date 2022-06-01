# GPIO

Up to [Documentation](../README.md).

GPIO (General Purpose I/O) is an interface between peripherals and a main board.
It appears as a set of pins each of which surfaces a logical line.

## Chip
There is a chip on the main board that defines the mapping of lines to pins as well as the meaning of each line.

## Line
Some lines are hardwired to provide constant voltage or ground.
The majority of lines are general purpose ones, and can be in one of two states:
- Voltage of a predefined level is present.
- No voltage is present.

This way, a line can carry 1's and 0's in each direction.

An additional dimension to the binary value could be the duration of time a given level is sustained.
That is how the ultrasound sensor works to measure the distance to an obstacle - an output (trigger) line pulses a high voltage level for a brief duration, and an input (echo) line sets its voltage level to high until the pulse bounces back.
The distance can be calculated by multiplying the speed of sound by the time it took for the pulse to travel to the obstacle and back. 

## PWM
Many peripherals take or produce a whole _range_ of values, not just 1's and 0's.
The technique is to send a percentage of the range width.
This technique is called Pulse Width Modulation (PWM).

PWM is best demonstrated with analog peripherals, like motors.
There is a short __Period__ of time during which the peripheral is inertial, i.e. if the voltage level drops down to 0 and then goes back up, the motor will continue spinning, eventually at a lower speed.

That could be taken further - to send a specific power level (or a specific percentage of the maximum power) to a motor using a line that can only have 2 discrete levels, we have to keep the voltage level at high for the desired percentage of the period and keep it at low for the rest of the period.

The percentage of the period during which the voltage level is high is called __Duty Cycle__.

The term __Frequency__ may be used interchangeably with Period where the relationship between the two can be defined like this:
``` c++
period_time_units = time_units_per_second / frequency;
```

For instance, a frequency of 50 Hz is equivalent to a period of 1,000 ms / 50 = 20 ms.

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
And if you have multiple PWM peripherals, e.g. 4 motors, servos, etc. those CPU cycles will be multiplied as many times.
As a result, the device's responsiveness may degrade, which may also include the precision of PWM emulation.
Furthermore, those CPU cycles would consume more energy (drain the battery faster) and would generate more heat that might require cooling (and additional energy).

That's why it is better to use a hardware module that implements PWM.
Such hardware modules are called HAT (Hardware Attached on Top).

HAT modules are quite primitive - they can't execute code like the one above, because they don't have CPUs.
What a HAT module has is a __Timer__ that operates at a module-specific __Frequency__.
It also has several __Registers__ where input parameters can be stored.

A HAT has no notion of "sleep" or "time units".
It operates entirely in terms of its own timer's ticks.
That's why knowing its timer's frequency is critical.
The first thing is to calculate the duration of the Period in terms of timer ticks.
When we need to set its Duty Cycle, we need to calculate it, again, in timer ticks.

Example: We want to set a Duty Cycle of 75% over a Frequency of 50 Hz on a motor connected to a HAT whose timer frequency is 72 MHz.

The first thing is to calculate the motor Period in terms of HAT timer ticks.  
Since `Period = 1 sec / 50 Hz`, then: `1 sec = 50 * Period`.  
Similarly for the timer: `1 sec = 72,000,000 ticks`.  
When we equate the right sides of the last two equations, we have: `50 * Period = 72,000,000 ticks`, which gives us: `Period = 1,440,000 ticks`.  
Lastly, `Duty Cycle = 75% * 1,444,000 ticks = 1,080,000 ticks`.

The register where the Period is stored is called __Auto Reload__ (ARR).  
The register where the Duty Cycle is stored is called __Capture Compare__ (CCR).

There is one last hurdle - we can't send such big numbers down the SMBus.
The SMBus protocol provisions a transfer of only 8-bit and 16-bit values.
To overcome this limitation, peripherals provision a __Prescaler__ register.

PWM channels use the Prescaler as follows:
>- `Period = Auto Reload * Prescaler`
>- `Duty Cycle = Capture Compare * Prescaler`

Choosing a good Prescaler value could be tricky.
If the Prescaler value is too small, there is a risk that the Period value may not fit in 16 bits.
If the Prescaler value is too big, the precision of the Duty Cycle would be to coarse.

One way to choose a Prescaler is to set Auto Reload to be a big number that fits in 16 bits, and to calculate the Prescaler based on that.
That would lead to a reasonably small Prescaler value, which would further lead to a good Duty Cycle precision.

Another way to choose a Prescaler value is to use the square root of the Period.
This method doesn't lead to the highest Duty Cycle precision, but it's simple to implement.

To summarize, using hardware PWM boils down to:
>1. Set the __Prescaler__ and __Auto Reload__ registers once, at initialization.
>2. Set the __Capture Compare__ register every time we want to change the power on the peripheral.

`abc` hides all this complexity behind a constructor and a `set_duty_cycle()` method.

## SMBus
__SMBus__ is a refinement/subset of an older protocol - __I2C__.
The two terms are often used interchangeably.
Though the protocol that peripherals support today is the SMBus subset.

A HAT is an SMBus __Target__.
A Target is identified by an 8-bit __Address__ on the SMBus.
A Target has a number of 8-bit __Registers__ associated with different peripherals.

An SMBus can send/receive a signal to/from a Register at an Address eventually along with an argument that could be an 8-bit, a 16-bit, or a block value.

`abc` exposes all the communication API on class `gpio_smbus`.
Class `gpio_smbus_target` is only for identification and initialization purposes.

## Higher-Level Entities
For convenience, `abc` provides some classes that represent frequently used higher-level concepts:
- `gpio_ultrasonic` - combines `gpio_output_line` that sends an ultrasonic pulse and a `gpio_input_line` that receives that pulse, and calculates the distance to the obstacle using the speed of sound.
- `gpio_smbus_motor` - combines a `gpio_smbus_pwm` that sends variable power to a motor connected to a HAT and a plain `gpio_output_line` that controls the direction in which the motor spins.
- `gpio_smbus_servo` - wraps around `gpio_smbus_pwm`.
- `gpio_smbus_grayscale` - wraps around `gpio_smbus` and communicates with a grayscale sensor.
