# Hierarchy

Up to [Documentation](README.md).


- __abc ::__
    - size.h
    - ascii.h
    - util.h
    - __streambuf ::__
        - buffer.h
        - multifile.h
    - __stream ::__
        - stream.h
        - table.h
    - __diag ::__
        - tag.h
        - timestamp.h
        - log.h
        - exception.h
        - assert.h
    - __net ::__
        - socket.h
        - http.h
        - json.h
        - endpoint.h
        - __streambuf ::__
            - socket.h
        - __openssl ::__
            - socket.h
    - __vmem ::__
        - layout.h
        - pool.h
        - linked.h
        - container.h
        - iterator.h
        - list.h
        - map.h
        - string.h
        - util.h
        - __streambuf ::__
            - string.h
    - __gpio ::__
        - base.h
        - chip.h
        - line.h
        - ultrasonic.h
        - pwm_emulator.h
        - __smbus ::__
            - base.h
            - pwm.h
            - motor.h
            - servo.h
            - grayscale.h
            - motion.h
            - motion_tracker.h
    - __test ::__
        - test.h

## Dependency
A class may depend on other classes:
- from the same sub-namespace, or
- from sub-namespaces defined above its sub-namespace on the diagram