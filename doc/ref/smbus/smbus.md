# smbus

Up to [Documentation](../README.md).

Purpose          | File
---------------- | ----
Include          | [smbus/*.h](../../../src/smbus/*.h)
Interface        | [smbus/i/*.i.h](../../../src/smbus/i/*.i.h)
Tests / Examples | [samples/picar_4wd/main.cpp](../../../samples/picar_4wd/hacks.cpp)

`controller` is used to send and receive values (typically non-binary) to peripherals/targets - sensors, motors, servos, etc. using the SMBus protocol.
The `controller` instance must be kept alive for as long as any target that references it is alive.

`target` is a struct that identifies a peripheral/target connected to an SMBus.

`pwm` sends or receives non-binary values using hardware PWM.
This is the recommended way to use PWM because it doesn't use any cycles on the main CPU.

`motor` drives a motor using hardware PWM.

`servo` turns a servo using hardware PWM.

`adc` (Analog-to-Digital Converter) converts analog signals to digital values.

`grayscale` reads a 3-way grayscale channel using hardware PWM.

`motion` reads acceleration and turning measurements from a MPU-6000/6500 motion sensor (accelerometer) connected through SMBus.

`motion_tracker` is a continuous tracker that reads measurements from a `motion` every 1 ms, and calculates relative location in terms of depth (forward) and width (lateral).
