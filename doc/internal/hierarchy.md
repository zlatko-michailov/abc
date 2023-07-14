# Hierarchy

Up to [Documentation](README.md).


- __abc ::__
    - size.h
    - ascii.h
    - util.h
    - __streambuf ::__
        - __i__
        - buffer.h
        - multifile.h
    - __stream ::__
        - __i__
        - stream.h
        - table.h
    - __diag ::__
        - __i__
        - tag.h
        - timestamp.h
        - log.h
        - exception.h
        - assert.h
    - __net ::__
        - __i__
        - socket.h
        - http.h
        - json.h
        - endpoint.h
        - __streambuf ::__
            - __i__
            - socket.h
        - __openssl ::__
            - __i__
            - socket.h
    - __vmem ::__
        - __i__
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
            - __i__
            - string.h
    - __gpio ::__
        - __i__
        - base.h
        - chip.h
        - line.h
        - ultrasonic.h
        - pwm_emulator.h
        - __smbus ::__
            - __i__
            - base.h
            - pwm.h
            - motor.h
            - servo.h
            - grayscale.h
            - motion.h
            - motion_tracker.h
    - __test ::__
        - __i__
        - test.h

## Dependency
A class may depend on other classes:
- from the same sub-namespace, or
- from sub-namespaces defined above its sub-namespace on the diagram